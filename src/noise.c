#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "gmath.h"
#include "noise.h"

#define OCTAVES 5
static int SEED = 444;

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
	return d.x; // F1 duplicated , F2 not computed
}








#define N_SITES 150
#define N_CELLS 1000
#define AA_RES 4 /* average over 4x4 supersampling grid */

static double site[N_SITES][2];
static unsigned char rgb_v[N_SITES][3];
static int size_x = 1024, size_y = 1024;
vec2 cells[N_CELLS];
 
static inline double sq2(double x, double y)
{
	return x * x + y * y;
}
 
static int nearest_site(double x, double y)
{
	int k, ret = 0;
	double d, dist = 0;
	for (k = 0; k < N_SITES; k++) {
		d = sq2(x - site[k][0], y - site[k][1]);
		if (!k || d < dist) {
			dist = d, ret = k;
		}
	}
	return ret;
}

/* see if a pixel is different from any neighboring ones */
static int at_edge(int *color, int y, int x)
{
	int i, j, c = color[y * size_x + x];
	for (i = y - 1; i <= y + 1; i++) {
		if (i < 0 || i >= size_y)	
			continue;

		for (j = x - 1; j <= x + 1; j++) {
			if (j < 0 || j >= size_x) 
				continue;

			if (color[i * size_x + j] != c) 
				return 1;
		}
	}
	return 0;
}
 
static void aa_color(unsigned char *pix, int y, int x)
{
	int i, j, n;
	double r = 0, g = 0, b = 0, xx, yy;
	for (i = 0; i < AA_RES; i++) {
		yy = y + 1. / AA_RES * i + .5;
		for (j = 0; j < AA_RES; j++) {
			xx = x + 1. / AA_RES * j + .5;
			n = nearest_site(xx, yy);
			r += rgb_v[n][0];
			g += rgb_v[n][1];
			b += rgb_v[n][2];
		}
	}
	pix[0] = r / (AA_RES * AA_RES);
	pix[1] = g / (AA_RES * AA_RES);
	pix[2] = b / (AA_RES * AA_RES);
}

// Determines how many cells there are
#define NUM_CELLS 8.0

// Arbitrary random, can be replaced with a function of your choice
float randv(vec2 co)
{
	vec2 xy = {co.y, co.x};
	vec2 r = {12.9898, 78.233};
	return fract(sin(vec2_dot(xy ,r)) * 43758.5453);
}

float vec2_dist(vec2 a, vec2 b)
{
	return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}

float vec2_length(vec2 v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}

// Returns the point in a given cell
vec2 get_cell_point(vec2 cell) {
	vec2 cell_base = {cell.x / NUM_CELLS, cell.y / NUM_CELLS};
	float noise_x = randv(cell);
	vec2 yx = {cell.y, cell.x};
	float noise_y = randv(yx);
	vec2 noise = {0.5 + 1.5 * noise_x, 0.5 + 1.5 * noise_y};
	vec2 point = {cell_base.x + noise.x / NUM_CELLS, cell_base.y + noise.y / NUM_CELLS};
	return point;
}

/* DEPCRECATED: TOO SLOW */
float old_worley_noise(float x, float y) 
{
	int xcoord = x * NUM_CELLS;
	int ycoord = y * NUM_CELLS;
	float dist = 1.0;
	vec2 coordv = {x, y};

	// Search in the surrounding 5x5 cell block
	for (int x = 0; x < 5; x++) {
		for (int y = 0; y < 5; y++) {
			vec2 cell = {xcoord + (x - 2), ycoord + (y - 2)};
			vec2 cell_point = get_cell_point(cell);
			dist = min(dist, vec2_dist(cell_point, coordv));

		}
	}

	vec2 len = {1.0 / NUM_CELLS, 1.0 / NUM_CELLS};
	dist /= vec2_length(len);
	//dist = 1.0 - sqrt(dist);
	return dist;
}

unsigned char *gen_voronoi_map()
{
	int k;
	for (k = 0; k < N_SITES; k++) {
		site[k][0] = frand(size_x);
		site[k][1] = frand(size_y);
		rgb_v[k][0] = frand(256);
		rgb_v[k][1] = frand(256);
		rgb_v[k][2] = frand(256);
	}

	int i, j;
	int *nearest = malloc(sizeof(int) * size_y * size_x);
	unsigned char *ptr, *buf, color;

	ptr = buf = malloc(3 * size_x * size_y);
	for (i = 0; i < size_y; i++) { 
		for (j = 0; j < size_x; j++) { 
			nearest[i * size_x + j] = nearest_site(j, i);
		}
	}

	for (i = 0; i < size_y; i++) {
		for (j = 0; j < size_x; j++) {
			if (!at_edge(nearest, i, j))
				memcpy(ptr, rgb_v[nearest[i * size_x + j]], 3);
			else /* at edge, do anti-alias rastering */
				aa_color(ptr, i, j);
			ptr += 3;
 		}
	}
 
	/* draw sites */
	for (k = 0; k < N_SITES; k++) {
		color = (rgb_v[k][0]*.25 + rgb_v[k][1]*.6 + rgb_v[k][2]*.15 > 80) ? 0 : 255;

		for (i = site[k][1] - 1; i <= site[k][1] + 1; i++) {
			if (i < 0 || i >= size_y) 
				continue;

			for (j = site[k][0] - 1; j <= site[k][0] + 1; j++) {
				if (j < 0 || j >= size_x) 
					continue;

				ptr = buf + 3 * (i * size_x + j);
				ptr[0] = ptr[1] = ptr[2] = color;
			}
		}
 	}
 
	return buf;
}



static inline float my_worley(float x, float y)
{
	float min = 1024;
	float d;
	for (int i = 0; i < N_CELLS; i++) {
		d = sqrt(sq2(x - cells[i].x, y - cells[i].y));				
		min = min(d, min);
	}
	return min;
}

unsigned char *gen_worley_map()
{
	unsigned char *buf = calloc(3 * 1024*1024, sizeof(unsigned char));
	float z[size_x*size_y];
	
	for (int i = 0; i < N_CELLS; i++) {
		cells[i].x = frand(1024);
		cells[i].y = frand(1024);
	}

	clock_t t;
	t = clock();
	printf("brute forcing: starts\n");

	float max = 0;
	int npixel = 0;
	for(int x = 0; x < size_x; x++) {
		for(int y = 0; y < size_y; y++) {
			z[npixel] = my_worley(x, y);
			max = max(max, z[npixel]);
			npixel++;
		}
	}

	t = clock() - t;
	double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
	printf("The brute force took %lf seconds to execute\n", time_taken);

	int nbuf = 0;
	for (int i = 0; i < size_x * size_y; i++) {
		z[i] /= max;
		//z[i] = 1.0 - sqrt(z[i]); //if you want steep mountains
		z[i] = 1.0 - z[i]; //if you want normal mountains

		buf[nbuf++] = 255*z[i]; buf[nbuf++] = 255*z[i]; buf[nbuf++] = 255*z[i];
	}

	return buf;
}

