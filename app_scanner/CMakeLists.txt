cmake_minimum_required(VERSION 3.15)
project(Scanner VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(../library_scanner scanner_library)


set_target_properties(scanner_library PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_executable(scanner main.cpp)


target_link_libraries(scanner PRIVATE scanner_library)
find_package(OpenSSL REQUIRED)
target_link_libraries(scanner PRIVATE OpenSSL::Crypto)



target_include_directories(scanner PRIVATE
    ${CMAKE_SOURCE_DIR}/library_scanner/include
)
