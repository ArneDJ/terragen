#version 460 core

uniform float subdivision = 16.0;
layout(vertices = 3) out;

void main(void)
{
	if(gl_InvocationID == 0) {
		 gl_TessLevelOuter[0] = subdivision;
		 gl_TessLevelOuter[1] = subdivision;
		 gl_TessLevelOuter[2] = subdivision;

		 gl_TessLevelInner[0] = subdivision;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
