#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "gmath.h"
#include "mesh.h"
#include "primitives.h"

struct mesh make_cube_mesh(void)
{
	struct mesh m;
	m.vcount = CUBE_PRIMITIVE_COUNT;

	glGenVertexArrays(1, &m.VAO);
	glBindVertexArray(m.VAO);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_cube_primitives), g_cube_primitives, GL_STATIC_DRAW);

	/* vertex points */
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), BUFFER_OFFSET(0));

	/* normals */
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), BUFFER_OFFSET(3*sizeof(float)));

	/* texture uv */
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), BUFFER_OFFSET(6*sizeof(float)));

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(1, &vbo);

	return m;
}

static struct vertex make_plane_vertex(float x, float y, float z)
{
	struct vertex v;
	v.position.x = x;
	v.position.y = y;
	v.position.z = z;

	v.normal.x = 0.0;
	v.normal.y = 1.0;
	v.normal.z = 0.0;

	v.uv.x = x;
	v.uv.y = z;

	return v;
}

struct mesh make_patch_mesh(int width, int length, float offset)
{
	struct mesh m;
	m.vcount = 4 * width * length;

	vec3 *v = calloc(m.vcount, sizeof(vec3));

	vec2 origin = {0.0, 0.0};
	int nvertex = 0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < length; j++) {
			v[nvertex++] = vec3_make(origin.x, 0.0, origin.y);
			v[nvertex++] = vec3_make(origin.x+offset, 0.0, origin.y);
			v[nvertex++] = vec3_make(origin.x, 0.0, origin.y+offset);
			v[nvertex++] = vec3_make(origin.x+offset, 0.0, origin.y+offset);

			origin.x += offset;
		}
		origin.x = 0.0;
		origin.y += offset;
	}

	glGenVertexArrays(1, &m.VAO);
	glBindVertexArray(m.VAO);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, m.vcount * sizeof(vec3), v, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &vbo);

	free(v);
	return m;
}

struct mesh make_grid_mesh(int width, int length, float offset)
{
	struct mesh m;
	m.vcount = 3 * 2 * width * length;
	struct vertex *v = calloc(m.vcount, sizeof(struct vertex));

	vec2 origin = {0.0, 0.0};
	int n = 0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < length; j++) {
			v[n++] = make_plane_vertex(origin.x+offset, 0.0, origin.y+offset);
			v[n++] = make_plane_vertex(origin.x+offset, 0.0, origin.y);
			v[n++] = make_plane_vertex(origin.x, 0.0, origin.y);
			v[n++] = make_plane_vertex(origin.x, 0.0, origin.y+offset);
			v[n++] = make_plane_vertex(origin.x+offset, 0.0, origin.y+offset);
			v[n++] = make_plane_vertex(origin.x, 0.0, origin.y);

			origin.x += offset;
		}
		origin.x = 0.0;
		origin.y += offset;
	}

	glGenVertexArrays(1, &m.VAO);
	glBindVertexArray(m.VAO);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, m.vcount * sizeof(struct vertex), v, GL_STATIC_DRAW);

	/* vertex points */
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), BUFFER_OFFSET(0));

	/* normals */
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), BUFFER_OFFSET(3*sizeof(float)));

	/* texture uv */
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), BUFFER_OFFSET(6*sizeof(float)));

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(1, &vbo);

	free(v);
	return m;
}

void instance_mesh(struct mesh *m, int amount, vec3 *positions)
{
	mat4 *model = calloc(amount, sizeof(mat4));
	for (int i = 0; i < amount; i++) {
		model[i] = identity_matrix();
		mat4_translate(&model[i], positions[i]);
	}

	glBindVertexArray(m->VAO);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(mat4), &model[0], GL_STATIC_DRAW);

	for (int i = 0; i < amount; i++) {
		size_t len = sizeof(vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * len, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * len, BUFFER_OFFSET(len));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * len, BUFFER_OFFSET(2 * len));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * len, BUFFER_OFFSET(3 * len));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}

	free(model);
}

