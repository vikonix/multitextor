cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME libiconv)
project(${PROJECT_NAME})

file(GLOB_RECURSE _LIB_INC "include/*.h*")
file(GLOB_RECURSE _LIB_LIB_INC "lib/*.h*")
file(GLOB_RECURSE _LIB_SRCLIB_INC "srclib/*.h*")
file(GLOB_RECURSE _LIB_SRC "lib/*.c*")
file(GLOB_RECURSE _LIBCHARSET_SRC "libcharset/*.c*")
file(GLOB_RECURSE _LIB_RC "win32/libiconv.rc")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${_LIB_INC} ${_LIB_LIB_INC} ${_LIB_SRCLIB_INC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${_LIB_SRC} ${_LIBCHARSET_SRC} ${_LIB_RC})

add_library(${PROJECT_NAME} SHARED
    ${_LIB_INC}
    ${_LIB_LIB_INC}
    ${_LIB_SRCLIB_INC}
    ${_LIB_SRC}
    ${_LIBCHARSET_SRC}
    ${_LIB_RC}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/srclib"
        "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UNICODE
        _UNICODE
        LIBICONV_DLL
        BUILDING_LIBICONV
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    # warning level 4
    #target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    # lots of warnings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()