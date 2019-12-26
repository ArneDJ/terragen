#version 460 core

uniform sampler2D tex;

in vec3 norm;
in vec2 uv;

void main(void)
{
	vec4 boxf = texture(tex, uv * 0.1);

	gl_FragColor = vec4(boxf.xyz, 1.0);
}
