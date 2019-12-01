#include <stdio.h>
#include <time.h>
#include "gmath.h"
#include "voronoi.h"

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

#define NSITES 500
#define NCHANNELS 3
#define NRIVERS 10

// http://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
static inline int orient(const jcv_point *a, const jcv_point *b, const jcv_point *c)
{
	return ((int)b->x - (int)a->x)*((int)c->y - (int)a->y) - ((int)b->y - (int)a->y)*((int)c->x - (int)a->x);
}

static inline int min3(int a, int b, int c)
{
	return min(a, min(b, c));
}
static inline int max3(int a, int b, int c)
{
	return max(a, max(b, c));
}

static void plot(int x, int y, unsigned char *image, int width, int height, unsigned char color[3])
{
	if (x < 0 || y < 0 || x > (width-1) || y > (height-1)) {
		return;
	}

	int index = y * width * NCHANNELS + x * NCHANNELS;

	for (int i = 0; i < NCHANNELS; i++) {
		image[index+i] = color[i];
	}
}

// http://members.chello.at/~easyfilter/bresenham.html
static void draw_line(int x0, int y0, int x1, int y1, unsigned char* image, int width, int height, int nchannels, unsigned char* color)
{
	int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = dx+dy, e2; // error value e_xy

	for(;;) {
		plot(x0,y0, image, width, height, color);
		if (x0==x1 && y0==y1)
			break;
		e2 = 2*err;
		if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0
		if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0
	}
}

static void draw_thick_line(int x0, int y0, int x1, int y1, unsigned char* image, int width, int height, unsigned char *color, float wd)
{
	int dx = abs(x1-x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0 < y1 ? 1 : -1;
	int err = dx-dy, e2, x2, y2;                          /* error value e_xy */
	float ed = dx+dy == 0 ? 1 : sqrt((float)dx*dx+(float)dy*dy);

	for (wd = (wd+1)/2; ; ) {                                   /* pixel loop */
		plot(x0,y0, image, width, height, color);
		//plot(x0,y0, image, width, height, nchannels, max(0,255*(abs(err-dx+dy)/ed-wd+1)));
		e2 = err; x2 = x0;
		if (2*e2 >= -dx) {                                           /* x step */
			for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
				plot(x0, y2 += sy, image, width, height, color);
			if (x0 == x1) 
				break;
			e2 = err; err -= dy; x0 += sx;
		}
		if (2*e2 <= dy) {                                            /* y step */
			for (e2 = dx-e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
				plot(x2 += sx, y0, image, width, height, color);
			if (y0 == y1) 
				break;
			err += dx; y0 += sy;
		}
	}
}

static void draw_triangle(const jcv_point *v0, const jcv_point *v1, const jcv_point *v2, unsigned char *image, int width, int height, unsigned char *color)
{
	int area = orient(v0, v1, v2);
	if (area == 0)
		return;

	// Compute triangle bounding box
	int minX = min3((int)v0->x, (int)v1->x, (int)v2->x);
	int minY = min3((int)v0->y, (int)v1->y, (int)v2->y);
	int maxX = max3((int)v0->x, (int)v1->x, (int)v2->x);
	int maxY = max3((int)v0->y, (int)v1->y, (int)v2->y);

	// Clip against screen bounds
	minX = max(minX, 0);
	minY = max(minY, 0);
	maxX = min(maxX, width - 1);
	maxY = min(maxY, height - 1);

	// Rasterize
	jcv_point p;
	for (p.y = (jcv_real)minY; p.y <= maxY; p.y++) {
		for (p.x = (jcv_real)minX; p.x <= maxX; p.x++) {
			// Determine barycentric coordinates
			int w0 = orient(v1, v2, &p);
			int w1 = orient(v2, v0, &p);
			int w2 = orient(v0, v1, &p);

			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				plot((int)p.x, (int)p.y, image, width, height, color);
			}
		}
	}
}

static void draw_dist_triangle(const jcv_point *center, const jcv_point *v1, const jcv_point *v2, unsigned char *image, int width, int height, unsigned char *color)
{
	int area = orient(center, v1, v2);
	if (area == 0)
		return;

	// Compute triangle bounding box
	int minX = min3((int)center->x, (int)v1->x, (int)v2->x);
	int minY = min3((int)center->y, (int)v1->y, (int)v2->y);
	int maxX = max3((int)center->x, (int)v1->x, (int)v2->x);
	int maxY = max3((int)center->y, (int)v1->y, (int)v2->y);

	// Clip against screen bounds
	minX = max(minX, 0);
	minY = max(minY, 0);
	maxX = min(maxX, width - 1);
	maxY = min(maxY, height - 1);

	// Rasterize
	jcv_point p;
	for (p.y = (jcv_real)minY; p.y <= maxY; p.y++) {
		for (p.x = (jcv_real)minX; p.x <= maxX; p.x++) {
			// Determine barycentric coordinates
			int w0 = orient(v1, v2, &p);
			int w1 = orient(v2, center, &p);
			int w2 = orient(center, v1, &p);

			//unsigned char dist_color = vec2_dist(
			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				vec2 a = {(float)p.x, (float)p.y};
				vec2 b = {(float)center->x, (float)center->y};
				float dist = 1.0 - (vec2_dist(a, b) / 150.0);
				unsigned char dist_color[3] = {255.0*dist, 255.0*dist, 255.0*dist};
				plot((int)p.x, (int)p.y, image, width, height, dist_color);
			}
		}
	}
}

