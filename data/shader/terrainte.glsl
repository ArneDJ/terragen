#version 460 core

layout(triangles) in;
uniform sampler2D heightmap;
uniform mat4 model, view, project;
uniform float amplitude = 2.0;

out vec3 bump;
out vec3 fpos;
out vec2 uv;

vec3 filter_normal(vec2 uv, float texel_size)
{
	vec4 h;
	ivec2 off1 = ivec2(10, 0);
	ivec2 off2 = ivec2(-10, 0);
	h.x = amplitude * textureOffset(heightmap, uv * texel_size, off2.yx).r;
	h.y = amplitude * textureOffset(heightmap, uv * texel_size, off2.xy).r;
	h.z = amplitude * textureOffset(heightmap, uv * texel_size, off1.xy).r;
	h.w = amplitude * textureOffset(heightmap, uv * texel_size, off1.yx).r;
	vec3 n;
	n.z = h.x - h.w;
	n.x = h.y - h.z;
	n.y = 2;

	return normalize(n);
}

void main(void)
{
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
			gl_TessCoord.y * gl_in[1].gl_Position +
			gl_TessCoord.z * gl_in[2].gl_Position);

	uv = gl_Position.xz;
	float height = texture(heightmap, uv * 0.015625).r;
	bump = filter_normal(uv, 0.015625);
	vec3 newpos = gl_Position.xyz;
	newpos.y = amplitude * height;
	gl_Position = project * view * model * vec4(newpos, 1.0);
}

