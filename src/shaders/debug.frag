#version 330 core
out vec4 color_frag;

uniform vec3 color_border;

void
main()
{
  color_frag = vec4(color_border, 1.0f);
}
