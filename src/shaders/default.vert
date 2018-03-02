#version 330 core
layout (location = 0) in vec3 pos_vert;

void main()
{
  gl_Position = vec4(pos_vert, 1.0f);
}
