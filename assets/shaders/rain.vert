#version 450 core

layout(binding = 0) uniform Matrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 lightSpaceMatrix;
    vec3 lightDirection;
    float ambientLight;
} u_Matrices;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 instance_Position;

void main() {
    vec3 position = in_Position + instance_Position;
    gl_Position = u_Matrices.projectionMatrix * u_Matrices.viewMatrix * vec4(position, 1.0);
}
