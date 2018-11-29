#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;


void main()
{
    vec3 n = normalize(vertex_normal);
    vec3 lp=vec3(10,-20,-400);
    vec3 ld = normalize(vertex_pos - lp);
    float diffuse = dot(n,ld);

    color = texture(tex, vertex_tex);
    color.a =(color.r + color.g + color.b)/3;
    color *= diffuse*1.0;



}
