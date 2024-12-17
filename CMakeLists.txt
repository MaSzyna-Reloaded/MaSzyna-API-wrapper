cmake_minimum_required(VERSION 3.25)
project(maszyna)
option(ARCHITECTURE "x64 or x86 CPU Architecture" "x64")

#TODO: Set CMAKE_SYSTEM_NAME properly according to build target
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/${ARCHITECTURE}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_GENERATOR_PLATFORM})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_GENERATOR_PLATFORM})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(QODANA_BUILD "If the build is for Qodana CI/CD" OFF)

AUX_SOURCE_DIRECTORY(src SRCES)
if (NOT QODANA_BUILD) #Qodana will run inspection on godot-cpp too. We don't need that
    message("Not building for Qodana")
    add_subdirectory(godot-cpp)
else ()
    message("Building for Qodana")
endif()
include_directories(src)

add_library(maszyna SHARED ${SRCES})
target_link_libraries(maszyna PUBLIC godot-cpp)