#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler samplers[];
layout(set = 0, binding = 1) uniform texture2D texture2Ds[];

layout(set = 2, binding = 0) uniform UBO {
    uvec2 sampler_slot;
    uvec2 texture_slot;
};

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(
        sampler2D(texture2Ds[texture_slot.x], samplers[sampler_slot.x]),
        uv
    );
}
