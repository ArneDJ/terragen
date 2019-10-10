#define IDENTITY_MATRIX {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}

typedef union {
	float f[2];
	struct { float x, y; };
} vec2;

typedef union {
	float f[3];
	struct { float x, y, z; };
} vec3;

typedef union {
	float f[4];
	struct { float x, y, z, w; };
} vec4;

typedef union {
	float f[9];
	vec3 row[3];
} mat3;

typedef union {
	float f[16];
	vec4 row[4];
} mat4;

struct sphere {
	vec3 c; // sphere center
	float r; // sphere radius
};

struct AABB {
	vec3 c; // center point of AABB
	float r[3]; // radius or halfwidth extents (rx, ry, rz)
};

struct OBB {
	vec3 c; // OBB center point
	vec3 u[3]; // local x, y, z axes
	vec3 e; // positive halfwidth extends of OBB along each axis
};

vec3 vec3_make(float x, float y, float z);
vec3 vec3_scale(float s, vec3 v);
vec3 vec3_normalize(vec3 v);
vec3 vec3_sum(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_cross(vec3 a, vec3 b);
vec3 vec3_crossn(vec3 a, vec3 b);

mat4 make_project_matrix(int fov, float aspect, float near, float far);

mat4 make_view_matrix(vec3 eye, vec3 center, vec3 up);

mat4 identity_matrix(void);

void mat4_translate(mat4 *m, vec3 v);
mat4 identity_matrix(void);
int test_ray_AABB(vec3 p, vec3 d, struct AABB a);
int test_ray_sphere(vec3 p, vec3 d, struct sphere s);
