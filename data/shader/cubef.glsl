#version 460 core

uniform sampler2D tex;
uniform vec3 fcolor;

in vec3 norm;
in vec2 uv;

const vec3 sun_dir = vec3(1.0, 1.0, 1.0);

void main(void)
{
	vec2 new_uv = {uv.x, 1.0 - uv.y};
	vec4 boxf = texture(tex, new_uv);;

	gl_FragColor = vec4(fcolor + boxf.xyz, 1.0);
}
