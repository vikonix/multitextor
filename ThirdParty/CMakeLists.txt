cmake_minimum_required(VERSION 3.9)

if(MSVC)
    if(USE_ICONV)
        add_subdirectory(libiconv)
    else()    
        file(GLOB_RECURSE _LIB_WIN_ICONV_INC "win-iconv/iconv.h")
        file(GLOB_RECURSE _LIB_WIN_ICONV_SRC "win-iconv/win_iconv.c")
    endif()    
endif()

set(PROJECT_NAME ThirdPartyLib)
project(${PROJECT_NAME})

file(GLOB_RECURSE _LIB_INC "inc/*.h*")
file(GLOB_RECURSE _LIB_SRC "src/*.c*")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_LIB_INC} ${_LIB_WIN_ICONV_INC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_LIB_SRC} ${_LIB_WIN_ICONV_SRC})

add_library(${PROJECT_NAME} STATIC
    ${_LIB_INC}
    ${_LIB_SRC}
    ${_LIB_WIN_ICONV_INC}    
    ${_LIB_WIN_ICONV_SRC}    
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/inc"
        "${CMAKE_CURRENT_SOURCE_DIR}/win-iconv"
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ELPP_NO_DEFAULT_LOG_FILE
        ELPP_THREAD_SAFE
        UNICODE
        _UNICODE
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    # warning level 4
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    if(USE_ICONV)
        target_link_libraries(${PROJECT_NAME} PUBLIC libiconv)
    endif()    
else()
    # lots of warnings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()