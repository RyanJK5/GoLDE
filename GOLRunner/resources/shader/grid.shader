#shader vertex
#version 330 core

layout (location = 0) in vec2 a_QuadPos;
layout (location = 1) in vec2 a_CellPos;

uniform mat4 u_MVP;
uniform vec2 u_CellSize;

void main()
{
    vec2 worldPos = (a_CellPos + a_QuadPos) * u_CellSize;
    gl_Position = u_MVP * vec4(worldPos, 0.0, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_Color;

void main() 
{
    color = u_Color;
}