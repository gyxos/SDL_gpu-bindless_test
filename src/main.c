#include <string.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_log.h>
#include <vulkan/vulkan.h>

#define TEXTURE_SIZE 256

typedef struct App App;

struct App {
    SDL_Window *window;
    SDL_GPUDevice *gpu_device;

    SDL_GPUSampler *sampler;
    SDL_GPUTexture *texture_1;
    SDL_GPUTexture *texture_2;

    SDL_GPUShader *shader_color_vert;
    SDL_GPUShader *shader_color_frag;
    SDL_GPUShader *shader_texture_vert;
    SDL_GPUShader *shader_texture_frag;
    size_t compute_code_size;
    Uint8 *compute_code;

    SDL_GPUGraphicsPipeline *pipeline_swapchain_color;
    SDL_GPUGraphicsPipeline *pipeline_swapchain_texture;
    SDL_GPUGraphicsPipeline *pipeline_standard_color;
    SDL_GPUGraphicsPipeline *pipeline_standard_texture;

    SDL_GPUComputePipeline *pipeline_compute_color;
};

typedef struct Vec4 {
    float x;
    float y;
    float z;
    float w;
} Vec4;

const Vec4 TRANSFORM_IDENTITY = {
    .x = 1.0f,
    .y = 1.0f,
    .z = 0.0f,
    .w = 0.0f,
};

const Vec4 TRANSFORM_TOP_LEFT = {
    .x = 0.5f,
    .y = 0.5f,
    .z = -0.5f,
    .w = 0.5f,
};

const Vec4 TRANSFORM_TOP_RIGHT = {
    .x = 0.5f,
    .y = 0.5f,
    .z = 0.5f,
    .w = 0.5f,
};

const Vec4 TRANSFORM_BOTTOM_LEFT = {
    .x = 0.5f,
    .y = 0.5f,
    .z = -0.5f,
    .w = -0.5f,
};

const Vec4 TRANSFORM_BOTTOM_RIGHT = {
    .x = 0.5f,
    .y = 0.5f,
    .z = 0.5f,
    .w = -0.5f,
};

const SDL_FColor COLOR_BLACK = {
    .r = 0.0f,
    .g = 0.0f,
    .b = 0.0f,
    .a = 1.0f,
};

const SDL_FColor COLOR_RED = {
    .r = 1.0f,
    .g = 0.0f,
    .b = 0.0f,
    .a = 1.0f,
};

const SDL_FColor COLOR_GREEN = {
    .r = 0.0f,
    .g = 1.0f,
    .b = 0.0f,
    .a = 1.0f,
};

const SDL_FColor COLOR_BLUE = {
    .r = 0.0f,
    .g = 0.0f,
    .b = 1.0f,
    .a = 1.0f,
};

