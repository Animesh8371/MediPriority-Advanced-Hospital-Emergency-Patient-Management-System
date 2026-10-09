/* ============================================================================
   MediPriority - nav.js
   Builds the sidebar (with inline SVG icons), theme toggle and session footer
   into any page that has an empty <div id="sidebar"></div>. Also hosts the
   small UI helpers shared by every page: toast(), openModal(), esc(),
   countUp(), initialsOf(). Keeping them here means one file to include.
   ============================================================================ */

const ICONS = {
    home: '<path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/>',
    userplus: '<path d="M16 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/><circle cx="8.5" cy="7" r="4"/><line x1="20" y1="8" x2="20" y2="14"/><line x1="23" y1="11" x2="17" y2="11"/>',
    search: '<circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/>',
    activity: '<polyline points="22 12 18 12 15 21 9 3 6 12 2 12"/>',
    users: '<path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/><circle cx="9" cy="7" r="4"/><path d="M23 21v-2a4 4 0 0 0-3-3.87"/><path d="M16 3.13a4 4 0 0 1 0 7.75"/>',
    bed: '<path d="M2 18v-8a2 2 0 0 1 2-2h16a2 2 0 0 1 2 2v8"/><path d="M2 14h20"/><path d="M2 20v-2"/><path d="M22 20v-2"/><path d="M6 11h4"/>',
    truck: '<rect x="1" y="3" width="15" height="13"/><polygon points="16 8 20 8 23 11 23 16 16 16 16 8"/><circle cx="5.5" cy="18.5" r="2.5"/><circle cx="18.5" cy="18.5" r="2.5"/>',
    calendar: '<rect x="3" y="4" width="18" height="18" rx="2" ry="2"/><line x1="16" y1="2" x2="16" y2="6"/><line x1="8" y1="2" x2="8" y2="6"/><line x1="3" y1="10" x2="21" y2="10"/>',
    clipboard: '<path d="M16 4h2a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2V6a2 2 0 0 1 2-2h2"/><rect x="8" y="2" width="8" height="4" rx="1" ry="1"/>',
    map: '<polygon points="1 6 1 22 8 18 16 22 23 18 23 2 16 6 8 2 1 6"/><line x1="8" y1="2" x2="8" y2="18"/><line x1="16" y1="6" x2="16" y2="22"/>',
    file: '<path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/>',
    chart: '<line x1="18" y1="20" x2="18" y2="10"/><line x1="12" y1="20" x2="12" y2="4"/><line x1="6" y1="20" x2="6" y2="14"/>',
    db: '<ellipse cx="12" cy="5" rx="9" ry="3"/><path d="M21 12c0 1.66-4 3-9 3s-9-1.34-9-3"/><path d="M3 5v14c0 1.66 4 3 9 3s9-1.34 9-3V5"/>',
    moon: '<path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"/>',
    out: '<path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/><polyline points="16 17 21 12 16 7"/><line x1="21" y1="12" x2="9" y2="12"/>',
    check: '<polyline points="20 6 9 17 4 12"/>',
    alert: '<path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/>',
    heart: '<path d="M20.84 4.61a5.5 5.5 0 0 0-7.78 0L12 5.67l-1.06-1.06a5.5 5.5 0 0 0-7.78 7.78l1.06 1.06L12 21.23l7.78-7.78 1.06-1.06a5.5 5.5 0 0 0 0-7.78z"/>',
};
function icon(name) { return `<svg class="ic" viewBox="0 0 24 24" aria-hidden="true">${ICONS[name] || ""}</svg>`; }

/* ---- theme (light/dark), remembered per browser ---- */
function applyTheme(t) { document.documentElement.setAttribute("data-theme", t); }
applyTheme(localStorage.getItem("medipriority_theme") || "light");

/* ---- shared helpers ---- */
function esc(v) {
    return String(v === null || v === undefined ? "" : v)
        .replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;").replace(/'/g, "&#39;");
}
function initialsOf(name) {
    const parts = String(name || "?").replace(/^Dr\.?\s*/i, "").trim().split(/\s+/);
    return ((parts[0] || "?")[0] + (parts.length > 1 ? parts[parts.length - 1][0] : "")).toUpperCase();
}
function hueGradient(seed) {
    let h = 0; for (const c of String(seed)) h = (h * 31 + c.charCodeAt(0)) % 360;
    return `--h1:hsl(${h},65%,48%);--h2:hsl(${(h + 45) % 360},70%,45%)`;
}
function codeOf(id) { return "MP-" + String(id).padStart(6, "0"); }

function toast(text, type = "info", ms = 4200) {
    let host = document.getElementById("toastHost");
    if (!host) { host = document.createElement("div"); host.id = "toastHost"; document.body.appendChild(host); }
    const el = document.createElement("div");
    el.className = "toast " + type;
    el.innerHTML = `<span>${esc(text)}</span>`;
    host.appendChild(el);
    setTimeout(() => { el.classList.add("out"); setTimeout(() => el.remove(), 260); }, ms);
}

/* Opens a modal with the given inner HTML; returns {el, close}. Esc / backdrop click closes. */
function openModal(html, opts = {}) {
    const back = document.createElement("div");
    back.className = "modal-back";
    back.innerHTML = `<div class="modal ${opts.wide ? "wide" : ""}" role="dialog" aria-modal="true">${html}</div>`;
    document.body.appendChild(back);
    const close = () => { back.remove(); document.removeEventListener("keydown", onKey); if (opts.onClose) opts.onClose(); };
    const onKey = (e) => { if (e.key === "Escape") close(); };
    document.addEventListener("keydown", onKey);
    back.addEventListener("mousedown", (e) => { if (e.target === back) close(); });
    back.querySelectorAll("[data-close]").forEach(b => b.addEventListener("click", close));
    return { el: back.firstElementChild, close };
}

