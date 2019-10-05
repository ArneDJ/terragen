#version 460 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vnorm;
layout(location = 2) in vec2 vtex;

uniform mat4 view, model, project;

void main(void)
{
	gl_Position = vec4(vpos, 1.0);
}