bool init_gpu(App *app);
bool refresh_gpu_resources(App *app);
bool load_gpu_shaders(App *app);
bool init_gpu_pipeline(App *app);
bool run_pipeline_color(App *app, SDL_GPUCommandBuffer *command_buffer, SDL_GPURenderPass *render_pass, SDL_GPUGraphicsPipeline *graphics_pipeline, Vec4 transform, SDL_FColor color);
bool run_compute_color(App *app, SDL_GPUCommandBuffer *command_buffer, SDL_GPUComputePass *compute_pass, SDL_GPUComputePipeline *compute_pipeline, SDL_GPUResourceHandle texture, SDL_FColor color);
bool run_pipeline_texture(App *app, SDL_GPUCommandBuffer *command_buffer, SDL_GPURenderPass *render_pass, SDL_GPUGraphicsPipeline *graphics_pipeline, Vec4 transform, SDL_GPUResourceHandle sampler_slot, SDL_GPUResourceHandle texture_slot);

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Failed");
        return SDL_APP_FAILURE;
    }

    App *app = SDL_calloc(1, sizeof(App));

    app->window = SDL_CreateWindow("SDL Bindless Test", 1200, 800, 0);
    init_gpu(app);
    SDL_ClaimWindowForGPUDevice(app->gpu_device, app->window);
    refresh_gpu_resources(app);
    load_gpu_shaders(app);
    init_gpu_pipeline(app);

    *appstate = app;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    App *app = appstate;

    refresh_gpu_resources(app);

    SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(app->gpu_device);
    SDL_GPUTexture *swapchain_texture;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, app->window, &swapchain_texture, NULL, NULL)) {
        return SDL_APP_FAILURE;
    }

    SDL_GPUColorTargetInfo color_target = {
        .texture = NULL,
        .mip_level = 0,
        .layer_or_depth_plane = 0,
        .clear_color = COLOR_BLACK,
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
        .resolve_texture = NULL,
        .resolve_mip_level = 0,
        .resolve_layer = 0,
        .cycle = false,
        .cycle_resolve_texture = false,
    };

    // We will render red to a texture, cycle it and render green to the same texture
    // Because we resolve after each cycle we can access the previous texture too
    // I'm unsure how common this would be but it shows that 

    color_target.texture = app->texture_1;
    color_target.cycle = true;

    SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target, 1, NULL);
    run_pipeline_color(app, command_buffer, render_pass, app->pipeline_standard_color, TRANSFORM_IDENTITY, COLOR_RED);
    SDL_EndGPURenderPass(render_pass);

    // When we resolve we get the texture at the time
    SDL_GPUResourceHandle texture_1_slot_red = SDL_AcquireGPUTextureHandle(command_buffer, app->texture_1, NULL);

    SDL_GPUComputePass *compute_pass = SDL_BeginGPUComputePass(command_buffer, NULL, 0, NULL, 0);
    SDL_GPUResourceHandle texture_1_slot_green = SDL_AcquireGPUTextureHandle(command_buffer, app->texture_1, &(SDL_GPUStorageTextureReadWriteBinding) {
        .mip_level = 0,
        .layer = 0,
        .cycle = true,
    });

    run_compute_color(app, command_buffer, compute_pass, app->pipeline_compute_color, texture_1_slot_green, COLOR_GREEN);
    SDL_EndGPUComputePass(compute_pass);

    texture_1_slot_green = SDL_AcquireGPUTextureHandle(command_buffer, app->texture_1, NULL);

    color_target.texture = swapchain_texture;
    color_target.cycle = false;

    render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target, 1, NULL);

    SDL_GPUResourceHandle texture_2_slot = SDL_AcquireGPUTextureHandle(command_buffer, app->texture_2, NULL);

    SDL_GPUResourceHandle sampler_slot = SDL_AcquireGPUSamplerHandle(command_buffer, app->sampler);
    run_pipeline_texture(app, command_buffer, render_pass, app->pipeline_swapchain_texture, TRANSFORM_TOP_LEFT, sampler_slot, texture_1_slot_red);
    run_pipeline_texture(app, command_buffer, render_pass, app->pipeline_swapchain_texture, TRANSFORM_TOP_RIGHT, sampler_slot, texture_1_slot_green);
    run_pipeline_texture(app, command_buffer, render_pass, app->pipeline_swapchain_texture, TRANSFORM_BOTTOM_LEFT, sampler_slot, texture_2_slot);
    run_pipeline_color(app, command_buffer, render_pass, app->pipeline_swapchain_color, TRANSFORM_BOTTOM_RIGHT, COLOR_BLUE);
    SDL_EndGPURenderPass(render_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);

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
        SDL_DestroyGPUDevice(app->gpu_device);
    }
}

bool init_gpu(App *app) {
    SDL_PropertiesID props = SDL_CreateProperties();
    if (props == 0) { return false; }

    // SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "vulkan");
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_DXIL_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_METALLIB_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_BINDLESS_BOOLEAN, true);

    SDL_GPUDevice *device = SDL_CreateGPUDeviceWithProperties(props);

    if (device == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "SDL_CreateGPUDeviceWithProperties: %s", SDL_GetError());
        return false;
    }

    SDL_DestroyProperties(props);

    app->gpu_device = device;

    return true;
}

SDL_GPUTexture * create_gpu_texture(App *app) {
    return SDL_CreateGPUTexture(app->gpu_device, &(SDL_GPUTextureCreateInfo) {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE,
        .width = TEXTURE_SIZE,
        .height = TEXTURE_SIZE,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
        .props = 0,
    });
}

