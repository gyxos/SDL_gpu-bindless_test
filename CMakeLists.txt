cmake_minimum_required(VERSION 3.16)
project(SDL3_bindless_test LANGUAGES C VERSION 0.1)

include(CPM.cmake)

CPMAddPackage(
    NAME SDL3
    GIT_REPOSITORY https://github.com/gyxos/SDL.git
    GIT_TAG feature/bindless-gpu
    EXCLUDE_FROM_ALL YES
    OPTIONS
        "SDL_SHARED OFF"
        "SDL_STATIC ON"
)

add_executable(${PROJECT_NAME} src/main.c src/gpu.c)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${SDL3_SOURCE_DIR}/include"
    "${SDL3_SOURCE_DIR}/src/video/khronos"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3-static
)
