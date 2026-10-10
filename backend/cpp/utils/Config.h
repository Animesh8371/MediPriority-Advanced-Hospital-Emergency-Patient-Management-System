#ifndef MEDIPRIORITY_CONFIG_H
#define MEDIPRIORITY_CONFIG_H

#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <vector>

namespace medipriority {

/* Reads configuration from OS environment variables, with an automatic
 * fallback to a ".env" file sitting next to the project (or a few parent
 * directories up, since the server binary usually runs from backend/build/).
 *
 * Why this exists: real environment variables set with PowerShell's
 * `$env:NAME="value"` are NOT visible to a Command Prompt (cmd.exe) session,
 * and vice versa -- `set NAME=value` in cmd.exe does not carry over to
 * PowerShell. Previously this project required the person to manually
 * export every variable in whichever shell they happened to open, which is
 * exactly the kind of thing that "works on my machine" and silently fails
 * for everyone else. Now the server reads .env directly, so it behaves the
 * same whether it's launched from cmd.exe, PowerShell, a VS Code task, or a
 * plain double-click.
 *
 * Precedence (highest wins): real OS environment variable > value from
 * .env file > built-in default. That way Docker/CI/production setups that
 * already export real env vars keep working unchanged. */
struct Config {
    std::string dbHost;
    std::string dbPort;
    std::string dbUser;
    std::string dbPassword;
    std::string dbName;
    int apiPort;

    static std::string trim(const std::string &s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static std::string stripQuotes(const std::string &s) {
        if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                              (s.front() == '\'' && s.back() == '\''))) {
            return s.substr(1, s.size() - 2);
        }
        return s;
    }

    /* Looks for a .env file in the current directory and a few parent
     * directories (covers running from project root, from backend/, and
     * from backend/build/, which is where the compiled .exe usually lives). */
    static std::unordered_map<std::string, std::string> loadDotEnv() {
        std::unordered_map<std::string, std::string> values;

        std::vector<std::string> candidates = {
            ".env", "../.env", "../../.env", "../../../.env"
        };
        if (const char *override = std::getenv("MEDIPRIORITY_ENV_FILE")) {
            candidates.insert(candidates.begin(), override);
        }

        for (const auto &path : candidates) {
            std::ifstream file(path);
            if (!file.is_open()) continue;

            std::cout << "[Config] Loading environment from " << path << "\n";
            std::string line;
            while (std::getline(file, line)) {
                std::string trimmed = trim(line);
                if (trimmed.empty() || trimmed[0] == '#') continue;

                size_t eq = trimmed.find('=');
                if (eq == std::string::npos) continue;

                std::string key = trim(trimmed.substr(0, eq));
                std::string value = stripQuotes(trim(trimmed.substr(eq + 1)));
                if (!key.empty()) values[key] = value;
            }
            break; // stop at the first .env file found
        }
        return values;
    }

    static std::string resolve(const std::unordered_map<std::string, std::string> &dotEnv,
                                const char *key, const std::string &fallback) {
        // 1. Real OS environment variable always wins.
        if (const char *value = std::getenv(key)) {
            return std::string(value);
        }
        // 2. Value from the .env file.
        auto it = dotEnv.find(key);
        if (it != dotEnv.end()) {
            return it->second;
        }
        // 3. Built-in default.
        return fallback;
    }

    static Config load() {
        auto dotEnv = loadDotEnv();

        Config c;
        c.dbHost = resolve(dotEnv, "MEDIPRIORITY_DB_HOST", "127.0.0.1");
        c.dbPort = resolve(dotEnv, "MEDIPRIORITY_DB_PORT", "3306");
        c.dbUser = resolve(dotEnv, "MEDIPRIORITY_DB_USER", "root");
        c.dbPassword = resolve(dotEnv, "MEDIPRIORITY_DB_PASSWORD", "");
        c.dbName = resolve(dotEnv, "MEDIPRIORITY_DB_NAME", "medipriority");
        c.apiPort = std::atoi(resolve(dotEnv, "MEDIPRIORITY_API_PORT", "8080").c_str());
        return c;
    }
};

} // namespace medipriority

#endif // MEDIPRIORITY_CONFIG_H
