cmake_minimum_required(VERSION 3.21)
project(atomic-kv VERSION 0.0.1 LANGUAGES CXX)

# Enable C++23 and other good stuff
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find packages installed by vcpkg
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

# Core library
add_library(kv_core STATIC src/kv_core.cpp)
target_link_libraries(kv_core PUBLIC fmt::fmt asio::asio spdlog::spdlog)

# CLI
add_executable(kv_cli src/cli.cpp)
target_link_libraries(kv_cli PRIVATE kv_core)

# Server
add_executable(kv_server src/server.cpp)
target_link_libraries(kv_server PRIVATE kv_core)

# Tests
enable_testing()
add_executable(kv_tests tests/placeholder.cpp)
target_link_libraries(kv_tests PRIVATE kv_core GTest::gtest_main)
add_test(NAME all COMMAND kv_tests)

# Benchmark
add_executable(kv_bench benchmarks/placeholder.cpp)
target_link_libraries(kv_bench PRIVATE kv_core benchmark::benchmark_main)

# Enable sanitizers (debugging)
option(ENABLE_SANITIZERS "Enable address/undefined sanitizers" ON)

if(ENABLE_SANITIZERS)
  message(STATUS "Building with sanitizers")
  foreach(target IN ITEMS kv_tests kv_server kv_cli kv_bench)
    target_compile_options(${target} PRIVATE -fsanitize=address,undefined)
    target_link_options(${target} PRIVATE -fsanitize=address,undefined)
  endforeach()
endif()

# Use lld if using Clang (faster linking)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
  add_link_options(-fuse-ld=lld)
endif()