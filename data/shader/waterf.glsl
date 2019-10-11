#version 460 core

uniform sampler2D water_normal;
uniform vec3 view_dir;
uniform vec3 eye;
uniform vec3 sunDirection = {1.0, 1.0, 1.0};
uniform vec3 sunColor = {1.0, 1.0, 1.0};
uniform vec3 waterColor = {0.0, 0.46, 0.74};

in vec3 fpos;
in vec2 uv;
in vec3 normal;

void sunLight(const vec3 surfaceNormal, const vec3 eyeDirection, float shiny, float spec, float diffuse, inout vec3 diffuseColor, inout vec3 specularColor){
    vec3 reflection = normalize(reflect(-sunDirection, surfaceNormal));
    float direction = max(0.0, dot(eyeDirection, reflection));
    specularColor += pow(direction, shiny)*sunColor*spec;
    diffuseColor += max(dot(sunDirection, surfaceNormal),0.0)*sunColor*diffuse;
}

void main(void)
{
	vec4 bump = texture(water_normal, uv);
	bump.xyz = normalize(bump.xyz);
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	vec3 worldToEye = eye - fpos;
	vec3 eyeDirection = normalize(worldToEye);
	sunLight(bump.xyz, eyeDirection, 100.0, 2.0, 0.5, diffuse, specular);

	gl_FragColor = vec4((diffuse+specular+vec3(0.1))*waterColor, 1.0);
}
