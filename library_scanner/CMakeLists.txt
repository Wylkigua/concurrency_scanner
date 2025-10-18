cmake_minimum_required(VERSION 3.15)
project(Library_scanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

add_library(scanner_library SHARED
    src/scanner_core.cpp
    src/scanner.cpp
    src/md_hash.cpp
    src/concurrency.cpp
    src/utils.cpp
)

target_include_directories(
    scanner_library
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include # публичный интерфейс
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include # внутренние заголовки
)

if(WIN32)
    target_compile_definitions(scanner_library PRIVATE LIB_SCANNER_EXPORTS)
endif()

target_link_libraries(scanner_library PRIVATE OpenSSL::Crypto)

# Output для вывода бинарников
set_target_properties(scanner_library PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
