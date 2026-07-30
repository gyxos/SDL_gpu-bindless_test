#include <metal_stdlib>
using namespace metal;

struct UBO
{
    uint sampler_slot;
    uint texture_slot;
};

struct Sampler { sampler value; };
struct Texture2D { texture2D<float> value; };

struct Bindless
{
    Sampler device* samplers;
    Texture2D device* texture2Ds;
};

struct VertexOutput
{
    float2 uv;
};

fragment float4 fragment_main(
    VertexOutput in [[stage_in]],
    constant Uniforms& uniforms [[buffer(1)]],
    Bindless constant* g [[buffer(30)]])
{
    sampler selected_sampler = g->samplers[uniforms.sampler_slot].value;
    texture2d<float> selected_texture = g->texture2Ds[ubo.texture_slot].value;
    return selected_texture.sample(selected_sampler, in.uv);
}
