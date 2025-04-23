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
target_include_directories(kv_core PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
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

# Smoke test for integration
add_test(NAME smoke_test
         COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/smoke_test.sh)
set_tests_properties(smoke_test PROPERTIES LABELS "integration")

# Benchmarks
add_executable(kv_bench benchmarks/lock_free_hash_map_bench.cpp)
target_include_directories(kv_bench PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(kv_bench PRIVATE kv_core benchmark::benchmark_main)

# Optionally enable sanitizers (default ON for CI/dev, OFF for perf builds)
option(ENABLE_SANITIZERS "Enable address/undefined sanitizers" ON)
if (ENABLE_SANITIZERS)
  message(STATUS "Building with sanitizers enabled")
  foreach(target IN ITEMS kv_tests kv_server kv_cli kv_bench)
    target_compile_options(${target} PRIVATE -fsanitize=address,undefined)
    target_link_options   (${target} PRIVATE -fsanitize=address,undefined)
  endforeach()
else()
  message(STATUS "Building WITHOUT sanitizers (perf mode)")
endif()

# Use LLD for faster linking with Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    add_link_options(-fuse-ld=lld)
endif()

# Install rules
install(TARGETS kv_core kv_cli kv_server
        EXPORT kvstoreTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(EXPORT kvstoreTargets
        FILE kvstoreConfig.cmake
        NAMESPACE kv::
        DESTINATION lib/cmake/kvstore)

install(DIRECTORY include/ DESTINATION include)