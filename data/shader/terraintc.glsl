#version 460 core

layout(vertices = 4) out;

uniform sampler2D heightmap;
uniform vec2 screen_size = {1920, 1080};
uniform float lod_factor = 1280.0;
uniform mat4 project, view;
uniform vec3 view_eye;

float lod(float dist)
{
	//return clamp(1/dist * lod_factor, 1, 64);
	//return clamp(exp(-0.02 * dist) * lod_factor, 1, 64);
	float z = length(dist);
 	float d = 0.02; // distance factor, if large the terrain will "pop" in more
 	return exp(-(d*z)*(d*z)) * lod_factor;
}

void main(void)
{
	const mat4 PV = project * view;
	vec4 e0 = (gl_in[0].gl_Position + gl_in[1].gl_Position / 4.0);
	vec4 e1 = (gl_in[0].gl_Position + gl_in[2].gl_Position / 4.0);
	vec4 e2 = (gl_in[2].gl_Position + gl_in[3].gl_Position / 4.0);
	vec4 e3 = (gl_in[1].gl_Position + gl_in[3].gl_Position / 4.0);

	vec3 c = vec3(e1.x, e1.y, e0.z);
	float tesselation = 4.0;

	if (gl_InvocationID == 0) {
		gl_TessLevelInner[0] = lod(distance(c, view_eye));
		gl_TessLevelInner[1] = lod(distance(c, view_eye));
		gl_TessLevelOuter[0] = lod(distance(e0.xyz, view_eye));
		gl_TessLevelOuter[1] = lod(distance(e1.xyz, view_eye));
		gl_TessLevelOuter[2] = lod(distance(e2.xyz, view_eye));
		gl_TessLevelOuter[3] = lod(distance(e3.xyz, view_eye));
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
