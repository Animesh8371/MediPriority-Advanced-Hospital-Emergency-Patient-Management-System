/* ============================================================================
   MediPriority - refdata.js
   Reference data + synthetic data generator used by the registration page and
   the "Sample Data" page.

   READ THIS BEFORE PRESENTING THE DATA AS "GOVERNMENT VERIFIED"
   -------------------------------------------------------------
   * This file was NOT downloaded from a government website. The app can't
     scrape live portals, and real patient records are private health data
     that government sites do not publish.
   * What IS here:
       - ICD-10 diagnosis codes. ICD-10 is the WHO classification that India's
         public health systems use; the codes/labels below were typed in from
         that standard, so verify any you plan to cite at https://icd.who.int.
       - India's public emergency numbers (112 / 108 / 102).
       - A department list that follows common Indian hospital departments.
   * The "suggested severity" per presentation is a SIMULATED academic value
     used to drive the allocator. It is not clinical triage guidance.
   * Everything under `Synthetic` is randomly generated. Names, ages and phone
     numbers are fictional; phones use the 98765xxxxx pattern like seed.sql, so
     a generated number could by coincidence match a real one - never use this
     to contact anybody.
   ============================================================================ */

const RefData = {
    sources: [
        { name: "ICD-10 / ICD-11 browser (WHO)", url: "https://icd.who.int", use: "Verify diagnosis codes and labels" },
        { name: "National Health Facility Registry (ABDM)", url: "https://nhfr.abdm.gov.in", use: "Official list of hospitals / health facilities" },
        { name: "Healthcare Professionals Registry (ABDM)", url: "https://hpr.abdm.gov.in", use: "Official register of doctors and health professionals" },
        { name: "Open Government Data Platform India", url: "https://data.gov.in", use: "Downloadable open datasets (health statistics, facility lists)" },
        { name: "National Medical Commission", url: "https://www.nmc.org.in", use: "Indian Medical Register / specialty names" },
    ],

    emergencyNumbers: [
        { number: "112", label: "Unified emergency response (police, fire, medical)" },
        { number: "108", label: "Emergency ambulance service (most states)" },
        { number: "102", label: "Patient transport / mother-and-child ambulance (many states)" },
    ],

    bloodGroups: ["A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"],

    /* category must match the keywords AdmissionService::specialtyPreference understands. */
    departments: [
        "Cardiology", "Neurology", "Orthopedics", "Pulmonology", "Pediatrics",
        "Obstetrics & Gynaecology", "Plastic Surgery", "Gastroenterology",
        "Emergency Medicine", "General Medicine",
    ],

    /* [ICD-10 code, label, category sent to backend, suggested severity 1-4] */
    presentations: [
        { code: "I46",  label: "Cardiac arrest",                         category: "Cardiac",        severity: 1 },
        { code: "I21",  label: "Acute myocardial infarction (heart attack)", category: "Cardiac",    severity: 1 },
        { code: "R07",  label: "Chest pain",                             category: "Cardiac",        severity: 2 },
        { code: "I63",  label: "Cerebral infarction (stroke)",           category: "Neurological",   severity: 1 },
        { code: "S06",  label: "Intracranial (head) injury",             category: "Neurological",   severity: 1 },
        { code: "G40",  label: "Epilepsy / seizure",                     category: "Neurological",   severity: 2 },
        { code: "R55",  label: "Syncope and collapse",                   category: "Neurological",   severity: 3 },
        { code: "J96",  label: "Respiratory failure",                    category: "Respiratory",    severity: 1 },
        { code: "J45",  label: "Asthma attack",                          category: "Respiratory",    severity: 2 },
        { code: "J18",  label: "Pneumonia",                              category: "Respiratory",    severity: 3 },
        { code: "T78",  label: "Anaphylaxis / severe allergic reaction", category: "Emergency",      severity: 1 },
        { code: "S36",  label: "Abdominal organ injury",                 category: "Trauma",         severity: 1 },
        { code: "S72",  label: "Femur fracture",                         category: "Trauma",         severity: 2 },
        { code: "T14",  label: "Injury, unspecified (minor trauma)",     category: "Trauma",         severity: 4 },
        { code: "T31",  label: "Burns (by body-surface area)",           category: "Burns",          severity: 2 },
        { code: "T65",  label: "Poisoning / toxic effect",               category: "Emergency",      severity: 2 },
        { code: "O14",  label: "Pre-eclampsia (pregnancy)",              category: "Obstetric",      severity: 2 },
        { code: "K35",  label: "Acute appendicitis",                     category: "Gastro",         severity: 2 },
        { code: "A91",  label: "Dengue haemorrhagic fever",              category: "General",        severity: 2 },
        { code: "E11",  label: "Diabetes mellitus (complication)",       category: "General",        severity: 3 },
        { code: "A09",  label: "Gastroenteritis",                        category: "Gastro",         severity: 3 },
        { code: "N39",  label: "Urinary tract infection",                category: "General",        severity: 4 },
        { code: "R50",  label: "Fever of unknown origin",                category: "General",        severity: 4 },
        { code: "R51",  label: "Headache",                               category: "General",        severity: 4 },
    ],

    findPresentation(code) { return this.presentations.find(p => p.code === code) || null; },
};

