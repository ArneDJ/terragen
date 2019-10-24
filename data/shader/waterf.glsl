#version 460 core

uniform float time;
uniform sampler2D water_normal;
uniform vec3 view_dir;
uniform vec3 view_eye;
uniform vec3 sunDirection = {1.0, 1.0, 1.0};
uniform vec3 sunColor = {1.0, 0.6, 0.5};
uniform vec3 waterColor = {0.0, 0.47, 0.75};

in vec3 fpos;
in vec2 uv;
in vec3 normal;
in float terrain_h;

void sunLight(const vec3 surfaceNormal, const vec3 eyeDirection, float shiny, float spec, float diffuse, inout vec3 diffuseColor, inout vec3 specularColor){
    vec3 reflection = normalize(reflect(-sunDirection, surfaceNormal));
    float direction = max(0.0, dot(eyeDirection, reflection));
    //specularColor += pow(direction, shiny)*sunColor*spec;
    diffuseColor += max(dot(sunDirection, surfaceNormal),0.0)*sunColor*diffuse;
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
	vec4 bump = texture(water_normal, uv);
	bump.xyz = normalize(bump.xyz);
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	vec3 worldToEye = view_eye - fpos;
	vec3 eyeDirection = normalize(worldToEye);
	sunLight(bump.xyz, eyeDirection, 100.0, 2.0, 0.5, diffuse, specular);

    	vec3 reflection = normalize(reflect(-sunDirection, bump.xyz));
	float direction = max(0.0, dot(eyeDirection, reflection));
	specular += pow(direction, 100.0) * sunColor * 2.0;

	float water_depth = 1.0 - terrain_h;
	//water_depth = clamp(water_depth, 0.6, 1.0);
	water_depth = smoothstep(0.3, 0.55, water_depth);

	vec3 material = (diffuse+specular+vec3(0.1))*waterColor;

	vec3 fog_color = {0.46, 0.7, 0.99};

	float fogf = fog(fpos, view_eye);
	material = mix(fog_color, material, fogf);

	gl_FragColor = vec4(material, water_depth);
}
