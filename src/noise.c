#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "gmath.h"

#define OCTAVES 6

static int SEED;

/*
static float lerp(float a, float b, float s)
{
	return a + s * (b - a);
}
*/

static int permutation(int x, int y)
{
	int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
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
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};
	int tmp = hash[(y + SEED) % 256];
	return hash[(tmp + x) % 256];
}

static float smooth(float x, float y, float s)
{
	return lerp(x, y, s * s * (3-2*s));
}

static float noise(float x, float y)
{
	int ix = x;
	int iy = y;

	int s = permutation(ix, iy);
	int t = permutation(ix+1, iy);
	int u = permutation(ix, iy+1);
	int v = permutation(ix+1, iy+1);

	float low = smooth(s, t, fract(x));
	float high = smooth(u, v, fract(x));

	return smooth(low, high, fract(y));
}

float fbm_noise(float x, float y) 
{
	float ampl = 1.0;
	float freq = 0.004;
	float fin = 0.0;
	float div = 0.0;
	float lacun = 2.5;
	float gain = 2.0;

	for (int i = 0; i < OCTAVES; i++) {
		fin += ampl * fabs(noise(x*freq, y*freq));
		div += 256 * ampl;
		x *= lacun; y *= lacun;
		ampl /= gain;
	}

	return fin / div;
}


#define N_SITES 150
double site[N_SITES][2];
unsigned char rgb[N_SITES][3];
 
int size_x = 1024, size_y = 1024;
 
static inline double sq2(double x, double y)
{
	return x * x + y * y;
}
 
int nearest_site(double x, double y)
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
int at_edge(int *color, int y, int x)
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
 
#define AA_RES 4 /* average over 4x4 supersampling grid */
void aa_color(unsigned char *pix, int y, int x)
{
	int i, j, n;
	double r = 0, g = 0, b = 0, xx, yy;
	for (i = 0; i < AA_RES; i++) {
		yy = y + 1. / AA_RES * i + .5;
		for (j = 0; j < AA_RES; j++) {
			xx = x + 1. / AA_RES * j + .5;
			n = nearest_site(xx, yy);
			r += rgb[n][0];
			g += rgb[n][1];
			b += rgb[n][2];
		}
	}
	pix[0] = r / (AA_RES * AA_RES);
	pix[1] = g / (AA_RES * AA_RES);
	pix[2] = b / (AA_RES * AA_RES);
}

#define for_i for (i = 0; i < size_y; i++)
#define for_j for (j = 0; j < size_x; j++)
#define frand(x) (rand() / (1. + RAND_MAX) * x)
unsigned char *gen_voronoi_map()
{
	int k;
	for (k = 0; k < N_SITES; k++) {
		site[k][0] = frand(size_x);
		site[k][1] = frand(size_y);
		rgb [k][0] = frand(256);
		rgb [k][1] = frand(256);
		rgb [k][2] = frand(256);
	}

	int i, j;
	int *nearest = malloc(sizeof(int) * size_y * size_x);
	unsigned char *ptr, *buf, color;

	ptr = buf = malloc(3 * size_x * size_y);
	for_i for_j nearest[i * size_x + j] = nearest_site(j, i);

	for_i for_j {
		if (!at_edge(nearest, i, j))
			memcpy(ptr, rgb[nearest[i * size_x + j]], 3);
		else /* at edge, do anti-alias rastering */
			aa_color(ptr, i, j);
		ptr += 3;
 	}
 
	/* draw sites */
	for (k = 0; k < N_SITES; k++) {
		color = (rgb[k][0]*.25 + rgb[k][1]*.6 + rgb[k][2]*.15 > 80) ? 0 : 255;

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
