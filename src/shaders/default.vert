#version 330 core
layout (location = 0) in vec3 pos_vert;

uniform mat4 transform;

void main()
{
  gl_Position = transform * vec4(pos_vert, 1.0f);
}