const Synthetic = (() => {
    const pick = (arr) => arr[Math.floor(Math.random() * arr.length)];
    const int = (a, b) => a + Math.floor(Math.random() * (b - a + 1));

    const male = ["Aarav", "Vihaan", "Arjun", "Rohan", "Karan", "Ankit", "Rahul", "Sandeep", "Mohit", "Deepak", "Vikas", "Nitin", "Pankaj", "Gaurav", "Ashish", "Himanshu", "Yash", "Manoj", "Suresh", "Dinesh"];
    const female = ["Aditi", "Ananya", "Diya", "Meera", "Sanya", "Priya", "Neha", "Kavita", "Pooja", "Ritu", "Sneha", "Shivani", "Anjali", "Isha", "Nisha", "Komal", "Rekha", "Sunita", "Geeta", "Mamta"];
    const last = ["Rawat", "Negi", "Bisht", "Panwar", "Thapliyal", "Nautiyal", "Bhatt", "Sharma", "Verma", "Singh", "Kumar", "Gupta", "Joshi", "Chauhan", "Rana", "Pant", "Semwal", "Dobhal", "Kandari", "Uniyal", "Khanduri", "Pundir", "Sati", "Kapoor", "Nair"];

    /* rough, illustrative blood-group mix (not an official statistic) */
    const bloodBag = ["B+", "B+", "B+", "O+", "O+", "O+", "A+", "A+", "AB+", "B-", "O-", "A-", "AB-"];

    /* more mild cases than critical ones, like a real emergency department */
    const sevBag = [1, 2, 2, 3, 3, 3, 4, 4, 4, 4];

    const phone = () => "98765" + String(int(0, 99999)).padStart(5, "0");

    function dobFor(age) {
        const d = new Date();
        d.setFullYear(d.getFullYear() - age);
        d.setDate(d.getDate() - int(0, 364));
        return d.toISOString().slice(0, 10);
    }

    function patient() {
        const gender = Math.random() < 0.03 ? "Other" : pick(["Male", "Female"]);
        const first = gender === "Female" ? pick(female) : pick(male);
        const age = pick([int(1, 12), int(18, 45), int(18, 45), int(30, 70), int(55, 90)]);
        return {
            name: first + " " + pick(last),
            age,
            gender,
            phone: phone(),
            blood_group: pick(bloodBag),
            emergency_contact: phone(),
            date_of_birth: dobFor(age),
        };
    }

    /* A full admission payload: patient + presentation (category + severity). */
    function admission() {
        const wanted = pick(sevBag);
        const pool = RefData.presentations.filter(p => p.severity === wanted);
        const pres = pick(pool.length ? pool : RefData.presentations);
        const p = patient();
        if (pres.category === "Obstetric") {      // keep the synthetic record medically sensible
            p.gender = "Female"; p.age = int(20, 38); p.date_of_birth = dobFor(p.age);
            p.name = pick(female) + " " + pick(last);
        }
        return Object.assign(p, { severity: pres.severity, category: pres.category, _label: pres.label, _code: pres.code });
    }

    return { patient, admission };
})();
