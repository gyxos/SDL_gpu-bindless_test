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

find_program(SLANGC NAMES slangc REQUIRED)

set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

function(slang_shader NAME)
    set(SOURCE "${SHADER_DIR}/${NAME}.slang")
    set(OUTPUT_VERT "${SHADER_DIR}/vulkan/${NAME}.vert.spv")
    set(OUTPUT_FRAG "${SHADER_DIR}/vulkan/${NAME}.frag.spv")

    add_custom_command(
        OUTPUT "${OUTPUT_VERT}"
        COMMAND "${SLANGC}" "${SOURCE}" -entry vs_main -stage vertex -target spirv -o "${OUTPUT_VERT}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${OUTPUT_FRAG}"
        COMMAND "${SLANGC}" "${SOURCE}" -entry fs_main -stage fragment -target spirv -o "${OUTPUT_FRAG}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    set(SHADER_OUTPUTS ${SHADER_OUTPUTS} "${OUTPUT_VERT}" "${OUTPUT_FRAG}" PARENT_SCOPE)
endfunction()

slang_shader(color)
slang_shader(texture)

add_custom_target(shaders ALL DEPENDS ${SHADER_OUTPUTS})
