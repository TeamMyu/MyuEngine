#version 450

layout(push_constant) uniform Push
{
    vec3 offset;
    vec3 color;
} push;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));

void main() {
    
    float lightIntensity = max(dot(fragNormalWorld, DIRECTION_TO_LIGHT), 0.1);
    lightIntensity = pow(lightIntensity, push.offset.y);
    if(push.offset.x > 0.0){
        float step = push.color.y / push.offset.x;
        for(float i = push.color.x; i < push.color.y; i+=step){
            if(lightIntensity * push.offset.z < i * push.color.z) {
                lightIntensity = i - step;
                break;
            }
        }
    }
    outColor = vec4(lightIntensity * texture(texSampler, fragUV).rgb, 1.0);
}
