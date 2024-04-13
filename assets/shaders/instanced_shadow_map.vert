#version 450 core

layout(binding = 0) uniform Matrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
} u_Matrices;

layout(location = 0) in vec3 in_Position;
layout(location = 3) in vec3 instance_Position;
layout(location = 4) in float instance_Rotation;

void main() {
    mat3 rotation_matrix = mat3(1.0);
    rotation_matrix[0] = vec3(cos(instance_Rotation), 0.0, sin(instance_Rotation));
    rotation_matrix[2] = vec3(-sin(instance_Rotation), 0.0, cos(instance_Rotation));
    vec3 position = rotation_matrix * in_Position + instance_Position;
    gl_Position = u_Matrices.projectionMatrix * u_Matrices.viewMatrix * vec4(position, 1.0);
}