function countUp(el, to, ms = 700) {
    const target = Number(to) || 0, start = performance.now();
    if (!window.requestAnimationFrame || target === 0) { el.textContent = target; return; }
    const tick = (now) => {
        const p = Math.min(1, (now - start) / ms);
        el.textContent = Math.round(target * (1 - Math.pow(1 - p, 3)));
        if (p < 1) requestAnimationFrame(tick);
    };
    requestAnimationFrame(tick);
}

function renderSidebar(activeHref) {
    const root = pathToRoot();
    const items = [
        ["index.html", "Dashboard", "home"],
        ["--", "Patient flow"],
        ["pages/patient-register.html", "Register & Admit", "userplus"],
        ["pages/triage.html", "Emergency Triage", "activity"],
        ["pages/patient-search.html", "Patient Search", "search"],
        ["pages/medical-history.html", "Medical History", "clipboard"],
        ["pages/appointments.html", "Appointments", "calendar"],
        ["--", "Resources"],
        ["pages/doctors.html", "Doctors", "users"],
        ["pages/beds.html", "Beds & ICU", "bed"],
        ["pages/ambulances.html", "Ambulances", "truck"],
        ["pages/routing.html", "Hospital Routing", "map"],
        ["--", "Insights"],
        ["pages/reports.html", "Reports", "file"],
        ["pages/analytics.html", "Analytics", "chart"],
        ["pages/data-tools.html", "Sample Data", "db"],
    ];

    let navHtml = "";
    for (const [href, label, ic] of items) {
        if (href === "--") { navHtml += `<div class="nav-sep">${label}</div>`; continue; }
        navHtml += `<a href="${root + href}" class="${activeHref === href ? "active" : ""}">${icon(ic)}${label}</a>`;
    }

    const username = Api.getUsername();
    const role = Api.getRole();

    document.getElementById("sidebar").innerHTML = `
        <div class="brand"><div class="logo"><svg viewBox="0 0 24 24"><path d="M12 5v14M5 12h14"/></svg></div>
            <div>MediPriority<small>Emergency &amp; Patient Management</small></div></div>
        <nav>${navHtml}</nav>
        <div class="session">
            Signed in as <strong>${esc(username || "?")}</strong> (${esc(role || "?")})
            <div class="row">
                <button id="themeBtn" title="Toggle dark mode">${icon("moon")}Theme</button>
                <button id="logoutBtn">${icon("out")}Log out</button>
            </div>
        </div>
    `;

    document.getElementById("themeBtn").addEventListener("click", () => {
        const next = document.documentElement.getAttribute("data-theme") === "dark" ? "light" : "dark";
        localStorage.setItem("medipriority_theme", next);
        applyTheme(next);
    });
    document.getElementById("logoutBtn").addEventListener("click", async () => {
        try { await Api.post("/api/auth/logout", {}); } catch (_) { /* ignore */ }
        Api.clearSession();
        window.location.href = root + "login.html";
    });

    renderNotificationBell();
}

/* ============================================================================
   Notification bell: polls GET /api/notifications, which computes live
   operational alerts (long waits, near-full ICU, no free ambulances, low
   doctor availability) straight from current MySQL state on every call --
   nothing is stored server-side, so there's no "unread" flag to manage.
   Injected once per page load into a fixed corner so every page gets it
   without every page.html needing its own markup.
   ============================================================================ */
function renderNotificationBell() {
    if (document.getElementById("notifBell")) return;

    const bell = document.createElement("div");
    bell.id = "notifBell";
    bell.className = "notif-bell";
    bell.innerHTML = `
        <button id="notifBtn" class="notif-btn" title="Live alerts" aria-label="Live alerts">
            <span class="notif-icon">&#128276;</span>
            <span id="notifCount" class="notif-count" style="display:none">0</span>
        </button>
        <div id="notifPanel" class="notif-panel" style="display:none">
            <div class="notif-panel-header">Live alerts</div>
            <div id="notifList" class="notif-list"><div class="empty-state">Loading...</div></div>
        </div>
    `;
    document.body.appendChild(bell);

    document.getElementById("notifBtn").addEventListener("click", (e) => {
        e.stopPropagation();
        const panel = document.getElementById("notifPanel");
        const opening = panel.style.display === "none";
        panel.style.display = opening ? "block" : "none";
        if (opening) refreshNotifications();
    });

    document.addEventListener("click", (e) => {
        const root = document.getElementById("notifBell");
        const panel = document.getElementById("notifPanel");
        if (root && panel && !root.contains(e.target)) panel.style.display = "none";
    });

    refreshNotifications();
    setInterval(refreshNotifications, 30000);
}

async function refreshNotifications() {
    const countEl = document.getElementById("notifCount");
    const listEl = document.getElementById("notifList");
    if (!countEl || !listEl) return;

    try {
        const res = await Api.get("/api/notifications");
        const alerts = res.alerts || [];

        if (alerts.length === 0) {
            countEl.style.display = "none";
            listEl.innerHTML = `<div class="empty-state">No active alerts.</div>`;
            return;
        }

        countEl.style.display = "inline-block";
        countEl.textContent = alerts.length;

        const badgeClass = (level) => level === "critical" ? "critical" : (level === "warning" ? "high" : "moderate");
        listEl.innerHTML = alerts.map(a => `
            <div class="notif-item">
                <span class="badge ${badgeClass(a.level)}">${a.level}</span>
                <span class="notif-item-text">${esc(a.message)}</span>
            </div>
        `).join("");
    } catch (err) {
        // Notification polling should never interrupt the page itself.
        listEl.innerHTML = `<div class="empty-state">Couldn't load alerts.</div>`;
    }
}
