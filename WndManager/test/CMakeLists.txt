cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME TestWinman)
project(${PROJECT_NAME})

file(GLOB_RECURSE _TEST_SRC "*")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_TEST_SRC})

add_executable(${PROJECT_NAME}
    ${_TEST_SRC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ThirdPartyLib
        UtilsLib
        ConsoleLib
        WndManagerLib
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UNICODE
        _UNICODE
        NOMINMAX
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}$(Configuration)"
)

if(MSVC)
    # warning level 4
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /Zc:__cplusplus)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")    
    if(VLD)
        target_compile_definitions(${PROJECT_NAME} PUBLIC USE_VLD)
        target_include_directories(${PROJECT_NAME} PUBLIC
            #"../../ThirdParty/inc/vld"
            "C:/Program Files (x86)/Visual Leak Detector/include"
        )
        target_link_libraries(${PROJECT_NAME} PUBLIC
            #"../../../ThirdParty/lib/vld"
            "C:/Program Files (x86)/Visual Leak Detector/lib/Win64/vld.lib"
        )
    endif()    
else()
    # lots of warnings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
