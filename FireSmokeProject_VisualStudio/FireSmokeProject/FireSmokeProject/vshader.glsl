#version 410

in vec4 vPosition;
in vec3 vNormal;

uniform mat4 ModelView;
uniform mat4 Projection;

// Texture Mapping
in vec2 vTexCoord;
out vec2 texCoord;

void main() 
{
    // Transform vertex position into camera (eye) coordinates
    vec3 pos = (ModelView * vPosition).xyz;
    gl_Position = Projection * ModelView * vPosition;
}