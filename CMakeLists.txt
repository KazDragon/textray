cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(textray VERSION 0.0.1)
cmake_policy(VERSION 3.2)

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    message("Using Conan due to conanbuildinfo.cmake")
    set(TEXTRAY_USE_CONAN ON)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (TEXTRAY_USE_CONAN)
    set(TEXTRAY_LIBRARIES
        CONAN_PKG::serverpp
        CONAN_PKG::telnetpp
        CONAN_PKG::terminalpp
        CONAN_PKG::munin
        CONAN_PKG::boost
    )
else()
    find_package(Boost 1.69.0 REQUIRED COMPONENTS container program_options)
    find_package(serverpp 0.2.0 REQUIRED)
    find_package(telnetpp 2.2.0 REQUIRED)
    find_package(terminalpp 2.4.0 REQUIRED)
    find_package(consolepp 0.1.0 REQUIRED)
    find_package(munin 0.7.0 REQUIRED)
    find_package(gsl-lite 0.38.0 REQUIRED)
    find_package(fmt 5.3 REQUIRED)
    find_package(nlohmann_json 3.3.0 REQUIRED)
    find_package(ZLIB REQUIRED)
    find_package(Threads REQUIRED)

    set (TEXTRAY_LIBRARIES
        KazDragon::serverpp
        KazDragon::telnetpp
        KazDragon::terminalpp
        KazDragon::munin
        Boost::boost
        Boost::program_options
        ${CMAKE_THREAD_LIBS_INIT}
    )
endif()

add_executable(textray src/main.cpp)

target_sources(textray
    PRIVATE
        src/application.cpp
        src/camera.cpp
        src/client.cpp
        src/connection.cpp
        src/ui.cpp
)

target_include_directories(textray
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
 
target_link_libraries(textray ${TEXTRAY_LIBRARIES})
