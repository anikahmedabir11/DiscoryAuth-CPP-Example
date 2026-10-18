#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace DiscoryAuth {

    struct UserData {
        std::string username;
        std::string expires_at;
        std::string hwid;
        std::string license_key;
        std::string session_token;
        bool is_lifetime = false;
    };

    struct AppData {
        std::string name;
        std::string version;
        bool hwid_lock = true;
    };

    class api {
    public:
        std::string name;
        std::string app_id;
        std::string secret;
        std::string version;
        std::string url;

        UserData user_data;
        AppData app_data;
        bool initialized = false;

        api(std::string _name, std::string _app_id, std::string _secret, std::string _version = "1.0", std::string _url = "https://discoryauth.cc/api/v1")
            : name(_name), app_id(_app_id), secret(_secret), version(_version), url(_url) {}

        // Simple JSON string value extractor
        static std::string get_json_val(const std::string& json, const std::string& key) {
            std::string search = "\"" + key + "\":";
            size_t pos = json.find(search);
            if (pos == std::string::npos) return "";
            pos += search.length();
            while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
            if (pos >= json.length()) return "";

            if (json[pos] == '\"') {
                size_t end = json.find('\"', pos + 1);
                if (end != std::string::npos) {
                    return json.substr(pos + 1, end - pos - 1);
                }
            } else {
                size_t end = json.find_first_of(",}\n\r", pos);
                if (end != std::string::npos) {
                    std::string val = json.substr(pos, end - pos);
                    return val;
                }
            }
            return "";
        }

        static bool get_json_bool(const std::string& json, const std::string& key) {
            std::string val = get_json_val(json, key);
            return (val == "true" || val == "1");
        }

        std::string get_hwid() {
            HW_PROFILE_INFO hwProfileInfo;
            if (GetCurrentHwProfile(&hwProfileInfo)) {
                char szHwId[HW_PROFILE_GUIDLEN];
                strcpy_s(szHwId, hwProfileInfo.szHwProfileGuid);
                std::string hwidStr(szHwId);
                // Clean braces
                if (!hwidStr.empty() && hwidStr.front() == '{') hwidStr.erase(0, 1);
                if (!hwidStr.empty() && hwidStr.back() == '}') hwidStr.pop_back();
                return hwidStr;
            }
            return "DEFAULT_DISCORY_HWID";
        }

        std::string http_post(const std::string& endpoint, const std::string& json_payload) {
            std::wstring full_url;
            std::string combined = url + endpoint;
            full_url.assign(combined.begin(), combined.end());

            URL_COMPONENTS urlComp;
            ZeroMemory(&urlComp, sizeof(urlComp));
            urlComp.dwStructSize = sizeof(urlComp);
            urlComp.dwSchemeLength    = (DWORD)-1;
            urlComp.dwHostNameLength  = (DWORD)-1;
            urlComp.dwUrlPathLength   = (DWORD)-1;

            if (!WinHttpCrackUrl(full_url.c_str(), (DWORD)full_url.length(), 0, &urlComp)) {
                return "";
            }

            std::wstring hostname(urlComp.lpszHostName, urlComp.dwHostNameLength);
            std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

            HINTERNET hSession = WinHttpOpen(L"DiscoryAuth-CPP/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) return "";

            HINTERNET hConnect = WinHttpConnect(hSession, hostname.c_str(), urlComp.nPort, 0);
            if (!hConnect) {
                WinHttpCloseHandle(hSession);
                return "";
            }

            DWORD dwFlags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Set SSL flags for security
            if (dwFlags & WINHTTP_FLAG_SECURE) {
                DWORD secFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
                WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &secFlags, sizeof(secFlags));
            }

            std::wstring headers = L"Content-Type: application/json\r\n";
            std::string response;

            if (WinHttpSendRequest(hRequest, headers.c_str(), (DWORD)headers.length(), (LPVOID)json_payload.c_str(), (DWORD)json_payload.length(), (DWORD)json_payload.length(), 0)) {
                if (WinHttpReceiveResponse(hRequest, NULL)) {
                    DWORD dwSize = 0;
                    do {
                        DWORD dwDownloaded = 0;
                        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
                        if (dwSize == 0) break;

                        std::vector<char> buffer(dwSize + 1);
                        if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                            buffer[dwDownloaded] = '\0';
                            response.append(buffer.data(), dwDownloaded);
                        }
                    } while (dwSize > 0);
                }
            }

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return response;
        }

        bool init() {
            std::string hwid = get_hwid();
            std::stringstream ss;
            ss << "{"
               << "\"app_id\":\"" << app_id << "\","
               << "\"app_secret\":\"" << secret << "\","
               << "\"hwid\":\"" << hwid << "\","
               << "\"version\":\"" << version << "\""
               << "}";

            std::string res = http_post("/init", ss.str());
            if (get_json_bool(res, "success")) {
                initialized = true;
                app_data.name = get_json_val(res, "name");
                app_data.version = get_json_val(res, "version");
                return true;
            } else {
                std::string msg = get_json_val(res, "message");
                if (msg.empty()) msg = "Could not reach authentication server.";
                std::cout << "[-] Init Error: " << msg << std::endl;
                return false;
            }
        }

        bool license(const std::string& key) {
            if (!initialized) {
                std::cout << "[-] Please call init() first!" << std::endl;
                return false;
            }

            std::string hwid = get_hwid();
            std::stringstream ss;
            ss << "{"
               << "\"app_id\":\"" << app_id << "\","
               << "\"app_secret\":\"" << secret << "\","
               << "\"license_key\":\"" << key << "\","
               << "\"hwid\":\"" << hwid << "\""
               << "}";

            std::string res = http_post("/license", ss.str());
            if (get_json_bool(res, "success")) {
                user_data.username = get_json_val(res, "username");
                if (user_data.username.empty()) user_data.username = "DiscoryUser";
                user_data.expires_at = get_json_val(res, "expires_at");
                user_data.hwid = hwid;
                user_data.license_key = key;
                user_data.session_token = get_json_val(res, "session_token");
                return true;
            } else {
                std::string msg = get_json_val(res, "message");
                std::cout << "[-] License Error: " << (msg.empty() ? "Invalid License Key." : msg) << std::endl;
                return false;
            }
        }

        bool login(const std::string& username, const std::string& password) {
            if (!initialized) return false;
            std::string hwid = get_hwid();

            std::stringstream ss;
            ss << "{"
               << "\"app_id\":\"" << app_id << "\","
               << "\"app_secret\":\"" << secret << "\","
               << "\"username\":\"" << username << "\","
               << "\"password\":\"" << password << "\","
               << "\"hwid\":\"" << hwid << "\""
               << "}";

            std::string res = http_post("/login", ss.str());
            if (get_json_bool(res, "success")) {
                user_data.username = username;
                user_data.expires_at = get_json_val(res, "expires_at");
                user_data.hwid = hwid;
                user_data.session_token = get_json_val(res, "session_token");
                return true;
            } else {
                std::string msg = get_json_val(res, "message");
                std::cout << "[-] Login Error: " << msg << std::endl;
                return false;
            }
        }

        bool regstr(const std::string& username, const std::string& password, const std::string& key) {
            if (!initialized) return false;
            std::string hwid = get_hwid();

            std::stringstream ss;
            ss << "{"
               << "\"app_id\":\"" << app_id << "\","
               << "\"app_secret\":\"" << secret << "\","
               << "\"username\":\"" << username << "\","
               << "\"password\":\"" << password << "\","
               << "\"license_key\":\"" << key << "\","
               << "\"hwid\":\"" << hwid << "\""
               << "}";

            std::string res = http_post("/register", ss.str());
            if (get_json_bool(res, "success")) {
                return true;
            } else {
                std::string msg = get_json_val(res, "message");
                std::cout << "[-] Registration Error: " << msg << std::endl;
                return false;
            }
        }

        std::string var(const std::string& var_id) {
            std::stringstream ss;
            ss << "{"
               << "\"app_id\":\"" << app_id << "\","
               << "\"app_secret\":\"" << secret << "\","
               << "\"var_id\":\"" << var_id << "\","
               << "\"session_token\":\"" << user_data.session_token << "\""
               << "}";

            std::string res = http_post("/var", ss.str());
            if (get_json_bool(res, "success")) {
                return get_json_val(res, "response");
            }
            return "";
        }

        bool check() {
            if (user_data.session_token.empty()) return false;
            std::stringstream ss;
            ss << "{"
               << "\"app_id\":\"" << app_id << "\","
               << "\"app_secret\":\"" << secret << "\","
               << "\"session_token\":\"" << user_data.session_token << "\""
               << "}";

            std::string res = http_post("/check", ss.str());
            return get_json_bool(res, "success");
        }

        void log(const std::string& message) {
            std::stringstream ss;
            ss << "{"
               << "\"app_id\":\"" << app_id << "\","
               << "\"app_secret\":\"" << secret << "\","
               << "\"message\":\"" << message << "\","
               << "\"pcuser\":\"WindowsUser\""
               << "}";

            http_post("/log", ss.str());
        }
    };
}
