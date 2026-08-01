# SDL GPU Bindless Test

This is a test repo for the proposed SDL GPU bindless api, currently implemented for Vulkan only, which can be found https://github.com/gyxos/SDL/tree/feature/bindless-gpu

## General Thinking

- One "Global" descriptor heap binding
- Samplers and Resources - No combined sampler + texture
- Resolve the slot/index at render time, this is because of cycling and memory defrag

## Requirements

- DirectX: Shader Model 6.6 + Resource Tier 3
- Metal: Metal 3 (macOS 13+, iOS 16+)
- Vulkan: Vulkan 1.2 features (descriptorIndexing, runtimeDescriptorArray, descriptorBindingPartiallyBound, descriptorBinding*UpdateAfterBind, optional mutableDescriptorType), might be possible to support earlier Vulkan versions

## Binding Model

- DirectX Graphics:
  - space 0: b0-b3 vertex uniforms or compute uniforms
  - space 1: b0-b3 fragment uniforms
  - global: sampler + resource descriptors

- Metal: Handles are resources, no bindings required
- Vulkan:
  - Vertex uniform buffers: (0-3, 0)
  - Fragment uniform buffers: (0-3, 1)
  - Bindless space 2, using slang with BindlessDescriptorOptions::None binding numbers

## Example

```c
// ****************** Setup ******************

// Turn the bindless resources feature flag on
SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_BINDLESS_RESOURCES_BOOLEAN, true);
SDL_GPUDevice *device = SDL_CreateGPUDeviceWithProperties(props);

// ****************** Rendering ******************

// Resolve the resource to a slot
SDL_GPUResourceHandle handle = SDL_ResolveGPUTexture(gpu_device, texture, NULL);
if (handle == 0) {
    /* resolving failed */
}

// Pass that slot in constants or a buffer etc
Constants constants = {
    .texture_handle = handle,
};

SDL_PushGPUFragmentUniformData(..., constants, sizeof(Constants));
```
