-- ============================================================================
-- MediPriority - Seed Data (SIMULATED, for academic demonstration only)
-- Run this AFTER schema.sql.
-- ============================================================================

USE medipriority;

-- Default admin user. Username: admin  Password: Admin@123
-- This hash/salt pair was actually generated (not fabricated) by running
-- PasswordUtil::hashPassword("Admin@123", <fixed salt>) -- see docs/testing.md
-- for the exact command. CHANGE THIS PASSWORD after first login in any
-- real deployment; a fixed salt here is only for reproducible seed data.
INSERT INTO users (username, password_hash, password_salt, role) VALUES
('admin', 'e6a92451fa374fb20dbb3661b8bd834b937cfbdcaa7ff78083426a6e93fffc96', 'a1b2c3d4e5f60718293a4b5c6d7e8f90', 'admin');

-- Patients (simulated demographic data)
INSERT INTO patients (name, age, gender, phone, blood_group, emergency_contact, date_of_birth) VALUES
('Aditi Sharma',  34, 'Female', '9876500001', 'B+',  '9876500011', '1992-04-11'),
('Rohan Verma',   51, 'Male',   '9876500002', 'O-',  '9876500012', '1975-01-22'),
('Meera Nair',    22, 'Female', '9876500003', 'A+',  '9876500013', '2004-08-30'),
('Karan Singh',   45, 'Male',   '9876500004', 'AB+', '9876500014', '1981-06-15'),
('Sanya Kapoor',  29, 'Female', '9876500005', 'O+',  '9876500015', '1997-02-19');

-- Doctors
INSERT INTO doctors (name, age, gender, specialization, available) VALUES
('Dr. Anil Kumar',    48, 'Male',   'Cardiology',      TRUE),
('Dr. Priya Menon',   39, 'Female', 'Emergency Medicine', TRUE),
('Dr. Suresh Rao',    52, 'Male',   'Orthopedics',     TRUE),
('Dr. Neha Joshi',    35, 'Female', 'General Medicine', TRUE),
('Dr. Vikram Desai',  44, 'Male',   'Neurology',       FALSE);

-- Beds: mix of General, Emergency, ICU
INSERT INTO beds (bed_type, occupied) VALUES
('General', FALSE), ('General', FALSE), ('General', TRUE),
('Emergency', FALSE), ('Emergency', FALSE),
('ICU', FALSE), ('ICU', TRUE);

-- Ambulances
INSERT INTO ambulances (vehicle_number, status) VALUES
('AMB-101', 'Available'),
('AMB-102', 'Available'),
('AMB-103', 'Maintenance');

-- Hospitals now come from the government directory: load database/seed_hospitals_govt.sql
-- and then database/seed_routes.sql (see README).
