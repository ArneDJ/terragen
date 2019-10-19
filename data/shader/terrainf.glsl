#version 460 core
#define PI 3.14159265

uniform sampler2D heightmap;
uniform sampler2D grass;
uniform sampler2D checker;
uniform sampler2D stone;
uniform sampler2D sand;
uniform sampler2D snow;
uniform sampler2D voronoi;
uniform vec3 view_center;
uniform vec3 view_eye;

in vec3 fpos;
in vec3 bump;
in vec2 uv;
in float height;

const vec3 light_dir = vec3(1.0, 1.0, 1.0);
const float scale = 0.015625;

float orenNayarDiffuse(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness, float albedo) 
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

void main(void)
{
	// The slope is one minus the y-component of the normal.
	// 1 = 90 degrees, 0 = 0 degrees
	float slope = 1.0 - bump.y;
	//slope = clamp(slope, 0.5, 1.0);

	vec3 grassf = texture(grass, uv).xyz;
	vec3 stonef = texture(stone, uv).xyz;
	vec3 sandf = texture(sand, uv).xyz;
	vec3 snowf = texture(snow, uv).xyz;
	vec3 voronoif = texture(voronoi,scale * uv).xyz;

	vec3 material = mix(grassf, snowf, smoothstep(0.7, 0.75, height));
	material = mix(material, stonef, smoothstep(0.0, 0.02, slope));
	material = mix(material, voronoif, 0.4);
	material = mix(sandf, material, smoothstep(0.57, 0.58, height));

	vec3 view_dir = normalize(view_eye - fpos);

	float diffuse = orenNayarDiffuse(light_dir, view_dir, bump, 0.5, 1.2);
	material *= diffuse;

	vec3 fog_color = {0.46, 0.7, 0.99};

	float fogf = fog(fpos, view_eye);
	material = mix(fog_color, material, fogf);

	gl_FragColor = vec4(material, 1.0);
}
