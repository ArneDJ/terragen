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

vec3 apply_fog( in vec3  rgb,      // original color of the pixel
               in float dist, // camera to point distance
               in vec3  rayDir,   // camera to point vector
               in vec3  sunDir )  // sun light direction
{
	float b = 0.03;
    float fogAmount = 1.0 - exp( -dist*b );
    float sunAmount = max( dot( rayDir, sunDir ), 0.0 );
    vec3  fogColor  = mix( vec3(0.46, 0.7, 0.99), // bluish
                           vec3(1.0,0.9,0.7), // yellowish
                           pow(sunAmount,8.0) );
    return mix( rgb, fogColor, fogAmount );
}

vec3 apply_fog_depth( in vec3  rgb,      // original color of the pixel
               in float dist, // camera to point distance
               in vec3  rayOri,   // camera position
               in vec3  rayDir )  // camera to point vector
{
float b = 1.0;
float c = 0.001;
    float fogAmount = c * exp(-rayOri.y*b) * (1.0-exp( -dist*rayDir.y*b ))/rayDir.y;
    vec3  fogColor  = vec3(0.46, 0.7, 0.99);
    return mix(rgb, fogColor, fogAmount);
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

	float diffuse = orenNayarDiffuse(light_dir, view_dir, bump, 0.5, 1.2);
	material *= diffuse;

	vec3 fog_color = {0.46, 0.7, 0.99};

	//float fogf = fog(fpos, view_eye);
	//material = mix(fog_color, material, fogf);
	vec3 dist = vec3(distance(fpos.x, view_eye.x), distance(fpos.y, view_eye.y), distance(fpos.z, view_eye.z));
	material = apply_fog(material, length(dist), normalize(fpos.xyz - view_eye), vec3(0.0, -1.0, 0.0));
	//material = apply_fog_depth(material, length(dist), view_eye, fpos.xyz - view_eye);

	gl_FragColor = vec4(material, 1.0);
}
