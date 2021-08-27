#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inCoord0;

out vec2 inCoord;

void main()
{
   gl_Position = vec4(inPos, 1.0);
   inCoord = inCoord0;
}