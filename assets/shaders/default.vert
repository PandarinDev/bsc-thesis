#version 450 core

uniform mat4 u_ProjectionMatrix;

layout(location = 0) in vec3 in_Position;

void main() {
    gl_Position = u_ProjectionMatrix * vec4(in_Position, 1.0);
}
