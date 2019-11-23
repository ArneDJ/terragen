#version 460 core
#define OCTAVES 6

layout(quads) in;

out vec2 uv;

uniform mat4 view, project;
uniform sampler2D heightmap;

void main(void)
{
	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.y);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.y);
	vec4 pos = mix(p1, p2, gl_TessCoord.x);
	
	const float texsize = 1.0 / 128.0;
	vec2 texcoord = pos.xz;
	uv = texcoord;
	float height = texture(heightmap, texsize * texcoord).x;
	pos.y = height;
	pos.y *= 8.0;
	pos.y += 1.0;

	gl_Position = project * view * pos;
	/*
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec4 a = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, u);
	vec4 b = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
	vec4 position = mix(a, b, v);
	//texcoord = position.xy;
	//float height = texture(terrain, texcoord).a;
	gl_Position = project * view * vec4(position.xyz, 1.0);
	*/
}
