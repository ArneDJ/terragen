#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gmath.h"

static void mult4x4(const mat4 a, const mat4 b, mat4 dest);
static void rotmat(mat4 m1, mat4 m2, mat4 dest);
static void identity(mat4 m);

void vec3_normalize(vec3 vec)
{
	float sum = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
	float len = sqrt(sum);

	vec[0] = vec[0] / len;
	vec[1] = vec[1] / len;
	vec[2] = vec[2] / len;
}

void vec3_cross(vec3 a, vec3 b, vec3 dest)
{
	dest[0] = a[1] * b[2] - a[2] * b[1];
	dest[1] = a[2] * b[0] - a[0] * b[2];
	dest[2] = a[0] * b[1] - a[1] * b[0];
}

void vec3_scale(vec3 vec, float scalar)
{
	vec[0] = scalar * vec[0];
	vec[1] = scalar * vec[1];
	vec[2] = scalar * vec[2];
}

void vec3_add(vec3 a, vec3 b, vec3 dest) 
{
	dest[0] = a[0] + b[0];
	dest[1] = a[1] + b[1];
	dest[2] = a[2] + b[2];
}

void vec3_sub(vec3 a, vec3 b, vec3 dest) 
{
	dest[0] = a[0] - b[0];
	dest[1] = a[1] - b[1];
	dest[2] = a[2] - b[2];
}

float vec3_dot(vec3 a, vec3 b) 
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vec3_crossn(vec3 a, vec3 b, vec3 dest) 
{
	vec3_cross(a, b, dest);
	vec3_normalize(dest);
}

void mat4_translate(mat4 matrix, vec3 translation)
{
	vec4 v1 = {matrix[0]*translation[0], matrix[1]*translation[0], 
		matrix[2]*translation[0], matrix[3]*translation[0]};
	vec4 v2 = {matrix[4]*translation[1], matrix[5]*translation[1], 
		matrix[6]*translation[1], matrix[7]*translation[1]};
	vec4 v3 = {matrix[8]*translation[2], matrix[9]*translation[2], 
		matrix[10]*translation[2], matrix[11]*translation[2]};

	matrix[12] += v1[0];
	matrix[13] += v1[1];
	matrix[14] += v1[2];
	matrix[15] += v1[3];

	matrix[12] += v2[0];
	matrix[13] += v2[1];
	matrix[14] += v2[2];
	matrix[15] += v2[3];

	matrix[12] += v3[0];
	matrix[13] += v3[1];
	matrix[14] += v3[2];
	matrix[15] += v3[3];
}

void mat4_scale(mat4 matrix, vec3 scale)
{
	matrix[0] *= scale[0]; 
	matrix[1] *= scale[0]; 
	matrix[2] *= scale[0]; 
	matrix[3] *= scale[0];
	
	matrix[4] *= scale[1]; 
	matrix[5] *= scale[1]; 
	matrix[6] *= scale[1]; 
	matrix[7] *= scale[1];
	
	matrix[8] *= scale[2]; 
	matrix[9] *= scale[2]; 
	matrix[10] *= scale[2]; 
	matrix[11] *= scale[2];
}

void mat4_rotate(mat4 matrix, vec3 rotation)
{
	mat4_rotate_x(matrix, rotation[0]);
	mat4_rotate_y(matrix, rotation[1]);
	mat4_rotate_z(matrix, rotation[2]);
}

void mat4_rotate_x(mat4 matrix, float angle)
{
	mat4 unit = IDENTITY_MATRIX; 
	float c = cosf(angle);
	float s = sinf(angle);

	unit[5] = c;
	unit[6] = s;
	unit[9] = -s;
	unit[10] = c;

	rotmat(matrix, unit, matrix);
}

void mat4_rotate_y(mat4 matrix, float angle)
{
	mat4 unit = IDENTITY_MATRIX; 
	float c = cosf(angle);
	float s = sinf(angle);

	unit[0] = c;
	unit[2] = -s;
	unit[8] = s;
	unit[10] = c;

	rotmat(matrix, unit, matrix);
}

void mat4_rotate_z(mat4 matrix, float angle)
{
	mat4 unit = IDENTITY_MATRIX; 
	float c = cosf(angle);
	float s = sinf(angle);

	unit[0] = c;
	unit[1] = s;
	unit[4] = -s;
	unit[5] = c;

	rotmat(matrix, unit, matrix);
}

