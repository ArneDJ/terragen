#version 460 core

uniform float time;
uniform sampler2D water_normal;
uniform vec3 view_dir;
uniform vec3 view_eye;
uniform vec3 sunDirection = {1.0, 1.0, 1.0};
uniform vec3 sunColor = {1.0, 0.5, 0.5};
uniform vec3 waterColor = {0.0, 0.5, 0.52};

in vec3 fpos;
in vec2 uv;
in vec3 normal;
in float terrain_h;

void sunLight(const vec3 surfaceNormal, const vec3 eyeDirection, float shiny, float spec, float diffuse, inout vec3 diffuseColor, inout vec3 specularColor){
    vec3 reflection = normalize(reflect(-sunDirection, surfaceNormal));
    float direction = max(0.0, dot(eyeDirection, reflection));
    //specularColor += pow(direction, shiny)*sunColor*spec;
    diffuseColor += max(dot(sunDirection, surfaceNormal),0.0)*diffuse;
}

vec3 fog(vec3 c, float dist, float height)
{
	vec3 fog_color = {0.46, 0.7, 0.99};
	float de = 0.015 * smoothstep(0.0, 3.3, 8.0 - height);
	float di = 0.015 * smoothstep(0.0, 5.5, 8.0 - height);
	float extinction = exp(-dist * de);
	float inscattering = exp(-dist * di);
	return c * extinction + fog_color * (1.0 - inscattering);
}

void main(void)
{
	//vec4 bump1 = texture(water_normal, uv + (0.0001*time));
	vec2 D1 = vec2(1.0, 0.0) * 0.0001 * time;
	vec2 D2 = vec2(0.0, 0.1) * 0.0001 * time;
	vec2 D3 = vec2(1.0, -1.0) * 0.0001 * time;
	vec2 D4 = vec2(0.3, -0.5) * 0.0001 * time;
	vec4 bump1 = texture(water_normal, uv + D1);
	vec4 bump2 = texture(water_normal, uv + D2);
	vec4 bump3 = texture(water_normal, uv + D3);
	vec4 bump4 = texture(water_normal, uv + D4);
	vec4 bump = bump1 + bump2 + bump3 + bump4;
	bump.xyz = normalize(bump.xyz);
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	vec3 worldToEye = view_eye - fpos;
	vec3 eyeDirection = normalize(worldToEye);
	sunLight(bump.xyz, eyeDirection, 100.0, 2.0, 0.5, diffuse, specular);

    	vec3 reflection = normalize(reflect(-sunDirection, bump.xyz));
	float direction = max(0.0, dot(eyeDirection, reflection));
	specular += pow(direction, 25.0) * sunColor * 0.5;

	float water_depth = 1.0 - terrain_h;
	//water_depth = clamp(water_depth, 0.6, 1.0);
	water_depth = smoothstep(0.5, 0.7, water_depth);
	vec3 watercolor2 = {0.0, 0.35, 0.36};
	vec3 material = mix(waterColor, watercolor2, water_depth);

	material *= (diffuse+specular);

	vec3 fog_color = {0.46, 0.7, 0.99};

	vec3 view_space = vec3(distance(fpos.x, view_eye.x), distance(fpos.y, view_eye.y), distance(fpos.z, view_eye.z));
	material = fog(material, length(view_space), fpos.y), 

	material = pow(material, vec3(1.0/1.5));

	gl_FragColor = vec4(material, 1.0);
}
