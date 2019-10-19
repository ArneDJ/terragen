#version 460 core

uniform sampler2D voronoi;

in vec2 uv;

void main(void)
{
	const float penis = 0.015625;
	vec4 ftexture = texture(voronoi, penis * uv);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	gl_FragColor = ftexture;
}

