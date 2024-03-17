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
layout(location = 0) out vec3 fs_Color;
layout(location = 1) out vec3 fs_Normal;
layout(location = 2) out vec4 fs_PositionInLightSpace;

void main() {
    fs_Color = in_Color;
    fs_Normal = in_Normal;
    vec3 position = in_Position + instance_Position;
    fs_PositionInLightSpace = u_Matrices.lightSpaceMatrix * vec4(position, 1.0);
    gl_Position = u_Matrices.projectionMatrix * u_Matrices.viewMatrix * vec4(position, 1.0);
}
