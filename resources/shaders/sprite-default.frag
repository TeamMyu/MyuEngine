#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D mainTex;
uniform vec4 spriteColor;

void main()
{
    FragColor = texture(mainTex, TexCoord) * spriteColor;
}
