cmake_minimum_required(VERSION 3.16)
project(SDL3_bindless_test LANGUAGES C VERSION 0.1)

include(CPM.cmake)

CPMAddPackage(
    NAME SDL3
    GIT_REPOSITORY https://github.com/gyxos/SDL.git
    GIT_TAG feature/bindless2
    EXCLUDE_FROM_ALL YES
    OPTIONS
        "SDL_SHARED OFF"
        "SDL_STATIC ON"
)

add_executable(${PROJECT_NAME} src/main.c)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${SDL3_SOURCE_DIR}/include"
    "${SDL3_SOURCE_DIR}/src/video/khronos"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3-static
)

find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)

set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

function(glsl_shader NAME STAGE)
    set(SOURCE "${SHADER_DIR}/${NAME}.${STAGE}.glsl")
    set(OUTPUT "${SHADER_DIR}/vulkan/${NAME}.${STAGE}.spv")

    add_custom_command(
        OUTPUT "${OUTPUT}"
        COMMAND "${GLSLANG_VALIDATOR}" -V -S "${STAGE}" "${SOURCE}" -o "${OUTPUT}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    set(SHADER_OUTPUTS ${SHADER_OUTPUTS} "${OUTPUT}" PARENT_SCOPE)
endfunction()

glsl_shader(quad vert)
glsl_shader(color frag)
glsl_shader(texture frag)

add_custom_target(shaders ALL DEPENDS ${SHADER_OUTPUTS})
