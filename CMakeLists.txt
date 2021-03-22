cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME Multitextor)
project(${PROJECT_NAME} VERSION 2.0.0)

#option(VLD "VLD" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/_build/bin/")
if(MSVC)
    add_link_options("/INCREMENTAL:NO")
    add_definitions(-DLIBICONV_DLL)
endif()

add_subdirectory(ThirdParty)
add_subdirectory(Utils)
add_subdirectory(Utils/test)
add_subdirectory(Console)
add_subdirectory(Console/test)
add_subdirectory(WndManager)
add_subdirectory(WndManager/test)
add_subdirectory(Editor)
