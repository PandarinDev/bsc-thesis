#version 450 core

layout(binding = 0) uniform Matrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 modelMatrix;
} u_Matrices;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;
layout(location = 0) out vec3 fs_Color;

void main() {
    gl_Position = u_Matrices.projectionMatrix * u_Matrices.viewMatrix * u_Matrices.modelMatrix * vec4(in_Position, 1.0);
    fs_Color = in_Color;
}
