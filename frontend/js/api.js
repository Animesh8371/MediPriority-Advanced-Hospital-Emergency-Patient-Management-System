/* ============================================================================
   MediPriority - api.js
   Thin fetch() wrapper. The session token is kept in localStorage ONLY so a
   staff member's login survives a page reload/navigation between the pages
   of this multi-page app (there is no other client-side state to lose) --
   never anything more sensitive than the opaque token itself.
   ============================================================================ */

const API_BASE = window.location.origin.includes("file://")
    ? "http://127.0.0.1:8080"
    : (window.MEDIPRIORITY_API_BASE || "http://127.0.0.1:8080");

const Api = {
    getToken() { return localStorage.getItem("medipriority_token"); },
    setSession(token, role, username) {
        localStorage.setItem("medipriority_token", token);
        localStorage.setItem("medipriority_role", role);
        localStorage.setItem("medipriority_username", username);
    },
    clearSession() {
        localStorage.removeItem("medipriority_token");
        localStorage.removeItem("medipriority_role");
        localStorage.removeItem("medipriority_username");
    },
    isLoggedIn() { return !!Api.getToken(); },
    getRole() { return localStorage.getItem("medipriority_role") || ""; },
    getUsername() { return localStorage.getItem("medipriority_username") || ""; },

    async request(path, options = {}) {
        const headers = Object.assign(
            { "Content-Type": "application/json" },
            options.headers || {}
        );
        const token = Api.getToken();
        if (token) headers["Authorization"] = "Bearer " + token;

        let response;
        try {
            response = await fetch(API_BASE + path, Object.assign({}, options, { headers }));
        } catch (networkErr) {
            throw new Error(
                "Could not reach the MediPriority server at " + API_BASE +
                ". Is the backend running? (see README.md)"
            );
        }

        let body = null;
        try { body = await response.json(); } catch (_) { /* empty body is fine for some responses */ }

        if (response.status === 401) {
            Api.clearSession();
            if (!window.location.pathname.endsWith("login.html")) {
                window.location.href = pathToRoot() + "login.html";
            }
            throw new Error("Session expired. Please log in again.");
        }

        if (!response.ok) {
            const msg = (body && body.message) ? body.message : ("Request failed (" + response.status + ")");
            throw new Error(msg);
        }
        return body;
    },

    get(path) { return Api.request(path, { method: "GET" }); },
    post(path, data) { return Api.request(path, { method: "POST", body: JSON.stringify(data || {}) }); },
    put(path, data) { return Api.request(path, { method: "PUT", body: JSON.stringify(data || {}) }); },

    /* Downloads a file from an authenticated endpoint. A plain <a href> to
     * an API URL can't carry the Authorization header, so this fetches the
     * response as a blob and saves it via a throwaway link instead. */
    async download(path, filename) {
        const token = Api.getToken();
        const headers = token ? { "Authorization": "Bearer " + token } : {};
        const response = await fetch(API_BASE + path, { headers });
        if (!response.ok) throw new Error("Export failed (" + response.status + ")");
        const blob = await response.blob();
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        a.remove();
        URL.revokeObjectURL(url);
    },
};

/* Converts an array of plain objects into a CSV file and downloads it
 * client-side -- no backend round trip needed for exporting whatever a
 * page already has loaded (e.g. the current patient search results). */
function downloadClientCsv(filename, rows) {
    if (!rows || rows.length === 0) {
        alert("Nothing to export yet.");
        return;
    }
    const headers = Object.keys(rows[0]);
    const escape = (v) => {
        const s = (v === null || v === undefined) ? "" : String(v);
        return /[",\n]/.test(s) ? '"' + s.replace(/"/g, '""') + '"' : s;
    };
    const lines = [headers.join(",")];
    for (const row of rows) lines.push(headers.map(h => escape(row[h])).join(","));

    const blob = new Blob([lines.join("\r\n")], { type: "text/csv;charset=utf-8;" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    a.remove();
    URL.revokeObjectURL(url);
}

/* Pages live either at frontend/*.html (root) or frontend/pages/*.html, so
 * relative links to css/js/other pages need a different prefix depending on
 * depth. This returns "" at root and "../" from inside pages/. */
function pathToRoot() {
    return window.location.pathname.includes("/pages/") ? "../" : "";
}

function requireLogin() {
    if (!Api.isLoggedIn()) {
        window.location.href = pathToRoot() + "login.html";
    }
}

function showMessage(el, text, type) {
    el.textContent = text;
    el.className = "message show " + type;
}

function hideMessage(el) {
    el.className = "message";
}
