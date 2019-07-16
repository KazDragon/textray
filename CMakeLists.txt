cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(textray)
cmake_policy(VERSION 3.2)

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

# Include the Conan recipe in order to get our dependencies
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

set(CMAKE_CXX_STANDARD 14)

add_executable(textray
    src/application.cpp
    src/camera.cpp
    src/client.cpp
    src/connection.cpp
    src/main.cpp
    src/ui.cpp
)

target_include_directories(textray
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
    
target_link_libraries(textray
    CONAN_PKG::serverpp
    CONAN_PKG::telnetpp
    CONAN_PKG::terminalpp
    CONAN_PKG::munin
    CONAN_PKG::boost_program_options
    CONAN_PKG::boost_format
)
    