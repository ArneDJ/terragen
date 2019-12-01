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
#include "noise.h"
#include "voronoi.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define TERRAIN_WIDTH 64
#define HEIGHTMAP_RES 2048

#define DTEX_WIDTH 1024
#define DTEX_HEIGHT 1024

static SDL_Window *init_window(int width, int height);
static SDL_GLContext init_glcontext(SDL_Window *window);

static GLuint depth_texture;

static GLuint vao;

static unsigned char *gen_terrain_map(int size_x, int size_y)
{
	const size_t len = size_x * size_y;
	unsigned char *buf = calloc(len, sizeof(unsigned char));
	float *evaluation = calloc(len, sizeof(float));

	const unsigned char ter_level = 0.25 * 255.0;

	unsigned char *mountainmap = voronoi_mountains(size_x, size_y);

	int nbuf = 0;
	int neval = 0;
	for(int x = 0; x < size_x; x++) {
		for(int y = 0; y < size_y; y++) {
			float z = fbm_noise(0.5*x, 0.5*y, 0.005, 2.5, 2.0);
			evaluation[neval++] = z;
			if (z < 0.55) {
				//z = 0.0;
				buf[nbuf++] = 0.0;
			} else {
				//z = 1.0;
				buf[nbuf++] = ter_level;
			}
		}
	}

	unsigned char *cpy = calloc(len, sizeof(unsigned char));
	const unsigned char greyclr = 100.0;
	const unsigned char lightgrey = 200.0;

	for(int x = 0; x < size_x; x++) {
		for(int y = 0; y < size_y; y++) {
			int index = y * size_y + x;
			if (buf[index] == ter_level) {
				memcpy(cpy, buf, size_x * size_y);
				int size = floodfill(x, y, cpy, size_x, size_y, buf[index], greyclr);
				if (size < 500) {
					floodfill(x, y, buf, size_x, size_y, buf[index], 0.0);
				} else {
					floodfill(x, y, buf, size_x, size_y, buf[index], greyclr);
				}
			}
		} 
	}

	for (int x = 0; x < size_x; x++) {
		for (int y = 0; y < size_y; y++) {
			int index = y * size_y + x;
			if (buf[index] == 0.0) {
				memcpy(cpy, buf, size_x * size_y);
				int size = floodfill(x, y, cpy, size_x, size_y, buf[index], lightgrey);
				if (size < 50000) {
					floodfill(x, y, buf, size_x, size_y, buf[index], ter_level);
				} else {
					floodfill(x, y, buf, size_x, size_y, buf[index], lightgrey);
				}
			}

		}
	}

	for(int x = 0; x < size_x; x++) {
		for(int y = 0; y < size_y; y++) {
			int index = y * size_y + x;
			if (buf[index] == greyclr) {
				floodfill(x, y, buf, size_x, size_y, buf[index], ter_level);

			} else if (buf[index] == lightgrey) {
				floodfill(x, y, buf, size_x, size_y, buf[index], 0.0);
			}
		}
	}

	/*
	for(int x = 0; x < size_x; x++) {
		for(int y = 0; y < size_y; y++) {
			int index = y * size_y + x;
			if (buf[index] == 0.0) {
				buf[index] = 255.0 * 0.25 * evaluation[index];
			}
		}
	}
	*/

	for (int x = 0; x < size_x; x++) {
		for (int y = 0; y < size_y; y++) {
			int index = y * size_y + x;
			int nrange = y * size_y * 3 + x * 3;
			float mountains = (sqrt(worley_noise(0.01*x, 0.015*y)));
		//float ridge = sqrt(worley_noise(x * 0.0015, y * 0.001));

			float range = mountainmap[nrange];
			mountains *= 2.0 * buf[index] / 255.0; 

			mountains *= range;
		//	mountains *= 0.5;

		//	buf[index] += mountains;
			mountains *= 1.75 * 255.0;
			buf[index] = clamp(buf[index]+range, 0.0, 255.0);
		}
	}

	free(mountainmap);
	free(cpy);
	free(evaluation);
	return buf;
}

GLuint make_terrain_texture(int width, int height)
{
	unsigned char *buf = gen_terrain_map(width, height);
	GLuint texnum = make_r_texture(buf, width, height);

	free(buf);

	return texnum;
}

static GLuint init_depth_framebuffer(void)
{
	GLuint depth_fbo;

	glGenFramebuffers(1, &depth_fbo);
	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DTEX_WIDTH, DTEX_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return depth_fbo;
}

