#version 450 core

layout (binding = 1) uniform sampler2D shadowMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragPositionInLightSpace;
layout(location = 0) out vec4 outColor;

float calculateShadowFactor() {
    vec3 projectedCoordinates = fragPositionInLightSpace.xyz / fragPositionInLightSpace.w;
    float closestDepth = texture(shadowMap, projectedCoordinates.xy).r;
    float currentDepth = projectedCoordinates.z;
    return currentDepth > closestDepth ? 0.8 : 0.0;
}

void main() {
    float shadowFactor = calculateShadowFactor();
    float lightFactor = max(dot(fragNormal, vec3(0.57735026919, 0.57735026919, 0.57735026919)), 0.2);
    outColor = vec4(lightFactor * (1.0 - shadowFactor) * fragColor, 1.0);
}
