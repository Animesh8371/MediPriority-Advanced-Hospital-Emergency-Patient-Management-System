#include "AdmissionService.h"
#include "../utils/Validation.h"
#include <algorithm>
#include <cctype>
#include <cstdio>

namespace medipriority {

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static bool contains(const std::string &haystack, const char *needle) {
    return haystack.find(needle) != std::string::npos;
}

std::string AdmissionService::patientCode(int patientId) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "MP-%06d", patientId);
    return buf;
}

/* Critical patients need an ICU bed; high-risk go to Emergency; the rest
 * to General, each with sensible fall-backs when the first ward is full.
 * ICU is deliberately NOT a fall-back for High/Moderate so scarce ICU beds
 * stay free for patients who are actually critical. */
std::vector<std::string> AdmissionService::bedPreference(int severity) {
    switch (severity) {
        case 1:  return {"ICU", "Emergency"};
        case 2:  return {"Emergency", "General"};
        case 3:  return {"General", "Emergency"};
        default: return {"General"};
    }
}

std::vector<std::string> AdmissionService::specialtyPreference(const std::string &category) {
    const std::string c = lower(category);
    std::vector<std::string> pref;
    if (contains(c, "cardi") || contains(c, "heart") || contains(c, "chest")) pref.push_back("Cardiology");
    if (contains(c, "neuro") || contains(c, "stroke") || contains(c, "seiz") || contains(c, "head")) pref.push_back("Neurology");
    if (contains(c, "trauma") || contains(c, "fracture") || contains(c, "ortho") || contains(c, "injur")) {
        pref.push_back("Orthopedics");
    }
    if (contains(c, "respir") || contains(c, "asthma") || contains(c, "breath") || contains(c, "lung")) {
        pref.push_back("Pulmonology");
    }
    if (contains(c, "pediat") || contains(c, "child")) pref.push_back("Pediatrics");
    if (contains(c, "obstet") || contains(c, "pregnan") || contains(c, "matern")) {
        pref.push_back("Obstetrics & Gynaecology");
    }
    if (contains(c, "burn")) pref.push_back("Plastic Surgery");
    if (contains(c, "gastro") || contains(c, "abdom")) pref.push_back("Gastroenterology");
    // Always-valid fall-backs, in order.
    pref.push_back("Emergency Medicine");
    pref.push_back("General Medicine");
    return pref;
}

/* Picks the doctor for a category: first specialty in the preference list
 * that has a doctor with free capacity; the candidate list is already sorted
 * least-loaded first, so the first match is also the least-loaded match.
 * Falls back to ANY doctor with capacity rather than leaving the patient
 * without one. Returns nullopt only if every doctor is off duty or full. */
std::optional<Doctor> AdmissionService::pickDoctor(const std::string &category) {
    auto candidates = doctors_.findAcceptingPatients();
    if (candidates.empty()) return std::nullopt;

    for (const auto &specialty : specialtyPreference(category)) {
        const std::string want = lower(specialty);
        for (const auto &d : candidates) {
            if (lower(d.specialization()) == want) return d;
        }
    }
    return candidates.front();
}

/* Tries each ward in preference order. A doctor is chosen first so the
 * transactional BedRepository::allocate() writes bed AND doctor into one
 * patient_assignments row. If another request grabs the bed between our
 * SELECT and the allocate, allocate() reports "already occupied" and we just
 * try the next free bed instead of failing. */
void AdmissionService::allocate(AdmissionResult &result, const std::string &category) {
    auto doctor = pickDoctor(category);
    const int doctorId = doctor ? doctor->id() : 0;

    for (const auto &bedType : bedPreference(result.severity)) {
        for (const auto &bed : beds_.findAvailable(bedType)) {
            auto r = beds_.allocate(bed.bedId(), result.patientId, doctorId);
            if (!r.success) continue;

            result.bedAllocated = true;
            result.bedId = bed.bedId();
            result.bedType = bedType;
            cases_.assignBed(result.caseId, bed.bedId());
            if (doctor) {
                cases_.assignDoctor(result.caseId, doctorId);
                result.doctorAssigned = true;
                result.doctorId = doctorId;
                result.doctorName = doctor->name();
                result.doctorSpecialization = doctor->specialization();
                result.doctorLoad = doctor->currentPatients() + 1;
                result.doctorCapacity = doctor->maxPatients();
            }
            return;
        }
    }
    result.waitlisted = true;
}

