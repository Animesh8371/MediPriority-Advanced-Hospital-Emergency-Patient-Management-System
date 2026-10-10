-- ============================================================================
-- MediPriority - routing network over REAL hospitals from the govt directory
-- Run AFTER schema.sql and seed_hospitals_govt.sql.
--
-- The directory CSV has NO distances or coordinates, so the road-km values
-- below are APPROXIMATE, typed in by hand (not government data). Verify them
-- on a map before presenting them as measured. Hospital NAMES/CITIES are
-- exactly as in the directory.
-- ============================================================================
USE medipriority;

UPDATE hospitals SET in_route_network = TRUE
WHERE (city = 'Dehradun'  AND name IN ('S Kalawati Hospital Ozone Thearpy & Trauma Centre',
                                       'SGRR Medical College',
                                       'Lifeline Hospital & Urology Institute'))
   OR (city = 'Rishikesh' AND name = 'AIIMS, Rishikesh')
   OR (city = 'Roorkee'   AND name = 'Eye-Q Super Speciality Eye Hospitals, Roorkee');

INSERT INTO hospital_routes (from_hospital_id, to_hospital_id, distance_cost)
SELECT a.hospital_id, b.hospital_id, r.km
FROM (
    SELECT 'SGRR Medical College' AS f, 'S Kalawati Hospital Ozone Thearpy & Trauma Centre' AS t, 6.0 AS km
    UNION ALL SELECT 'SGRR Medical College', 'Lifeline Hospital & Urology Institute', 6.0
    UNION ALL SELECT 'S Kalawati Hospital Ozone Thearpy & Trauma Centre', 'Lifeline Hospital & Urology Institute', 1.5
    UNION ALL SELECT 'Lifeline Hospital & Urology Institute', 'AIIMS, Rishikesh', 42.0
    UNION ALL SELECT 'AIIMS, Rishikesh', 'Eye-Q Super Speciality Eye Hospitals, Roorkee', 55.0
    UNION ALL SELECT 'S Kalawati Hospital Ozone Thearpy & Trauma Centre', 'SGRR Medical College', 6.0
    UNION ALL SELECT 'Lifeline Hospital & Urology Institute', 'SGRR Medical College', 6.0
    UNION ALL SELECT 'Lifeline Hospital & Urology Institute', 'S Kalawati Hospital Ozone Thearpy & Trauma Centre', 1.5
    UNION ALL SELECT 'AIIMS, Rishikesh', 'Lifeline Hospital & Urology Institute', 42.0
    UNION ALL SELECT 'Eye-Q Super Speciality Eye Hospitals, Roorkee', 'AIIMS, Rishikesh', 55.0
) r
JOIN hospitals a ON a.name = r.f
JOIN hospitals b ON b.name = r.t;
