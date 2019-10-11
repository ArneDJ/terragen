struct shader {
	GLenum type;
	const char *fpath;
	GLuint program;
};

//  LoadShaders() takes an array of ShaderFile structures, each of which
//    contains the type of the shader, and a pointer a C-style character
//    string (i.e., a NULL-terminated array of characters) containing the
//    entire shader source.
//
//  The array of structures is terminated by a final Shader with the
//    "type" field set to GL_NONE.
//
//  loadShaders() returns the shader program value (as returned by
//    glCreateProgram()) on success, or zero on failure. 
GLuint load_shaders(struct shader *shaders);
