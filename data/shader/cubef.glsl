#version 460 core

uniform sampler2D tex;

in vec3 norm;
in vec2 uv;

const vec3 sun_dir = vec3(1.0, 1.0, 1.0);

void main(void)
{
	vec4 boxf = texture(tex, uv);
	float exposure = max(dot(norm, sun_dir), 0.6);
	boxf.xyz *= exposure;

	gl_FragColor = boxf;
}
