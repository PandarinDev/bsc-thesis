#version 450 core

layout(binding = 0) uniform Matrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
} u_Matrices;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
} u_PushConstants;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Color;
layout(location = 0) out vec3 fs_Color;
layout(location = 1) out vec3 fs_Normal;

void main() {
    fs_Color = in_Color;
    fs_Normal = in_Normal;
    gl_Position = u_Matrices.projectionMatrix * u_Matrices.viewMatrix * u_PushConstants.modelMatrix * vec4(in_Position, 1.0);
}
