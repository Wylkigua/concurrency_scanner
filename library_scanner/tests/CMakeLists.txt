cmake_minimum_required(VERSION 3.15)
project(scanner_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(OpenSSL REQUIRED)

add_executable(scanner_tests
    main.cpp
    ../src/scanner_core.cpp
    ../src/md_hash.cpp
    ../src/concurrency.cpp
    ../src/utils.cpp
)

target_include_directories(scanner_tests
    PRIVATE
    ../src/include
    ${gtest_SOURCE_DIR}/include
)

target_link_libraries(scanner_tests
    PRIVATE
    gtest_main
    OpenSSL::Crypto
)

enable_testing()
