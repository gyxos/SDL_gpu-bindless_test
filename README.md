# SDL GPU Bindless Test

This is a test repo for a proposed SDL GPU bindless api, which can be found https://github.com/gyxos/SDL/tree/feature/bindless-gpu

## General Thinking

- One "global" descriptor heap binding
- Samplers and Resources - No combined sampler + textures
- Resolve a resource handle at render time

## Requirements

- DirectX: Shader Model 6.6 Resource Tier 3
- Metal: Metal 3 (macOS 13, iOS 16, tvOS 16)
- Vulkan: Vulkan 1.2

## Binding Model

- DirectX Graphics:
  - space 0: b0-b3 vertex uniforms or compute uniforms
  - space 1: b0-b3 fragment uniforms
  - global: sampler + resource descriptors
- Metal:
  - buffer 0-3: uniforms
- Vulkan:
  - set 0 binding 0-3: vertex uniforms or compute uniforms
  - set 1 binding 0-3: fragment uniforms
  - set 2: 0 samplers, 2 sampled images, 3 storage images, 7 storage buffers - matches slangs BindlessDescriptorOptions::None, search for BindlessDescriptorOptions in https://shader-slang.org/slang/user-guide/convenience-features for more details

## Example

```c
// ****************** Setup ******************

// Turn the bindless resources feature flag on
SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_BINDLESS_RESOURCES_BOOLEAN, true);
SDL_GPUDevice *device = SDL_CreateGPUDeviceWithProperties(props);

// ****************** Rendering ******************

SDL_BeginGPURenderPass(command_buffer, ...);

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

## Slang shaders

You can use other shaders but this all comes together quite well when using slang shaders
- Bindings for uniforms work across all GPU drivers without changing code
- DescriptorHandle convenience feature nicely handles the differences between GPU APIs, search for DescriptorHandle in https://shader-slang.org/slang/user-guide/convenience-features for more details

```slang
export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor {
    return defaultGetDescriptorFromHandle(handle, BindlessDescriptorOptions::None);
}

struct VertexUniforms {
    float4 transform;
};

[vk::binding(0, 0)]
ConstantBuffer<VertexUniforms> vertex_uniforms : register(b0, space0);

struct FragmentUniforms {
    SamplerState.Handle sampler;
    Texture2D.Handle texture;
};

[vk::binding(0, 1)]
ConstantBuffer<FragmentUniforms> fragment_uniforms : register(b0, space1);

struct VertOut {
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

[shader("vertex")]
VertOut vs_main(uint vertex_id: SV_VertexID) {
    uint index = QUAD_INDEX[vertex_id];
    float2 position = QUAD_POSITION[index] * vertex_uniforms.transform.xy + vertex_uniforms.transform.zw;

    VertOut output;
    output.position = float4(position, 0.0, 1.0);
    output.texcoord = QUAD_TEXCOORD0[index];

    return output;
}

[shader("fragment")]
float4 fs_main(VertOut fin) : SV_Target0 {
    return fragment_uniforms.texture.Sample(
        fragment_uniforms.sampler,
        fin.texcoord
    );
}
```
