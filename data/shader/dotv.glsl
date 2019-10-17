#version 460 core

layout(location = 0) in vec2 vpos;

void main(void)
{
	vec3 newpos = vec3(0.0, 0.0, 0.0);
	gl_Position = vec4(newpos, 1.0);
}


