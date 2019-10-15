#version 460 core
#define OCTAVES 6

layout(triangles) in;
uniform sampler2D heightmap;
uniform mat4 view, project;
uniform float amplitude = 8.0;

out vec3 bump;
out vec3 fpos;
out vec2 uv;
out float height;

float random(in vec2 st) {
	return fract(sin(dot(st.xy, vec2(1.9898,78.233)))*43758.5453123);
}

float noise(in vec2 st) {
	vec2 i = floor(st);
	vec2 f = fract(st);

	// Four corners in 2D of a tile
	float a = random(i);
	float b = random(i + vec2(1.0, 0.0));
	float c = random(i + vec2(0.0, 1.0));
	float d = random(i + vec2(1.0, 1.0));

	vec2 u = f * f * (3.0 - 2.0 * f);

	return mix(a, b, u.x) +
	    (c - a)* u.y * (1.0 - u.x) +
	    (d - b) * u.x * u.y;
}

float fbm(in vec2 st) {
	// Initial values
	float value = 0.0;
	float amplitude = 1.0;
	float frequency = 1.0;
	float lacunarity = 2.0;
	float gain = 0.5;
	//
	// Loop of octaves
	for (int i = 0; i < OCTAVES; i++) {
		value += amplitude * abs(noise(st*frequency));
		st *= lacunarity;
		amplitude *= gain;
	}
	return value;
}

vec3 filter_normal(vec2 uv, float texel_size)
{
	vec4 h;
	ivec2 off1 = ivec2(10, 0);
	ivec2 off2 = ivec2(-10, 0);
	h.x = amplitude * textureOffset(heightmap, uv * texel_size, off2.yx).r;
	h.y = amplitude * textureOffset(heightmap, uv * texel_size, off2.xy).r;
	h.z = amplitude * textureOffset(heightmap, uv * texel_size, off1.xy).r;
	h.w = amplitude * textureOffset(heightmap, uv * texel_size, off1.yx).r;
	vec3 n;
	n.z = h.x - h.w;
	n.x = h.y - h.z;
	n.y = 2;

	return normalize(n);
}

void main(void)
{
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
			gl_TessCoord.y * gl_in[1].gl_Position +
			gl_TessCoord.z * gl_in[2].gl_Position);

	uv = gl_Position.xz;
	height = texture(heightmap, uv * 0.015625).r;
	bump = filter_normal(uv, 0.015625);
	vec3 newpos = gl_Position.xyz;
	newpos.y = amplitude * height;
	fpos = newpos;
	gl_Position = project * view * vec4(newpos, 1.0);
}

