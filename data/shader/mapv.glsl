#version 460 core

uniform mat4 view, model, project;

layout(location = 0) in vec3 vpos;

out vec2 uv;

void main(void)
{
	vec3 newpos = vpos;
//	newpos.y += 10.0;
	uv = newpos.xz;

	gl_Position = project * view * vec4(newpos, 1.0);
}


