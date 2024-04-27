#version 450 core

layout(binding = 0) uniform ParticleMatrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    float ambientLight;
} u_Matrices;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 instance_Position;
layout(location = 0) out float fs_LightFactor;

void main() {
    fs_LightFactor = u_Matrices.ambientLight;
    vec3 position = instance_Position +
        vec3(u_Matrices.inverseViewMatrix[0]) * in_Position.x +
        vec3(u_Matrices.inverseViewMatrix[1]) * in_Position.y;
    gl_Position = u_Matrices.projectionMatrix * u_Matrices.viewMatrix * vec4(position, 1.0);
}
