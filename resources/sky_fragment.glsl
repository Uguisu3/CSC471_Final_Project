#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex1;

void main()
{

color = texture(tex1, vertex_tex);
color.a = 1;

}
