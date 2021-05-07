cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Multitextor)

if(MSVC)
    add_link_options("/INCREMENTAL:NO")
    # Set MSVC runtime to MultiThreaded (/MT)
    cmake_policy(SET CMP0091 NEW)
endif()

project(${PROJECT_NAME} VERSION 2.0.0)

#option(USE_VLD "VLD" ON)
#option(USE_ICONV "LIBICONV" ON)
#option(BUILD_TEST "BUILD_TEST" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/_build/bin/")

add_subdirectory(ThirdParty)
add_subdirectory(Utils)
add_subdirectory(Console)
add_subdirectory(WndManager)
add_subdirectory(Editor)

if(BUILD_TEST)
    add_subdirectory(Utils/test)
    add_subdirectory(Console/test)
    add_subdirectory(WndManager/test)
endif()
