#version 460 core

layout(triangles) in;
uniform mat4 model, view, project;
uniform float time;

out vec2 uv;
out vec3 normal;
out vec3 fpos;

void main(void)
{
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
			gl_TessCoord.y * gl_in[1].gl_Position +
			gl_TessCoord.z * gl_in[2].gl_Position);

	vec3 wave_pos = gl_Position.xyz;
	wave_pos.y = wave_pos.y + 0.95;
	uv = wave_pos.xz;
	fpos = wave_pos;
	gl_Position = project * view * model * vec4(wave_pos, 1.0);
}

