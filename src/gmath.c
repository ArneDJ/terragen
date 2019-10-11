#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include "gmath.h"

static void swap(float *a, float *b)
{
	float tmp = *b;
	*b = *a;
	*a = tmp;
}

float lerp(float a, float b, float t) 
{
	return (1.0 - t) * a + t * b;
}

float fract(float x)
{
	return x - floorf(x);
}

/* clamp n to lie within the range [min, max] */
float clamp(float n, float min, float max)
{
	if (n < min) return min;
	if (n > max) return max;

	return n;
}

vec3 vec3_make(float x, float y, float z)
{
	vec3 v = {x, y, z};
	return v;
}

float vec3_magnitude(vec3 v)
{
	float sum = v.x * v.x + v.y * v.y + v.z * v.z;
	return sqrt(sum);
}

float vec3_dot(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 vec3_scale(float s, vec3 v)
{
	vec3 tmp = {s*v.x, s*v.y, s*v.z};
	return tmp;
}

vec3 vec3_normalize(vec3 v) 
{
	float len = vec3_magnitude(v);
	vec3 tmp = {v.x/len, v.y/len, v.z/len};
	return tmp;
}

vec3 vec3_sum(vec3 a, vec3 b)
{
	vec3 tmp = {a.x + b.x, a.y + b.y, a.z + b.z};
	return tmp;
}

vec3 vec3_sub(vec3 a, vec3 b)
{
	vec3 tmp = {a.x - b.x, a.y - b.y, a.z - b.z};
	return tmp;
}

vec3 vec3_cross(vec3 a, vec3 b)
{
	vec3 tmp;
	tmp.x = a.y * b.z - a.z * b.y;
	tmp.y = a.z * b.x - a.x * b.z;
	tmp.z = a.x * b.y - a.y * b.x;

	return tmp;
}

vec3 vec3_crossn(vec3 a, vec3 b)
{
	vec3 tmp;
	tmp = vec3_cross(a, b);

	return vec3_normalize(tmp);
}

float scalar_triple_product(vec3 u, vec3 v, vec3 w) 
{
	return vec3_dot(vec3_cross(u, v), w);
}

vec3 barycentric(vec3 a, vec3 b, vec3 c, vec3 p) 
{
	vec3 v0 = vec3_sub(b, a);
	vec3 v1 = vec3_sub(c, a);
	vec3 v2 = vec3_sub(p, a);
	
	float d00 = vec3_dot(v0, v0);
	float d01 = vec3_dot(v0, v1);
	float d11 = vec3_dot(v1, v1);
	float d20 = vec3_dot(v2, v0);
	float d21 = vec3_dot(v2, v1);

	float denom = d00 * d11 - d01 * d01;
	
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0 - v - w;

	vec3 tmp = {u, v, w};
	return tmp;
}

int pt_in_triangle(vec3 p, vec3 a, vec3 b, vec3 c)
{
	vec3 bc = barycentric(a, b, c, p);
	return bc.y >= 0.0 && bc.z >= 0.0 && (bc.y + bc.z) <= 1.0;
}

int test_ray_sphere(vec3 p, vec3 d, struct sphere s) 
{
	vec3 m = vec3_sub(p, s.c);
	float c = vec3_dot(m, m) - s.r * s.r;

	// if there is one real root there must be an intersection
	if (c <= 0.0)	
		return 1;

	// exit if r's origin outside s and r pointing away from s
	float b = vec3_dot(m, d);
	if (b > 0.0)	
		return 0;

	// negative discriminant means ray misses the sphere
	float discr = b*b - c;
	if (discr < 0.0)	
		return 0;

	return 1;
}

/*
int test_ray_AABB(vec3 p, vec3 d, struct AABB a)
{
	float tmin = 0.0f;
	float tmax = FLT_MAX;
	//vec3 min = {a.c.x - a.r[0], a.c.y - a.r[1], a.c.z - a.r[2]};
	//vec3 max = {a.c.x + a.r[0], a.c.y + a.r[1], a.c.z + a.r[2]};
	vec3 min = {-0.5, -0.5, -0.5};
	vec3 max = {0.5, 0.5, 0.5};

	for(int i = 0; i < 3; i++) {
		if (fabs(d.f[i]) < FLT_EPSILON) {
			// ray is parallel to slab
			if (p.f[i] < min.f[i] || p.f[i] > max.f[i])	return 0;
		} else {
			float ood = 1.0 / d.f[i];
			float t1 = (min.f[i] - p.f[i]) * ood;
			float t2 = (max.f[i] - p.f[i]) * ood;

			if (t1 > t2)	swap(&t1, &t2);

			if (t1 > tmin)	tmin = t1;
			if (t2 > tmax)	tmax = t2;

			if (tmin > tmax)	return 0;
		}
	}
	
	return 1;
}
*/

int test_ray_AABB(vec3 p, vec3 d, struct AABB a)
{
	vec3 min = {a.c.x - a.r[0], a.c.y - a.r[1], a.c.z - a.r[2]};
	vec3 max = {a.c.x + a.r[0], a.c.y + a.r[1], a.c.z + a.r[2]};

	float tmin = (min.x - p.x) / d.x;
	float tmax = (max.x - p.x) / d.x;

	if (tmin > tmax) 
		swap(&tmin, &tmax);

	float tymin = (min.y - p.y) / d.y;
	float tymax = (max.y - p.y) / d.y;

	if (tymin > tymax) 
		swap(&tymin, &tymax);

	if ((tmin > tymax) || (tymin > tmax))	
		return 0;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (min.z - p.z) / d.z;
	float tzmax = (max.z - p.z) / d.z;

	if (tzmin > tzmax) 
		swap(&tzmin, &tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return 0;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	return 1;
}

/* COUNTER CLOCKWISE */
int test_ray_triangle(vec3 p, vec3 q, vec3 a, vec3 b, vec3 c)
{
	vec3 bc = vec3_sub(b, c);
	vec3 ac = vec3_sub(a, c);
	vec3 qp = vec3_sub(p, q);

	vec3 n = vec3_cross(bc, ac);

	float d = vec3_dot(qp, n);
	if (d <= 0.0)	
		return 0;

	vec3 ap = vec3_sub(p, c);
	float t = vec3_dot(ap, n);
	if (t < 0.0)	
		return 0;

	vec3 e = vec3_cross(qp, ap);
	float v = vec3_dot(ac, e);
	if (v < 0.0 || v > d)	
		return 0;

	float w = -vec3_dot(bc, e);
	if (w < 0.0 || v + w > d)	
		return 0;

	return 1;
}

vec3 barycentric_ray_triangle(vec3 p, vec3 q, vec3 a, vec3 b, vec3 c)
{
	vec3 tmp;

	vec3 bc = vec3_sub(b, c);
	vec3 ac = vec3_sub(a, c);
	vec3 qp = vec3_sub(p, q);

	vec3 n = vec3_cross(bc, ac);
	float d = vec3_dot(qp, n);	

	vec3 ap = vec3_sub(p, c);
	vec3 e = vec3_cross(qp, ap);
	tmp.y = vec3_dot(ac, e);
	tmp.z = -vec3_dot(bc, e);

	float ood = 1.0 / d;

	tmp.y *= ood;
	tmp.z *= ood;
	tmp.x = 1.0 - tmp.y - tmp.z;

	return tmp;
}

vec3 barycentric_to_cartesian(vec3 bar, vec3 a, vec3 b, vec3 c)
{
	vec3 tmp;

	vec3 ca = vec3_sub(a, c);
	vec3 cb = vec3_sub(b, c);

	ca = vec3_scale(bar.x, ca);
	cb = vec3_scale(bar.y, cb);

	tmp = vec3_sum(c, ca);
	tmp = vec3_sum(tmp, cb);

	return tmp;
}

mat4 make_project_matrix(int fov, float aspect, float near, float far)
{
	float rad = (float)fov * ((2.0 * M_PI) / 360.0);
	float range = tan(0.5 * rad) * near;

	float x = (2.0 * near) / (range * aspect + range * aspect);
	float y = near / range;
	float z = -(far + near) / (far - near);
	float w = -(2.0 * far * near) / (far - near);

	mat4 project = {
		x, 0.0, 0.0, 0.0,
		0.0, y, 0.0, 0.0,
		0.0, 0.0, z, -1.0,
		0.0, 0.0, w, 0.0
	};

	return project;
}

mat4 make_view_matrix(vec3 eye, vec3 center, vec3 up)
{
	vec3 target = vec3_sum(eye, center);

	vec3 f = vec3_sub(target, eye);
	f = vec3_normalize(f);

	vec3 s = vec3_crossn(f, up);
	vec3 u = vec3_cross(s, f);

	mat4 view = {
		s.x, u.x, -f.x, 0.0,
		s.y, u.y, -f.y, 0.0,
		s.z, u.z, -f.z, 0.0,
		-vec3_dot(s, eye), -vec3_dot(u, eye), vec3_dot(f, eye), 1.0
	};

	return view;
}

void mat4_translate(mat4 *m, vec3 v)
{
	m->f[12] = v.x;
	m->f[13] = v.y;
	m->f[14] = v.z;
}

mat4 identity_matrix(void)
{
	mat4 tmp = IDENTITY_MATRIX;
	return tmp;
}
