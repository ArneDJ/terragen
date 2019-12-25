#include <stdio.h>
#include <math.h>
#include <time.h>
#include "gmath.h"
#include "vec.h"
#include "imp.h"
#define JC_VORONOI_IMPLEMENTATION
#include "voronoi.h"

#define OCTAVES 5
#define NSITES 500
#define NRIVERS 10

static void push(vec_int_t *stack, int x, int y);
static int pop(vec_int_t *stack, int *x, int *y);
static inline int orient(float x0, float y0, float x1, float y1, float x2, float y2);
static inline int min3(int a, int b, int c);
static inline int max3(int a, int b, int c);
static inline vec4 permute(vec4 v);
static inline float mod(float x, float y);
static inline float noise(float x, float y);
static inline float smooth(float x, float y, float s);
static inline int permutation(int x, int y);

static const int SEED = 444;

void plot(int x, int y, unsigned char *image, int width, int height, int nchannels, unsigned char color[3])
{
	if (x < 0 || y < 0 || x > (width-1) || y > (height-1)) {
		return;
	}

	int index = y * width * nchannels + x * nchannels;

	for (int i = 0; i < nchannels; i++) {
		image[index+i] = color[i];
	}
}

// http://members.chello.at/~easyfilter/bresenham.html
void draw_line(int x0, int y0, int x1, int y1, unsigned char *image, int width, int height, int nchannels, unsigned char* color)
{
	int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = dx+dy, e2; // error value e_xy

	for(;;) {
		plot(x0,y0, image, width, height, 3, color);
		if (x0==x1 && y0==y1)
			break;
		e2 = 2*err;
		if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0
		if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0
	}
}

void draw_thick_line(int x0, int y0, int x1, int y1, unsigned char *image, int width, int height, unsigned char *color, float wd)
{
	int dx = abs(x1-x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0 < y1 ? 1 : -1;
	int err = dx-dy, e2, x2, y2;                          /* error value e_xy */
	float ed = dx+dy == 0 ? 1 : sqrt((float)dx*dx+(float)dy*dy);

	for (wd = (wd+1)/2; ; ) {                                   /* pixel loop */
		plot(x0,y0, image, width, height, 3, color);
		//plot(x0,y0, image, width, height, nchannels, max(0,255*(abs(err-dx+dy)/ed-wd+1)));
		e2 = err; x2 = x0;
		if (2*e2 >= -dx) {                                           /* x step */
			for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
				plot(x0, y2 += sy, image, width, height, 3, color);
			if (x0 == x1) 
				break;
			e2 = err; err -= dy; x0 += sx;
		}
		if (2*e2 <= dy) {                                            /* y step */
			for (e2 = dx-e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
				plot(x2 += sx, y0, image, width, height, 3, color);
			if (y0 == y1) 
				break;
			err += dx; y0 += sy;
		}
	}
}

void draw_triangle(float x0, float y0, float x1, float y1, float x2, float y2, unsigned char *image, int width, int height, unsigned char *color)
{
	int area = orient(x0, y0, x1, y1, x2, y2);
	if (area == 0)
		return;

	// Compute triangle bounding box
	int minX = min3((int)x0, (int)x1, (int)x2);
	int minY = min3((int)y0, (int)y1, (int)y2);
	int maxX = max3((int)x0, (int)x1, (int)x2);
	int maxY = max3((int)y0, (int)y1, (int)y2);

	// Clip against screen bounds
	minX = max(minX, 0);
	minY = max(minY, 0);
	maxX = min(maxX, width - 1);
	maxY = min(maxY, height - 1);

	// Rasterize
	float px, py;
	for (py = minY; py <= maxY; py++) {
		for (px = minX; px <= maxX; px++) {
			// Determine barycentric coordinates
			int w0 = orient(x1, y1, x2, y2, px, py);
			int w1 = orient(x2, y2, x0, y0, px, py);
			int w2 = orient(x0, y0, x1, y1, px, py);

			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				plot((int)px, (int)py, image, width, height, 3, color);
			}
		}
	}
}

void draw_dist_triangle(float centerx, float centery, float x1, float y1, float x2, float y2, unsigned char *image, int width, int height)
{
	int area = orient(centerx, centery, x1, y1, x2, y2);
	if (area == 0)
		return;

	// Compute triangle bounding box
	int minX = min3((int)centerx, (int)x1, (int)x2);
	int minY = min3((int)centery, (int)y1, (int)y2);
	int maxX = max3((int)centerx, (int)x1, (int)x2);
	int maxY = max3((int)centery, (int)y1, (int)y2);

	// Clip against screen bounds
	minX = max(minX, 0);
	minY = max(minY, 0);
	maxX = min(maxX, width - 1);
	maxY = min(maxY, height - 1);

	// Rasterize
	float px, py;
	for (py = minY; py <= maxY; py++) {
		for (px = minX; px <= maxX; px++) {
			// Determine barycentric coordinates
			int w0 = orient(x1, y1, x2, y2, px, py);
			int w1 = orient(x2, y2, centerx, centery, px, py);
			int w2 = orient(centerx, centery, x1, y1, px, py);

			//unsigned char dist_color = vec2_dist(
			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				vec2 a = {px, py};
				vec2 b = {centerx, centery};
				float dist = 1.0 - (vec2_dist(a, b) / 150.0);
				unsigned char dist_color[3] = {255.0*dist, 255.0*dist, 255.0*dist};
				plot((int)px, (int)py, image, width, height, 3, dist_color);
			}
		}
	}
}

