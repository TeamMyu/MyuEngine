#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform Push
{
    vec3 offset;
    vec3 color;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outColor;

void main() {
    vec3 inPosition_w = (ubo.model * vec4(inPosition, 1.0)).xyz; 
    vec3 inNormal_w = normalize(mat3(ubo.model) * inNormal);
    vec3 surfaceNormal = normalize(inNormal_w);
    vec3 cameraPos_w = -ubo.view[3].xyz;
    vec3 viewDirection = (cameraPos_w - inPosition_w);
    float viewDistance = normalize(dot(viewDirection, viewDirection));

    vec3 offset = inNormal * push.offset.x * viewDistance;
    outColor = push.color;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition+offset, 1.0);
    
}