#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "gmath.h"
#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "imp.h"
#include "voronoi.h"
#define IIR_GAUSS_BLUR_IMPLEMENTATION
#include "gauss.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

enum celltype {
	COASTAL,
	INLAND,
	MOUNTAIN,
};

struct vorcell {
	vec2 center;
	enum celltype type;
        jcv_site site;
};

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
	GLuint heightmap;
	GLuint texture[5];
};

struct water {
	struct mesh m;
	GLuint shader;
	GLuint diffuse;
	GLuint normal;
	GLuint depthmap;
	GLuint depthtexture;
	GLuint depth_fbo;
};

static GLuint make_terrain_heightmap(unsigned int res);

static struct object make_skybox(void)
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

static void display_skybox(struct object *sky)
{
	glUseProgram(sky->shader);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);

	glBindVertexArray(sky->m.VAO);
	glDrawArrays(GL_TRIANGLES, 0, sky->m.vcount);

	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

static struct water make_water(GLuint depthmap)
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

static void display_water(struct water *wat)
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
	glBindTexture(GL_TEXTURE_2D, wat->depthtexture);
	glBindVertexArray(wat->m.VAO);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glDrawArrays(GL_PATCHES, 0, wat->m.vcount);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

static struct terrain make_terrain(int resolution)
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

	ter.heightmap = make_terrain_heightmap(resolution);
	ter.texture[0] = load_dds_texture("media/texture/grass.dds");
	ter.texture[1] = load_dds_texture("media/texture/rock.dds");
	ter.texture[2] = load_dds_texture("media/texture/graydirt.dds");
	ter.texture[3] = load_dds_texture("media/texture/snowrocks.dds");

	mat4 project = make_project_matrix(90, (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 200.0);
	glUseProgram(ter.shader);
	glUniformMatrix4fv(glGetUniformLocation(ter.shader, "project"), 1, GL_FALSE, project.f);

	return ter;
}

static void display_terrain(struct terrain *ter)
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

// Remaps the point from the input space to image space
static inline jcv_point remap(const jcv_point* pt, const jcv_point* min, const jcv_point* max, int width, int height)
{
    jcv_point p;
    p.x = (pt->x - min->x)/(max->x - min->x) * (jcv_real)width;
    p.y = (pt->y - min->y)/(max->y - min->y) * (jcv_real)height;
    return p;
}

