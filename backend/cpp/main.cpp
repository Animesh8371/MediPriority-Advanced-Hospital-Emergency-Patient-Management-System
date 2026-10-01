#include "crow_all.h"

#include "utils/Config.h"
#include "utils/Database.h"

#include "repositories/UserRepository.h"
#include "repositories/PatientRepository.h"
#include "repositories/DoctorRepository.h"
#include "repositories/EmergencyCaseRepository.h"
#include "repositories/BedRepository.h"
#include "repositories/AmbulanceRepository.h"
#include "repositories/AppointmentRepository.h"
#include "repositories/MedicalHistoryRepository.h"
#include "repositories/HospitalRepository.h"
#include "repositories/ReportRepository.h"

#include "services/AuthService.h"
#include "services/PatientService.h"
#include "services/DoctorService.h"
#include "services/TriageService.h"
#include "services/BedService.h"
#include "services/AmbulanceService.h"
#include "services/AppointmentService.h"
#include "services/MedicalHistoryService.h"
#include "services/RoutingService.h"
#include "services/ReportService.h"

#include "controllers/AuthController.h"
#include "controllers/PatientController.h"
#include "controllers/DoctorController.h"
#include "controllers/EmergencyController.h"
#include "controllers/BedController.h"
#include "controllers/AmbulanceController.h"
#include "controllers/AppointmentController.h"
#include "controllers/MedicalHistoryController.h"
#include "controllers/RoutingController.h"
#include "controllers/ReportController.h"

#include <iostream>

using namespace medipriority;

int main() {
    // --- Configuration & database connection ---
    Config config = Config::load();

    std::cout << "MediPriority backend starting...\n";
    std::cout << "Connecting to MySQL at " << config.dbHost << ":" << config.dbPort
              << ", schema '" << config.dbName << "'\n";

    Database db(config);
    std::cout << "Database connection established.\n";

    // --- Repositories (MySQL persistence, one per entity) ---
    UserRepository userRepo(db);
    PatientRepository patientRepo(db);
    DoctorRepository doctorRepo(db);
    EmergencyCaseRepository emergencyRepo(db);
    BedRepository bedRepo(db);
    AmbulanceRepository ambulanceRepo(db);
    AppointmentRepository appointmentRepo(db);
    MedicalHistoryRepository historyRepo(db);
    HospitalRepository hospitalRepo(db);
    ReportRepository reportRepo(db);

    // --- Services (business logic; the 5 C data structures are wired in here) ---
    AuthService authService(userRepo);
    PatientService patientService(patientRepo);       // -> C hash table
    DoctorService doctorService(doctorRepo);
    TriageService triageService(emergencyRepo);        // -> C min-heap
    BedService bedService(bedRepo);
    AmbulanceService ambulanceService(ambulanceRepo);
    AppointmentService appointmentService(appointmentRepo); // -> C FIFO queue
    MedicalHistoryService historyService(historyRepo);       // -> C linked list
    RoutingService routingService(hospitalRepo);             // -> C graph + Dijkstra
    ReportService reportService(reportRepo);

    // --- Warm up in-memory structures from MySQL (so the C data structures
    //     agree with the persistent database even after a fresh restart) ---
    std::cout << "Loading in-memory data structures from MySQL...\n";
    patientService.loadFromDatabase();
    triageService.loadFromDatabase();
    appointmentService.loadFromDatabase();
    routingService.loadFromDatabase();
    std::cout << "  - Patient hash table warmed (recent patients cached)\n";
    std::cout << "  - Emergency priority heap warmed: " << triageService.waitingCount() << " waiting case(s)\n";
    std::cout << "  - Appointment queue warmed\n";
    std::cout << "  - Hospital transfer graph loaded\n";

    // --- HTTP API ---
    // CORSHandler is Crow's real middleware for cross-origin requests, needed
    // so the static frontend (served on its own port during development, or
    // opened directly in a browser) can call this API.
    crow::App<crow::CORSHandler> app;
    auto &cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .headers("Content-Type", "Authorization");

    registerAuthRoutes(app, authService);
    registerPatientRoutes(app, patientService, authService);
    registerDoctorRoutes(app, doctorService, authService);
    registerEmergencyRoutes(app, triageService, authService);
    registerBedRoutes(app, bedService, authService);
    registerAmbulanceRoutes(app, ambulanceService, authService);
    registerAppointmentRoutes(app, appointmentService, authService);
    registerMedicalHistoryRoutes(app, historyService, authService);
    registerRoutingRoutes(app, routingService, authService);
    registerReportRoutes(app, reportService, authService);

    CROW_ROUTE(app, "/api/health").methods("GET"_method)
    ([]() {
        crow::json::wvalue j;
        j["success"] = true;
        j["message"] = "MediPriority API is running";
        return crow::response(200, j);
    });

    std::cout << "Starting HTTP server on port " << config.apiPort << "...\n";
    app.port(config.apiPort).run();

    return 0;
}
