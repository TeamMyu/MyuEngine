#version 450

layout(binding = 1) uniform sampler2D texSampler;


layout(push_constant) uniform Push
{
    vec2 offset;
    vec3 color;
} push;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
//    outColor = vec4(0.0, 0.0, gl_FragDepth, 1.0);
    outColor = texture(texSampler, fragTexCoord);
//    outColor = vec4(fragColor, 1.0);
}