AdmissionResult AdmissionService::admitNewPatient(const AdmissionRequest &req) {
    AdmissionResult result;
    std::lock_guard<std::mutex> lock(mutex_);

    // Validate the clinical part BEFORE creating the patient so a bad
    // severity can never leave a registered patient with no case behind it.
    if (!Validation::isValidSeverity(req.severity)) {
        result.message = "Severity must be between 1 (Critical) and 4 (Low)";
        return result;
    }
    if (req.category.empty()) {
        result.message = "Category is required";
        return result;
    }

    auto reg = patients_.registerPatient(req.name, req.age, req.gender, req.phone,
                                         req.bloodGroup, req.emergencyContact, req.dateOfBirth);
    if (!reg.success) {
        result.message = reg.message;
        return result;
    }
    result.patientId = reg.patientId;
    result.patientCode = patientCode(reg.patientId);
    result.duplicateWarning = reg.duplicateWarning;
    result.severity = req.severity;

    auto triage = triage_.admitCase(reg.patientId, req.severity, req.category);
    if (!triage.success) {
        // Patient row exists (registration is its own durable step) but could not be queued.
        result.message = "Patient registered as " + result.patientCode +
                         " but the emergency case failed: " + triage.message;
        return result;
    }
    result.caseId = triage.caseId;
    result.success = true;

    allocate(result, req.category);

    if (result.waitlisted) {
        result.message = "Registered as " + result.patientCode + " and queued by priority, but no " +
                         bedPreference(req.severity).front() + " bed is free yet. Patient is waitlisted.";
    } else if (!result.doctorAssigned) {
        result.message = "Registered as " + result.patientCode + ", " + result.bedType + " bed #" +
                         std::to_string(result.bedId) + " allocated. No doctor has free capacity right now.";
    } else {
        result.message = "Registered as " + result.patientCode + ", " + result.bedType + " bed #" +
                         std::to_string(result.bedId) + " allocated, assigned to Dr. " + result.doctorName + ".";
    }
    if (result.duplicateWarning) {
        result.message += " NOTE: a patient with the same name and phone already exists - please verify this is not a duplicate.";
    }
    return result;
}

AdmissionResult AdmissionService::allocateForCase(int caseId) {
    AdmissionResult result;
    std::lock_guard<std::mutex> lock(mutex_);

    auto ec = cases_.findById(caseId);
    if (!ec.has_value()) { result.message = "Case not found"; return result; }
    if (ec->assignedBedId() > 0) { result.message = "This case already has a bed allocated"; return result; }
    if (ec->status() == "Discharged") { result.message = "Case is already discharged"; return result; }

    result.patientId = ec->patientId();
    result.patientCode = patientCode(ec->patientId());
    result.caseId = ec->caseId();
    result.severity = ec->severity();
    result.success = true;

    allocate(result, ec->category());

    if (result.waitlisted) {
        result.message = "Still no suitable bed free - patient remains waitlisted.";
    } else {
        result.message = result.bedType + " bed #" + std::to_string(result.bedId) + " allocated" +
                         (result.doctorAssigned ? ", assigned to Dr. " + result.doctorName + "." : ".");
    }
    return result;
}

AllocationPreview AdmissionService::preview(int severity, const std::string &category) {
    AllocationPreview p;
    if (!Validation::isValidSeverity(severity)) return p;

    p.bedPreference = bedPreference(severity);
    for (const auto &bedType : p.bedPreference) {
        auto free = beds_.findAvailable(bedType);
        if (!free.empty()) {
            p.targetBedType = bedType;
            p.freeBedsInTarget = static_cast<int>(free.size());
            break;
        }
    }

    auto doctor = pickDoctor(category.empty() ? std::string("General") : category);
    if (doctor) {
        p.doctorFound = true;
        p.doctorId = doctor->id();
        p.doctorName = doctor->name();
        p.doctorSpecialization = doctor->specialization();
        p.doctorLoad = doctor->currentPatients();
        p.doctorCapacity = doctor->maxPatients();
    }
    return p;
}

} // namespace medipriority
