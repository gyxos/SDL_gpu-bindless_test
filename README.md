# SDL GPU Bindless Test

This is a test repo for the proposed SDL GPU bindless api, currently implemented for Vulkan only, which can be found https://github.com/gyxos/SDL/tree/feature/bindless-gpu

## General Thinking

- One "Global" descriptor heap binding
- Samplers and Resources - No combined sampler + texture
- Resolve the slot/index at render time, this is because of cycling and memory defrag

## Requirements

- DirectX: Shader Model 6.0 + Resource Tier 2 (already a soft requirement)
- Metal: Metal 3 (macOS 13+, iOS 16+)
- Vulkan: Vulkan 1.2 features (descriptorIndexing, runtimeDescriptorArray, descriptorBindingPartiallyBound, descriptorBinding*UpdateAfterBind, optional mutableDescriptorType), might be possible to support earlier Vulkan versions

## Binding Model

- DirectX: TBD
- Metal: TBD
- Vulkan:
  - Bindless samplers (0, 0), sampled textures (1, 0), storage textures (2, 0), storage buffers (3, 0)
  - Vertex uniform buffers: (0-3, 1)
  - Fragment uniform buffers: (0-3, 2)

## Example

```c
// ****************** Setup ******************

// Turn the bindless resources feature flag on
SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_BINDLESS_RESOURCES_BOOLEAN, true);
SDL_GPUDevice *device = SDL_CreateGPUDeviceWithProperties(props);

// ****************** Rendering ******************

// Resolve the resource to a slot
SDL_GPUResourceHandle handle;
if (handle == 0) {
    /* resolving failed */
}

// Pass that slot in constants or a buffer etc
Constants constants = {
    .texture_slot = slot,
};

SDL_PushGPUFragmentUniformData(..., constants, sizeof(Constants));
```
