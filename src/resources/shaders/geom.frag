#version 460

layout(location = 0) in struct {
    vec3 normal;
} fs_in;

layout(location = 0) out vec4 fragColor;

void main() {
    vec3 color = abs(fs_in.normal);
    fragColor = vec4(1, 0, 0, 1);
}