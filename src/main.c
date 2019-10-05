#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>

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
	SDL_Event event;

	while(event.type != SDL_QUIT) {
		while(SDL_PollEvent(&event));
		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	exit(EXIT_SUCCESS);
}
