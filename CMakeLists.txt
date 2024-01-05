cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(textray VERSION 0.0.1)

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost 1.69.0 REQUIRED COMPONENTS container program_options)
find_package(serverpp 0.2.0 REQUIRED)
find_package(telnetpp 3.0.0 REQUIRED)
find_package(terminalpp 3.0.0 REQUIRED)
# This is only required if munin was built with MUNIN_WITH_CONSOLEPP=ON.
find_package(consolepp 0.1.1)
find_package(munin 0.7.1 REQUIRED)
find_package(gsl-lite 0.38.0 REQUIRED)
find_package(fmt 5.3 REQUIRED)
find_package(nlohmann_json 3.3.0 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

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
 
target_link_libraries(textray 
    PRIVATE
        KazDragon::serverpp
        KazDragon::telnetpp
        KazDragon::terminalpp
        KazDragon::munin
        Boost::boost
        Boost::program_options
        ${CMAKE_THREAD_LIBS_INIT}
)