float terrain_height(float x, float y, float freq, float lacun, float gain)
{
	float noise = fbm_noise(x, y, freq, lacun, gain);

	// if the frequency is too low the terrain will look "terraced" or "blocky", to prevent this increase the frequancy
	if (noise > 0.48)	
		noise = lerp(0.28, 0.75, noise) + (0.02 * fbm_noise(x*64.0, y*64.0, freq, lacun, gain));
		//noise = lerp(0.28, 0.75, noise);
		//noise = lerp(0.28, 0.75, noise) + (0.02 * worley_noise(0.05*x, 0.05*y));

	float mountains = 1.0 * (1.0 - sqrt(worley_noise(0.01*x, 0.015*y))); //this should always be between 0 an 1
	//mountains *= noise * pow(mountains, noise); // correction so mountains don't spawn in seas
	mountains *= noise;
	float ridge = 1.5 *  sqrt(worley_noise(x * 0.015, y * 0.01));
	ridge *= noise * pow(ridge, noise); 

	float range = fbm_noise(x*1.5, y*1.5, freq, lacun, gain);
	range = smoothstep(0.6, 0.8, range); // sigmoid correction so mountains and terrain transition appears smooth
	range = clamp(range, 0.0, 1.0);

	mountains *= range;
	mountains *= 0.25;

	ridge *= range;
	ridge *= 0.25;

	noise = clamp(noise + ridge + mountains, 0.0, 0.99);
	return noise; //this should always return something between 0 and 1
}
	
struct object {
	struct mesh m;
	GLuint texture;
	GLuint shader;
	vec3 translation;
	vec3 rotation;
	vec3 scale;
	struct AABB bbox;
};

struct map {
	struct mesh m;
	GLuint shader;
	GLuint heightmap;
	GLuint texture;
};

struct terrain {
	struct mesh m;
	GLuint shader;
	struct triangle *surface;
	struct mesh surface_mesh; /* for debugging */
	GLuint heightmap;
	GLuint texture[5];
};

struct water {
	struct mesh m;
	GLuint shader;
	GLuint diffuse;
	GLuint normal;
	GLuint depthmap;
	GLuint depth_fbo;
};

struct scene {
	struct terrain terrain;
	struct water water;
	struct object skybox;
	struct object object;
	struct map map;
	GLuint heightmap;
};

struct context {
	struct scene scene;
	struct camera camera;
	float delta;
};

vec3 fcolor;
float gtime;

struct map make_map(void) 
{
	struct map map;

	map.m = make_grid_mesh(1, 1, 256.0); 

	struct shader pipeline[] = {
		{GL_VERTEX_SHADER, "data/shader/mapv.glsl"},
		{GL_FRAGMENT_SHADER, "data/shader/mapf.glsl"},
		{GL_NONE, NULL}
	};
	map.shader = load_shaders(pipeline);

	map.texture = make_voronoi_texture(2048, 2048);
	//map.texture = make_mountain_texture(2048, 2048);
	//map.texture = make_river_texture(1024, 1024);
	//map.texture = make_worley_texture(512, 512);
	//map.texture = make_perlin_texture(512, 512);

	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(map.shader);
	glUniformMatrix4fv(glGetUniformLocation(map.shader, "project"), 1, GL_FALSE, project.f);

	return map;
}

static vec3 sample_height(float x, float z)
{
	float amplitude = 8.0;
	return vec3_make(x, amplitude * terrain_height(x*16, z*16, 0.004, 2.5, 2.0), z);
}

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
	vec3 center = {25.0, 25.0, 25.0};
	obj.bbox.c = center;
	obj.bbox.r[0] = 0.5;
	obj.bbox.r[1] = 0.5;
	obj.bbox.r[2] = 0.5;

	obj.texture = load_dds_texture("media/texture/placeholder.dds");

	mat4 model = IDENTITY_MATRIX;
	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(obj.shader);
	glUniformMatrix4fv(glGetUniformLocation(obj.shader, "project"), 1, GL_FALSE, project.f);
	mat4_translate(&model, obj.bbox.c);
	glUniformMatrix4fv(glGetUniformLocation(obj.shader, "model"), 1, GL_FALSE, model.f);

	return obj;
}

void display_standard_object(struct object *obj)
{
	mat4 model = IDENTITY_MATRIX;
	mat4_translate(&model, obj->bbox.c);

	glUseProgram(obj->shader);
	glUniform3fv(glGetUniformLocation(obj->shader, "fcolor"), 1, fcolor.f);
	glUniformMatrix4fv(glGetUniformLocation(obj->shader, "model"), 1, GL_FALSE, model.f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, obj->texture);
	glBindVertexArray(obj->m.VAO);
	glDrawArrays(GL_TRIANGLES, 0, obj->m.vcount);
}

