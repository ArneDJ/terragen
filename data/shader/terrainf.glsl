#version 460 core
#define PI 3.14159265

vec3 fog(vec3 c, float dist, float height)
{
	vec3 fog_color = {0.46, 0.7, 0.99};
	float de = 0.035 * smoothstep(0.0, 3.3, 8.0 - height);
	float di = 0.035 * smoothstep(0.0, 5.5, 8.0 - height);
	float extinction = exp(-dist * de);
	float inscattering = exp(-dist * di);
	return c * extinction + fog_color * (1.0 - inscattering);
}

vec3 filter_normal(vec2 uv, float ratio, float ampl, sampler2D heightm)
{
	const ivec3 offset = ivec3(-1, 0, 1);
	float delta = 24.0 * ampl / textureSize(heightm, 0 ).x;
	float left = ampl * textureOffset(heightm, uv * ratio, offset.xy).r;
	float right = ampl * textureOffset(heightm, uv * ratio, offset.zy).r;
	float top = ampl * textureOffset(heightm, uv * ratio, offset.yz).r;
	float bottom = ampl * textureOffset(heightm, uv * ratio, offset.yx).r;

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
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
