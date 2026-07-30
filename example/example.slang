#ifdef METAL

struct Entry<T> where T : IOpaqueDescriptor { DescriptorHandle<T> h; };

struct Bindless {
    ConstBufferPointer<Entry<SamplerState>>        samplers;
    ConstBufferPointer<Entry<Texture2D<float4>>>   texure2Ds;
    ConstBufferPointer<Entry<TextureCube<float4>>> texureCubes;
    ConstBufferPointer<Entry<ByteAddressBuffer>>   buffers;
};

ConstantBuffer<Bindless> g : register(b30);

SamplerState        g_sampler(uint slot)     { return g.samplers[slot].h; }
Texture2D<float4>   g_texture2D(uint slot)   { return g.texure2Ds[slot].h; }
TextureCube<float4> g_textureCube(uint slot) { return g.texureCubes[slot].h; }
ByteAddressBuffer   g_buffer(uint slot)      { return g.buffers[slot].h; }

#else

[[vk::binding(0, 4)]] SamplerState        g_samplers[]     : register(s0, space4);
[[vk::binding(0, 5)]] Texture2D<float4>   g_texture2Ds[]   : register(t0, space4);
[[vk::binding(0, 5)]] TextureCube<float4> g_textureCubes[] : register(t0, space5);
[[vk::binding(0, 7)]] ByteAddressBuffer   g_buffers[]      : register(t0, space6);

SamplerState        g_sampler(uint slot)     { return g_samplers[slot]; }
Texture2D<float4>   g_texture2D(uint slot)   { return g_texture2Ds[slot]; }
TextureCube<float4> g_textureCube(uint slot) { return g_textureCubes[slot]; }
ByteAddressBuffer   g_buffer(uint slot)      { return g_buffers[slot]; }

#endif

struct VertUniforms {
    float4x4 world_matrix;
    uint mesh_vertex_slot;
    uint mesh_texcoord_slot;
};

[[vk::binding(0, 1)]]
ConstantBuffer<VertUniforms> v_uniforms : register(b0, space1);

struct VertOut {
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

[shader("vertex")]
VertOut vs_main(uint vertex_id: SV_VertexID) {
    ByteAddressBuffer vertices = g_buffer(v_uniforms.mesh_vertex_slot);
    ByteAddressBuffer texcoords = g_buffer(v_uniforms.mesh_texcoord_slot);

    float3 local_pos = vertices.Load<float3>(vertex_id * 12);

    VertOut output;
    output.position = mul(v_uniforms.world_matrix, float4(local_pos, 1.0f));
    output.texcoord = texcoords.Load<float2>(vertex_id * 8);

    return output;
}

struct FragUniforms {
    uint sampler_slot;
    uint diffuse_slot;
    uint environment_slot;
};

[[vk::binding(0, 3)]]
ConstantBuffer<FragUniforms> f_uniforms : register(b0, space3);

[shader("fragment")]
float4 fs_main(VertOut fin) : SV_Target0 {
    SamplerState sampler = g_sampler(f_uniforms.sampler_slot);
    float4 diffuse = g_texture2D(f_uniforms.diffuse_slot).Sample(sampler, fin.texcoord);

    float3 reflectDir = float3(0.0, 0.0, 0.0); // reflect(viewDir, normal);

    float4 reflection = g_textureCube(f_uniforms.environment_slot).Sample(sampler, reflectDir);

    return lerp(diffuse, reflection, 0.5f);
}
