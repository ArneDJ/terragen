#version 460 core

layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vnorm;
layout(location = 2) in vec2 vtex;
layout(location = 3) in mat4 instance_model;

uniform mat4 view, model, project;

void main(void)
{
	gl_Position = project * view * instance_model * vec4(vpos, 1.0);
}