static GLuint make_terrain_heightmap(unsigned int res)
{
	const size_t size = res * res;
	unsigned char *perlin = calloc(size, sizeof(unsigned char));
	unsigned char *mountainr = calloc(size, sizeof(unsigned char));
	unsigned char *image = calloc(size, sizeof(unsigned char));
	unsigned char *cpy = calloc(size, sizeof(unsigned char));

	unsigned char red = 255.0;
	unsigned char black = 0.0;
	unsigned char water = 0.0;
	unsigned char land = 100.0;
	unsigned char coastclr = 200.0;

	int nbuf = 0;
	for(int x = 0; x < res; x++) {
		for(int y = 0; y < res; y++) {
			float z = fbm_noise(0.5*x, 0.5*y, 0.005, 2.5, 2.0);
			perlin[nbuf] = z * 255.0;
			if (z > 0.55) {
				z = land;
			} else {
				z = water;
			}
			image[nbuf++] = z;
		}
	}

	const int MIN_LAKE_SIZE = res * 2;
	const int MIN_ISLAND_SIZE = res;

	/* remove small lakes */
	memcpy(cpy, image, size);
	for (int x = 0; x < res; x++) {
		for (int y = 0; y < res; y++) {
			int index = y * res + x ;
			int size = floodfill(x, y, cpy, res, res, water, land);
			if (size < MIN_LAKE_SIZE && size > 1) {
				floodfill(x, y, image, res, res, water, land);
			}
		}
	}

	/* remove small islands */
	memcpy(cpy, image, size);
	for (int x = 0; x < res; x++) {
		for (int y = 0; y < res; y++) {
			int index = y * res + x ;
			int size = floodfill(x, y, cpy, res, res, land, water);
			if (size < MIN_ISLAND_SIZE && size > 1) {
				floodfill(x, y, image, res, res, land, water);
			}
		}
	}

	/* generate site points */
	const int MAX_SITES = 500;
	jcv_point site[MAX_SITES];

	int nsite = 0;
	while (nsite < MAX_SITES) {
		float x = frand(res);
		float y = frand(res);
		int index = (int)y * res + (int)x;
		if (image[index] == land) {
			site[nsite].x = x;
			site[nsite].y = y;
			nsite++;
		}
	}

	jcv_diagram diagram;
	memset(&diagram, 0, sizeof(jcv_diagram));
	jcv_diagram_generate(MAX_SITES, site, 0, 0, &diagram);

	/* plot the sites */
	unsigned char sitecolor = 155.0;
	const jcv_site *sites = jcv_diagram_get_sites(&diagram);
	/*
	for (int i = 0; i < diagram.numsites; i++) {
		const jcv_site *site = &sites[i];
		jcv_point p = site->p;

		plot((int)p.x, (int)p.y, image, res, res, 1, &sitecolor);
	}
	*/

	/* find the coastal cells */
	/* points don't account for image space! convert them! */
	struct vorcell vcell[MAX_SITES];
	for (int i = 0; i < MAX_SITES; i++) {
		vcell[i].site = sites[i];
		vcell[i].center.x = sites[i].p.x;
		vcell[i].center.y = sites[i].p.y;

		const jcv_graphedge *e = vcell[i].site.edges;

		while (e) {
			jcv_point p1 = remap(&e->pos[0], &diagram.min, &diagram.max, res, res);
			jcv_point p2 = remap(&e->pos[1], &diagram.min, &diagram.max, res, res);
			const int index1 = (int)p1.y * res + (int)p1.x;
			const int index2 = (int)p2.y * res + (int)p2.x;
			if (image[index1] == water || image[index2] == water) {
				vcell[i].type = COASTAL;
				break;
			} else {
				vcell[i].type = INLAND;
			}
			e = e->next;
		}
	}

	iir_gauss_blur(res, res, 1, image, 5.0);

	/* ADD MOUNTAINS */
	const float MIN_MOUNTAIN_HEIGHT = 0.7;
	for (int i = 0; i < MAX_SITES; i++) {
		const int index = (int)vcell[i].center.y * res + (int)vcell[i].center.x;
		const float hsample = perlin[index]/255.f;
		if (hsample > MIN_MOUNTAIN_HEIGHT && vcell[i].type == INLAND) {
			vcell[i].type = MOUNTAIN;
			const jcv_graphedge *e = vcell[i].site.edges;

			while (e) {
				draw_triangle(vcell[i].center.x, vcell[i].center.y, e->pos[0].x, e->pos[0].y, e->pos[1].x, e->pos[1].y, mountainr, res, res, 1, &red);
				e = e->next;
			}
		}
	}

	iir_gauss_blur(res, res, 1, mountainr, 10.0);

	for (int x = 0; x < res; x++) {
		for (int y = 0; y < res; y++) {
			int index = y * res + x;

			float mountains = 1.0 - (sqrt(worley_noise(0.02*x, 0.030*y)));
			float ridge = worley_noise(x * 0.03, y * 0.02);

			float range = mountainr[index]/255.f;

			mountains *= range * 0.6;
			ridge *= range * 0.6;

			image[index] = 255.0 * (image[index]/255.f + ((mountains + ridge) / 2.0));

		}
	}

	/* ADD RIVERS */
	/* maker river candidate network */
	srand(time(NULL));
	const float RIVER_WIDTH = 8.0;
    	unsigned char color_line = 0.0;
	unsigned char *riverr = calloc(size, sizeof(unsigned char));
	for (int i = 0; i < size; i++) {
		riverr[i] = 255.0;
	}
	//memcpy(riverr, image, size);

	for (int i = 0; i < 20; i++) {
		int startindex = rand() % MAX_SITES;
		const jcv_site *rsite = &vcell[startindex].site;

		if (vcell[startindex].type == MOUNTAIN) {
			for (int i = 0; i < 500; i++) {
				jcv_point c1 = remap(&rsite->p, &diagram.min, &diagram.max, res, res);
				const jcv_graphedge *e = rsite->edges;
				rsite = e->neighbor;
				jcv_point c2 = remap(&rsite->p, &diagram.min, &diagram.max, res, res);
				draw_thick_line(c1.x, c1.y, c2.x, c2.y, riverr, res, res, 1, &color_line, RIVER_WIDTH);
				int out = 0;
				while (e) {
					jcv_point p1 = remap(&e->pos[0], &diagram.min, &diagram.max, res, res);
					jcv_point p2 = remap(&e->pos[1], &diagram.min, &diagram.max, res, res);
					const int index1 = (int)p1.y * res + (int)p1.x;
					const int index2 = (int)p2.y * res + (int)p2.x;

					if (image[index1] == water) {
						draw_thick_line(c2.x, c2.y, p1.x, p1.y, riverr, res, res, 1, &color_line, RIVER_WIDTH);
						out = 1;
						break;
					} else if (image[index2] == water) {
						draw_thick_line(c2.x, c2.y, p2.x, p2.y, riverr, res, res, 1, &color_line, RIVER_WIDTH);
						out = 1;
						break;
					}
					e = e->next;
				}

				if (out) {
					break;
				}
			}

		}

	}
	iir_gauss_blur(res, res, 1, riverr, 5.0);
	for (int i = 0; i < size; i++) {
		image[i] = 255.0 * (image[i]/255.f) * (riverr[i]/255.f);
	}


	/*
        const jcv_edge *edge = jcv_diagram_get_edges(&diagram);
        while (edge) {
		draw_line((int)edge->pos[0].x, (int)edge->pos[0].y, (int)edge->pos[1].x, (int)edge->pos[1].y, image, res, res, 1, &sitecolor);
		edge = jcv_diagram_get_next_edge(edge);
        }
	*/


	GLuint texnum = make_r_texture(image, res, res);

	jcv_diagram_free(&diagram);
	free(image);
	free(mountainr);
	free(cpy);
	free(perlin);
	free(riverr);
	return texnum;
}

