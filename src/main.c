#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "gmath.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "imp.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

static GLuint make_terrain(unsigned int res)
{
	const size_t size = res * res;
	unsigned char *image = calloc(size, sizeof(unsigned char));
	unsigned char *cpy = calloc(size, sizeof(unsigned char));

	int nbuf = 0;
	for(int x = 0; x < res; x++) {
		for(int y = 0; y < res; y++) {
			float z = fbm_noise(0.5*x, 0.5*y, 0.005, 2.5, 2.0);
			if (z > 0.55) {
				z = 1.0;
			} else {
				z = 0.0;
			}
			image[nbuf++] = z * 255.0;
		}
	}

	const int MIN_LAKE_SIZE = res * 2;
	const int MIN_ISLAND_SIZE = res;
	const unsigned char red = 255.0;
	const unsigned char black = 0.0;
	const unsigned char half = 125.0;

	/* remove small lakes */
	memcpy(cpy, image, size);
	for (int x = 0; x < res; x++) {
		for (int y = 0; y < res; y++) {
			int index = y * res + x ;
			int size = floodfill(x, y, cpy, res, res, black, red);
			if (size < MIN_LAKE_SIZE && size > 1) {
				floodfill(x, y, image, res, res, black, red);
			}
		}
	}

	/* remove small islands */
	memcpy(cpy, image, size);
	for (int x = 0; x < res; x++) {
		for (int y = 0; y < res; y++) {
			int index = y * res + x ;
			int size = floodfill(x, y, cpy, res, res, red, black);
			if (size < MIN_ISLAND_SIZE && size > 1) {
				floodfill(x, y, image, res, res, red, black);
			}
		}
	}


	GLuint texnum = make_r_texture(image, res, res);

	free(image);
	free(cpy);
	return texnum;
}

static void run_loop(SDL_Window *window)
{
	float start, end = 0.0;
	SDL_SetRelativeMouseMode(SDL_TRUE);
	struct camera cam = init_camera(1.0, 1.0, 1.0, 90.0, 0.2);

	struct mesh cube = make_grid_mesh(1, 1, 10.0);
	GLuint texture = make_terrain(2048);
	//GLuint texture = make_voronoi_texture(4096, 4096);

	struct shader pipeline[] = {
		{GL_VERTEX_SHADER, "data/shader/cubev.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/cubef.glsl"},
		{GL_NONE, NULL}
	};

	GLuint program = load_shaders(pipeline);

	mat4 model = IDENTITY_MATRIX;
	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "project"), 1, GL_FALSE, project.f);
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, model.f);

	SDL_Event event;
	while (event.type != SDL_QUIT) {
		start = (float)SDL_GetTicks();
		float delta = start - end;
		while(SDL_PollEvent(&event));

		update_free_camera(&cam, 0.001 * delta);
		mat4 view = make_view_matrix(cam.eye, cam.center, cam.up);

		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, view.f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(cube.VAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, cube.vcount);

		SDL_GL_SwapWindow(window);
		end = start;
	}

}

static SDL_Window *init_window(int width, int height)
{
	SDL_Window *window;

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Terragen", 0, 0, 
			WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	if (window == NULL) {
		printf("error: could not create window: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	return window;
}

static SDL_GLContext init_glcontext(SDL_Window *window)
{
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

	if (glewInit()) {
		fprintf(stderr, "unable to init glew\n");
		exit(EXIT_FAILURE);
	}

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return glcontext;
}

int main(int argc, char *argv[])
{
	SDL_Window *window = init_window(WINDOW_WIDTH, WINDOW_HEIGHT);
	SDL_GLContext glcontext = init_glcontext(window);

	run_loop(window);

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	exit(EXIT_SUCCESS);
}
