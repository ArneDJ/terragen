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

float smoothstep(float edge0, float edge1, float x)
{
	float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
	return t * t * (3.0 - 2.0 * t);
}

/* Vector stuff
 *
 *
 *
 *
 */

float vec2_dot(vec2 a, vec2 b)
{
	return a.x * b.x + a.y * b.y;
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

/*
 *
 *	Matrix stuff
 *
 */
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

static mat4 rot_mat4_mat4(mat4 m1, mat4 m2)
{
	mat4 dest;

	float a00 = m1.f[0], a01 = m1.f[1], a02 = m1.f[2], a03 = m1.f[3],
	a10 = m1.f[4], a11 = m1.f[5], a12 = m1.f[6], a13 = m1.f[7],
	a20 = m1.f[8], a21 = m1.f[9], a22 = m1.f[10], a23 = m1.f[11],
	a30 = m1.f[12], a31 = m1.f[13], a32 = m1.f[14], a33 = m1.f[15],

	b00 = m2.f[0], b01 = m2.f[1], b02 = m2.f[2],
	b10 = m2.f[4], b11 = m2.f[5], b12 = m2.f[6],
	b20 = m2.f[8], b21 = m2.f[9], b22 = m2.f[10];

	dest.f[0] = a00 * b00 + a10 * b01 + a20 * b02;
	dest.f[1] = a01 * b00 + a11 * b01 + a21 * b02;
	dest.f[2] = a02 * b00 + a12 * b01 + a22 * b02;
	dest.f[3] = a03 * b00 + a13 * b01 + a23 * b02;

	dest.f[4] = a00 * b10 + a10 * b11 + a20 * b12;
	dest.f[5] = a01 * b10 + a11 * b11 + a21 * b12;
	dest.f[6] = a02 * b10 + a12 * b11 + a22 * b12;
	dest.f[7] = a03 * b10 + a13 * b11 + a23 * b12;

	dest.f[8] = a00 * b20 + a10 * b21 + a20 * b22;
	dest.f[9] = a01 * b20 + a11 * b21 + a21 * b22;
	dest.f[10] = a02 * b20 + a12 * b21 + a22 * b22;
	dest.f[11] = a03 * b20 + a13 * b21 + a23 * b22;

	dest.f[12] = a30;
	dest.f[13] = a31;
	dest.f[14] = a32;
	dest.f[15] = a33;

	return dest;
}

void mat4_translate(mat4 *m, vec3 v)
{
	m->f[12] = v.x;
	m->f[13] = v.y;
	m->f[14] = v.z;
}

mat4 mat4_rotate_x(mat4 matrix, float angle)
{
	mat4 unit = IDENTITY_MATRIX;
	float c = cosf(angle);
	float s = sinf(angle);

	unit.f[5] = c;
	unit.f[6] = s;
	unit.f[9] = -s;
	unit.f[10] = c;

	return rot_mat4_mat4(matrix, unit);
}

mat4 mat4_rotate_y(mat4 matrix, float angle)
{
	mat4 unit = IDENTITY_MATRIX;
	float c = cosf(angle);
	float s = sinf(angle);

	unit.f[0] = c;
	unit.f[2] = -s;
	unit.f[8] = s;
	unit.f[10] = c;

	return rot_mat4_mat4(matrix, unit);
}

mat4 mat4_rotate_z(mat4 matrix, float angle)
{
	mat4 unit = IDENTITY_MATRIX;
	float c = cosf(angle);
	float s = sinf(angle);

	unit.f[0] = c;
	unit.f[1] = s;
	unit.f[4] = -s;
	unit.f[5] = c;

	return rot_mat4_mat4(matrix, unit);
}

mat4 mat4_rotate_xyz(mat4 matrix, float x, float y, float z)
{
	mat4 mat;
	mat = mat4_rotate_x(matrix, x);
	mat = mat4_rotate_y(mat, y);
	mat = mat4_rotate_z(mat, z);

	return mat;
}

mat4 identity_matrix(void)
{
	mat4 tmp = IDENTITY_MATRIX;
	return tmp;
}

/*
 *
 *	Ray intersection stuff
 *
 */
int ray_intersects_triangle(vec3 rayOrigin, vec3 rayVector, struct triangle *tri, vec3 *out, float *dist)
{
	const float EPSILON = 0.0000001;
	vec3 vertex0 = tri->a;
	vec3 vertex1 = tri->b;
	vec3 vertex2 = tri->c;
	vec3 edge1, edge2, h, s, q;
	float a,f,u,v;
	edge1 = vec3_sub(vertex1, vertex0);
	edge2 = vec3_sub(vertex2, vertex0);
	h = vec3_cross(rayVector, edge2);
	a = vec3_dot(edge1, h);

	if (a > -EPSILON && a < EPSILON)
		return 0;    // This ray is parallel to this triangle.

	f = 1.0/a;
	s = vec3_sub(rayOrigin, vertex0);
	u = f * vec3_dot(s, h);

	if (u < 0.0 || u > 1.0)
		return 0;

	q = vec3_cross(s, edge1);
	v = f * vec3_dot(rayVector, q);

	if (v < 0.0 || u + v > 1.0)
		return 0;

	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * vec3_dot(edge2, q);
	if (t > EPSILON && t < 1/EPSILON) {
		vec3 tmp = vec3_sum(rayOrigin, vec3_scale(t, rayVector));
		out->x = tmp.x; out->y = tmp.y; out->z = tmp.z;
		dist = &t;
		return 1;
	} else // This means that there is a line intersection but not a ray intersection.
		return 0;
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
