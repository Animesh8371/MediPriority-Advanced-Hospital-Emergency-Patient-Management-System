-- Upgrade an EXISTING database to the government hospital directory.
--   mysql -u root -p medipriority < database/upgrade_govt_hospitals.sql
--   mysql -u root -p medipriority < database/seed_hospitals_govt.sql
--   mysql -u root -p medipriority < database/seed_routes.sql
USE medipriority;
DELETE FROM hospital_routes;
DELETE FROM hospitals;
ALTER TABLE hospitals
    MODIFY name VARCHAR(200) NOT NULL,
    ADD COLUMN state VARCHAR(60) AFTER location,
    ADD COLUMN city VARCHAR(80) AFTER state,
    ADD COLUMN category VARCHAR(10) AFTER city,
    ADD COLUMN system_of_medicine VARCHAR(120) AFTER category,
    ADD COLUMN pin_code CHAR(6) AFTER system_of_medicine,
    ADD COLUMN contact_details VARCHAR(400) AFTER pin_code,
    ADD COLUMN phone VARCHAR(255) AFTER contact_details,
    ADD COLUMN email VARCHAR(255) AFTER phone,
    ADD COLUMN website VARCHAR(255) AFTER email,
    ADD COLUMN specializations TEXT AFTER website,
    ADD COLUMN services TEXT AFTER specializations,
    ADD COLUMN in_route_network BOOLEAN NOT NULL DEFAULT FALSE AFTER services,
    ADD INDEX idx_hospitals_state_city (state, city),
    ADD INDEX idx_hospitals_network (in_route_network);
