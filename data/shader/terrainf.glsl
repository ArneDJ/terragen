#version 460 core
#define PI 3.14159265

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

const vec3 light_dir = vec3(1.0, 1.0, 1.0);
const float scale = 0.015625;

float orenay(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness, float albedo) 
{
	float LdotV = dot(lightDirection, viewDirection);
	float NdotL = dot(lightDirection, surfaceNormal);
	float NdotV = dot(surfaceNormal, viewDirection);

	float s = LdotV - NdotL * NdotV;
	float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

	float sigma2 = roughness * roughness;
	float A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
	float B = 0.45 * sigma2 / (sigma2 + 0.09);

	return albedo * max(0.0, NdotL) * (A + B * s / t) / PI;
}

float fog(vec3 fpos, vec3 view_pos)
{
	vec3 dist = vec3(distance(fpos.x, view_pos.x), distance(fpos.y, view_pos.y), distance(fpos.z, view_pos.z));
	float z = length(dist);
	float d = 0.03;
	float fog_factor = exp(-(d*z)*(d*z));
	return clamp(fog_factor, 0.0, 1.0);
}

vec3 fog(vec3 c, float dist, float height)
{
	vec3 fog_color = {0.46, 0.7, 0.99};
	float de = 0.025 * smoothstep(0.0, 3.3, 8.0 - height);
	float di = 0.045 * smoothstep(0.0, 5.5, 8.0 - height);
	float extinction = exp(-dist * de);
	float inscattering = exp(-dist * di);
	return c * extinction + fog_color * (1.0 - inscattering);
}

void main(void)
{
	float amp = 8.0;
	// The slope is one minus the y-component of the normal.
	// 1 = 90 degrees, 0 = 0 degrees
	float slope = 1.0 - bump.y;
	//slope = clamp(slope, 0.5, 1.0);

	vec3 grassf = texture(grass, uv).xyz;
	vec3 stonef = texture(stone, uv).xyz;
	vec3 sandf = texture(sand, uv).xyz;
	vec3 snowf = texture(snow, 0.1 * uv).xyz;

	float darkness = smoothstep(amp * 0.6, amp * 0.75, fpos.y);
	grassf = mix(grassf, vec3(0.2, 0.3, 0.2), darkness);

	vec3 material = mix(grassf, snowf, smoothstep(0.65 * amp, 0.81 * amp, fpos.y));
	material = mix(material, stonef, smoothstep(0.0, 0.08, slope));
	material = mix(sandf, material, smoothstep(0.44 * amp, 0.54 * amp, fpos.y));

	vec3 view_dir = normalize(view_eye - fpos);

	float diffuse = orenay(light_dir, view_dir, bump, 0.5, 1.2);
	material *= diffuse;

	vec3 fog_color = {0.46, 0.7, 0.99};

	vec3 view_space = vec3(distance(fpos.x, view_eye.x), distance(fpos.y, view_eye.y), distance(fpos.z, view_eye.z));
	float dist = length(view_space);
	material = fog(material, dist, fpos.y);

	gl_FragColor = vec4(material, 1.0);
}
