#include <GL/glew.h>
#include <GL/gl.h>
#include "gmath.h"
#include "mesh.h"
#include "display.h"

void display_skybox(struct mesh box)
{
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);

	glBindVertexArray(box.VAO);
	glDrawArrays(GL_TRIANGLES, 0, box.vcount);

	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}
