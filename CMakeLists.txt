cmake_minimum_required(VERSION 3.15)
project(HostelManagementSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(include)

# Common core source files
set(CORE_SOURCES
    src/core/ActionCommand.cpp
    src/core/HostelSystem.cpp
    src/core/AuthManager.cpp
    src/core/DataGenerator.cpp
    src/core/BinaryStorage.cpp
)

# Main Executable (Dual Console & Web Server)
add_executable(hostel_app
    ${CORE_SOURCES}
    src/server/HttpServer.cpp
    src/main.cpp
)

# Link Windows Socket and Crypto libraries on Windows
if(WIN32)
    target_link_libraries(hostel_app PRIVATE ws2_32 crypt32)
endif()

# Phase 1: DFD Core Tests
add_executable(test_dfd_core
    src/core/ActionCommand.cpp
    src/core/HostelSystem.cpp
    tests/test_dfd_core.cpp
)

# Phase 2: Advanced DSA Tests
add_executable(test_advanced_dsa
    tests/test_advanced_dsa.cpp
)

enable_testing()
add_test(NAME DFDCustomCoreTest COMMAND test_dfd_core)
add_test(NAME AdvancedDSATest COMMAND test_advanced_dsa)
