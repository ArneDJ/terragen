#version 460 core

layout(location = 0) in vec4 vpos;

void main(void)
{
	gl_Position = vpos;
}
