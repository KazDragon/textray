cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(munin-acceptance)
cmake_policy(VERSION 3.2)

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Telnetpp REQUIRED)
find_package(Terminalpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Munin REQUIRED)

include(FindBoost)
find_package(Boost
    REQUIRED
    COMPONENTS
        program_options
)

find_package(Threads)
        
add_executable(munin_acceptance
    src/application.cpp
    src/camera.cpp
    src/client.cpp
    src/connection.cpp
    src/context_impl.cpp
    src/main.cpp
    src/server.cpp
    src/socket.cpp
)

target_include_directories(munin_acceptance
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
    
target_link_libraries(munin_acceptance
    telnetpp
    terminalpp
    munin
    Boost::program_options
    Threads::Threads
)
    