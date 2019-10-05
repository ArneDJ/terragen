#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "shader.h"
#include "mesh.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 960

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("SDL2/OpenGL Demo", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

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

	SDL_Event event;
	while(event.type != SDL_QUIT) {
		while(SDL_PollEvent(&event));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(cube_program);
		glBindVertexArray(cube.VAO);
		glDrawArrays(GL_TRIANGLES, 0, cube.vcount);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	exit(EXIT_SUCCESS);
}
