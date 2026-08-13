#include "common.slangh"

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
