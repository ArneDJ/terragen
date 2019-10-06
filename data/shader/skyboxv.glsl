#version 460 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vnorm;
layout(location = 2) in vec2 vtex;

uniform mat4 view, project;

out vec3 fpos;
out vec2 uv;

void main(void)
{
	fpos = vpos;
	uv = vtex;
	vec4 pos = project * view * vec4(vpos, 1.0);
	gl_Position = pos.xyww;
}


