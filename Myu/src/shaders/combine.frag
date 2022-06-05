#version 450


layout (binding = 0) uniform sampler2D colorMap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
    //outColor = texture(colorMap, inUV).rgba;
}
