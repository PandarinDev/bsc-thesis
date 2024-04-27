#version 450 core

layout(location = 0) in float in_LightFactor;
layout(location = 0) out vec4 out_Color;

void main() {
    out_Color = vec4(
        in_LightFactor * 0.063879,
        in_LightFactor * 0.481435,
        in_LightFactor * 0.817017,
        0.5);
}