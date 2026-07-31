#version 450

layout(set = 2, binding = 0) uniform UBO {
    vec4 color;
};

layout(location = 0) out vec4 out_color;

void main() {
    out_color = color;
}
