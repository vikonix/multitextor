cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME ConsoleLib)
project(${PROJECT_NAME})


file(GLOB_RECURSE _LIB_INC "inc/*.h*")
file(GLOB_RECURSE _LIB_SRC "src/*.c*")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_LIB_INC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_LIB_SRC})

add_library(${PROJECT_NAME} STATIC
    ${_LIB_INC}
    ${_LIB_SRC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ThirdPartyLib
        UtilsLib
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
        NOMINMAX
)

target_compile_features(${PROJECT_NAME} 
    PUBLIC 
        cxx_std_17
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    # warning level 4
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /Zc:__cplusplus)
else()
    # lots of warnings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
    
    #additional libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE termcap)
endif()
