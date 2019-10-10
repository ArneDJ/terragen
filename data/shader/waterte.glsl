#version 460 core

layout(triangles) in;
uniform mat4 model, view, project;
uniform float time;

float wave(vec2 xy, float t)
{
	float A = 0.5;
	float w = 1.0;
	float p = 1.0;
	vec2 D = vec2(1.0, 0.5);
	return A * sin(dot(D, xy) * w + t * p);
}

void main(void)
{
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
			gl_TessCoord.y * gl_in[1].gl_Position +
			gl_TessCoord.z * gl_in[2].gl_Position);

	gl_Position.y = wave(gl_Position.xz, time) + 2.0;
	gl_Position = project * view * model * gl_Position;
}

