#version 450 core

layout(binding = 0) uniform Matrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 lightSpaceMatrix;
} u_Matrices;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Color;
layout(location = 3) in vec3 instance_Position;
layout(location = 4) in float instance_Rotation;
layout(location = 0) out vec3 fs_Color;
layout(location = 1) out vec3 fs_Normal;
layout(location = 2) out vec4 fs_PositionInLightSpace;

void main() {
    fs_Color = in_Color;
    mat3 rotation_matrix = mat3(1.0);
    rotation_matrix[0] = vec3(cos(instance_Rotation), 0.0, sin(instance_Rotation));
    rotation_matrix[2] = vec3(-sin(instance_Rotation), 0.0, cos(instance_Rotation));
    fs_Normal = rotation_matrix * in_Normal;
    vec3 position = rotation_matrix * in_Position + instance_Position;
    fs_PositionInLightSpace = u_Matrices.lightSpaceMatrix * vec4(position, 1.0);
    gl_Position = u_Matrices.projectionMatrix * u_Matrices.viewMatrix * vec4(position, 1.0);
}
