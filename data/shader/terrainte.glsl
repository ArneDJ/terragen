#version 460 core
#define OCTAVES 6

layout(quads, fractional_even_spacing, ccw) in;

out vec2 uv;
out float height;
out vec3 fpos;

uniform mat4 view, project;
uniform sampler2D heightmap;


float random(in vec2 st)
{
	return fract(sin(dot(st.xy, vec2(1.9898, 78.233)))*43758.5453123);
}

float noise(in vec2 st)
{
	vec2 i = floor(st);
	vec2 f = fract(st);

	float a = random(i);
	float b = random(i + vec2(1.0, 0.0));
	float c = random(i + vec2(0.0, 1.0));
	float d = random(i + vec2(1.0, 1.0));

	vec2 u = f * f * (3.0 - 2.0 * f);

	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(in vec2 st)
{
	float displacement = 0.0;
	float amplitude = 0.3;
	float frequency = 2.0;
	float lacunarity = 2.0;
	float gain = 0.5;

	for(int i = 0; i < OCTAVES; i++) {
		displacement += amplitude * abs(noise(frequency*st));
		st *= lacunarity;
		amplitude *= gain;
	}

	return displacement;
}


void main(void)
{
	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.y);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.y);
	vec4 pos = mix(p1, p2, gl_TessCoord.x);
	
	const float texsize = 1.0 / 64.0;
	vec2 texcoord = pos.xz;
	uv = texcoord;
	height = texture(heightmap, texsize * texcoord).x;
	pos.y = height;
	pos.y *= 2.0;
	pos.y += 1.0;
//	pos.y += 1.5 * fbm(0.2*pos.xz);

	fpos = pos.xyz;
	gl_Position = project * view * pos;
}
