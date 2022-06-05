#version 450

layout(push_constant) uniform Push
{
    vec3 offset;
    vec3 color;
} push;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUV;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    vec4 positionWorld = ubo.model * vec4(inPosition, 1.0);

    fragNormalWorld = normalize(mat3(ubo.model) * inNormal);
    fragPosWorld = positionWorld.xyz;
    fragColor = inColor;
    fragUV = inUV;
}
