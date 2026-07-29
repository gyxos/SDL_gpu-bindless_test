#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_log.h>
#include "gpu.h"

typedef struct App App;

struct App {
    SDL_Window *window;
    SDL_GPUDevice *gpu_device;
    SDL_GPUResourceSet *resource_set;
    SDL_GPUTexture *texture_1;
    SDL_GPUResourceID resouce_1;
    SDL_GPUTexture *texture_2;
    SDL_GPUResourceID resouce_2;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
#ifdef __APPLE__
    if (!SDL_GetHint(SDL_HINT_VULKAN_LIBRARY)) {
        SDL_SetHint(SDL_HINT_VULKAN_LIBRARY, "/opt/homebrew/lib/libvulkan.dylib");
    }
#endif

    SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Failed");
        return SDL_APP_FAILURE;
    }

    App *app = SDL_calloc(1, sizeof(App));

    app->window = SDL_CreateWindow("SDL Bindless Test", 1200, 800, 0);
    app->gpu_device = CreateGPUDevice();
    SDL_ClaimWindowForGPUDevice(app->gpu_device, app->window);

    app->resource_set = SDL_CreateGPUResourceSet(app->gpu_device, &(SDL_GPUResourceSetCreateInfo){
        .num_samplers = 8,
        .num_resources = 256,
    });

    app->texture_1 = SDL_CreateGPUTexture(app->gpu_device, &(SDL_GPUTextureCreateInfo) {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
        .width = 256,
        .height = 256,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
        .props = 0,
    });

    app->texture_2 = SDL_CreateGPUTexture(app->gpu_device, &(SDL_GPUTextureCreateInfo) {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
        .width = 256,
        .height = 256,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
        .props = 0,
    });

    app->resouce_1 = SDL_AllocateGPUResourceTexture(app->gpu_device, app->resource_set, app->texture_1);
    app->resouce_2 = SDL_AllocateGPUResourceTexture(app->gpu_device, app->resource_set, app->texture_2);

    *appstate = app;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    App *app = appstate;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    App *app = appstate;

    if (event->type == SDL_EVENT_QUIT || (event->type == SDL_EVENT_KEY_DOWN && event->key.scancode == SDL_SCANCODE_ESCAPE)) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    App *app = appstate;

    if (app->gpu_device != NULL) {
        if (app->texture_1 != NULL) { SDL_ReleaseGPUTexture(app->gpu_device, app->texture_1); }
        if (app->texture_2 != NULL) { SDL_ReleaseGPUTexture(app->gpu_device, app->texture_2); }
        if (app->resource_set != NULL) { SDL_ReleaseGPUResourceSet(app->gpu_device, app->resource_set); }
        SDL_DestroyGPUDevice(app->gpu_device);
    }
}
