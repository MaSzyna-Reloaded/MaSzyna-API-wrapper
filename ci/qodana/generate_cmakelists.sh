#!/bin/bash
echo "cmake_minimum_required(VERSION 3.25)
      project(maszyna)
      option(ARCHITECTURE \"x64 or x86 CPU Architecture\" \"x64\")

      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \"\${CMAKE_CURRENT_SOURCE_DIR}/\${CMAKE_BUILD_TYPE}/\${ARCHITECTURE}\")
      set(CMAKE_CXX_STANDARD 17)
      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY \${CMAKE_BINARY_DIR}/lib/\${CMAKE_GENERATOR_PLATFORM})
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY \${CMAKE_BINARY_DIR}/bin/\${CMAKE_GENERATOR_PLATFORM})
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \${CMAKE_BINARY_DIR}/bin/\${CMAKE_GENERATOR_PLATFORM})
      set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
      option(CODECHECK_BUILD \"If the build is for Code Check\" OFF)

      if (NOT CODECHECK_BUILD) #Qodana will run inspection on godot-cpp too. We don't need that
          message(\"Not building for Code Check\")
          add_subdirectory(godot-cpp)
          message(\"Looking for sources in \${CMAKE_CURRENT_SOURCE_DIR}\")
          FILE(GLOB children RELATIVE_PATH src \${CMAKE_CURRENT_SOURCE_DIR}/src/*)
          STRING(LENGTH \${CMAKE_CURRENT_SOURCE_DIR} PATH_PREFIX_LEN)

          FOREACH(DIR \${children})
              IF(IS_DIRECTORY \${DIR})
                  LIST(APPEND dirlist \${DIR})
                  include_directories(\${DIR})
                  AUX_SOURCE_DIRECTORY(\${DIR} SRCES)
              ENDIF()
          ENDFOREACH()

          message(\"\${dirlist} found...\")
          add_library(maszyna SHARED \${SRCES})
          target_link_libraries(maszyna PUBLIC godot-cpp)
      else ()
          message(\"Building for Code Check\")
          message(\"Looking for sources in \${CMAKE_CURRENT_SOURCE_DIR}\")
          FILE(GLOB children RELATIVE_PATH src \${CMAKE_CURRENT_SOURCE_DIR}/src/*)
          STRING(LENGTH \${CMAKE_CURRENT_SOURCE_DIR} PATH_PREFIX_LEN)

          FOREACH(DIR \${children})
              IF(IS_DIRECTORY \${DIR} AND NOT \${DIR} MATCHES \"maszyna$\")
                  LIST(APPEND dirlist \${DIR})
                  include_directories(\${DIR})
                  AUX_SOURCE_DIRECTORY(\${DIR} SRCES)
              ENDIF()
          ENDFOREACH()

          message(\"\${dirlist} found...\")
          add_library(maszyna SHARED \${SRCES})
          target_link_libraries(maszyna PUBLIC godot-cpp)
      endif()" > ../../CMakeLists.txt