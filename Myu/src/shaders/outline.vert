#version 450

struct PointLight {
    vec4 position; // w is onoff
    vec4 color; // w is intensity
};

struct GlobalLight {
    vec4 position; // w is onoff
    vec4 color; // w is intensity
};

struct Material {
    float Ns; // ¹Ý»çÀ²
    float Ni; // ±¼Àý·ü
    vec3 Ka; // ambient
    vec3 Kd; // diffuse
    vec3 Ks; // specular
    float d; // Åõ¸íµµ
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 effects[10]; // 0 : effect type, 1 : effect intensity, 2~3 : config value
    PointLight pLight[10];
    GlobalLight gLight[10];
    Material mat;
} ubo;

layout(binding = 1) uniform sampler2D diffuseMap;
layout(binding = 2) uniform sampler2D ambientMap;
layout(binding = 3) uniform sampler2D specularMap;
layout(binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragCoord;

void main() {
    vec3 inPosition_w = (ubo.model * vec4(inPosition, 1.0)).xyz; 
    vec3 inNormal_w = normalize(mat3(ubo.model) * inNormal);
    vec3 surfaceNormal = normalize(inNormal_w);
    vec3 cameraPos_w = -ubo.view[3].xyz;
    vec3 viewDirection = (cameraPos_w - inPosition_w);
    float viewDistance = normalize(dot(viewDirection, viewDirection));

    vec3 offset = inNormal * 0.6;
    offset *= viewDistance;

    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    fragCoord = inUV;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition+offset, 1.0);
    
}
