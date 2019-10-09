#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "gmath.h"
#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "texture.h"
#include "display.h"

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
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	struct shader_info shaders[] = {
		{GL_VERTEX_SHADER, "data/shader/cubev.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/cubef.glsl"},
		{GL_NONE, NULL}
	};
	struct shader_info skybox_info[] = {
		{GL_VERTEX_SHADER, "data/shader/skyboxv.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/skyboxf.glsl"},
		{GL_NONE, NULL}
	};
	struct shader_info terrain_info[] = {
		{GL_VERTEX_SHADER, "data/shader/terrainv.glsl"},
		{GL_TESS_CONTROL_SHADER, "data/shader/terraintc.glsl"},
		{GL_TESS_EVALUATION_SHADER, "data/shader/terrainte.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/terrainf.glsl"},
		{GL_NONE, NULL}
	};
	GLuint cube_program = load_shaders(shaders);
	GLuint skybox_program = load_shaders(skybox_info);
	GLuint terrain_program = load_shaders(terrain_info);

	const int amount = 400;
	vec3 *positions = calloc(amount, sizeof(vec3));
	int n = 0;
	for(int i = 0; i < 20; i++) {
		for(int j = 0; j < 20; j++) {
			positions[n].x = (float)i + (float)i;
			positions[n].y = 10.0;
			positions[n].z = (float)j + (float)j;
			n++;
		}
	}
	struct mesh cube = make_cube_mesh();
	instance_mesh(&cube, amount, positions);
	free(positions);
	GLuint wood_texture = load_dds_texture("data/texture/wood.dds");
	GLuint heightmap_texture = load_dds_texture("data/texture/heightmap.dds");
	GLuint grass_texture = load_dds_texture("data/texture/grass.dds");
	GLuint stone_texture = load_dds_texture("data/texture/stone.dds");

	struct mesh skybox = make_cube_mesh();
	struct mesh plane = make_grid_mesh(64, 64, 1.0);

	mat4 model = IDENTITY_MATRIX;
	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(cube_program);
	glUniformMatrix4fv(glGetUniformLocation(cube_program, "project"), 1, GL_FALSE, project.f);
	glUniformMatrix4fv(glGetUniformLocation(cube_program, "model"), 1, GL_FALSE, model.f);
	glUseProgram(skybox_program);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "project"), 1, GL_FALSE, project.f);
	glUseProgram(terrain_program);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "project"), 1, GL_FALSE, project.f);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "model"), 1, GL_FALSE, model.f);

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
		mat4 skybox_view = view;
		skybox_view.f[12] = 0.0;
		skybox_view.f[13] = 0.0;
		skybox_view.f[14] = 0.0;

		glUseProgram(terrain_program);
		glUniform3fv(glGetUniformLocation(terrain_program, "view_dir"), 1, cam.center.f);
		glUniformMatrix4fv(glGetUniformLocation(terrain_program, "view"), 1, GL_FALSE, view.f);
		glUniform1i(glGetUniformLocation(terrain_program, "heightmap"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightmap_texture);
		glUniform1i(glGetUniformLocation(terrain_program, "grass"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, grass_texture);
		glUniform1i(glGetUniformLocation(terrain_program, "stone"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, stone_texture);
		glBindVertexArray(plane.VAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_PATCHES, 0, plane.vcount);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUseProgram(cube_program);
		glUniformMatrix4fv(glGetUniformLocation(cube_program, "view"), 1, GL_FALSE, view.f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood_texture);
		glBindVertexArray(cube.VAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, cube.vcount, amount);

		glUseProgram(skybox_program);
		glUniformMatrix4fv(glGetUniformLocation(skybox_program, "view"), 1, GL_FALSE, skybox_view.f);
		display_skybox(skybox);

		SDL_GL_SwapWindow(window);
		end = start;
	}

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	exit(EXIT_SUCCESS);
}
