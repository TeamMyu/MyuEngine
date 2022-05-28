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

void calcPointLight(){
}

void calcDiffuse(inout vec3 diffuseLight, vec3 inNormal_w, vec3 inPosition_w, vec3 color , float intensity, float ambient){
    diffuseLight += color * (intensity * max(dot(inNormal_w, inPosition_w), ambient));
}

void calcSpecular(inout vec3 specularLight, vec3 viewDirection, vec3 directionToLight, vec3 surfaceNormal, vec3 intensity, float Ks) {
    vec3 halfAngle = normalize(directionToLight + viewDirection); // 사이각 벡터 = directionToLight(버텍스 표면 -> 라이트) + viewDirection(버텍스 표면 -> 카메라)
    float blinnTerm = dot(surfaceNormal, halfAngle); // 벡터의 내적 = 사이각(최종 빛 세기) 각도가 같으면 1, 수직일때 0
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, Ks); // higher values -> sharper highlight
    specularLight += intensity * blinnTerm;
}

void calcRim(inout vec3 rimLight, vec3 viewDirection, vec3 surfaceNormal, vec3 color, float intensity) {
    // rim은 외각선에만 넣음, 기존 라이트와 동일하게 라이트의 포지션값을 할당하면 기존 diffuse와 중첩되서 적용됨
    float rim  = 1 - clamp(dot(viewDirection, surfaceNormal), 0.0, 1.0); // (카메라라를 빛처럼 연산)을 inverse(모델 외곽으로 갈수록 강한 빛 = 외각선에 강한 빛)
    rim = rim * pow(rim, 2.0); 
    rimLight += color * rim; 
}

void calcRim_v2(inout vec3 rimLight, vec3 viewDirection, vec3 directionToLight, vec3 surfaceNormal, vec3 color, float intensity) {
    float rim  = 1 - clamp(dot(normalize(viewDirection), surfaceNormal), 0.0, 1.0);
    rim = rim * pow(rim, 2.0); 
    rimLight += color * rim; 
}

void calcRim_v3(inout vec3 rimLight, vec3 viewDirection, vec3 directionToLight, vec3 surfaceNormal, vec3 color, float intensity) {
    float rim  = 1 - clamp(dot(normalize(viewDirection), surfaceNormal), 0.0, 1.0);

    rim = rim * pow(rim, 2.0);

    if (rim > 0.8) {
    rim = 10;
    } else {
    rim = 1;
    }
 
    rimLight += vec3(1.0) * rim;  
}

// Fresnel is same as method used by Rim Light
void calcOutline_Fresnel(inout vec3 color, vec3 viewDirection, vec3 directionToLight, vec3 surfaceNormal) {
    float rim  = dot(viewDirection, surfaceNormal);
   
    if (rim < 0.3) {
        vec3 halfAngle = normalize(directionToLight + viewDirection); // 사이각 벡터 = directionToLight(버텍스 표면 -> 라이트) + viewDirection(버텍스 표면 -> 카메라)
        float blinnTerm = dot(surfaceNormal, halfAngle); // 벡터의 내적 = 사이각(최종 빛 세기) 각도가 같으면 1, 수직일때 0
        blinnTerm = clamp(blinnTerm, 0, 1);
        if (blinnTerm < 0.45)
            color = vec3(100.0);
        else
            color = vec3(-100.0);
    }
        
}

void main() {
    // _w 접미어 : world 좌표계
    vec3 inPosition_w = (ubo.model * vec4(inPosition, 1.0)).xyz; 
    vec3 inNormal_w = normalize(mat3(ubo.model) * inNormal);
    vec3 surfaceNormal = normalize(inNormal_w);
    vec3 cameraPos_w = -ubo.view[3].xyz;
    vec3 viewDirection = normalize(cameraPos_w - inPosition_w);

    const float directionalLight_itensity = 0.0; // 전역라이트 : 감쇠없음 
    const float diffuseLight_itensity = 0.0;
    const float ambientLight_itensity = 0.0; 
    const float specularLight_itensity = 0.0;
    const float rimLight_itensity = 0.0;

    vec3 directionalLight = vec3(0.0);
    vec3 diffuseLight = vec3(0.0);
    vec3 ambientLight = texture(diffuseMap, inUV).rgb;
    vec3 specularLight = vec3(0.0);
    vec3 rimLight = vec3(0.0);
    vec3 outline = vec3(0.0);

    for (int i = 0; i < 10; i++) {
        if (ubo.gLight[i].position.w == 0.0) break;

        calcDiffuse(directionalLight, inNormal_w, ubo.gLight[i].position.xyz, ubo.gLight[i].color.xyz , ubo.gLight[i].color.w, 0.1);
    }

    for (int i = 0; i < 10; i++) {
        if (ubo.pLight[i].position.w == 0.0) break;
        if (ubo.pLight[i].position.w == 2.0) { // this is light
            ambientLight = ubo.pLight[i].color.xyz;
            break;
        }

        PointLight light = ubo.pLight[i];

        vec3 directionToLight = light.position.xyz - inPosition_w; // vector (Vertex World Position -> Light)
        float attenuation = 1.0 / dot(directionToLight, directionToLight); // 감쇠수치 = 거리 (같은거 dot production 하면 distance squared)
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0); // 노멀라이즈 된 두 벡터를 dot production 하면 각도가 같으면 1, 수직일때 0
        vec3 intensity = light.color.xyz * light.color.w * attenuation; // 거리에따라 감쇠 적용

        diffuseLight += intensity * cosAngIncidence; // 포인트라이트 -> 버텍스 표면 각도에따라 빛의 세기 결정

        calcSpecular(specularLight, viewDirection, directionToLight, surfaceNormal, intensity, 52.0);

        calcRim_v2(rimLight, cameraPos_w, directionToLight, inNormal_w, light.color.xyz, 1.0);

        //calcOutline_Fresnel(outline, viewDirection, directionToLight, surfaceNormal);
    }
 
    //fragColor = ambientLight + directionalLight + diffuseLight +  specularLight + rimLight - outline;

    fragColor = ambientLight;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    
}
