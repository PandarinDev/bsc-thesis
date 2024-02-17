#version 450 core

layout (binding = 1) uniform sampler2D shadowMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 0) out vec4 outColor;

void main() {
    float lightFactor = max(dot(fragNormal, vec3(0.57735026919, 0.57735026919, 0.57735026919)), 0.2);
    outColor = vec4(lightFactor * fragColor, 1.0);
}
