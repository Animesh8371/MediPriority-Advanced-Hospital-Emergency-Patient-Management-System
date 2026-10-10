-- ============================================================================
-- MediPriority - upgrade for an EXISTING database (advanced admission update)
-- Run once against a database that was created with the OLD schema.sql.
-- A fresh install does not need this: schema.sql already contains it.
--
--   mysql -u root -p medipriority < database/upgrade_advanced.sql
-- ============================================================================
USE medipriority;

-- 1) A doctor can now hold several patients at once, up to max_patients.
ALTER TABLE doctors
    ADD COLUMN max_patients TINYINT UNSIGNED NOT NULL DEFAULT 5 AFTER available;

ALTER TABLE doctors
    ADD CONSTRAINT chk_doctor_capacity CHECK (max_patients BETWEEN 1 AND 50);

-- 2) Indexes that keep "how many active patients does this doctor/bed have" fast.
ALTER TABLE patient_assignments
    ADD INDEX idx_assign_doctor_active (doctor_id, released_at),
    ADD INDEX idx_assign_bed_active (bed_id, released_at);
