@echo off
setlocal
echo =======================================================
echo   HOSTEL MANAGEMENT SYSTEM - ONE-CLICK RUNNER
echo =======================================================

rem Set path to modern compiler if MSYS2 exists
if exist "C:\msys64\ucrt64\bin" (
    set "PATH=C:\msys64\ucrt64\bin;%PATH%"
)

if "%1"=="--console" goto run_console
if "%1"=="-c" goto run_console
if "%1"=="--test" goto run_tests
if "%1"=="-t" goto run_tests

echo [1/2] Building Hostel Management System (Web Mode)...
g++ -std=c++17 -O2 -Iinclude src/core/ActionCommand.cpp src/core/HostelSystem.cpp src/core/AuthManager.cpp src/core/DataGenerator.cpp src/core/BinaryStorage.cpp src/server/HttpServer.cpp src/main.cpp -o hostel_app.exe -lws2_32 -lcrypt32
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Compilation failed!
    exit /b %ERRORLEVEL%
)

echo [2/2] Starting Web Server on http://localhost:8080 ...
echo Default Admin credentials: admin / Admin@123
start http://localhost:8080
hostel_app.exe
goto end

:run_console
echo Starting Interactive Console Mode...
hostel_app.exe --console
goto end

:run_tests
echo Running Comprehensive Unit Tests...
g++ -std=c++17 -Iinclude src/core/ActionCommand.cpp src/core/HostelSystem.cpp tests/test_dfd_core.cpp -o test_dfd_core.exe
test_dfd_core.exe
g++ -std=c++17 -Iinclude tests/test_advanced_dsa.cpp -o test_advanced_dsa.exe
test_advanced_dsa.exe
goto end

:end
endlocal
