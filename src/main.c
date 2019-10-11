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
	SDL_Window *window = SDL_CreateWindow("SDL2/OpenGL Demo", 0, 0, 
			WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

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

	struct shader shaders[] = {
		{GL_VERTEX_SHADER, "data/shader/cubev.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/cubef.glsl"},
		{GL_NONE, NULL}
	};
	struct shader skybox_info[] = {
		{GL_VERTEX_SHADER, "data/shader/skyboxv.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/skyboxf.glsl"},
		{GL_NONE, NULL}
	};
	struct shader terrain_info[] = {
		{GL_VERTEX_SHADER, "data/shader/terrainv.glsl"},
		{GL_TESS_CONTROL_SHADER, "data/shader/terraintc.glsl"},
		{GL_TESS_EVALUATION_SHADER, "data/shader/terrainte.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/terrainf.glsl"},
		{GL_NONE, NULL}
	};
	struct shader water_info[] = {
		{GL_VERTEX_SHADER, "data/shader/waterv.glsl"},
		{GL_TESS_CONTROL_SHADER, "data/shader/watertc.glsl"},
		{GL_TESS_EVALUATION_SHADER, "data/shader/waterte.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/waterf.glsl"},
		{GL_NONE, NULL}
	};
	GLuint cube_program = load_shaders(shaders);
	GLuint skybox_program = load_shaders(skybox_info);
	GLuint terrain_program = load_shaders(terrain_info);
	GLuint water_program = load_shaders(water_info);

	struct mesh cube = make_cube_mesh();
	struct AABB box = {
		{5.0, 5.0, 5.0},
		{0.5, 0.5, 0.5},
	};
	struct sphere s = {
		{0.0, 0.0, 0.0},
		0.5
	};

	GLuint heightmap_generated = make_heightmap_texture();
	GLuint wood_texture = load_dds_texture("data/texture/placeholder.dds");
	GLuint heightmap_texture = load_dds_texture("data/texture/heightmap.dds");
	GLuint grass_texture = load_dds_texture("data/texture/grass.dds");
	GLuint stone_texture = load_dds_texture("data/texture/stone.dds");
	GLuint water_texture = load_dds_texture("data/texture/water.dds");
	GLuint water_n_texture = load_dds_texture("data/texture/water_n.dds");

	struct mesh skybox = make_cube_mesh();
	struct mesh plane = make_grid_mesh(64, 64, 1.0);

	mat4 model = IDENTITY_MATRIX;
	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(skybox_program);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "project"), 1, GL_FALSE, project.f);
	glUseProgram(terrain_program);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "project"), 1, GL_FALSE, project.f);
	glUniformMatrix4fv(glGetUniformLocation(terrain_program, "model"), 1, GL_FALSE, model.f);
	glUseProgram(water_program);
	glUniformMatrix4fv(glGetUniformLocation(water_program, "project"), 1, GL_FALSE, project.f);
	glUniformMatrix4fv(glGetUniformLocation(water_program, "model"), 1, GL_FALSE, model.f);

	glUseProgram(cube_program);
	glUniformMatrix4fv(glGetUniformLocation(cube_program, "project"), 1, GL_FALSE, project.f);
	mat4_translate(&model, box.c);
	glUniformMatrix4fv(glGetUniformLocation(cube_program, "model"), 1, GL_FALSE, model.f);

	struct camera cam = init_camera(10.0, 5.0, 10.0, 90.0, 0.2);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_Event event;
	float start, end = 0.0;
	while (event.type != SDL_QUIT) {
		start = (float)SDL_GetTicks() * 0.001;
		float delta = start - end;
		while(SDL_PollEvent(&event));
		const Uint8 *keystates = SDL_GetKeyboardState(NULL);

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
		glBindTexture(GL_TEXTURE_2D, heightmap_generated);
		glUniform1i(glGetUniformLocation(terrain_program, "grass"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, grass_texture);
		glUniform1i(glGetUniformLocation(terrain_program, "stone"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, stone_texture);
		glBindVertexArray(plane.VAO);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_PATCHES, 0, plane.vcount);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//
		//
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUseProgram(water_program);
		glUniform1f(glGetUniformLocation(water_program, "time"), start);
		glUniform3fv(glGetUniformLocation(water_program, "view_dir"), 1, cam.center.f);
		glUniform3fv(glGetUniformLocation(water_program, "eye"), 1, cam.eye.f);
		glUniformMatrix4fv(glGetUniformLocation(water_program, "view"), 1, GL_FALSE, view.f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, water_n_texture);
		glBindVertexArray(plane.VAO);
		glDrawArrays(GL_PATCHES, 0, plane.vcount);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		vec3 fcolor = {0.0, 0.0, 0.0};
		vec3 p = {10.0, 10.0, 10.0};
		vec3 d = {-5.0, -5.0, -5.0};
		if (test_ray_AABB(cam.eye, cam.center, box)) {
			if (keystates[SDL_SCANCODE_SPACE]) {
				box.c = vec3_sum(cam.eye, cam.center);
			}
			fcolor.y = 0.5;
		}

		mat4_translate(&model, box.c);
		glUseProgram(cube_program);
		glUniform3fv(glGetUniformLocation(cube_program, "fcolor"), 1, fcolor.f);
		glUniformMatrix4fv(glGetUniformLocation(cube_program, "model"), 1, GL_FALSE, model.f);
		glUniformMatrix4fv(glGetUniformLocation(cube_program, "view"), 1, GL_FALSE, view.f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood_texture);
		glBindVertexArray(cube.VAO);
		glDrawArrays(GL_TRIANGLES, 0, cube.vcount);

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
