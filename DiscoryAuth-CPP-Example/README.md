# DiscoryAuth-CPP-Example : Please star ⭐

DiscoryAuth C++ example SDK for [https://discoryauth.cc](https://discoryauth.cc) license key & API auth.

This example uses native Windows WinHTTP (`winhttp.lib`) for ultra-fast, lightweight, dependency-free HTTPS communication.

---

### Features
* **Key-Only Authentication**: Login directly with license key without requiring registration.
* **Username & Password Login**: Full user authentication.
* **User Registration**: Register new user credentials linked to a license key.
* **Hardware ID (HWID)**: Automatic machine profile hashing to prevent key sharing.
* **Cloud Variables**: Securely download remote secret strings and configs.
* **Audit Logs**: Send execution and security events to your developer dashboard.

---

### Getting Started

1. **Configure Credentials in `src/main.cpp`:**
   Get your App ID and App Secret from the developer dashboard at [https://discoryauth.cc/app/](https://discoryauth.cc/app/):
   ```cpp
   const std::string APP_NAME    = "My C++ Protected App";
   const std::string APP_ID      = "app_xxxxxxxx";
   const std::string APP_SECRET  = "sec_xxxxxxxxxxxxxxxxxxxx";
   const std::string APP_VERSION = "1.0";
   const std::string API_URL     = "https://discoryauth.cc/api/v1";
   ```

2. **Compiling with Visual Studio:**
   - Create a new C++ Console Application in Visual Studio.
   - Add `src/discoryauth.hpp` and `src/main.cpp` to your project.
   - Build as **Release x64** or **Release x86**.

3. **Compiling with CMake:**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

4. **Compiling with MinGW / g++:**
   ```bash
   g++ -O2 src/main.cpp -o DiscoryAuth_Example.exe -lwinhttp
   ```

---

### License
MIT License.
