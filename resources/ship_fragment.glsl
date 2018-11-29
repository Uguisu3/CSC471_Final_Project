#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;
uniform sampler2D tex3;

void main()
{
vec3 n = normalize(vertex_normal);
vec3 lp=vec3(10,-20,-100);
vec3 ld = normalize(vertex_pos - lp);
float diffuse = dot(n,ld);

color = texture(tex, vertex_tex)*diffuse + texture(tex3,vertex_tex)*(1-diffuse);

color *= diffuse*1.0;
color += vec4(.5,.5,.5,1);

vec3 cd = normalize(vertex_pos - campos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec,1000);
color += vec4(1,1,1,1)*spec*3 *texture(tex2,vertex_tex).r;
color.a = 1;



}