void make_river(const jcv_diagram *diagram, unsigned char *image, int width, int height)
{
	//frand(time(NULL));
    	unsigned char color_line[] = {150, 150, 150};
	const int RIVER_SIZE = 20;
	const jcv_site *sites = jcv_diagram_get_sites(diagram);
	int nsites = diagram->numsites;
        const jcv_site *site = &sites[rand() % nsites];

	int x[RIVER_SIZE];
	int y[RIVER_SIZE];

	for (int i = 0; i < RIVER_SIZE; i++) {
		const jcv_graphedge *e = site->edges;
		site = e->neighbor;
		x[i] = (int)site->p.x;
		y[i] = (int)site->p.y;
	}

	for (int i = 0; i < RIVER_SIZE-1; i++) {
		draw_thick_line(x[i], y[i], x[i+1], y[i+1], image, width, height, color_line, 32.0);
	}

}

unsigned char *do_voronoi(int width, int height)
{
	unsigned char *image = calloc(3 * width*height, sizeof(unsigned char));
	jcv_point site[NSITES];

	for (int i = 0; i < NSITES; i++) {
		site[i].x = frand(width);
		site[i].y = frand(height);
	}

	jcv_diagram diagram;
	memset(&diagram, 0, sizeof(jcv_diagram));
	jcv_diagram_generate(NSITES, site, 0, 0, &diagram);

	/* plot the sites */
	unsigned char sitecolor[3] = {255.0, 255.0, 255.0};
	const jcv_site *sites = jcv_diagram_get_sites(&diagram);
	for (int i = 0; i < diagram.numsites; i++) {
		const jcv_site *site = &sites[i];
		jcv_point p = site->p;

		plot((int)p.x, (int)p.y, image, width, height, sitecolor);
	}

	/* fill the cells */
	const jcv_site *cells = jcv_diagram_get_sites(&diagram);
	for (int i = 0; i < diagram.numsites; i++) {
		unsigned char rcolor[3];
		unsigned char basecolor = 120;
		rcolor[0] = basecolor + (unsigned char)(rand() % (235 - basecolor));
		rcolor[1] = basecolor + (unsigned char)(rand() % (235 - basecolor));
		rcolor[2] = basecolor + (unsigned char)(rand() % (235 - basecolor));
		const jcv_site *site = &cells[i];
		const jcv_graphedge *e = site->edges;

		while (e) {
			draw_dist_triangle(&site->p, &e->pos[0], &e->pos[1], image, width, height, rcolor);
			e = e->next;
		}
	}

	jcv_diagram_free(&diagram);
	return image;
}

void make_mountains(const jcv_diagram *diagram, unsigned char *image, int width, int height)
{
	//frand(time(NULL));
    	unsigned char color_line[] = {255.0, 255.0, 255.0};
	const int MOUNTAIN_RANGE_SIZE = 20;
	const jcv_site *sites = jcv_diagram_get_sites(diagram);
	int nsites = diagram->numsites;
        const jcv_site *site = &sites[rand() % nsites];

	int x[MOUNTAIN_RANGE_SIZE];
	int y[MOUNTAIN_RANGE_SIZE];

	for (int i = 0; i < MOUNTAIN_RANGE_SIZE; i++) {
		const jcv_graphedge *e = site->edges;
		site = e->neighbor;
		/*
		x[i] = (int)site->p.x;
		y[i] = (int)site->p.y;
		draw_triangle(&site->p, &e->pos[0], &e->pos[1], image, width, height, color_line);
		*/
		while (e) {
			draw_dist_triangle(&site->p, &e->pos[0], &e->pos[1], image, width, height, color_line);
			e = e->next;
		}
	}
}

unsigned char *voronoi_rivers(int width, int height)
{
	frand(time(NULL));
	unsigned char *image = calloc(3 * width*height, sizeof(unsigned char));
	for (int i = 0; i < 3 * width*height; i++) {
		image[i] = 255.0;
	}
	jcv_point site[NSITES];

	for (int i = 0; i < NSITES; i++) {
		site[i].x = frand(width);
		site[i].y = frand(height);
	}

	jcv_diagram diagram;
	memset(&diagram, 0, sizeof(jcv_diagram));
	jcv_diagram_generate(NSITES, site, 0, 0, &diagram);

	for (int i = 0; i < NRIVERS; i++) {
		make_river(&diagram, image, width, height);
	}

	jcv_diagram_free(&diagram);
	return image;
}

unsigned char *voronoi_mountains(int width, int height)
{
	frand(time(NULL));
	unsigned char *image = calloc(3 * width*height, sizeof(unsigned char));

	jcv_point site[NSITES];

	for (int i = 0; i < NSITES; i++) {
		site[i].x = frand(width);
		site[i].y = frand(height);
	}

	jcv_diagram diagram;
	memset(&diagram, 0, sizeof(jcv_diagram));
	jcv_diagram_generate(NSITES, site, 0, 0, &diagram);

	for (int i = 0; i < NRIVERS; i++) {
		make_mountains(&diagram, image, width, height);
	}

	jcv_diagram_free(&diagram);
	return image;
}

