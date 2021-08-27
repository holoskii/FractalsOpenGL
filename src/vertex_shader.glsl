#version 330 core
in vec3 inPos;

// does nothing
void main()
{
   gl_Position = vec4(inPos, 1.0);
}