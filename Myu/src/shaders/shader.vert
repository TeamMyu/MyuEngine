#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform sampler2D diffuseMap;
layout(binding = 2) uniform sampler2D ambientMap;
layout(binding = 3) uniform sampler2D specularMap;
layout(binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));

void main() {
    vec3 normalWorldSpace = normalize(mat3(ubo.model) * inNormal);

    float lightIntensity = max(dot(normalWorldSpace,DIRECTION_TO_LIGHT), 0);

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    
    fragColor = lightIntensity * texture(diffuseMap, inUV).rgb;
}
