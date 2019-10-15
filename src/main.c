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

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960

static SDL_Window *init_window(int width, int height);
static SDL_GLContext init_glcontext(SDL_Window *window);

struct object {
	struct mesh m;
	GLuint texture;
	GLuint shader;
	vec3 translation;
	vec3 rotation;
	vec3 scale;
	struct AABB bbox;
};

struct terrain {
	struct mesh m;
	GLuint shader;
	// collision surface
	GLuint heightmap;
	GLuint texture[5];
};

struct water {
	struct mesh m;
	GLuint shader;
	GLuint diffuse;
	GLuint normal;
	GLuint depthmap;
};

struct object make_standard_object(void)
{
	struct object obj;
	struct shader pipeline[] = {
		{GL_VERTEX_SHADER, "data/shader/cubev.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/cubef.glsl"},
		{GL_NONE, NULL}
	};

	obj.shader = load_shaders(pipeline);
	obj.m = make_cube_mesh();
	vec3 center = {5.0, 5.0, 5.0};
	obj.bbox.c = center;
	obj.bbox.r[0] = 0.5;
	obj.bbox.r[1] = 0.5;
	obj.bbox.r[2] = 0.5;

	obj.texture = load_dds_texture("data/texture/placeholder.dds");

	mat4 model = IDENTITY_MATRIX;
	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(obj.shader);
	glUniformMatrix4fv(glGetUniformLocation(obj.shader, "project"), 1, GL_FALSE, project.f);
	mat4_translate(&model, obj.bbox.c);
	glUniformMatrix4fv(glGetUniformLocation(obj.shader, "model"), 1, GL_FALSE, model.f);

	return obj;
}

struct water make_water(void)
{
	struct water wat;
	struct shader pipeline[] = {
		{GL_VERTEX_SHADER, "data/shader/waterv.glsl"},
		{GL_TESS_CONTROL_SHADER, "data/shader/watertc.glsl"},
		{GL_TESS_EVALUATION_SHADER, "data/shader/waterte.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/waterf.glsl"},
		{GL_NONE, NULL}
	};
	wat.shader = load_shaders(pipeline);
	wat.m = make_grid_mesh(64, 64, 1.0);
	
	wat.normal = load_dds_texture("data/texture/water_normal.dds");

	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(wat.shader);
	glUniformMatrix4fv(glGetUniformLocation(wat.shader, "project"), 1, GL_FALSE, project.f);

	return wat;
}

