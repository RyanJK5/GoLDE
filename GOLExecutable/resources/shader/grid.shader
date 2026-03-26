#shader vertex
#version 330 core

layout (location = 0) in vec2 a_QuadPos;
layout (location = 1) in vec2 a_CellRelPos; // Relative offsets from Camera
layout (location = 2) in float a_Opacity;

uniform mat4 u_MVP;
uniform vec2 u_CellSize;
uniform float u_CellScale;

out float v_Opacity;

void main()
{
    vec2 scaledCell = u_CellSize * u_CellScale;
    vec2 worldPos = a_CellRelPos * scaledCell + a_QuadPos * scaledCell;
    gl_Position = u_MVP * vec4(worldPos, 0.0, 1.0);
    v_Opacity = a_Opacity;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_Color;
in float v_Opacity;

void main() 
{
    color = vec4(u_Color.rgb, u_Color.a * v_Opacity);
}