cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME Multitextor)
project(${PROJECT_NAME} VERSION 2.0.0)

#option(VLD "VLD" ON)

add_subdirectory(easyloggingpp)
add_subdirectory(Console)
add_subdirectory(Console/test)
add_subdirectory(WndManager)
add_subdirectory(WndManager/test)
