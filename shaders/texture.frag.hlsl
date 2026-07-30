struct FragUniforms {
    uint sampler_slot;
    uint texture_slot;
};

ConstantBuffer<FragUniforms> uniforms : register(b0, space3);

SamplerState      samplers[]   : register(s0, space4);
Texture2D<float4> texture2Ds[] : register(t0, space4);

struct VertOut {
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

[shader("pixel")]
float4 fs_main(VertOut fin) : SV_Target0 {
    return texture2Ds[uniforms.texture_slot].Sample(
        samplers[uniforms.sampler_slot],
        fin.uv
    );
}
