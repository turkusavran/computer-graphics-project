#version 410

in vec2 texCoord;

uniform sampler2D tex;

out vec4 fcolor;

void main()
{
    fcolor = texture2D(tex, texCoord);
}