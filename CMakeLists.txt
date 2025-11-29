CMAKE_MINIMUM_REQUIRED(VERSION 3.17)
SET(CMAKE_CXX_STANDARD 17)
# Do not hard‑force a generator; allow users to select one suitable for cross‑compiling.
# Silence unused variable warning when specified from toolchain
IF (CMAKE_C_COMPILER)
ENDIF ()
SET(CMAKE_GENERATOR Ninja)
# Allow selecting platform via -DGODOTCPP_PLATFORM=android|windows|linux|macos|ios|web
IF (DEFINED GODOTCPP_PLATFORM AND NOT CMAKE_SYSTEM_NAME)
    STRING(TOLOWER "${GODOTCPP_PLATFORM}" _req_platform)
    IF (_req_platform STREQUAL "android")
        # Prefer the official Android NDK toolchain if not explicitly set
        # Allow manual override via ANDROID_NDK_TOOLCHAIN (full path) or ANDROID_NDK_ROOT (NDK dir)
        IF (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
            # 1) Explicit toolchain file provided by user
            IF (DEFINED ANDROID_NDK_TOOLCHAIN)
                SET(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK_TOOLCHAIN}" CACHE FILEPATH "Android NDK toolchain (manual)" FORCE)
            # 2) NDK root directory provided by user
            ELSEIF (DEFINED ANDROID_NDK_ROOT)
                SET(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake" CACHE FILEPATH "Android NDK toolchain (from ANDROID_NDK_ROOT)" FORCE)
            # 3) Environment variables commonly used
            ELSEIF (DEFINED ENV{ANDROID_NDK})
                SET(CMAKE_TOOLCHAIN_FILE "$ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake" CACHE FILEPATH "Android NDK toolchain (from ANDROID_NDK)" FORCE)
            ELSEIF (DEFINED ENV{ANDROID_NDK_HOME})
                SET(CMAKE_TOOLCHAIN_FILE "$ENV{ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake" CACHE FILEPATH "Android NDK toolchain (from ANDROID_NDK_HOME)" FORCE)
            ENDIF ()
        ENDIF ()

        # Tell CMake we are targeting Android when using the NDK toolchain
        SET(CMAKE_SYSTEM_NAME Android CACHE STRING "" FORCE)

        # Provide reasonable defaults if not supplied by user
        IF (NOT DEFINED ANDROID_ABI)
            SET(ANDROID_ABI arm64-v8a CACHE STRING "Android ABI")
        ENDIF ()
        IF (NOT DEFINED ANDROID_PLATFORM)
            SET(ANDROID_PLATFORM 34 CACHE STRING "Android API level")
        ENDIF ()

        IF (DEFINED CMAKE_TOOLCHAIN_FILE)
            MESSAGE(STATUS "Configuring cross‑compile for Android (ABI=${ANDROID_ABI}, API=${ANDROID_PLATFORM}) using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
        ELSE ()
            MESSAGE(STATUS "Configuring cross‑compile for Android (ABI=${ANDROID_ABI}, API=${ANDROID_PLATFORM}).")
        ENDIF ()
    ELSEIF (_req_platform STREQUAL "windows")
        # Windows cross‑compile via MinGW (recommended from non‑Windows hosts)
        IF (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
            # Best effort: if cross compilers are present, set them. This must be before project().
            FIND_PROGRAM(MINGW_C "x86_64-w64-mingw32-gcc")
            FIND_PROGRAM(MINGW_CXX "x86_64-w64-mingw32-g++")
            IF (MINGW_C AND MINGW_CXX)
                SET(CMAKE_SYSTEM_NAME Windows CACHE STRING "" FORCE)
                SET(CMAKE_C_COMPILER   ${MINGW_C}  CACHE FILEPATH "" FORCE)
                SET(CMAKE_CXX_COMPILER ${MINGW_CXX} CACHE FILEPATH "" FORCE)
                MESSAGE(STATUS "Configuring cross‑compile for Windows using MinGW (x86_64).")
            ELSE()
                # Fallback: still set system name; user must provide a toolchain or compilers.
                SET(CMAKE_SYSTEM_NAME Windows CACHE STRING "" FORCE)
                MESSAGE(WARNING "GODOTCPP_PLATFORM=windows set but no MinGW cross compiler auto‑detected. Provide a toolchain file or set CMAKE_C_COMPILER/CMAKE_CXX_COMPILER.")
            ENDIF ()
        ELSE()
            SET(CMAKE_SYSTEM_NAME Windows CACHE STRING "" FORCE)
            MESSAGE(STATUS "Configuring cross‑compile for Windows using specified toolchain: ${CMAKE_TOOLCHAIN_FILE}")
        ENDIF ()
    ENDIF ()
ENDIF ()

SET(LIBNAME "libmaszyna" CACHE STRING "The name of the library")
SET(GODOT_PROJECT_DIR "demo" CACHE STRING "The directory of a Godot project folder")
SET(IS_DEBUG_BUILD)

FIND_PACKAGE(Python3 3.4 REQUIRED)
ADD_SUBDIRECTORY(godot-cpp SYSTEM)

# Add godot-cpp's module path and include the exported functions.
# This is made available for documentation generation
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${godot-cpp_SOURCE_DIR}/cmake")
INCLUDE(GodotCPPModule)

# The godot-cpp target has some of useful properties attached that can be retrieved like so.
GET_TARGET_PROPERTY(GODOTCPP_SUFFIX godot::cpp GODOTCPP_SUFFIX)
GET_TARGET_PROPERTY(GODOTCPP_PLATFORM godot::cpp GODOTCPP_PLATFORM)

PROJECT(Maszyna-API-Wrapper
        VERSION 1.0
        DESCRIPTION "MaSzyna API wrapper for Godot Engine written in C++ as GDExtension"
        HOMEPAGE_URL "https://github.com/MaSzyna-Reloaded/MaSzyna-API-wrapper"
        LANGUAGES CXX
)

FILE(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp")

ADD_LIBRARY(${LIBNAME} SHARED ${SRC_FILES})
TARGET_INCLUDE_DIRECTORIES(${LIBNAME} PRIVATE src)
TARGET_SOURCES(${LIBNAME} PRIVATE)

FILE(GLOB_RECURSE DOC_XML LIST_DIRECTORIES NO CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/doc_classes/*.xml")

IF (DOC_XML)
    IF (GODOTCPP_TARGET MATCHES "editor|template_debug")
        TARGET_DOC_SOURCES(${LIBNAME} ${DOC_XML})
    ENDIF ()
ENDIF ()

TARGET_LINK_LIBRARIES(${LIBNAME} godot-cpp)

SET_TARGET_PROPERTIES(${LIBNAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "$<1:${PROJECT_SOURCE_DIR}/demo/bin/libmaszyna/${GODOTCPP_PLATFORM}>"
        PREFIX ""
        OUTPUT_NAME "${LIBNAME}"
)

# Determine architecture suffix for output filename
SET(ARCH_SUFFIX "")
IF (APPLE)
    SET(ARCH_SUFFIX "")
ELSE ()
    # For Android, prefer ANDROID_ABI when available
    IF (DEFINED ANDROID_ABI)
        STRING(TOLOWER "${ANDROID_ABI}" _proc_lc)
        IF (_proc_lc MATCHES "^arm64\-v8a$")
            SET(_proc_lc "arm64")
        ELSEIF (_proc_lc MATCHES "^armeabi\-v7a$")
            SET(_proc_lc "arm32")
        ELSEIF (_proc_lc STREQUAL "x86_64")
            SET(_proc_lc "x86_64")
        ELSEIF (_proc_lc STREQUAL "x86")
            SET(_proc_lc "x86")
        ENDIF ()
    ELSE ()
        STRING(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _proc_lc)
    ENDIF ()
    IF (_proc_lc MATCHES "^(x86_64|amd64)$")
        SET(ARCH_SUFFIX ".64")
    ELSEIF (_proc_lc MATCHES "^(aarch64|arm64)$")
        SET(ARCH_SUFFIX ".arm64")
    ELSEIF (_proc_lc MATCHES "^(riscv64|rv64)$")
        SET(ARCH_SUFFIX ".rv64")
    ELSEIF (_proc_lc MATCHES "^arm(32)?$")
        SET(ARCH_SUFFIX ".arm32")
    ELSEIF (_proc_lc MATCHES "^x86$")
        SET(ARCH_SUFFIX ".32")
    ELSE ()
        # Fallback: use pointer size to detect generic 64-bit x86 when processor string is unknown
        IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
            SET(ARCH_SUFFIX ".64")
        ENDIF ()
    ENDIF ()
ENDIF ()

# Compose final output name using GODOTCPP_TARGET for debug/release and architecture suffix
# GODOTCPP_TARGET values: template_debug | template_release
SET(DEBUG_SUFFIX "")
IF (GODOTCPP_TARGET STREQUAL "template_debug")
    SET(DEBUG_SUFFIX ".debug")
ENDIF ()

SET(_OUTPUT_NAME "${LIBNAME}${DEBUG_SUFFIX}${ARCH_SUFFIX}")
SET(GODOT_PROJECT_BINARY_DIR "${PROJECT_SOURCE_DIR}/${GODOT_PROJECT_DIR}/bin/libmaszyna/${GODOTCPP_PLATFORM}")

SET_TARGET_PROPERTIES(${LIBNAME} PROPERTIES OUTPUT_NAME "${_OUTPUT_NAME}" LIBRARY_OUTPUT_DIRECTORY "${GODOT_PROJECT_BINARY_DIR}")

SET(ADDONS_DEMO_DIR "${PROJECT_SOURCE_DIR}/${GODOT_PROJECT_DIR}/addons")
ADD_CUSTOM_COMMAND(TARGET ${LIBNAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ADDONS_DEMO_DIR}"
)

SET(SRC_ADDON_LIBMASZYNA "${PROJECT_SOURCE_DIR}/addons/libmaszyna")
SET(DST_ADDON_LIBMASZYNA "${ADDONS_DEMO_DIR}/libmaszyna")
MESSAGE(NOTICE "Copying/linking addons and submodules to demo/addons...")
IF (UNIX)
    # On UNIX: prefer symlinks; if destination already a symlink, do nothing.
    IF (NOT IS_SYMLINK "${DST_ADDON_LIBMASZYNA}")
        MESSAGE(NOTICE "No symlinks detected, linking...")
        ADD_CUSTOM_COMMAND(TARGET ${LIBNAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E rm -rf "${DST_ADDON_LIBMASZYNA}"
                COMMAND ${CMAKE_COMMAND} -E create_symlink "${SRC_ADDON_LIBMASZYNA}" "${DST_ADDON_LIBMASZYNA}"
        )
    ENDIF ()
else ()
    # Non-UNIX: copy directory; if destination is a symlink, omit copying.
    IF (NOT IS_SYMLINK "${DST_ADDON_LIBMASZYNA}")
        ADD_CUSTOM_COMMAND(TARGET ${LIBNAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E rm -rf "${DST_ADDON_LIBMASZYNA}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${SRC_ADDON_LIBMASZYNA}" "${DST_ADDON_LIBMASZYNA}"
        )
    ENDIF ()
ENDIF ()

SET(SRC_ADDON_GUT "${PROJECT_SOURCE_DIR}/vendor/gut/addons/gut")
SET(DST_ADDON_GUT "${ADDONS_DEMO_DIR}/gut")
IF (UNIX)
    # On UNIX: prefer symlinks; if destination already a symlink, do nothing.
    IF (NOT IS_SYMLINK "${DST_ADDON_GUT}")
        MESSAGE(NOTICE "No symlinks detected, linking...")
        ADD_CUSTOM_COMMAND(TARGET ${LIBNAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E rm -rf "${DST_ADDON_GUT}"
                COMMAND ${CMAKE_COMMAND} -E create_symlink "${SRC_ADDON_GUT}" "${DST_ADDON_GUT}"
        )
    ENDIF ()
else ()
    # Non-UNIX: copy directory; if destination is a symlink, omit copying.
    IF (NOT IS_SYMLINK "${DST_ADDON_GUT}")
        ADD_CUSTOM_COMMAND(TARGET ${LIBNAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E rm -rf "${DST_ADDON_GUT}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${SRC_ADDON_GUT}" "${DST_ADDON_GUT}"
        )
    ENDIF ()
ENDIF ()
