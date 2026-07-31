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

## API

```c
SDL_GPUResourceSet *SDL_CreateGPUResourceSet(SDL_GPUDevice *device, const SDL_GPUResourceSetCreateInfo *createinfo);
void SDL_BindGPUResourceSet(SDL_GPUCommandBuffer *command_buffer, SDL_GPUResourceSet *resource_set);
void SDL_ReleaseGPUResourceSet(SDL_GPUDevice *device, SDL_GPUResourceSet *resource_set);

SDL_GPUResourceID SDL_AllocateGPUResourceSampler(SDL_GPUDevice *device, SDL_GPUResourceSet *resource_set, SDL_GPUSampler *sampler);
SDL_GPUResourceID SDL_AllocateGPUResourceTexture(SDL_GPUDevice *device, SDL_GPUResourceSet *resource_set, SDL_GPUTexture *texture);
SDL_GPUResourceID SDL_AllocateGPUResourceStorageTexture(SDL_GPUDevice *device, SDL_GPUResourceSet *resource_set, SDL_GPUTexture *texture);
SDL_GPUResourceID SDL_AllocateGPUResourceStorageBuffer(SDL_GPUDevice *device, SDL_GPUResourceSet *resource_set, SDL_GPUBuffer *buffer);

bool SDL_ResolveGPUResource(SDL_GPUCommandBuffer *command_buffer, SDL_GPUResourceSet *resource_set, SDL_GPUResourceID resource, Uint32 *slot);
void SDL_ReleaseGPUResource(SDL_GPUDevice *device, SDL_GPUResourceSet *resource_set, SDL_GPUResourceID resource);
```

## Example

```c
// ****************** Setup ******************

// Turn the bindless resources feature flag on
SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_BINDLESS_RESOURCES_BOOLEAN, true);
SDL_GPUDevice *device = SDL_CreateGPUDeviceWithProperties(props);

// Create a resource set
resource_set = SDL_CreateGPUResourceSet(gpu_device, { num_samplers, num_resources });

// When you create/load a texture allocate it to the resource_set
texture = SDL_CreateGPUTexture(...);
resource_id = SDL_AllocateGPUResourceTexture(gpu_device, resource_set, texture);


// ****************** Rendering ******************

// Bind the resource set before graphics pipeline
SDL_BindGPUResourceSet(command_buffer, resource_set);
SDL_BindGPUGraphicsPipeline(...);

// Resolve the resource to a slot
Uint32 slot;
if (!SDL_ResolveGPUResource(command_buffer, resource_set, resource_id, &slot)) {
    /* resolving failed, possibly ResourceID has been released, or using Resource ID on wrong resource_set etc */
}

// Pass that slot in constants or a buffer etc
Constants constants = {
    .texture_slot = slot,
};

SDL_PushGPUFragmentUniformData(..., constants, sizeof(Constants));
```