SDL_GPUTexture * load_gpu_texture(App *app, const char *file) {
    SDL_Surface *surface = SDL_LoadPNG(file);

    if (surface->format != SDL_PIXELFORMAT_RGBA32) {
        SDL_Surface *converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(surface);
        surface = converted;
    }

    SDL_GPUTexture *texture = SDL_CreateGPUTexture(app->gpu_device, &(SDL_GPUTextureCreateInfo) {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = surface->w,
        .height = surface->h,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
        .props = 0,
    });

    Uint32 size = surface->w * surface->h * 4;
    Uint32 row_size = surface->w * 4;

    SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(app->gpu_device, &(SDL_GPUTransferBufferCreateInfo) {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = size,
    });

    Uint8 *mapped = SDL_MapGPUTransferBuffer(app->gpu_device, transfer, false);

    for (Uint32 y = 0; y < surface->h; y++) {
        SDL_memcpy(mapped + y * row_size, surface->pixels + y * surface->pitch, row_size);
    }

    SDL_UnmapGPUTransferBuffer(app->gpu_device, transfer);

    SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(app->gpu_device);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    SDL_GPUTextureTransferInfo source = {
        .transfer_buffer = transfer,
        .offset = 0,
        .pixels_per_row = surface->w,
        .rows_per_layer = surface->h,
    };

    SDL_GPUTextureRegion destination = {
        .texture = texture,
        .mip_level = 0,
        .layer = 0,
        .x = 0,
        .y = 0,
        .z = 0,
        .w = surface->w,
        .h = surface->h,
        .d = 1,
    };

    SDL_UploadToGPUTexture(copy_pass, &source, &destination, false);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_ReleaseGPUTransferBuffer(app->gpu_device, transfer);

    SDL_DestroySurface(surface);

    return texture;
}

bool refresh_gpu_resources(App *app) {
    if (app->sampler != NULL) {
        SDL_ReleaseGPUSampler(app->gpu_device, app->sampler);
    }
    app->sampler = SDL_CreateGPUSampler(app->gpu_device, &(SDL_GPUSamplerCreateInfo) {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .mip_lod_bias = 0,
        .max_anisotropy = 0,
        .compare_op = SDL_GPU_COMPAREOP_INVALID,
        .min_lod = 0,
        .max_lod = 0,
        .enable_anisotropy = false,
        .enable_compare = false,
    });

    if (app->texture_1 != NULL) {
        SDL_ReleaseGPUTexture(app->gpu_device, app->texture_1);
    }
    app->texture_1 = create_gpu_texture(app);

    if (app->texture_2 == NULL) {
        app->texture_2 = load_gpu_texture(app, "resources/test.png");
    }

    return !(app->texture_1 == NULL || app->texture_2 == NULL);
}

SDL_GPUShader * load_gpu_shader(App *app, SDL_GPUShaderFormat format, SDL_GPUShaderStage stage, const char *file) {
    size_t code_size;
    Uint8 *code = SDL_LoadFile(file, &code_size);
    const char *entrypoint = "main";

    if (format == SDL_GPU_SHADERFORMAT_METALLIB) {
        entrypoint = stage == SDL_GPU_SHADERSTAGE_VERTEX ? "vs_main" : "fs_main";
    }

    SDL_GPUShader *shader = SDL_CreateGPUShader(app->gpu_device, &(SDL_GPUShaderCreateInfo) {
        .code_size = code_size,
        .code = code,
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 1,
    });

    if (shader == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shader %s: %s", file, SDL_GetError());
    }

    return shader;
}

Uint8 * load_gpu_compute(App *app, const char *file, size_t *code_size) {
    Uint8 *code = SDL_LoadFile(file, code_size);

    if (code == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load shader %s: %s", file, SDL_GetError());
    }

    return code;
}

