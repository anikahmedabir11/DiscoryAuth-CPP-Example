#include <iostream>
#include <string>
#include "discoryauth.hpp"

// ==============================================================================
// DISCORyAUTH CONFIGURATION (Obtain from https://discoryauth.cc/app/)
// ==============================================================================
const std::string APP_NAME    = "My C++ Protected App";
const std::string APP_ID      = "app_xxxxxxxx";             // Replace with your App ID
const std::string APP_SECRET  = "sec_xxxxxxxxxxxxxxxxxxxx"; // Replace with your App Secret
const std::string APP_VERSION = "1.0";
const std::string API_URL     = "https://discoryauth.cc/api/v1";

void print_banner() {
    std::cout << R"(
  ____  _                               _         _   _     
 |  _ \(_)___  ___ ___  _ __ _   _     / \  _   _| |_| |__  
 | | | | / __|/ __/ _ \| '__| | | |   / _ \| | | | __| '_ \ 
 | |_| | \__ \ (_| (_) | |  | |_| |  / ___ \ |_| | |_| | | |
 |____/|_|___/\___\___/|_|   \__, | /_/   \_\__,_|\__|_| |_|
                             |___/                          
    :: Official C++ Example | https://discoryauth.cc ::
)" << std::endl;
}

int main() {
    SetConsoleTitleA("DiscoryAuth C++ Example | https://discoryauth.cc");
    print_banner();

    DiscoryAuth::api auth(APP_NAME, APP_ID, APP_SECRET, APP_VERSION, API_URL);

    std::cout << "[*] Connecting to DiscoryAuth Network..." << std::endl;
    if (!auth.init()) {
        std::cout << "[-] Failed to initialize application. Check credentials or connection." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "[+] Initialized successfully!" << std::endl;
    std::cout << "[+] Hardware ID: " << auth.get_hwid() << "\n" << std::endl;

    std::cout << "Select an Option:" << std::endl;
    std::cout << " [1] Login" << std::endl;
    std::cout << " [2] Register" << std::endl;
    std::cout << " [3] License Key Only" << std::endl;
    std::cout << " [4] Exit" << std::endl;

    std::cout << "\nChoice > ";
    int choice = 0;
    std::cin >> choice;

    if (choice == 1) {
        std::string username, password;
        std::cout << "Username: ";
        std::cin >> username;
        std::cout << "Password: ";
        std::cin >> password;

        std::cout << "[*] Logging in..." << std::endl;
        if (!auth.login(username, password)) {
            std::cin.ignore();
            std::cin.get();
            return 1;
        }
        std::cout << "[+] Logged in as: " << auth.user_data.username << std::endl;

    } else if (choice == 2) {
        std::string username, password, key;
        std::cout << "Enter Username: ";
        std::cin >> username;
        std::cout << "Enter Password: ";
        std::cin >> password;
        std::cout << "Enter License Key: ";
        std::cin >> key;

        std::cout << "[*] Registering account..." << std::endl;
        if (!auth.regstr(username, password, key)) {
            std::cin.ignore();
            std::cin.get();
            return 1;
        }
        std::cout << "[+] Registered successfully! You can now log in." << std::endl;
        std::cin.ignore();
        std::cin.get();
        return 0;

    } else if (choice == 3) {
        std::string key;
        std::cout << "Enter License Key: ";
        std::cin >> key;

        std::cout << "[*] Validating License Key..." << std::endl;
        if (!auth.license(key)) {
            std::cin.ignore();
            std::cin.get();
            return 1;
        }
        std::cout << "[+] License Accepted! Welcome " << auth.user_data.username << "!" << std::endl;

    } else {
        return 0;
    }

    // --------------------------------------------------------------------------
    // AUTHENTICATED USER SESSION
    // --------------------------------------------------------------------------
    std::cout << "\n==================================================" << std::endl;
    std::cout << "           USER AUTHENTICATED PROFILE             " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << " Username    : " << auth.user_data.username << std::endl;
    std::cout << " Expires At  : " << auth.user_data.expires_at << std::endl;
    std::cout << " Hardware ID : " << auth.user_data.hwid << std::endl;
    std::cout << "==================================================" << std::endl;

    // Fetch cloud variable
    std::cout << "\n[*] Fetching remote variable 'secret_data'..." << std::endl;
    std::string secretVar = auth.var("secret_data");
    if (!secretVar.empty()) {
        std::cout << "[+] Variable Value: " << secretVar << std::endl;
    } else {
        std::cout << "[-] Variable empty or not configured." << std::endl;
    }

    auth.log("C++ user authenticated payload successfully.");
    std::cout << "[+] Session Heartbeat: Active!" << std::endl;
    std::cout << "[*] Running protected software features...\n" << std::endl;

    std::cout << "Press Enter to exit application..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    return 0;
}
