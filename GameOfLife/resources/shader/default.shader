#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

out vec2 TexCoords;

void main() 
{
    gl_Position = position;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void main() 
{
    color = vec4(1.0, 1.0, 1.0, 1.0);
}