bool load_gpu_shaders(App *app) {
    SDL_GPUShaderFormat shader_format = SDL_GetGPUShaderFormats(app->gpu_device);

    if ((shader_format & SDL_GPU_SHADERFORMAT_SPIRV) != 0) {
        shader_format = SDL_GPU_SHADERFORMAT_SPIRV;
        app->shader_color_vert = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_VERTEX, "shaders/color/vert.spv");
        app->shader_color_frag = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_FRAGMENT, "shaders/color/frag.spv");
        app->shader_texture_vert = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_VERTEX, "shaders/texture/vert.spv");
        app->shader_texture_frag = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_FRAGMENT, "shaders/texture/frag.spv");
        app->compute_code = load_gpu_compute(app, "shaders/color/compute.spv", &app->compute_code_size);
    } else if ((shader_format & SDL_GPU_SHADERFORMAT_DXIL) != 0) {
        shader_format = SDL_GPU_SHADERFORMAT_DXIL;
        app->shader_color_vert = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_VERTEX, "shaders/color/vert.dxil");
        app->shader_color_frag = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_FRAGMENT, "shaders/color/frag.dxil");
        app->shader_texture_vert = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_VERTEX, "shaders/texture/vert.dxil");
        app->shader_texture_frag = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_FRAGMENT, "shaders/texture/frag.dxil");
        app->compute_code = load_gpu_compute(app, "shaders/color/compute.dxil", &app->compute_code_size);
    } else if ((shader_format & SDL_GPU_SHADERFORMAT_METALLIB) != 0) {
        shader_format = SDL_GPU_SHADERFORMAT_METALLIB;
        app->shader_color_vert = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_VERTEX, "shaders/color/vert.metallib");
        app->shader_color_frag = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_FRAGMENT, "shaders/color/frag.metallib");
        app->shader_texture_vert = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_VERTEX, "shaders/texture/vert.metallib");
        app->shader_texture_frag = load_gpu_shader(app, shader_format, SDL_GPU_SHADERSTAGE_FRAGMENT, "shaders/texture/frag.metallib");
        app->compute_code = load_gpu_compute(app, "shaders/color/compute.metallib", &app->compute_code_size);
    } else {
        return false;
    }

    app->pipeline_compute_color = SDL_CreateGPUComputePipeline(app->gpu_device, &(SDL_GPUComputePipelineCreateInfo) {
        .code_size = app->compute_code_size,
        .code = app->compute_code,
        .entrypoint = shader_format == SDL_GPU_SHADERFORMAT_METALLIB ? "cs_main" : "main",
        .format = shader_format,
        .num_uniform_buffers = 1,
        .threadcount_x = 16,
        .threadcount_y = 16,
        .threadcount_z = 1,
    });

    return true;
}

