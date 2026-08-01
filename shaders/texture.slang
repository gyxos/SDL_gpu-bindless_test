#include "quad.slangh"

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
    return defaultGetDescriptorFromHandle(handle, BindlessDescriptorOptions::None);
}

struct VertUniforms {
    float4 transform;
};

[vk::binding(0, 0)]
ConstantBuffer<VertUniforms> v_uniforms : register(b0, space0);

struct VertOut {
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

[shader("vertex")]
VertOut vs_main(uint vertex_id: SV_VertexID) {
    uint index = QUAD_INDEX[vertex_id];
    float2 position = QUAD_POSITION[index] * v_uniforms.transform.xy + v_uniforms.transform.zw;

    VertOut output;
    output.position = float4(position, 0.0, 1.0);
    output.texcoord = QUAD_TEXCOORD0[index];

    return output;
}

struct FragUniforms {
    DescriptorHandle<SamplerState> sampler;
    DescriptorHandle<Texture2D> texture;
};

[vk::binding(0, 1)]
ConstantBuffer<FragUniforms> f_uniforms : register(b0, space1);

[shader("fragment")]
float4 fs_main(VertOut fin) : SV_Target0 {
    return f_uniforms.texture.Sample(
        f_uniforms.sampler,
        fin.texcoord
    );
}
