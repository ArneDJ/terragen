#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "texture.h"
#include "gmath.h"
#include "noise.h"
#include "voronoi.h"

GLuint load_dds_texture(const char *fpath) 
{
	GLuint texnum;
	/* load the texture image from dds */
	struct dds header;
	FILE *fp = fopen(fpath, "rb");
	if (fp == NULL) {
		perror(fpath);
		return 0;
	}

	/* verify the type of file */
	fread(header.identifier, 1, 4, fp);
	if (strncmp(header.identifier, "DDS ", 4) != 0) {
		printf("error: %s: not a valid DDS file\n", fpath);
		fclose(fp);
		return 0;
	}

	/* the header is 128 bytes, 124 after reading the file type */
	unsigned char header_buf[124];
	fread(&header_buf, 124, 1, fp);
	header.height = *(uint32_t*)&(header_buf[8]);
	header.width = *(uint32_t*)&(header_buf[12]);
	header.linear_size = *(uint32_t*)&(header_buf[16]);
	header.mip_levels = *(uint32_t*)&(header_buf[24]);
	header.dxt_codec = *(uint32_t*)&(header_buf[80]);

	/* now get the actual image data */
	unsigned char *image;
	uint32_t len = (header.mip_levels > 1) ? (header.linear_size * 2) : header.linear_size;
	image = calloc(len, sizeof(unsigned char));
	fread(image, 1, len, fp);

	/* now opengl */
	uint32_t components = (header.dxt_codec == FOURCC_DXT1) ? 3 : 4;
	uint32_t format;
	uint32_t block_size;
	switch (header.dxt_codec) {
	case FOURCC_DXT1: format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; block_size = 8; break;
	case FOURCC_DXT3: format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; block_size = 16; break;
	case FOURCC_DXT5: format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; block_size = 16; break;
	default: 
		printf("error: not a valid DXT format found for %s\n", fpath); 
		free(image); 
		return 0;
	};

	/* now make the opengl texture */
	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, header.mip_levels-1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	unsigned int offset = 0;
	for (unsigned int i = 0; i < header.mip_levels; i++) {
		if (header.width <= 4 || header.height <= 4) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i-1);
			break;
		}
		/* now to actually get the compressed image into opengl */
		unsigned int size = ((header.width+3)/4) * ((header.height+3)/4) * block_size;
		glCompressedTexImage2D(GL_TEXTURE_2D, i, format, 
				header.width, header.height, 0, size, image + offset);

		offset += size;
		header.width = header.width/2;
		header.height = header.height/2;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	fclose(fp);
	free(image);

	return texnum;
}





GLuint make_rgb_texture(rgb *image, int width, int height)
{
	GLuint texnum;

	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texnum;
}

GLuint make_r_texture(unsigned char *image, int width, int height)
{
	GLuint texnum;

	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, image);
	// to prevent terrain terracing
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texnum;
}

/*
GLuint make_voronoi_texture(void)
{
	GLuint texnum;
	size_t len = 1024 * 1024;
	unsigned char *buf = gen_voronoi_map();
	rgb *image = calloc(len, sizeof(rgb));
	int nbuf = 0;
	for (int i = 0; i < len; i++) {
		image[i][0] = buf[nbuf++];
		image[i][1] = buf[nbuf++];
		image[i][2] = buf[nbuf++];
	}

	texnum = make_rgb_texture(image, 1024, 1024);

	free(buf);
	free(image);

	return texnum;
}
*/

GLuint make_worley_texture(void)
{
	GLuint texnum;
	size_t len = 1024 * 1024;
	unsigned char *buf = gen_worley_map();
	rgb *image = calloc(len, sizeof(rgb));
	int nbuf = 0;
	for (int i = 0; i < len; i++) {
		image[i][0] = buf[nbuf++];
		image[i][1] = buf[nbuf++];
		image[i][2] = buf[nbuf++];
	}

	texnum = make_rgb_texture(image, 1024, 1024);

	free(buf);
	free(image);

	return texnum;
}

GLuint make_voronoi_texture(void)
{
	GLuint texnum;
	size_t len = 512 * 512;
	unsigned char *buf = do_voronoi();
	rgb *image = calloc(len, sizeof(rgb));
	int nbuf = 0;
	for (int i = 0; i < len; i++) {
		image[i][0] = buf[nbuf++];
		image[i][1] = buf[nbuf++];
		image[i][2] = buf[nbuf++];
	}

	texnum = make_rgb_texture(image, 512, 512);

	free(buf);
	free(image);

	return texnum;
}

GLuint make_river_texture(void)
{
	GLuint texnum;
	size_t len = 512 * 512;
	unsigned char *buf = voronoi_rivers();
	rgb *image = calloc(len, sizeof(rgb));
	int nbuf = 0;
	for (int i = 0; i < len; i++) {
		image[i][0] = buf[nbuf++];
		image[i][1] = buf[nbuf++];
		image[i][2] = buf[nbuf++];
	}

	texnum = make_rgb_texture(image, 512, 512);

	free(buf);
	free(image);

	return texnum;
}


GLuint make_depth_texture(void)
{

}


