#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec4 color;

void main()
{
    FragColor = texture(image, TexCoords) * color;
} 