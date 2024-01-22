#version 450 core

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));
    // TODO: This does not look right for fixed normal vectors
    float lightFactor = max(dot(fragNormal, lightDirection), 0.0);
    outColor = vec4(lightFactor * fragColor, 1.0);
}
