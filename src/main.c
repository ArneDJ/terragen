#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "shader.h"
#include "mesh.h"
#include "gmath.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define CAMERA_SPEED 2

struct camera {
	float yaw;
	float pitch;
	vec3 center;
	vec3 up;
	vec3 eye;
};

enum {
	FORWARD = 26,
	BACKWARD = 22,
	RIGHT = 7,
	LEFT = 4,
};

static void view(struct camera *cam, float delta)
{
	int x, y;
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);
	SDL_GetRelativeMouseState(&x, &y);

	cam->yaw += (float)x * 0.1 * delta;
	cam->pitch -= (float)y * 0.1 * delta;
	cam->pitch = (cam->pitch > 1.57) ? 1.57 : (cam->pitch < -1.57) ? -1.57 : cam->pitch;

	/* point the camera in a direction based on mouse input */
	cam->center.x = cos(cam->yaw) * cos(cam->pitch);
	cam->center.y = sin(cam->pitch);
	cam->center.z = sin(cam->yaw) * cos(cam->pitch);		
	cam->center = vec3_normalize(cam->center);

	vec3 cross = vec3_crossn(cam->center, cam->up);

	cam->center = vec3_scale(CAMERA_SPEED * delta, cam->center);
	cross = vec3_scale(CAMERA_SPEED * delta, cross);

	if(keystates != NULL) {
	if(keystates[FORWARD]) cam->eye = vec3_sum(cam->eye, cam->center);
	if(keystates[BACKWARD]) cam->eye = vec3_sub(cam->eye, cam->center);
	if(keystates[RIGHT]) cam->eye = vec3_sum(cam->eye, cross);
	if(keystates[LEFT]) cam->eye = vec3_sub(cam->eye, cross);
	}
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("SDL2/OpenGL Demo", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	if (window == NULL) {
		printf("error: could not create window: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_GLContext glcontext = SDL_GL_CreateContext(window);
	if (glewInit()) {
		fprintf(stderr, "unable to init glew\n");
		exit(EXIT_FAILURE);
	}
	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);

	struct shader_info shaders[] = {
		{GL_VERTEX_SHADER, "data/shader/cubev.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/cubef.glsl"},
		{GL_NONE, NULL}
	};
	GLuint cube_program = load_shaders(shaders);
	struct mesh cube = make_cube_mesh();

	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(cube_program);
	glUniformMatrix4fv(glGetUniformLocation(cube_program, "project"), 1, GL_FALSE, project.f);
	mat4 model = IDENTITY_MATRIX;
	glUniformMatrix4fv(glGetUniformLocation(cube_program, "model"), 1, GL_FALSE, model.f);

	struct camera cam = {
		0.0, 0.0,
		{0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0},
	};
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_Event event;
	float start, end = 0.0;
	while(event.type != SDL_QUIT) {
		start = (float)SDL_GetTicks() * 0.001;
		float delta = start - end;
		while(SDL_PollEvent(&event));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		view(&cam, delta);
		mat4 view = make_view_matrix(cam.eye, cam.center, cam.up);

		glUseProgram(cube_program);

		glUniformMatrix4fv(glGetUniformLocation(cube_program, "view"), 1, GL_FALSE, view.f);
		glBindVertexArray(cube.VAO);
		glDrawArrays(GL_TRIANGLES, 0, cube.vcount);

		SDL_GL_SwapWindow(window);
		end = start;
	}

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	exit(EXIT_SUCCESS);
}
