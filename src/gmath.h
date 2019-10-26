#define IDENTITY_MATRIX {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define frand(x) (rand() / (1. + RAND_MAX) * x)

typedef union {
	float f[2]; struct { float x, y; };
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

struct triangle {
	vec3 a, b, c;
	vec3 normal;
};

struct rect {
	vec2 min;
	vec2 max;
};

struct quadtree {
	struct rect area;
	float minheight;
	float maxheight;
};

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

float lerp(float a, float b, float t);
float fract(float x);
float clamp(float n, float min, float max);

float vec2_dot(vec2 a, vec2 b);

vec3 vec3_make(float x, float y, float z);
vec3 vec3_scale(float s, vec3 v);
vec3 vec3_normalize(vec3 v);
vec3 vec3_sum(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_cross(vec3 a, vec3 b);
vec3 vec3_crossn(vec3 a, vec3 b);
float vec3_magnitude(vec3 v);

mat4 make_project_matrix(int fov, float aspect, float near, float far);
mat4 make_view_matrix(vec3 eye, vec3 center, vec3 up);
mat4 identity_matrix(void);
void mat4_translate(mat4 *m, vec3 v);
mat4 identity_matrix(void);
mat4 mat4_rotate_x(mat4 matrix, float angle);
mat4 mat4_rotate_y(mat4 matrix, float angle);
mat4 mat4_rotate_z(mat4 matrix, float angle);
mat4 mat4_rotate_xyz(mat4 matrix, float x, float y, float z);

int test_ray_AABB(vec3 p, vec3 d, struct AABB a);
int test_ray_sphere(vec3 p, vec3 d, struct sphere s);
vec3 barycentric_ray_triangle(vec3 p, vec3 q, vec3 a, vec3 b, vec3 c);
vec3 barycentric_to_cartesian(vec3 bar, vec3 a, vec3 b, vec3 c);
int ray_intersects_triangle(vec3 rayOrigin, vec3 rayVector, struct triangle *tri, vec3 *out, float *dist);
