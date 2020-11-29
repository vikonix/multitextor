cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME TestConsole)
project(${PROJECT_NAME})

file(GLOB_RECURSE _TEST_SRC "*")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_TEST_SRC})

add_executable(${PROJECT_NAME}
    ${_TEST_SRC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ThirdPartyLib
        ConsoleLib
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UNICODE
        _UNICODE
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}$(Configuration)"
)

if(MSVC)
    # warning level 4
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    # lots of warnings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
