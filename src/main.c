#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "shader.h"
#include "mesh.h"
#include "gmath.h"
#include "camera.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960

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

	struct camera cam = init_camera(0.0, 0.0, 0.0, 90.0, 0.2);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_Event event;
	float start, end = 0.0;
	while(event.type != SDL_QUIT) {
		start = (float)SDL_GetTicks() * 0.001;
		float delta = start - end;
		while(SDL_PollEvent(&event));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update_camera(&cam, delta);
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
