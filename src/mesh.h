#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct vertex {
	float position[3];
	float normal[3];
	float uv[2];
};

struct mesh {
	GLuint VAO;
	GLsizei vcount;
};

struct mesh make_cube_mesh(void);
