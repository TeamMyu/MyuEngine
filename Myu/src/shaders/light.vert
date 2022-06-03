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
    float Ns; // 반사율
    float Ni; // 굴절률
    vec3 Ka; // ambient
    vec3 Kd; // diffuse
    vec3 Ks; // specular
    float d; // 투명도
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


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;

void calcDiffuse(inout vec3 diffuseLight, vec3 inNormal_w, vec3 inPosition_w, vec3 color , float intensity, float ambient){
    diffuseLight += color * (intensity * max(dot(inNormal_w, inPosition_w), ambient));
}

void main() {
    // _w 접미어 : world 좌표계
    vec3 inPosition_w = (ubo.model * vec4(inPosition, 1.0)).xyz; 
    vec3 inNormal_w = normalize(mat3(ubo.model) * inNormal);
    vec3 surfaceNormal = normalize(inNormal_w);
    vec3 cameraPos_w = -ubo.view[3].xyz;
    vec3 viewDirection = normalize(cameraPos_w - inPosition_w);

    float directionalLight_itensity = 0.0; // 전역라이트 : 감쇠없음 
    float diffuseLight_itensity = 0.0;
    float ambientLight_itensity = 0.0; 
    float specularLight_itensity = 0.0;
    float rimLight_itensity = 0.0;

    vec3 directionalLight = vec3(0.0);
    vec3 diffuseLight = vec3(0.0);
    vec3 ambientLight = vec3(0.0);
    vec3 specularLight = vec3(0.0);
    vec3 rimLight = vec3(0.0);
    vec3 outline = vec3(0.0);

    for (int i = 0; i < 10; i++) {
        if (ubo.gLight[i].position.w == 0.0) break;
        if (ubo.gLight[i].position.w == 2.0) { // this is light
            ambientLight = ubo.gLight[i].color.xyz;
            break;
        }

        GlobalLight light = ubo.gLight[i];

        calcDiffuse(directionalLight, inNormal_w, light.position.xyz, light.color.xyz , light.color.w, 0.1);
    }

    fragColor = directionalLight;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    
}
