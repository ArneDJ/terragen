#version 460 core
#define PI 3.14159265

uniform float heightmap_scale = 0.015625;
uniform sampler2D heightmap;
uniform sampler2D grass;
uniform sampler2D checker;
uniform sampler2D stone;
uniform sampler2D sand;
uniform sampler2D snow;
uniform vec3 view_center;
uniform vec3 view_eye;

in vec3 fpos;
in vec3 bump;
in vec2 uv;
in float height;

const vec3 light_dir = vec3(0.0, 1.0, -1.0);

vec3 fog(vec3 c, float dist, float height)
{
	vec3 fog_color = {0.46, 0.7, 0.99};
	float de = 0.035 * smoothstep(0.0, 3.3, 8.0 - height);
	float di = 0.035 * smoothstep(0.0, 5.5, 8.0 - height);
	float extinction = exp(-dist * de);
	float inscattering = exp(-dist * di);
	return c * extinction + fog_color * (1.0 - inscattering);
}

vec3 filter_normal(vec2 uv, float texel_scale, float amp)
{
	const ivec3 offset = ivec3(-1, 0, 1);
	float delta = 24.0 * amp / textureSize(heightmap, 0 ).x;
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
	float amp = 8.0;
	// The slope is one minus the y-component of the normal.
	// 1 = 90 degrees, 0 = 0 degrees
	vec3 bump = filter_normal(uv, heightmap_scale, 8.0);
	float slope = 1.0 - bump.y;
	//slope = clamp(slope, 0.5, 1.0);

	vec3 grassf = texture(grass, uv).xyz;
	vec3 stonef = texture(stone, uv).xyz;
	vec3 sandf = texture(sand, uv).xyz;
	vec3 snowf = texture(snow, 0.5 * uv).xyz;

	vec3 material = mix(grassf, snowf, smoothstep(0.63 * amp, 0.71 * amp, fpos.y));
	material = mix(material, stonef, smoothstep(0.4, 0.7, slope));
	material = mix(sandf, material, smoothstep(0.44 * amp, 0.54 * amp, fpos.y));

	vec3 view_dir = normalize(view_eye - fpos);
	float diff = max(dot(bump, normalize(light_dir)), 0.0);
	material *= diff;

	vec3 fog_color = {0.46, 0.7, 0.99};

	vec3 view_space = vec3(distance(fpos.x, view_eye.x), distance(fpos.y, view_eye.y), distance(fpos.z, view_eye.z));
	float dist = length(view_space);
	material = fog(material, dist, fpos.y);

	material = pow(material, vec3(1.0/1.5));
	gl_FragColor = vec4(material, 1.0);
}
