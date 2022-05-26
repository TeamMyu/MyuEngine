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

layout(binding = 1) uniform sampler2D diffuseMap;
layout(binding = 2) uniform sampler2D ambientMap;
layout(binding = 3) uniform sampler2D specularMap;
layout(binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;

void main() {
    // _w 접미어 : world 좌표계
    vec3 inPosition_w = (ubo.model * vec4(inPosition, 1.0)).xyz; 
    vec3 inNormal_w = normalize(mat3(ubo.model) * inNormal);
    vec3 surfaceNormal = normalize(inNormal_w);
    vec3 cameraPos_w = -ubo.view[3].xyz;
    vec3 viewDirection = normalize(cameraPos_w - inPosition_w.xyz);

    float directionalLight_itensity = 0.0; // 전역라이트 : 감쇠없음 
    float diffuseLight_itensity = 0.0;
    float ambientLight_itensity = 0.0; 
    float specularLight_itensity = 0.0;
    float rimLight_itensity = 0.0;

    vec3 directionalLight = vec3(0.0);
    vec3 diffuseLight = vec3(0.0);
    vec3 ambientLight = texture(diffuseMap, inUV).rgb; 
    vec3 specularLight = vec3(0.0);
    vec3 rimLight = vec3(0.0);

    for (int i = 0; i < 10; i++) {
        if (ubo.gLight[i].position.w == 0.0) break;

        directionalLight += ubo.gLight[i].color.xyz * (ubo.gLight[i].color.w * max(dot(inNormal_w.xyz, ubo.gLight[i].position.xyz), 0.1));
    }

    for (int i = 0; i < 10; i++) {
        if (ubo.pLight[i].position.w == 0.0) break;

        PointLight light = ubo.pLight[i];

        vec3 directionToLight = light.position.xyz - inPosition_w; // vector (Vertex World Position -> Light)
        float attenuation = 1.0 / dot(directionToLight, directionToLight); // 감쇠수치 = 거리 (같은거 dot production 하면 distance squared)
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0); // 노멀라이즈 된 두 벡터를 dot production 하면 각도가 같으면 1, 수직일때 0
        vec3 intensity = light.color.xyz * light.color.w * attenuation; // 거리에따라 감쇠 적용

        diffuseLight += intensity * cosAngIncidence; // 포인트라이트 -> 버텍스 표면 각도에따라 빛의 세기 결정

        // specular lighting
        vec3 halfAngle = normalize(directionToLight + viewDirection); // 
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 512.0); // higher values -> sharper highlight
        specularLight += intensity * blinnTerm;
    }
 
    fragColor += ambientLight + directionalLight + diffuseLight + specularLight;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    
}
