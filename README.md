# SDL GPU Bindless Test

This is a test repo for the proposed SDL GPU bindless api, currently implemented for Vulkan only

I've given this a go, currently implemented for Vulkan only
- In this branch: https://github.com/gyxos/SDL/tree/feature/bindless-gpu
- And here is an example repo using it: 

## General Thinking

- One "Global" descriptor heap binding
- Samplers and Resources - No combined sampler + texture
- Considered not supporting cycling, but the vulkan memory defrag invalidates backing resources, so we need to detect those which would also mean we detect cycling
- Considered using indexes, e.g. Allocate/Register function returns an index usually where you create the resource, but this would break on defrag/cycle

## Requirements

- DirectX: Shader Model 6.0 + Resource Tier 2 (already a soft requirement)
- Metal: Metal 3 (macOS 13+, iOS 16+)
- Vulkan: Vulkan 1.2 features (descriptorIndexing, runtimeDescriptorArray, descriptorBindingPartiallyBound, descriptorBinding*UpdateAfterBind, optional mutableDescriptorType), might be possible to support earlier Vulkan versions

## Binding Model

- DirectX: samplers (s0, space4), resources (t0, space4) (t0, space5) (t0, space6) etc
- Metal: constant buffer with constant pointers \[\[buffer(30)]] first item in the constant buffer is samplers, otherwise resources
- Vulkan: samplers (0, 4), sampled textures (0, 5), storage textures (0, 6), storage buffers (0, 7)

## API

```c
// ****************** Setup ******************

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
SDL_PushGPUFragmentUniformData(..., constants_with_slot, constants_size);
```
