cmake_minimum_required(VERSION 3.15)

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
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")    
else()
    # lots of warnings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)

    #additional libraries
    # Use CURSES_NEED_NCURSES to enable termcap compatibility API in ncurses
    # (needed for tgoto() from <term.h>). Some Linux distributions don't provide
    # it by default as part of -lcurses / -lncurses:
    #    https://bugs.gentoo.org/836155
    #    https://gitlab.kitware.com/cmake/cmake/-/issues/23236
    # The alternative would be to detect the need for a termcap library.
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
endif()