struct water make_water(GLuint depthmap)
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
	wat.depthmap = depthmap;
	
	wat.normal = load_dds_texture("media/texture/water_normal.dds");

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
	glUniform1i(glGetUniformLocation(wat->shader, "water_depth_buffer"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glBindVertexArray(wat->m.VAO);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glDrawArrays(GL_PATCHES, 0, wat->m.vcount);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void display_map(struct map *map)
{
	glUseProgram(map->shader);
	glUniform1i(glGetUniformLocation(map->shader, "voronoi"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, map->texture);
	glUniform1i(glGetUniformLocation(map->shader, "depth_map"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glBindVertexArray(map->m.VAO);
	glDrawArrays(GL_TRIANGLES, 0, map->m.vcount);
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

struct terrain make_terrain(GLuint heightmap)
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
	ter.m = make_patch_mesh(64,64, 1.0);

	ter.heightmap = make_terrain_texture(HEIGHTMAP_RES, HEIGHTMAP_RES);
	ter.texture[0] = load_dds_texture("media/texture/grass.dds");
	ter.texture[1] = load_dds_texture("media/texture/rock.dds");
	ter.texture[2] = load_dds_texture("media/texture/graydirt.dds");
	ter.texture[3] = load_dds_texture("media/texture/snowrocks.dds");

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

	glUniform1i(glGetUniformLocation(ter->shader, "gravel"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ter->texture[2]);

	glUniform1i(glGetUniformLocation(ter->shader, "snow"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, ter->texture[3]);

	glUniform1i(glGetUniformLocation(ter->shader, "heightmap"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ter->heightmap);

	glBindVertexArray(ter->m.VAO);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawArrays(GL_PATCHES, 0, ter->m.vcount);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void display_scene(struct scene *scene)
{
	glViewport(0, 0, DTEX_WIDTH, DTEX_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, scene->water.depth_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	display_terrain(&scene->terrain);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	display_terrain(&scene->terrain);
	display_water(&scene->water);
	display_standard_object(&scene->object);
	display_skybox(&scene->skybox);
	display_map(&scene->map);
}

void load_scene(struct scene *scene)
{

	scene->skybox = make_skybox();
	scene->terrain = make_terrain(scene->heightmap);
	scene->water = make_water(scene->heightmap);
	scene->object = make_standard_object();
	scene->map = make_map();

	scene->terrain.texture[4] = scene->map.texture;
}

void update_context(struct context *context)
{
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);
	context->camera.speed = 1.0;
	if (keystates[SDL_SCANCODE_LSHIFT]) {
		context->camera.speed = 4.0;
	}

	update_free_camera(&context->camera, context->delta);
	//update_strategy_camera(&context->camera, context->delta);
	mat4 view = make_view_matrix(context->camera.eye, context->camera.center, context->camera.up);
	mat4 skybox_view = view;
	skybox_view.f[12] = 0.0;
	skybox_view.f[13] = 0.0;
	skybox_view.f[14] = 0.0;

	fcolor.x = 0.0;
	fcolor.y = 0.0;
	fcolor.z = 0.0;
	if (test_ray_AABB(context->camera.eye, context->camera.center, context->scene.object.bbox)) {
		fcolor.y = 0.5;
	}

	glUseProgram(context->scene.terrain.shader);
	glUniformMatrix4fv(glGetUniformLocation(context->scene.terrain.shader, "view"), 1, GL_FALSE, view.f);
	glUniform3fv(glGetUniformLocation(context->scene.terrain.shader, "view_center"), 1, context->camera.center.f);
	glUniform3fv(glGetUniformLocation(context->scene.terrain.shader, "view_eye"), 1, context->camera.eye.f);
	glUseProgram(context->scene.water.shader);
	glUniformMatrix4fv(glGetUniformLocation(context->scene.water.shader, "view"), 1, GL_FALSE, view.f);
	glUniform3fv(glGetUniformLocation(context->scene.water.shader, "view_dir"), 1, context->camera.center.f);
	glUniform3fv(glGetUniformLocation(context->scene.water.shader, "view_eye"), 1, context->camera.eye.f);
	glUniform1f(glGetUniformLocation(context->scene.water.shader, "time"), gtime);
	glUseProgram(context->scene.object.shader);
	glUniformMatrix4fv(glGetUniformLocation(context->scene.object.shader, "view"), 1, GL_FALSE, view.f);
	glUseProgram(context->scene.map.shader);
	glUniformMatrix4fv(glGetUniformLocation(context->scene.map.shader, "view"), 1, GL_FALSE, view.f);
	glUseProgram(context->scene.skybox.shader);
	glUniformMatrix4fv(glGetUniformLocation(context->scene.skybox.shader, "view"), 1, GL_FALSE, skybox_view.f);
}

void run_game(SDL_Window *window)
{
	struct context context = {0};
	context.camera = init_camera(10.0, 8.0, 10.0, 90.0, 0.2);
	load_scene(&context.scene);

	// INITIALIZE DEPTH FRAME BUFFER //
	context.scene.water.depth_fbo = init_depth_framebuffer();

	// RENDER LOOP //
	float start, end = 0.0;
	SDL_SetRelativeMouseMode(SDL_TRUE);

	SDL_Event event;
	while (event.type != SDL_QUIT) {
		gtime = (float)SDL_GetTicks();
		start = gtime * 0.001;
		context.delta = start - end;

		while(SDL_PollEvent(&event));
		update_context(&context);
		display_scene(&context.scene);

		SDL_GL_SwapWindow(window);
		end = start;
	}

	glDeleteFramebuffers(1, &context.scene.water.depth_fbo);
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

