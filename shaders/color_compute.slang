#include "common.slangh"

struct Uniforms {
    RWTexture2D.Handle texture;
    float4 color;
};

[[vk::binding(0, 0)]]
ConstantBuffer<Uniforms> uniforms : register(b0, space0);

[shader("compute")]
[numthreads(16, 16, 1)]
void cs_main(uint3 dispatchThreadID: SV_DispatchThreadID) {
    uint width;
    uint height;
    uniforms.texture.GetDimensions(width, height);

    if (dispatchThreadID.x >= width || dispatchThreadID.y >= height) {
        return;
    }

    uniforms.texture[dispatchThreadID.xy] = uniforms.color;
}
