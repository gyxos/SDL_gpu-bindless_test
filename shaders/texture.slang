#include "quad.slangh"

[vk::binding(0, 0)] __DynamicResource<__DynamicResourceKind.Sampler> samplers[];
[vk::binding(1, 0)] __DynamicResource<__DynamicResourceKind.General> textures[];

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
    __target_switch
    {
    case spirv:
        if (T.kind == DescriptorKind.Sampler)
            return samplers[((uint2)handle).x].asOpaqueDescriptor<T>();
        else
            return textures[((uint2)handle).x].asOpaqueDescriptor<T>();
    default:
        return defaultGetDescriptorFromHandle(handle);
    }
}

struct VertUniforms {
    float4 transform;
};

[vk::binding(0, 1)]
ConstantBuffer<VertUniforms> v_uniforms : register(b0, space1);

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

[vk::binding(0, 2)]
ConstantBuffer<FragUniforms> f_uniforms : register(b0, space2);

[shader("fragment")]
float4 fs_main(VertOut fin) : SV_Target0 {
    return f_uniforms.texture.Sample(
        f_uniforms.sampler,
        fin.texcoord
    );
}
