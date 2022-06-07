#version 450

layout(binding = 1) uniform sampler2D diffuseMap;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(vec3(0.1) * texture(diffuseMap, fragCoord).rgb, 10.0);
}
