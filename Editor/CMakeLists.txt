cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME multitextor)
project(${PROJECT_NAME})

file(GLOB_RECURSE _EDITOR_INC "inc/*.h*")
file(GLOB_RECURSE _EDITOR_SRC "src/*.c*")
if(MSVC)
    file(GLOB _EDITOR_RES_INC "res/*.h")
    set(_EDITOR_RC "editor.rc")
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_EDITOR_INC} ${_EDITOR_RES_INC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_EDITOR_SRC})

set(PRODUCT_NAME "Multitextor")
set(PRODUCT_NAME_COLOR "&M&u&l&t&i&t&e&x&t&o&r")
set(PRODUCT_VERSION 2.0.0)
set(PRODUCT_VERSION_RC 2,0,0)
set(PRODUCT_VERSION_FULL "${PRODUCT_VERSION} MVP (alpha)")
configure_file("res/editor.rc.in" "editor.rc" @ONLY)
configure_file("res/Version.h.in" "Version.h" @ONLY)

add_executable(${PROJECT_NAME}
    ${_EDITOR_INC}
    ${_EDITOR_SRC}
    ${_EDITOR_RES_INC}
    ${_EDITOR_RC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ThirdPartyLib
        UtilsLib
        ConsoleLib
        WndManagerLib
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/inc"
        "${CMAKE_CURRENT_SOURCE_DIR}/res"
        "${CMAKE_CURRENT_BINARY_DIR}"
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
    if(USE_VLD)
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

