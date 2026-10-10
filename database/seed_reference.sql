-- ============================================================================
-- MediPriority - Extra reference + synthetic data (OPTIONAL, run after seed.sql)
--
--   mysql -u root -p medipriority < database/seed_reference.sql
--
-- WHAT IS REAL AND WHAT IS NOT
--  * Specialization names follow the usual Indian clinical-department list.
--  * Hospitals are NOT in this file any more: they are loaded from the
--    government hospital directory (seed_hospitals_govt.sql).
--  * Every doctor name and every patient name is SYNTHETIC. No real
--    individual's data is used anywhere.
-- ============================================================================

USE medipriority;

-- ---- Synthetic doctors: wider specialty coverage, different capacities -----
INSERT INTO doctors (name, age, gender, specialization, available, max_patients) VALUES
('Dr. Ritu Bansal',      41, 'Female', 'Pulmonology',               TRUE, 5),
('Dr. Manish Tiwari',    46, 'Male',   'Pediatrics',                TRUE, 6),
('Dr. Kavita Rawat',     38, 'Female', 'Obstetrics & Gynaecology',  TRUE, 4),
('Dr. Harish Negi',      50, 'Male',   'Plastic Surgery',           TRUE, 3),
('Dr. Pooja Chauhan',    36, 'Female', 'Gastroenterology',          TRUE, 5),
('Dr. Rajiv Bhatt',      55, 'Male',   'Cardiology',                TRUE, 4),
('Dr. Sneha Pundir',     33, 'Female', 'Emergency Medicine',        TRUE, 6),
('Dr. Amit Joshi',       42, 'Male',   'Neurology',                 TRUE, 4),
('Dr. Deepa Sati',       37, 'Female', 'General Medicine',          TRUE, 8),
('Dr. Naveen Kandari',   45, 'Male',   'Orthopedics',               TRUE, 5);

-- ---- More beds so a demo can admit a good number of patients --------------
INSERT INTO beds (bed_type, occupied) VALUES
('ICU', FALSE), ('ICU', FALSE), ('ICU', FALSE),
('Emergency', FALSE), ('Emergency', FALSE), ('Emergency', FALSE), ('Emergency', FALSE),
('General', FALSE), ('General', FALSE), ('General', FALSE), ('General', FALSE),
('General', FALSE), ('General', FALSE), ('General', FALSE), ('General', FALSE);

-- Hospitals and routes: see seed_hospitals_govt.sql and seed_routes.sql.
