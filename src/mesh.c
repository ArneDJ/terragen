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

void instance_mesh(struct mesh *m, int amount, vec3 *positions)
{
	mat4 *model = calloc(amount, sizeof(mat4));
	for(int i = 0; i < amount; i++) {
		model[i] = identity_matrix();
		mat4_translate(&model[i], positions[i]);
	}

	glBindVertexArray(m->VAO);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(mat4), &model[0], GL_STATIC_DRAW);

	for(int i = 0; i < amount; i++) {
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

mat4 identity_matrix(void)
{
	mat4 tmp = IDENTITY_MATRIX;
	return tmp;
}
