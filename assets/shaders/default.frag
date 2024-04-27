#version 450 core

layout (binding = 1) uniform sampler2D shadowMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragPositionInLightSpace;
layout(location = 3) flat in int fragTransparency;
layout(location = 4) flat in float ambientLight;
layout(location = 5) flat in vec3 lightDirection;
layout(location = 0) out vec4 outColor;

float calculateShadowFactor() {
    vec3 projectedCoordinates = fragPositionInLightSpace.xyz / fragPositionInLightSpace.w;
    // While Vulkan NDC Z axis is [0,1] the X and Y axes are [-1,1] which need to be mapped to [0,1]
    projectedCoordinates.x = projectedCoordinates.x * 0.5 + 0.5;
    projectedCoordinates.y = projectedCoordinates.y * 0.5 + 0.5;
    float currentDepth = projectedCoordinates.z;
    if (currentDepth > 1.0) {
        currentDepth = 1.0;
    }
    if (projectedCoordinates.x > 1.0 ||
        projectedCoordinates.x < 0.0 ||
        projectedCoordinates.y > 1.0 ||
        projectedCoordinates.y < 0.0) {
        return 0.0;
    }
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    // PCF implemented by sampling the two neighboring texels also and averaging the result
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float depth = texture(shadowMap, projectedCoordinates.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > depth ? 1.0 : 0.0;
        }
    }

    return min(shadow / 9.0, 0.8) * ambientLight;
}

void main() {
    if (fragTransparency == 1) {
        outColor = vec4(fragColor, 0.1);
        return;
    }
    float shadowFactor = calculateShadowFactor();
    float lightFactor = max(dot(fragNormal, -lightDirection), 0.2);
    float finalLightFactor = ambientLight * lightFactor * (1.0 - shadowFactor);
    outColor = vec4(finalLightFactor * fragColor, 1.0);
}
