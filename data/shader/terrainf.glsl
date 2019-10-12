#version 460 core
#define PI 3.14159265

uniform sampler2D heightmap;
uniform sampler2D grass;
uniform sampler2D checker;
uniform sampler2D stone;
uniform sampler2D sand;
uniform vec3 view_dir;

in vec3 fpos;
in vec3 bump;
in vec2 uv;
in float height;

vec3 sun_dir = vec3(1.0, 1.0, 1.0);

void main(void)
{
	// The slope is one minus the y-component of the normal.
	// 1 = 90 degrees, 0 = 0 degrees
	float slope = 1.0 - bump.y;
	//slope = clamp(slope, 0.5, 1.0);

	vec3 grassf = texture(grass, uv).xyz;
	vec3 stonef = texture(stone, uv).xyz;
	vec3 sandf = texture(sand, uv).xyz;
	vec3 diffuse = mix(grassf, stonef, smoothstep(0.01, 0.04, slope));
	diffuse = mix(sandf, diffuse, smoothstep(0.58, 0.61, height));

	vec3 normal = vec3(0.0, 1.0, 0.0);
	float exposure = max(dot(bump, sun_dir), 0.5);

//	vec3 diffuse = texture(checker, uv * 0.5).xyz;
	diffuse *= exposure;

	gl_FragColor = vec4(diffuse, 1.0);
}
