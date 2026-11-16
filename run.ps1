# One-Click PowerShell Runner for Hostel Management System
param (
    [switch]$Console,
    [switch]$Test
)

Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host "   HOSTEL MANAGEMENT SYSTEM - ONE-CLICK RUNNER" -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan

if (Test-Path "C:\msys64\ucrt64\bin") {
    $env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
}

if ($Test) {
    Write-Host "[1/2] Running DFD Core Tests..." -ForegroundColor Yellow
    & g++ -std=c++17 -Iinclude src/core/ActionCommand.cpp src/core/HostelSystem.cpp tests/test_dfd_core.cpp -o test_dfd_core.exe
    & .\test_dfd_core.exe

    Write-Host "`n[2/2] Running Advanced DSA Tests..." -ForegroundColor Yellow
    & g++ -std=c++17 -Iinclude tests/test_advanced_dsa.cpp -o test_advanced_dsa.exe
    & .\test_advanced_dsa.exe
    exit
}

if ($Console) {
    Write-Host "Starting Interactive Console Mode..." -ForegroundColor Green
    & .\hostel_app.exe --console
    exit
}

Write-Host "[1/2] Compiling Hostel Management System (Web Mode)..." -ForegroundColor Yellow
& g++ -std=c++17 -O2 -Iinclude src/core/ActionCommand.cpp src/core/HostelSystem.cpp src/core/AuthManager.cpp src/core/DataGenerator.cpp src/core/BinaryStorage.cpp src/server/HttpServer.cpp src/main.cpp -o hostel_app.exe -lws2_32 -lcrypt32

if ($LASTEXITCODE -eq 0) {
    Write-Host "[2/2] Starting Web Server on http://localhost:8080 ..." -ForegroundColor Green
    Write-Host "Default Admin credentials: admin / Admin@123" -ForegroundColor Cyan
    Start-Process "http://localhost:8080"
    & .\hostel_app.exe
} else {
    Write-Host "[ERROR] Build failed." -ForegroundColor Red
}
