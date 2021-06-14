#version 410

uniform vec4 color;
out vec4 fcolor;

void main()
{
    // fcolor = texture2D(tex, texCoord);
    fcolor = color;
}