int floodfill(int x, int y, unsigned char *image, int width, int height, unsigned char old, unsigned char new)
{
	if(old == new) {
		return 1;
	}

	int x1;
	int above, below;
	int size = 0;

	vec_int_t stack;
	vec_init(&stack);
	push(&stack, x, y);

	while (pop(&stack, &x, &y)) {
		x1 = x;

		while (x1 >= 0 && image[y * width + x1] == old) {
			x1--;
		}

		x1++;
		above = below = 0;
		while (x1 < width && image[y * width + x1] == old) {
			image[y * width + x1] = new;
			size++;

			if (!above && y > 0 && image[(y - 1) * width + x1] == old) {
				push(&stack, x1, y - 1);
				above = 1;
			} else if(above && y > 0 && image[(y - 1) * width + x1] != old) {
				above = 0;
			}

			if (!below && y < height - 1 && image[(y + 1) * width + x1] == old) {
				push(&stack, x1, y + 1);
				below = 1;
			} else if (below && y < height - 1 && image[(y + 1) * width + x1] != old) {
				below = 0;
			}

			x1++;
		}
	}

	vec_deinit(&stack);
	return size;
}

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

void do_voronoi(int width, int height, unsigned char *image)
{
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

void voronoi_rivers(int width, int height, unsigned char *image)
{
	frand(time(NULL));
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
}

// Remaps the point from the input space to image space
static inline jcv_point remap(const jcv_point* pt, const jcv_point* min, const jcv_point* max, int width, int height)
{
    jcv_point p;
    p.x = (pt->x - min->x)/(max->x - min->x) * (jcv_real)width;
    p.y = (pt->y - min->y)/(max->y - min->y) * (jcv_real)height;
    return p;
}


static void draw_mountains(const jcv_diagram *diagram, unsigned char *image, int width, int height)
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

void voronoi_mountains(int width, int height, unsigned char *image)
{
	frand(time(NULL));
	srand(time(0)); 

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
}

float fbm_noise(float x, float y, float freq, float lacun, float gain) 
{
	float n = 0.0;
	float div = 0.0;
	float ampl = 1.0;

	for (int i = 0; i < OCTAVES; i++) {
		//n += ampl * ((1.0 - fabs(noise(x*freq, y*freq))) * 2.0 - 1.0);
		n += ampl * fabs(noise(x*freq, y*freq));
		div += 256 * ampl;
		x *= lacun; 
		y *= lacun;
		ampl /= gain;
	}

	return n /= div;
}

float worley_noise(float x, float y)
{
	const float K = 1.0/7.0;
	const float K2 = 0.5/7.0;
	const float jitter = 0.8; // jitter 1.0 makes F1 wrong more often

	vec2 Pi = {mod(floor(x), 289.0), mod(floor(y), 289.0)};
	vec2 Pf = {fract(x), fract(y)};
	vec4 Pfx = {Pf.x - 0.5, Pf.x - 1.5, Pf.x - 0.5, Pf.x - 1.5};
	vec4 Pfy = {Pf.y - 0.5, Pf.y - 0.5, Pf.y - 1.5, Pf.y - 1.5};

	vec4 p = {Pi.x + 0.0, Pi.x + 1.0, Pi.x + 0.0, Pi.x + 1.0};
	p = permute(p);
	vec4 pp = {p.x + Pi.y + 0.0, p.y + Pi.y + 0.0, p.z + Pi.y + 1.0, p.w + Pi.y + 1.0};
	p = permute(pp);
	vec4 ox = {mod(p.x, 7.0) * K + K2, mod(p.y, 7.0) * K + K2, mod(p.z, 7.0) * K + K2, mod(p.w, 7.0) * K + K2};
	vec4 oy = {mod(floor(p.x * K) ,7.0) * K + K2, mod(floor(p.y * K) ,7.0) * K + K2, mod(floor(p.z * K) ,7.0) * K + K2, mod(floor(p.w * K) ,7.0) * K + K2};
	vec4 dx = {Pfx.x + jitter * ox.x, Pfx.y + jitter * ox.y, Pfx.z + jitter * ox.z, Pfx.w + jitter * ox.w};
	vec4 dy = {Pfy.x + jitter * oy.x, Pfy.y + jitter * oy.y, Pfy.z + jitter * oy.z, Pfy.w + jitter * oy.w};
	vec4 d = {dx.x * dx.x + dy.x * dy.x, dx.y * dx.y + dy.y * dy.y, dx.z * dx.z + dy.z * dy.z, dx.w * dx.w + dy.w * dy.w}; // d i s t a n c e s squared
	// Cheat and pick only F1 for the return value
	d.x = min(d.x, d.z);
	d.y = min(d.y, d.w);
	d.x = min(d.x , d.y);
	return clamp(d.x, 0.0, 1.0); // F1 duplicated , F2 not computed
}




static void push(vec_int_t *stack, int x, int y)
{
	vec_push(stack, x);
	vec_push(stack, y);
}

static int pop(vec_int_t *stack, int *x, int *y)
{
	if(stack->length < 2)
		return 0; // it's empty

	*y = vec_pop(stack);
	*x = vec_pop(stack);

	return 1;
}

// http://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
static inline int orient(float x0, float y0, float x1, float y1, float x2, float y2)
{
	return ((int)x1 - (int)x0)*((int)y2 - (int)y0) - ((int)y1 - (int)y0)*((int)x2 - (int)x0);
}

static inline int min3(int a, int b, int c)
{
	return min(a, min(b, c));
}
static inline int max3(int a, int b, int c)
{
	return max(a, max(b, c));
}

static inline int permutation(int x, int y)
{
	const int hash_table[] = {
	208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
	185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
	9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
	70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
	203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
	164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
	228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
	232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
	193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
	101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
	135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
	114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219
	};
	int tmp = hash_table[(y + SEED) % 256];
	return hash_table[(tmp + x) % 256];
}

static inline float smooth(float x, float y, float s)
{
	return lerp(x, y, s * s * (3-2*s));
}

static inline float noise(float x, float y)
{
	int ix = x;
	int iy = y;

	/* square gradients */
	int s = permutation(ix, iy);
	int t = permutation(ix+1, iy);
	int u = permutation(ix, iy+1);
	int v = permutation(ix+1, iy+1);

	float low = smooth(s, t, fract(x));
	float high = smooth(u, v, fract(x));

	return smooth(low, high, fract(y));
}

static inline float mod(float x, float y)
{
	return x - y * floorf(x/y);
}

static inline vec4 permute(vec4 v)
{
	vec4 tmp = {
		mod((34.0 * v.x + 1.0) * v.x, 289.0), 
		mod((34.0 * v.y + 1.0) * v.y, 289.0), 
		mod((34.0 * v.z + 1.0) * v.z, 289.0), 
		mod((34.0 * v.w + 1.0) * v.w, 289.0)
	};
	return tmp;
}

