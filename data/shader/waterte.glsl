#version 460 core

layout(triangles) in;
uniform mat4 view, project;
uniform sampler2D terrain_height;
uniform float heightmap_scale = 0.015625;

out vec2 uv;
out vec3 normal;
out vec3 fpos;
out vec4 clip;
out float terrain_h;

void main(void)
{
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
			gl_TessCoord.y * gl_in[1].gl_Position +
			gl_TessCoord.z * gl_in[2].gl_Position);

	vec3 wave_pos = gl_Position.xyz;
	wave_pos.y += 1.2;
	uv = wave_pos.xz;
	fpos = wave_pos;
	terrain_h = texture(terrain_height, uv * heightmap_scale).r;
	clip = project * view * vec4(wave_pos, 1.0);
	gl_Position = clip;
}