void display_water(struct water *wat)
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(wat->shader);
	glUniform1i(glGetUniformLocation(wat->shader, "heightmap_terrain"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wat->depthmap);
	glUniform1i(glGetUniformLocation(wat->shader, "water_normal"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, wat->normal);
	glBindVertexArray(wat->m.VAO);
	glDrawArrays(GL_PATCHES, 0, wat->m.vcount);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

struct object make_skybox(void)
{
	struct object skybox = {0};
	struct shader pipeline[] = {
		{GL_VERTEX_SHADER, "data/shader/skyboxv.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/skyboxf.glsl"},
		{GL_NONE, NULL}
	};

	skybox.shader = load_shaders(pipeline);
	mat4 model = IDENTITY_MATRIX;
	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(skybox.shader);
	glUniformMatrix4fv(glGetUniformLocation(skybox.shader, "project"), 1, GL_FALSE, project.f);

	skybox.m = make_cube_mesh();

	return skybox;
}

void display_skybox(struct object *sky)
{
	glUseProgram(sky->shader);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);

	glBindVertexArray(sky->m.VAO);
	glDrawArrays(GL_TRIANGLES, 0, sky->m.vcount);

	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

void display_static_object(struct object *obj)
{
	glUseProgram(obj->shader);
	mat4 model = IDENTITY_MATRIX;
	mat4_translate(&model, obj->translation);

	glBindVertexArray(obj->m.VAO);
	glDrawArrays(GL_TRIANGLES, 0, obj->m.vcount);
}

struct terrain make_terrain(void)
{
	struct terrain ter = {0};

	struct shader pipeline[] = {
		{GL_VERTEX_SHADER, "data/shader/terrainv.glsl"},
		{GL_TESS_CONTROL_SHADER, "data/shader/terraintc.glsl"},
		{GL_TESS_EVALUATION_SHADER, "data/shader/terrainte.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/terrainf.glsl"},
		{GL_NONE, NULL}
	};
	ter.shader = load_shaders(pipeline);
	ter.m = make_grid_mesh(64, 64, 1.0);

	ter.heightmap = make_heightmap_texture();
	ter.texture[0] = load_dds_texture("data/texture/grass.dds");
	ter.texture[1] = load_dds_texture("data/texture/stone.dds");
	ter.texture[2] = load_dds_texture("data/texture/sand.dds");
	ter.texture[3] = load_dds_texture("data/texture/snow.dds");

	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(ter.shader);
	glUniformMatrix4fv(glGetUniformLocation(ter.shader, "project"), 1, GL_FALSE, project.f);

	return ter;
}

void display_terrain(struct terrain *ter)
{
	glUseProgram(ter->shader);

	glUniform1i(glGetUniformLocation(ter->shader, "grass"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ter->texture[0]);

	glUniform1i(glGetUniformLocation(ter->shader, "stone"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ter->texture[1]);

	glUniform1i(glGetUniformLocation(ter->shader, "sand"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ter->texture[2]);

	glUniform1i(glGetUniformLocation(ter->shader, "snow"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, ter->texture[3]);

	glUniform1i(glGetUniformLocation(ter->shader, "heightmap"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ter->heightmap);

	glBindVertexArray(ter->m.VAO);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_PATCHES, 0, ter->m.vcount);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void run_game(SDL_Window *window)
{
	struct object skybox = make_skybox();
	struct terrain terrain = make_terrain();
	struct water water = make_water();
	struct object box = make_standard_object();
	water.depthmap = terrain.heightmap;

	struct camera cam = init_camera(10.0, 5.0, 10.0, 90.0, 0.2);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_Event event;
	float start, end = 0.0;
	while (event.type != SDL_QUIT) {
		start = (float)SDL_GetTicks() * 0.001;
		float delta = start - end;

		while(SDL_PollEvent(&event));
		const Uint8 *keystates = SDL_GetKeyboardState(NULL);
		cam.speed = 1.0;
		if (keystates[SDL_SCANCODE_LSHIFT]) {
			cam.speed = 4.0;
		}

		update_camera(&cam, delta);
		mat4 view = make_view_matrix(cam.eye, cam.center, cam.up);
		mat4 skybox_view = view;
		skybox_view.f[12] = 0.0;
		skybox_view.f[13] = 0.0;
		skybox_view.f[14] = 0.0;

		vec3 fcolor = {0.0, 0.0, 0.0};
		if (test_ray_AABB(cam.eye, cam.center, box.bbox)) {
			if (keystates[SDL_SCANCODE_SPACE]) {
				box.bbox.c = vec3_sum(cam.eye, cam.center);
			}
			fcolor.y = 0.5;
		}

		glUseProgram(terrain.shader);
		glUniformMatrix4fv(glGetUniformLocation(terrain.shader, "view"), 1, GL_FALSE, view.f);
		glUseProgram(water.shader);
		glUniformMatrix4fv(glGetUniformLocation(water.shader, "view"), 1, GL_FALSE, view.f);
		glUseProgram(box.shader);
		glUniformMatrix4fv(glGetUniformLocation(box.shader, "view"), 1, GL_FALSE, view.f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(terrain.shader);
		glUniform3fv(glGetUniformLocation(terrain.shader, "view_center"), 1, cam.center.f);
		glUniform3fv(glGetUniformLocation(terrain.shader, "view_eye"), 1, cam.eye.f);
		display_terrain(&terrain);

		mat4 model = IDENTITY_MATRIX;
		mat4_translate(&model, box.bbox.c);
		glUseProgram(box.shader);
		glUniform3fv(glGetUniformLocation(box.shader, "fcolor"), 1, fcolor.f);
		glUniformMatrix4fv(glGetUniformLocation(box.shader, "model"), 1, GL_FALSE, model.f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, box.texture);
		glBindVertexArray(box.m.VAO);
		glDrawArrays(GL_TRIANGLES, 0, box.m.vcount);

		glUseProgram(water.shader);
		glUniform1f(glGetUniformLocation(water.shader, "time"), start);
		glUniform3fv(glGetUniformLocation(water.shader, "view_dir"), 1, cam.center.f);
		glUniform3fv(glGetUniformLocation(water.shader, "view_eye"), 1, cam.eye.f);
		display_water(&water);

//		glDisable(GL_BLEND);

		glUseProgram(skybox.shader);
		glUniformMatrix4fv(glGetUniformLocation(skybox.shader, "view"), 1, GL_FALSE, skybox_view.f);
		display_skybox(&skybox);

		SDL_GL_SwapWindow(window);
		end = start;
	}
}

int main(int argc, char *argv[])
{
	SDL_Window *window = init_window(WINDOW_WIDTH, WINDOW_HEIGHT);
	SDL_GLContext glcontext = init_glcontext(window);

	run_game(window);

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	exit(EXIT_SUCCESS);
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

	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return glcontext;
}
