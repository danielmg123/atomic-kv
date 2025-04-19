cmake_minimum_required(VERSION 3.21)
project(atomic-kv VERSION 0.0.1 LANGUAGES CXX)

# Enable C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies via vcpkg
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

# Core KV library
add_library(kv_core STATIC src/kv_core.cpp)
target_include_directories(kv_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(kv_core PUBLIC fmt::fmt asio::asio spdlog::spdlog)

# CLI tool
add_executable(kv_cli src/cli.cpp)
target_link_libraries(kv_cli PRIVATE kv_core)

# Server
add_executable(kv_server src/server.cpp)
target_link_libraries(kv_server PRIVATE kv_core)

# Tests
enable_testing()
add_executable(kv_tests
  tests/lock_free_hash_map_test.cpp
  tests/lock_free_hash_map_concurrency_test.cpp
)

target_include_directories(kv_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(kv_tests PRIVATE kv_core GTest::gtest)
add_test(NAME all COMMAND kv_tests)

# Benchmarks
add_executable(kv_bench benchmarks/placeholder.cpp)
target_include_directories(kv_bench PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(kv_bench PRIVATE kv_core benchmark::benchmark)

# Enable sanitizers (AddressSanitizer, UndefinedBehaviorSanitizer)
option(ENABLE_SANITIZERS "Enable address/undefined sanitizers" ON)
if (ENABLE_SANITIZERS)
    message(STATUS "Building with sanitizers enabled")
    foreach(target IN ITEMS kv_tests kv_server kv_cli kv_bench)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endforeach()
endif()

# Use LLD for faster linking with Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    add_link_options(-fuse-ld=lld)
endif()