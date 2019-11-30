#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "gmath.h"
#include "noise.h"

#define OCTAVES 4
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


