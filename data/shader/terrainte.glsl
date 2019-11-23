#version 460 core
#define OCTAVES 6

layout(triangles) in;
uniform sampler2D heightmap;
uniform sampler2D rivers;
uniform mat4 view, project;
uniform float heightmap_scale = 0.015625;

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

vec4 permute(vec4 x)
{
	return mod ((34.0 * x + 1.0) * x , 289.0) ;
}

vec2 cellular(vec2 P)
{
	const float K = 1.0/7.0;
	const float K2 = 0.5/7.0;
	const float jitter = 0.8; // jitter 1.0 makes F1 wrong more often
	vec2 Pi = mod ( floor ( P ) , 289.0) ;
	vec2 Pf = fract ( P ) ;
	vec4 Pfx = Pf . x + vec4 ( -0.5 , -1.5 , -0.5 , -1.5) ;
	vec4 Pfy = Pf . y + vec4 ( -0.5 , -0.5 , -1.5 , -1.5) ;
	vec4 p = permute ( Pi . x + vec4 (0.0 , 1.0 , 0.0 , 1.0) ) ;
	p = permute ( p + Pi . y + vec4 (0.0 , 0.0 , 1.0 , 1.0) ) ;
	vec4 ox = mod (p , 7.0) * K + K2 ;
	vec4 oy = mod ( floor ( p * K ) ,7.0) * K + K2 ;
	vec4 dx = Pfx + jitter * ox ; vec4 dy = Pfy + jitter * oy ; vec4 d = dx * dx + dy * dy ; // d i s t a n c e s squared
	// Cheat and pick only F1 for the return value
	d.xy = min ( d.xy , d.zw ) ;
	d.x = min ( d.x , d.y ) ;
	return d.xx ; // F1 duplicated , F2 not computed
}

/* DEPRECATED
vec3 filter_normal(vec2 uv, float texel_size, float ampl)
{
	vec4 h;
	float B = ampl * textureOffset(heightmap, uv * texel_size, ivec2(0, -4)).r;
	float L = ampl * textureOffset(heightmap, uv * texel_size, ivec2(-4, 0)).r;
	float R = ampl * textureOffset(heightmap, uv * texel_size, ivec2(4, 0)).r;
	float T = ampl * textureOffset(heightmap, uv * texel_size, ivec2(0, 4)).r;
	vec3 n;
	n.z = (B - T);
	n.x = (L - R);
	n.y = 2.0;

	return normalize(n);
}
*/

vec3 filter_normal(vec2 uv, float texel_scale, float amp)
{
	const ivec3 offset = ivec3(-1, 0, 1);
	float delta = 2.0 * amp / textureSize(heightmap, 0 ).x;
	float left = amp * textureOffset(heightmap, uv * heightmap_scale, offset.xy).r;
	float right = amp * textureOffset(heightmap, uv * heightmap_scale, offset.zy).r;
	float top = amp * textureOffset(heightmap, uv * heightmap_scale, offset.yz).r;
	float bottom = amp * textureOffset(heightmap, uv * heightmap_scale, offset.yx).r;

	vec3 x = normalize( vec3( delta, right - left, 0.0 ) );
	vec3 z = normalize( vec3( 0.0, top - bottom, delta ) );
	vec3 n = cross( z, x );

	return normalize(n);
}

void main(void)
{
	float amplitude = 8.0;
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
			gl_TessCoord.y * gl_in[1].gl_Position +
			gl_TessCoord.z * gl_in[2].gl_Position);

	uv = gl_Position.xz;
	height = texture(heightmap, uv * heightmap_scale).r;
	//float river_h = texture(rivers, uv * 0.001953).r;
	const float ratio = 0.00195;
	float river_h = texture(rivers, uv * heightmap_scale).r;

	vec3 newpos = gl_Position.xyz;

	newpos.y = amplitude * height;
	fpos = newpos;
	gl_Position = project * view * vec4(newpos, 1.0);
}

