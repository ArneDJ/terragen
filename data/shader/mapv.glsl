#version 460 core

uniform mat4 view, model, project;

layout(location = 0) in vec3 vpos;

out vec2 uv;
out vec4 clip;

void main(void)
{
	vec3 newpos = vpos;
//	newpos.y += 10.0;
	uv = newpos.xz;

	clip = project * view * vec4(newpos, 1.0);
	gl_Position = clip;
}


