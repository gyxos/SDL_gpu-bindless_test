#version 450

layout(set = 1, binding = 0) uniform UBO {
    vec4 transform;
};

const vec2 positions[4] = vec2[4](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);

const vec2 uvs[4] = vec2[4](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

const uint indexes[6] = uint[](
    0u, 1u, 2u,
    1u, 3u, 2u
);

layout(location = 0) out vec2 uv;

void main() {
    uint index = indexes[gl_VertexIndex];
    vec2 position = positions[index] * transform.xy + transform.zw;
    gl_Position = vec4(position, 0.0, 1.0);
    uv = uvs[index];
}
