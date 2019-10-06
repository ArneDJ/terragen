#version 460 core

uniform sampler2D tex;

in vec3 fpos;
in vec2 uv;

void main(void)
{
	vec3 horizon = {0.46, 0.7, 0.99};
	vec3 zenith = {0.0, 0.5, 1.0};

	vec3 atmosphere = normalize(fpos);
	float a = -atmosphere.y;
	vec3 color = mix(horizon, zenith, a);
	gl_FragColor = vec4(color, 1.0);
}
