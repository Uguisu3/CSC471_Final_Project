#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform int xpos;
uniform int ypos;
uniform sampler2D tex;


void main()
{
    color = texture(tex, + vec2( (vertex_tex.x + xpos)/8,(vertex_tex.y + ypos)/23));
}