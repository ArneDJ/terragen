#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct vertex {
	vec3 position;
	vec3 normal;
	vec2 uv;
};

struct mesh {
	GLuint VAO;
	GLsizei vcount;
};

struct mesh make_cube_mesh(void);

void instance_mesh(struct mesh *m, int amount, vec3 *positions);




struct mesh make_grid_mesh(int width, int length, float offset);
