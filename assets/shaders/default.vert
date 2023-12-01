#version 450 core

layout(binding = 0) uniform Matrices {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 modelMatrix;
} u_Matrices;

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(-3.0, -3.0),
    vec2(3.0, -3.0),
    vec2(0.0, 3.0)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = u_Matrices.projectionMatrix * u_Matrices.viewMatrix * u_Matrices.modelMatrix * vec4(positions[gl_VertexIndex], -5.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