SDL_GPUGraphicsPipeline *create_gpu_pipeline(App *app, SDL_GPUShader *vertex_shader, SDL_GPUShader *fragment_shader, SDL_GPUTextureFormat target_format) {
    return SDL_CreateGPUGraphicsPipeline(app->gpu_device, &(SDL_GPUGraphicsPipelineCreateInfo) {
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
        .vertex_input_state = (SDL_GPUVertexInputState) {
            .vertex_buffer_descriptions = NULL,
            .num_vertex_buffers = 0,
            .vertex_attributes = NULL,
            .num_vertex_attributes = 0,
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = (SDL_GPURasterizerState) {
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_NONE,
            .front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
            .depth_bias_constant_factor = 0.0f,
            .depth_bias_clamp = 0.0f,
            .depth_bias_slope_factor = 0.0f,
            .enable_depth_bias = false,
            .enable_depth_clip = false,
        },
        .multisample_state = (SDL_GPUMultisampleState) {
            .sample_count = SDL_GPU_SAMPLECOUNT_1,
            .sample_mask = 0,
            .enable_mask = false,
            .enable_alpha_to_coverage = false,
        },
        .depth_stencil_state = (SDL_GPUDepthStencilState) {
            .compare_op = SDL_GPU_COMPAREOP_INVALID,
            .back_stencil_state = (SDL_GPUStencilOpState) {
                .fail_op = SDL_GPU_STENCILOP_INVALID,
                .pass_op = SDL_GPU_STENCILOP_INVALID,
                .depth_fail_op = SDL_GPU_STENCILOP_INVALID,
                .compare_op = SDL_GPU_COMPAREOP_INVALID,
            },
            .front_stencil_state = (SDL_GPUStencilOpState) {
                .fail_op = SDL_GPU_STENCILOP_INVALID,
                .pass_op = SDL_GPU_STENCILOP_INVALID,
                .depth_fail_op = SDL_GPU_STENCILOP_INVALID,
                .compare_op = SDL_GPU_COMPAREOP_INVALID,
            },
            .compare_mask = 0,
            .write_mask = 0,
            .enable_depth_test = false,
            .enable_depth_write = false,
            .enable_stencil_test = false,
        },
        .target_info = (SDL_GPUGraphicsPipelineTargetInfo) {
            .color_target_descriptions = &(SDL_GPUColorTargetDescription) {
                .format = target_format,
                .blend_state = (SDL_GPUColorTargetBlendState) {
                    .src_color_blendfactor = SDL_GPU_BLENDFACTOR_INVALID,
                    .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_INVALID,
                    .color_blend_op = SDL_GPU_BLENDOP_INVALID,
                    .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_INVALID,
                    .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_INVALID,
                    .alpha_blend_op = SDL_GPU_BLENDOP_INVALID,
                    .color_write_mask = 0xF,
                    .enable_blend = false,
                    .enable_color_write_mask = true,
                },
            },
            .num_color_targets = 1,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_INVALID,
            .has_depth_stencil_target = false,
        },
    });
}

bool init_gpu_pipeline(App *app) {
    SDL_GPUTextureFormat swapchain_format = SDL_GetGPUSwapchainTextureFormat(app->gpu_device, app->window);
    SDL_GPUTextureFormat texture_format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    app->pipeline_swapchain_color = create_gpu_pipeline(app, app->shader_color_vert, app->shader_color_frag, swapchain_format);
    app->pipeline_swapchain_texture = create_gpu_pipeline(app, app->shader_texture_vert, app->shader_texture_frag, swapchain_format);
    app->pipeline_standard_color = create_gpu_pipeline(app, app->shader_color_vert, app->shader_color_frag, texture_format);
    app->pipeline_standard_texture = create_gpu_pipeline(app, app->shader_texture_vert, app->shader_texture_frag, texture_format);

    return app->pipeline_swapchain_color != NULL && app->pipeline_swapchain_texture != NULL && app->pipeline_standard_color != NULL && app->pipeline_standard_texture != NULL;
}

bool run_pipeline_color(App *app, SDL_GPUCommandBuffer *command_buffer, SDL_GPURenderPass *render_pass, SDL_GPUGraphicsPipeline *graphics_pipeline, Vec4 transform, SDL_FColor color) {
    SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);
    SDL_PushGPUVertexUniformData(command_buffer, 0, &transform, sizeof(Vec4));
    SDL_PushGPUFragmentUniformData(command_buffer, 0, &color, sizeof(Vec4));
    SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);

    return true;
}

typedef struct ComputeColorConstants {
    SDL_GPUResourceHandle texture;
    SDL_FColor color;
} ComputeColorConstants;

bool run_compute_color(App *app, SDL_GPUCommandBuffer *command_buffer, SDL_GPUComputePass *compute_pass, SDL_GPUComputePipeline *compute_pipeline, SDL_GPUResourceHandle texture, SDL_FColor color) {
    ComputeColorConstants constants = {
        .texture = texture,
        .color = color,
    };

    Uint32 groupcount = (TEXTURE_SIZE + 15) / 16;

    SDL_BindGPUComputePipeline(compute_pass, compute_pipeline);
    SDL_PushGPUComputeUniformData(command_buffer, 0, &constants, sizeof(constants));
    SDL_DispatchGPUCompute(compute_pass, groupcount, groupcount, 1);

    return true;
}

bool run_pipeline_texture(App *app, SDL_GPUCommandBuffer *command_buffer, SDL_GPURenderPass *render_pass, SDL_GPUGraphicsPipeline *graphics_pipeline, Vec4 transform, SDL_GPUResourceHandle sampler_slot, SDL_GPUResourceHandle texture_slot) {
    SDL_GPUResourceHandle uniform_data[2] = { sampler_slot, texture_slot };
    SDL_BindGPUGraphicsPipeline(render_pass, graphics_pipeline);
    SDL_PushGPUVertexUniformData(command_buffer, 0, &transform, sizeof(Vec4));
    SDL_PushGPUFragmentUniformData(command_buffer, 0, &uniform_data, sizeof(SDL_GPUResourceHandle) * 2);
    SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);

    return true;
}
