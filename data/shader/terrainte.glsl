#version 460 core
#define OCTAVES 6

layout(triangles) in;
uniform sampler2D heightmap;
uniform sampler2D range;
uniform mat4 view, project;

out vec3 bump;
out vec3 fpos;
out vec2 uv;
out float height;

float mask;

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

vec3 filter_normal(vec2 uv, float texel_size, float ampl)
{
	vec4 h;
	ivec2 off1 = ivec2(10, 0);
	ivec2 off2 = ivec2(-10, 0);
	h.x = (1.0 + mask) * ampl * textureOffset(heightmap, uv * texel_size, off2.yx).r;
	h.y = (1.0 + mask) * ampl * textureOffset(heightmap, uv * texel_size, off2.xy).r;
	h.z = (1.0 + mask) * ampl * textureOffset(heightmap, uv * texel_size, off1.xy).r;
	h.w = (1.0 + mask) * ampl * textureOffset(heightmap, uv * texel_size, off1.yx).r;
	vec3 n;
	n.z = h.x - h.w;
	n.x = h.y - h.z;
	n.y = 2;

	return normalize(n);
}

void main(void)
{
	float amplitude = 8.0;
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
			gl_TessCoord.y * gl_in[1].gl_Position +
			gl_TessCoord.z * gl_in[2].gl_Position);

	uv = gl_Position.xz;
	height = texture(heightmap, uv * 0.015625).r;
	mask = texture(range, uv * 0.015625).r;
//	mask = mask * mask;
	mask = mask * height;
	if (height < 0.58) {
		mask  = mask * pow(2.0, height * height * height * height);
	}

	vec3 newpos = gl_Position.xyz;
	bump = filter_normal(uv, 0.015625, amplitude);
	newpos.y = amplitude * (1.0 + mask) * height;
	fpos = newpos;
	gl_Position = project * view * vec4(newpos, 1.0);
}