void make_projection_matrix(mat4 matrix, int angle, float aspect, float near, float far)
{
	float fov = (float)angle * ((2.0 * M_PI) / 360.0);
	float range = tan(fov * 0.5) * near;

	float x = (2.0 * near) / (range * aspect + range * aspect);
	float y = near / range;
	float z = -(far + near) / (far - near);
	float w = -(2.0 * far * near) / (far - near);

	const mat4 temp = {
		x, 0.0, 0.0, 0.0,
		0.0, y, 0.0, 0.0,
		0.0, 0.0, z, -1.0,
		0.0, 0.0, w, 0.0
	};

	memcpy(matrix, temp, sizeof(mat4));
}

void make_view_matrix(mat4 matrix, vec3 eye, vec3 center, vec3 up)
{
	vec3 target = {0};
	vec3_add(eye, center, target);
	vec3 f, u, s;

	vec3_sub(target, eye, f);
	vec3_normalize(f);

	vec3_crossn(f, up, s);
	vec3_cross(s, f, u);

	matrix[0] = s[0];
	matrix[1] = u[0];
	matrix[2] = -f[0];
	matrix[4] = s[1];
	matrix[5] = u[1];
	matrix[6] = -f[1];
	matrix[7] = 0.0f;
	matrix[8] = s[2];
	matrix[9] = u[2];
	matrix[10] = -f[2];
	matrix[11] = 0.0f;
	matrix[12] = -vec3_dot(s, eye);
	matrix[13] = -vec3_dot(u, eye);
	matrix[14] = vec3_dot(f, eye);
	matrix[15] = 1.0f;
}

static void mult4x4(const mat4 a, const mat4 b, mat4 dest)
{
	dest[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8] + a[3]*b[12];
	dest[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9] + a[3]*b[13];
	dest[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
	dest[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

	dest[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8] + a[7]*b[12];
	dest[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9] + a[7]*b[13];
	dest[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
	dest[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

	dest[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
	dest[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
	dest[10] = a[8]*b[2] + a[9]*b[6] + a[10]*b[10] + a[11]*b[14];
	dest[11] = a[8]*b[3] + a[9]*b[7] + a[10]*b[11] + a[11]*b[15];

	dest[12] = a[12]*b[0] + a[13]*b[4] + a[14]*b[8] + a[15]*b[12];
	dest[13] = a[12]*b[1] + a[13]*b[5] + a[14]*b[9] + a[15]*b[13];
	dest[14] = a[12]*b[2] + a[13]*b[6] + a[14]*b[10] + a[15]*b[14];
	dest[15] = a[12]*b[3] + a[13]*b[7] + a[14]*b[11] + a[15]*b[15];
}

static void rotmat(mat4 m1, mat4 m2, mat4 dest)
{
	float a00 = m1[0], a01 = m1[1], a02 = m1[2], a03 = m1[3],
        a10 = m1[4], a11 = m1[5], a12 = m1[6], a13 = m1[7],
        a20 = m1[8], a21 = m1[9], a22 = m1[10], a23 = m1[11],
        a30 = m1[12], a31 = m1[13], a32 = m1[14], a33 = m1[15],

        b00 = m2[0], b01 = m2[1], b02 = m2[2],
        b10 = m2[4], b11 = m2[5], b12 = m2[6],
        b20 = m2[8], b21 = m2[9], b22 = m2[10];

	dest[0] = a00 * b00 + a10 * b01 + a20 * b02;
	dest[1] = a01 * b00 + a11 * b01 + a21 * b02;
	dest[2] = a02 * b00 + a12 * b01 + a22 * b02;
	dest[3] = a03 * b00 + a13 * b01 + a23 * b02;

	dest[4] = a00 * b10 + a10 * b11 + a20 * b12;
	dest[5] = a01 * b10 + a11 * b11 + a21 * b12;
	dest[6] = a02 * b10 + a12 * b11 + a22 * b12;
	dest[7] = a03 * b10 + a13 * b11 + a23 * b12;

	dest[8] = a00 * b20 + a10 * b21 + a20 * b22;
	dest[9] = a01 * b20 + a11 * b21 + a21 * b22;
	dest[10] = a02 * b20 + a12 * b21 + a22 * b22;
	dest[11] = a03 * b20 + a13 * b21 + a23 * b22;

	dest[12] = a30;
	dest[13] = a31;
	dest[14] = a32;
	dest[15] = a33;
}

static void identity(mat4 m)
{
	mat4 temp = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	memcpy(m, temp, sizeof(mat4));
}
