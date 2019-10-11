#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "shader.h"

static char *import(const char *filename);

GLuint load_shaders(struct shader *shaders)
{
	if (shaders == NULL)
		return 0;

 	GLuint program = glCreateProgram();

	struct shader *entry = shaders;
	while (entry->type != GL_NONE) {
		GLuint shader = glCreateShader(entry->type);
		entry->program = shader;
		char *source = import(entry->fpath);

		//if no file was found or something worse
		if (source == NULL) {
			for (entry = shaders; entry->type != GL_NONE; entry++) {
				glDeleteShader(entry->program);
				entry->program = 0;
			}
			return 0;
		}

		glShaderSource(shader, 1, (const char **)&source, NULL);
		free(source);

		glCompileShader(shader);
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		//if shader compilation failed print the error logs
		if (!compiled) {
			GLsizei len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		 	GLchar *log = calloc(len + 1, sizeof(GLchar));
			glGetShaderInfoLog(shader, len, &len, log);
			printf("%s\n", log);
		
			free(log);
			return 0;
		}
		glAttachShader(program, shader);
		entry++;
	}

	glLinkProgram(program);
	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if (!linked) {
		GLsizei len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

		GLchar *log = calloc(len + 1, sizeof(GLchar));
		glGetProgramInfoLog(program, len, &len, log);
		printf("%s\n", log);
		free(log);
		
		for (entry = shaders; entry->type != GL_NONE; entry++) {
			glDeleteShader(entry->program);
			entry->program = 0;
		}
		return 0;
	}
	
	return program;
}

static char *import(const char *filename)
{
	char *source;
	FILE *file;

	if (file = fopen(filename, "rb")){
		fseek(file, 0, SEEK_END);
		const long int len = ftell(file);
		rewind(file);
		source = calloc(len + 1, sizeof(char));
		fread(source, 1, len, file);
		fclose(file);
	} else 
		perror(filename);
		
	return source;
}
