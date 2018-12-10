#version 330 core
out vec4 color;
in vec2 vertex_tex;
uniform int xpos;
uniform int ypos;
uniform sampler2D tex;

void main()
{
    color = texture(tex, + vec2( (vertex_tex.x + xpos)/13,(vertex_tex.y + ypos)/6));
}