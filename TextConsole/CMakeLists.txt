cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME ctest)
project(${PROJECT_NAME} VERSION 2.0.0)


add_subdirectory(easyloggingpp)


file(GLOB_RECURSE _LIB_INC "inc/*.h*")
file(GLOB_RECURSE _LIB_SRC "src/*.c*")
file(GLOB_RECURSE _TEST_SRC "test/*")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_LIB_INC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_LIB_SRC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_TEST_SRC})

add_executable(${PROJECT_NAME}
    ${_LIB_INC}
    ${_LIB_SRC}
    ${_TEST_SRC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        LoggerLib
        ${CMAKE_DL_LIBS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/inc"
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
    
    #additional libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE termcap)
endif()
