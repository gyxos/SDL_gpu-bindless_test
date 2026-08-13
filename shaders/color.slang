#include "common.slangh"

struct VertexUniforms {
    float4 transform;
};

[[vk::binding(0, 0)]]
ConstantBuffer<VertexUniforms> vertex_uniforms : register(b0, space0);

struct VertOut {
    float4 position : SV_Position;
};

[shader("vertex")]
VertOut vs_main(uint vertex_id: SV_VertexID) {
    uint index = QUAD_INDEX[vertex_id];
    float2 position = QUAD_POSITION[index] * vertex_uniforms.transform.xy + vertex_uniforms.transform.zw;

    VertOut output;
    output.position = float4(position, 0.0, 1.0);

    return output;
}

struct FragmentUniforms {
    float4 color;
};

[[vk::binding(0, 1)]]
ConstantBuffer<FragmentUniforms> fragment_uniforms : register(b0, space1);

[shader("fragment")]
float4 fs_main(VertOut fin) : SV_Target0 {
    return fragment_uniforms.color;
}
