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
find_program(DXC NAMES DXC REQUIRED)
find_program(METALC NAMES metal HINTS "C:/Program Files/Metal Developer Tools/bin")
get_filename_component(METAL_BIN_DIR "${METALC}" DIRECTORY)

set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

function(slang_shader NAME)
    set(SOURCE "${SHADER_DIR}/${NAME}.slang")
    set(OUTPUT_VERT_SPV "${SHADER_DIR}/${NAME}/vert.spv")
    set(OUTPUT_FRAG_SPV "${SHADER_DIR}/${NAME}/frag.spv")
    set(OUTPUT_VERT_DXIL "${SHADER_DIR}/${NAME}/vert.dxil")
    set(OUTPUT_FRAG_DXIL "${SHADER_DIR}/${NAME}/frag.dxil")
    set(OUTPUT_VERT_MTL "${SHADER_DIR}/${NAME}/vert.metallib")
    set(OUTPUT_FRAG_MTL "${SHADER_DIR}/${NAME}/frag.metallib")

    add_custom_command(
        OUTPUT "${OUTPUT_VERT_SPV}"
        COMMAND "${SLANGC}" "${SOURCE}" -entry vs_main -stage vertex -target spirv -bindless-space-index 3 -o "${OUTPUT_VERT_SPV}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${OUTPUT_FRAG_SPV}"
        COMMAND "${SLANGC}" "${SOURCE}" -entry fs_main -stage fragment -target spirv -bindless-space-index 3 -o "${OUTPUT_FRAG_SPV}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${OUTPUT_VERT_DXIL}"
        COMMAND "${SLANGC}" "${SOURCE}" -entry vs_main -stage vertex -target dxil -profile sm_6_6 -o "${OUTPUT_VERT_DXIL}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${OUTPUT_FRAG_DXIL}"
        COMMAND "${SLANGC}" "${SOURCE}" -entry fs_main -stage fragment -target dxil -profile sm_6_6 -o "${OUTPUT_FRAG_DXIL}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${OUTPUT_VERT_MTL}"
        COMMAND "${SLANGC}" "${SOURCE}" -entry vs_main -stage vertex -target metallib -metal-path "${METAL_BIN_DIR}" -o "${OUTPUT_VERT_MTL}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${OUTPUT_FRAG_MTL}"
        COMMAND "${SLANGC}" "${SOURCE}" -entry fs_main -stage fragment -target metallib -metal-path "${METAL_BIN_DIR}" -o "${OUTPUT_FRAG_MTL}"
        DEPENDS "${SOURCE}"
        VERBATIM
    )

    set(SHADER_OUTPUTS ${SHADER_OUTPUTS} "${OUTPUT_VERT_SPV}" "${OUTPUT_FRAG_SPV}" "${OUTPUT_VERT_DXIL}" "${OUTPUT_FRAG_DXIL}" "${OUTPUT_VERT_MTL}" "${OUTPUT_FRAG_MTL}" PARENT_SCOPE)
endfunction()

slang_shader(color)
slang_shader(texture)

add_custom_target(shaders ALL DEPENDS ${SHADER_OUTPUTS})
