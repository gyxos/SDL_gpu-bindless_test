# SDL GPU Bindless Test

This is a test repo for a proposed SDL GPU bindless api, which can be found https://github.com/gyxos/SDL/tree/feature/bindless-gpu

## General Thinking

- One "global" descriptor heap binding
- Samplers and Resources - No combined sampler + textures
- Resolve the slot/index at render time, this is because of cycling and memory defrag

## Requirements

- DirectX: Shader Model 6.6 Resource Tier 3
- Metal: Metal 3 (macOS 13, iOS 16, tvOS 16)
- Vulkan: Vulkan 1.2

## Binding Model

- DirectX Graphics:
  - space 0: b0-b3 vertex uniforms or compute uniforms
  - space 1: b0-b3 fragment uniforms
  - global: sampler + resource descriptors
- Metal: Handles are resources, no bindings required
- Vulkan:
  - set 0 binding 0-3: vertex uniforms or compute uniforms
  - set 1 binding 0-3: fragment uniforms
  - set 2: bindless descriptors, bound using slang with BindlessDescriptorOptions::None binding numbers (sampler 0, sampled image 2, storage image 3, storage buffer 7)

## Example

```c
// ****************** Setup ******************

// Turn the bindless resources feature flag on
SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_BINDLESS_RESOURCES_BOOLEAN, true);
SDL_GPUDevice *device = SDL_CreateGPUDeviceWithProperties(props);

// ****************** Rendering ******************

SDL_GPUBeginRenderPass(command_buffer, ...);

// Resolve the resources to handles

SDL_GPUResourceHandle sampler_handle = SDL_AcquireGPUSamplerHandle(command_buffer, sampler);
if (sampler_handle == 0) { /* resolving failed */ }

SDL_GPUResourceHandle texture_handle = SDL_AcquireGPUTextureHandle(gpu_device, texture, NULL); // if writing then the NULL needs to be replaced with a SDL_GPUStorageTextureReadWriteBinding*
if (texture_handle == 0) { /* resolving failed */ }

// Pass those handles in constants
Constants constants = {
    .sampler = sampler_handle,
    .texture = texture_handle,
};

SDL_PushGPUFragmentUniformData(..., constants, sizeof(Constants));
```
