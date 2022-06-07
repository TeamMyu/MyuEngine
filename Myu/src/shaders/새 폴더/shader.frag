#version 450

layout(binding = 1) uniform sampler2D diffuseMap;
layout(binding = 5) uniform sampler2D computeMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragCoord;

layout(location = 0) out vec4 outColor;


void main() {
    outColor = vec4(fragColor + texture(diffuseMap, fragCoord).rgb, 1.0);
    //outColor = vec4(0.1 * texture(diffuseMap, fragCoord).rgb + texture(computeMap, fragCoord).rgb, 1.0);
    //outColor = vec4(fragColor, 1.0);
}