static void run_loop(SDL_Window *window)
{
	float start, end = 0.0;
	SDL_SetRelativeMouseMode(SDL_TRUE);
	struct camera cam = init_camera(1.0, 1.0, 1.0, 90.0, 0.2);

	struct terrain terra = make_terrain(2048);
	struct object sky = make_skybox();
	struct mesh cube = make_grid_mesh(1, 1, 10.0);
	GLuint texture = terra.heightmap;

	// INITIALIZE DEPTH FRAME BUFFER //
	struct water wat = make_water(terra.heightmap);
	wat.depth_fbo = init_depth_framebuffer(&wat.depthtexture, 1024, 1024);

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

	/* MASTER LOOP */
	SDL_Event event;
	while (event.type != SDL_QUIT) {
		const float gtime = (float)SDL_GetTicks();
		start = gtime;
		float delta = start - end;
		while(SDL_PollEvent(&event));

		/* update camera */
		update_free_camera(&cam, 0.001 * delta);
		mat4 view = make_view_matrix(cam.eye, cam.center, cam.up);
		mat4 skybox_view = view;
		skybox_view.f[12] = 0.0;
		skybox_view.f[13] = 0.0;
		skybox_view.f[14] = 0.0;

		/* update scene */
		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, view.f);
		glUseProgram(terra.shader);
		glUniformMatrix4fv(glGetUniformLocation(terra.shader, "view"), 1, GL_FALSE, view.f);
		glUniformMatrix4fv(glGetUniformLocation(terra.shader, "view"), 1, GL_FALSE, view.f);
		glUniform3fv(glGetUniformLocation(terra.shader, "view_center"), 1, cam.center.f);
		glUniform3fv(glGetUniformLocation(terra.shader, "view_eye"), 1, cam.eye.f);

		glUseProgram(wat.shader);
		glUniformMatrix4fv(glGetUniformLocation(wat.shader, "view"), 1, GL_FALSE, view.f);
		glUniform3fv(glGetUniformLocation(wat.shader, "view_dir"), 1, cam.center.f);
		glUniform3fv(glGetUniformLocation(wat.shader, "view_eye"), 1, cam.eye.f);
		glUniform1f(glGetUniformLocation(wat.shader, "time"), gtime);

		glUseProgram(sky.shader);
		glUniformMatrix4fv(glGetUniformLocation(sky.shader, "view"), 1, GL_FALSE, skybox_view.f);

		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, wat.depth_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		display_terrain(&terra);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		/* render */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(cube.VAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, cube.vcount);

		display_terrain(&terra);
		display_water(&wat);
		display_skybox(&sky);

		/* swap buffer */
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
