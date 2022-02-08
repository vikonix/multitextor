cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME multitextor)
project(${PROJECT_NAME})

set(PRODUCT_NAME "Multitextor")
set(PRODUCT_NAME_COLORED "&M&u&l&t&i&t&e&x&t&o&r")
set(PRODUCT_VERSION 2.0.0)
set(PRODUCT_VERSION_RC 2,0,0)
set(PRODUCT_VERSION_FULL "${PRODUCT_VERSION}-beta")

file(GLOB_RECURSE _EDITOR_INC "inc/*.h*")
file(GLOB_RECURSE _EDITOR_SRC "src/*.c*")
if(MSVC)
    file(GLOB _EDITOR_RES_INC "res/*.h")
    set(_EDITOR_RC "editor.rc")
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_EDITOR_INC} ${_EDITOR_RES_INC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_EDITOR_SRC})

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
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /Zc:__cplusplus /Zi)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
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

    target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
    target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../configurations/config ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}$<CONFIG>/config/
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../configurations/parser ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}$<CONFIG>/parser/
    )

else()
    find_package( Threads REQUIRED)
    
    # lots of warnings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
    target_link_options(${PROJECT_NAME} PRIVATE -pthread)
    #target_link_options(${PROJECT_NAME} PRIVATE -static)

    file(COPY ../configurations/config DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file(COPY ../configurations/parser DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

if(UNIX)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT binaries)
endif()

if(APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE -liconv)
endif()

