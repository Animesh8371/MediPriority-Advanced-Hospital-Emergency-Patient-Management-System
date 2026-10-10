-- ============================================================================
-- MediPriority - Database Schema
-- Academic prototype using simulated hospital data.
-- ============================================================================

CREATE DATABASE IF NOT EXISTS medipriority
    CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE medipriority;

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------------------------------------------------------
-- users: authentication (Module 0 - basic auth, added to MVP)
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS users;
CREATE TABLE users (
    user_id        INT AUTO_INCREMENT PRIMARY KEY,
    username       VARCHAR(50) NOT NULL UNIQUE,
    password_hash  CHAR(64) NOT NULL,   -- hex-encoded SHA-256, see PasswordUtil
    password_salt  CHAR(32) NOT NULL,   -- hex-encoded 16-byte salt
    role           ENUM('admin', 'staff') NOT NULL DEFAULT 'staff',
    created_at     TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- patients: Module 1 (Registration), Module 2 (Search)
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS patients;
CREATE TABLE patients (
    patient_id         INT AUTO_INCREMENT PRIMARY KEY,
    name               VARCHAR(100) NOT NULL,
    age                TINYINT UNSIGNED NOT NULL CHECK (age > 0 AND age <= 130),
    gender             ENUM('Male', 'Female', 'Other') NOT NULL,
    phone              VARCHAR(15) NOT NULL,
    blood_group        ENUM('A+','A-','B+','B-','AB+','AB-','O+','O-','Unknown') NOT NULL DEFAULT 'Unknown',
    emergency_contact  VARCHAR(15),
    date_of_birth      DATE,
    created_at         TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_patients_name (name),
    INDEX idx_patients_phone (phone)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- doctors: Module 4
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS doctors;
CREATE TABLE doctors (
    doctor_id       INT AUTO_INCREMENT PRIMARY KEY,
    name            VARCHAR(100) NOT NULL,
    age             TINYINT UNSIGNED NOT NULL CHECK (age > 0 AND age <= 100),
    gender          ENUM('Male', 'Female', 'Other') NOT NULL,
    specialization  VARCHAR(100) NOT NULL,
    available       BOOLEAN NOT NULL DEFAULT TRUE,          -- on-duty flag (manual busy/free)
    max_patients    TINYINT UNSIGNED NOT NULL DEFAULT 5,    -- how many patients one doctor can hold at once
    created_at      TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT chk_doctor_capacity CHECK (max_patients BETWEEN 1 AND 50),
    INDEX idx_doctors_specialization (specialization),
    INDEX idx_doctors_available (available)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- beds: Module 5 (General/Emergency/ICU)
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS beds;
CREATE TABLE beds (
    bed_id                 INT AUTO_INCREMENT PRIMARY KEY,
    bed_type               ENUM('General', 'Emergency', 'ICU') NOT NULL,
    occupied               BOOLEAN NOT NULL DEFAULT FALSE,
    occupied_by_patient_id INT NULL,
    CONSTRAINT fk_beds_patient FOREIGN KEY (occupied_by_patient_id)
        REFERENCES patients(patient_id) ON DELETE SET NULL,
    INDEX idx_beds_type_occupied (bed_type, occupied)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- ambulances: Module 6
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS ambulances;
CREATE TABLE ambulances (
    ambulance_id    INT AUTO_INCREMENT PRIMARY KEY,
    vehicle_number  VARCHAR(20) NOT NULL UNIQUE,
    status          ENUM('Available', 'Assigned', 'Maintenance') NOT NULL DEFAULT 'Available'
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- emergency_cases: Module 3 (Triage) - mirrors the C heap's EmergencyCase
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS emergency_cases;
CREATE TABLE emergency_cases (
    case_id               INT AUTO_INCREMENT PRIMARY KEY,
    patient_id            INT NOT NULL,
    severity              TINYINT NOT NULL CHECK (severity BETWEEN 1 AND 4), -- 1=Critical..4=Low
    category              VARCHAR(50) NOT NULL,          -- simulated category, e.g. "Trauma"
    arrival_time          BIGINT NOT NULL,                -- unix-like timestamp used for tie-breaking
    assigned_doctor_id    INT NULL,
    assigned_bed_id       INT NULL,
    assigned_ambulance_id INT NULL,
    status                ENUM('Waiting', 'In Treatment', 'Discharged') NOT NULL DEFAULT 'Waiting',
    created_at            TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_ec_patient   FOREIGN KEY (patient_id)   REFERENCES patients(patient_id)   ON DELETE CASCADE,
    CONSTRAINT fk_ec_doctor    FOREIGN KEY (assigned_doctor_id)    REFERENCES doctors(doctor_id)       ON DELETE SET NULL,
    CONSTRAINT fk_ec_bed       FOREIGN KEY (assigned_bed_id)       REFERENCES beds(bed_id)             ON DELETE SET NULL,
    CONSTRAINT fk_ec_ambulance FOREIGN KEY (assigned_ambulance_id) REFERENCES ambulances(ambulance_id) ON DELETE SET NULL,
    INDEX idx_ec_status (status),
    INDEX idx_ec_severity (severity)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- appointments: Module 8
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS appointments;
CREATE TABLE appointments (
    appointment_id  INT AUTO_INCREMENT PRIMARY KEY,
    patient_id      INT NOT NULL,
    doctor_id       INT NOT NULL,
    scheduled_time  BIGINT NOT NULL,
    status          ENUM('Scheduled', 'Completed', 'Cancelled') NOT NULL DEFAULT 'Scheduled',
    created_at      TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_appt_patient FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
    CONSTRAINT fk_appt_doctor  FOREIGN KEY (doctor_id)  REFERENCES doctors(doctor_id)   ON DELETE CASCADE,
    INDEX idx_appt_doctor_time (doctor_id, scheduled_time)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- medical_history: Module 9
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS medical_history;
CREATE TABLE medical_history (
    record_id   INT AUTO_INCREMENT PRIMARY KEY,
    patient_id  INT NOT NULL,
    diagnosis   VARCHAR(128) NOT NULL,
    notes       VARCHAR(256),
    timestamp   BIGINT NOT NULL,
    CONSTRAINT fk_history_patient FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
    INDEX idx_history_patient_time (patient_id, timestamp)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- hospitals (govt directory) + hospital_routes: Module 7
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS hospital_routes;
DROP TABLE IF EXISTS hospitals;

CREATE TABLE hospitals (
    hospital_id         INT AUTO_INCREMENT PRIMARY KEY,
    name                VARCHAR(200) NOT NULL,
    location            VARCHAR(150),            -- "City, State"
    state               VARCHAR(60),
    city                VARCHAR(80),
    category            VARCHAR(10),             -- Public / Private (from the govt directory)
    system_of_medicine  VARCHAR(120),
    pin_code            CHAR(6),
    contact_details     VARCHAR(400),
    phone               VARCHAR(255),
    email               VARCHAR(255),
    website             VARCHAR(255),
    specializations     TEXT,
    services            TEXT,
    -- The Dijkstra graph in C holds at most 32 nodes, so only hospitals flagged
    -- here (see seed_routes.sql) join the routing network; the rest are the directory.
    in_route_network    BOOLEAN NOT NULL DEFAULT FALSE,
    INDEX idx_hospitals_state_city (state, city),
    INDEX idx_hospitals_network (in_route_network)
) ENGINE=InnoDB;

CREATE TABLE hospital_routes (
    route_id          INT AUTO_INCREMENT PRIMARY KEY,
    from_hospital_id  INT NOT NULL,
    to_hospital_id    INT NOT NULL,
    distance_cost     DECIMAL(8,2) NOT NULL CHECK (distance_cost > 0),
    CONSTRAINT fk_route_from FOREIGN KEY (from_hospital_id) REFERENCES hospitals(hospital_id) ON DELETE CASCADE,
    CONSTRAINT fk_route_to   FOREIGN KEY (to_hospital_id)   REFERENCES hospitals(hospital_id) ON DELETE CASCADE,
    CONSTRAINT uq_route_pair UNIQUE (from_hospital_id, to_hospital_id)
) ENGINE=InnoDB;

-- ----------------------------------------------------------------------------
-- patient_assignments: junction/history table for bed+doctor allocation events
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS patient_assignments;
CREATE TABLE patient_assignments (
    assignment_id  INT AUTO_INCREMENT PRIMARY KEY,
    patient_id     INT NOT NULL,
    bed_id         INT NULL,
    doctor_id      INT NULL,
    assigned_at    TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    released_at    TIMESTAMP NULL,
    -- A doctor's current load = rows here with released_at IS NULL, so one
    -- doctor can be linked to many patients; this index keeps that count fast.
    INDEX idx_assign_doctor_active (doctor_id, released_at),
    INDEX idx_assign_bed_active (bed_id, released_at),
    CONSTRAINT fk_assign_patient FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
    CONSTRAINT fk_assign_bed     FOREIGN KEY (bed_id)     REFERENCES beds(bed_id)         ON DELETE SET NULL,
    CONSTRAINT fk_assign_doctor  FOREIGN KEY (doctor_id)  REFERENCES doctors(doctor_id)   ON DELETE SET NULL
) ENGINE=InnoDB;

SET FOREIGN_KEY_CHECKS = 1;
