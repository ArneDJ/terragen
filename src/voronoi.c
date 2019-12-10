#include <stdio.h>
#include <time.h>
#include "gmath.h"
#include "voronoi.h"
#include "imp.h"

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

#define NSITES 500
#define NCHANNELS 3
#define NRIVERS 10

void make_river(const jcv_diagram *diagram, unsigned char *image, int width, int height)
{
	//frand(time(NULL));
    	unsigned char color_line[] = {0.0, 0.0, 0.0};
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
		draw_thick_line(x[i], y[i], x[i+1], y[i+1], image, width, height, color_line, 24.0);
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

		plot((int)p.x, (int)p.y, image, width, height, 3, sitecolor);
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
			draw_triangle(site->p.x, site->p.y, e->pos[0].x, e->pos[0].y, e->pos[1].x, e->pos[1].y, image, width, height, rcolor);
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
			draw_dist_triangle(site->p.x, site->p.y, e->pos[0].x, e->pos[0].y, e->pos[1].x, e->pos[1].y, image, width, height);
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

// Remaps the point from the input space to image space
static inline jcv_point remap(const jcv_point* pt, const jcv_point* min, const jcv_point* max, int width, int height)
{
    jcv_point p;
    p.x = (pt->x - min->x)/(max->x - min->x) * (jcv_real)width;
    p.y = (pt->y - min->y)/(max->y - min->y) * (jcv_real)height;
    return p;
}


void draw_mountains(const jcv_diagram *diagram, unsigned char *image, int width, int height)
{
 unsigned char rgb[] = {255.0, 255.0, 255.0};
 const jcv_site *sites = jcv_diagram_get_sites(diagram);
 int nsites = diagram->numsites;
  int start_site = rand() % nsites;
  printf("picking start site: %d\n", start_site);

        const jcv_site *site = &sites[start_site];

 int i = 0;
 while (i < 50) {
  /* color cell */
  const jcv_graphedge *ge = site->edges;
  jcv_point s = remap(&site->p, &diagram->min, &diagram->max, width, height);
  while (ge) {
   jcv_point p0 = remap(&ge->pos[0], &diagram->min, &diagram->max, width, height);
   jcv_point p1 = remap(&ge->pos[1], &diagram->min, &diagram->max, width, height);

   draw_triangle(s.x, s.y, p0.x, p0.y, p1.x, p1.y, image, width, height, &rgb[0]);
   ge = ge->next;
  }

  /* find a neighbouring cell */
  const jcv_graphedge *e = site->edges;
  float dist = 0.0;
  while (e) {
	vec2 e1 = {e->pos[0].x, e->pos[0].y};
	vec2 e2 = {e->pos[1].x, e->pos[1].y};
   dist = vec2_dist(e1, e2);
   if (dist > 10) {
    break;
   }

   e = e->next;
  }

  if (!e) {
   printf("UH OH\n");
   break;
  }

  site = e->neighbor;

  i++;
 }
}

unsigned char *voronoi_mountains(int width, int height)
{
	frand(time(NULL));
	srand(time(0)); 

	unsigned char *image = calloc(3 * width*height, sizeof(unsigned char));

	jcv_point site[NSITES];

	for (int i = 0; i < NSITES; i++) {
		site[i].x = frand(width);
		site[i].y = frand(height);
	}

	jcv_diagram diagram;
	memset(&diagram, 0, sizeof(jcv_diagram));
	jcv_diagram_generate(NSITES, site, 0, 0, &diagram);

	draw_mountains(&diagram, image, width, height);
	draw_mountains(&diagram, image, width, height);

	jcv_diagram_free(&diagram);
	return image;
}

