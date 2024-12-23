#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

void main()
{
    float alpha = texture(text, TexCoords).r;  // RED 채널만 사용
    color = vec4(textColor.rgb, textColor.a * alpha);
}