#version 450 core

layout(binding = 0) uniform Matrices {
    mat4 projectionMatrix;
} u_Matrices;

layout(location = 0) in vec3 in_Position;

void main() {
    gl_Position = u_Matrices.projectionMatrix * vec4(in_Position, 1.0);
}
