#include <GL/glew.h>
#include <GL/gl.h>
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
