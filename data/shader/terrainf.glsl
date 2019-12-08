#version 460 core
#define PI 3.14159265

uniform sampler2D heightmap;
uniform sampler2D grass;
uniform sampler2D stone;
uniform sampler2D snow;
uniform sampler2D gravel;
uniform vec3 view_eye;

in vec2 uv;
in float height;
in vec3 fpos;

vec3 fog(vec3 c, float dist, float height)
{
	vec3 fog_color = {0.46, 0.7, 0.99};
	float de = 0.035 * smoothstep(0.0, 3.3, 8.0 - height);
	float di = 0.035 * smoothstep(0.0, 5.5, 8.0 - height);
	float extinction = exp(-dist * de);
	float inscattering = exp(-dist * di);
	return c * extinction + fog_color * (1.0 - inscattering);
}

vec3 filter_normal(vec2 texcoords, float ratio, float ampl, sampler2D heightm)
{
	const ivec3 offset = ivec3(-1, 0, 1);
	float detail = 64.0;
	float delta = detail * ampl / textureSize(heightm, 0 ).x;
	float left = ampl * textureOffset(heightm, texcoords * ratio, offset.xy).r;
	float right = ampl * textureOffset(heightm, texcoords * ratio, offset.zy).r;
	float top = ampl * textureOffset(heightm, texcoords * ratio, offset.yz).r;
	float bottom = ampl * textureOffset(heightm, texcoords * ratio, offset.yx).r;

	vec3 x = vec3( delta, right - left, 0.0 );
	vec3 z = vec3( 0.0, top - bottom, delta );
	vec3 n = cross(z, x);

	return n;
}

// this can have a performance impact
vec3 tri_planar_texture(vec3 wnorm, sampler2D samp, vec3 fragpos)
{
	vec3 blending = abs(wnorm);
	blending = normalize(max(blending, 0.00001)); // Force weights to sum to 1.0
	float b = (blending.x + blending.y + blending.z);
	blending /= vec3(b, b, b);

	vec4 xaxis = texture2D(samp, fragpos.yz);
	vec4 yaxis = texture2D(samp, fragpos.xz);
	vec4 zaxis = texture2D(samp, fragpos.xy);
	// blend the results of the 3 planar projections.
	vec4 tex = xaxis * blending.x + xaxis * blending.y + zaxis * blending.z;

	return tex.xyz;
}

void main(void)
{
	const vec3 light_dir = vec3(-1.0, 1.0, -1.0);
	const float texsize = 1.0 / 64.0;
	vec3 wnorm = filter_normal(uv, texsize, 1.0, heightmap);

	vec4 grassf = texture(grass, uv);
	vec3 stonef = tri_planar_texture(wnorm, stone, fpos);
	vec4 snowf = texture(snow, 0.5 * uv);
	vec4 gravelf = texture(gravel, uv);

	vec3 n = normalize(wnorm);
	float slope = 1.0 - n.y;
	float diff = max(dot(n, normalize(light_dir)), 0.0);
	vec3 material = grassf.xyz;
	material = mix(material, snowf.xyz, smoothstep(0.4, 0.6 , height));
	material = mix(gravelf.xyz, material, smoothstep(0.12, 0.15 , height));
	material = mix(material, stonef.xyz, smoothstep(0.1, 0.7, slope));
	material *= clamp(diff, 0.5, 1.0);

	vec3 view_space = vec3(distance(fpos.x, view_eye.x), distance(fpos.y, view_eye.y), distance(fpos.z, view_eye.z));
 	float dist = length(view_space);
	material = fog(material, dist, fpos.y);

	gl_FragColor = vec4(material, 1.0);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
