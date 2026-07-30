#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 3, binding = 0) uniform UBO {
    uint sampler_slot;
    uint texture_slot;
};

layout(set = 4, binding = 0) uniform sampler samplers[];
layout(set = 5, binding = 0) uniform texture2D texture2Ds[];

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(
        sampler2D(texture2Ds[texture_slot], samplers[sampler_slot]),
        uv
    );
}
