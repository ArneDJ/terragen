#version 460 core

layout(vertices = 4) out;

uniform vec2 screen_size = {1920, 1080};
uniform float lod_factor = 16.0;
uniform mat4 project, view;

vec4 projection(vec4 vertex)
{
    vec4 result = project * view * vertex;
    result /= result.w;
    return result;
}

vec2 screen_space(vec4 vertex){
    return (clamp(vertex.xy, -1.3, 1.3)+1) * (screen_size*0.5);
}

float level(vec2 v0, vec2 v1){
     return clamp(distance(v0, v1)/lod_factor, 1, 64);
 }

/*
void main(void)
{
	const float tesselation = 16.0;
	if (gl_InvocationID == 0) {
		gl_TessLevelInner[0] = tesselation;
		gl_TessLevelInner[1] = tesselation;
		gl_TessLevelOuter[0] = tesselation;
		gl_TessLevelOuter[1] = tesselation;
		gl_TessLevelOuter[2] = tesselation;
		gl_TessLevelOuter[3] = tesselation;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
*/

void main(){
     #define id gl_InvocationID
     gl_out[id].gl_Position = gl_in[id].gl_Position;
     if(id == 0){
	vec4 v0 = projection(gl_in[0].gl_Position);
	vec4 v1 = projection(gl_in[1].gl_Position);
	vec4 v2 = projection(gl_in[2].gl_Position);
	vec4 v3 = projection(gl_in[3].gl_Position);

	vec2 ss0 = screen_space(v0);
	vec2 ss1 = screen_space(v1);
	vec2 ss2 = screen_space(v2);
	vec2 ss3 = screen_space(v3);

	float e0 = level(ss1, ss2);
	float e1 = level(ss0, ss1);
	float e2 = level(ss3, ss0);
	float e3 = level(ss2, ss3);

	gl_TessLevelInner[0] = mix(e1, e2, 0.5);
	gl_TessLevelInner[1] = mix(e0, e3, 0.5);
	gl_TessLevelOuter[0] = e0;
	gl_TessLevelOuter[1] = e1;
	gl_TessLevelOuter[2] = e2;
	gl_TessLevelOuter[3] = e3;
     }
 }
