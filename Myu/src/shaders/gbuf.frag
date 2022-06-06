#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outColor;
layout (location = 3) out vec4 outUV;

void main() 
{
	outPosition = vec4(inPosition, 1.0);
    outNormal = vec4(inNormal, 1.0);
    outColor = vec4(inColor, 1.0);
    outUV = vec4(inUV, vec2(0.0));
}
