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
#include "vec.h"

static GLuint make_rgb_texture(unsigned char *buf, int width, int height);
static void rgbchannel(rgb *image, unsigned char *buf, int width, int height);
static unsigned char *gen_worley_map(int size_x, int size_y);
static unsigned char *gen_perlin_map(int size_x, int size_y);

void plot(int x, int y, unsigned char *image, int width, int height, int nchannels, unsigned char *color)
{
    if( x < 0 || y < 0 || x > (width-1) || y > (height-1) )
        return;
    int index = y * width * nchannels + x * nchannels;
    for( int i = 0; i < nchannels; ++i )
    {
        image[index+i] = color[i];
    }
}


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

GLuint make_voronoi_texture(int width, int height)
{
	unsigned char *buf = do_voronoi(width, height);
	GLuint texnum = make_rgb_texture(buf, width, height);

	free(buf);

	return texnum;
}

GLuint make_river_texture(int width, int height)
{
	unsigned char *buf = voronoi_rivers(width, height);

	unsigned char *cpy = calloc(3 * width * height, sizeof(unsigned char));
	memcpy(cpy, buf, 3 * width * height);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < width; y++) {
			unsigned char rgb[3];
			gauss_filter_rgb(x, y, buf, width, height, rgb);
			plot(x, y, cpy, width, height, 3, rgb);
		}
	}
	memcpy(buf, cpy, 3 * width * height);

	GLuint texnum = make_rgb_texture(buf, width, height);

	free(cpy);
	free(buf);

	return texnum;
}

GLuint make_mountain_texture(int width, int height)
{
	unsigned char *buf = voronoi_mountains(width, height);

	 size_t isize = width * height * 3;
	 unsigned char *cpy = calloc(isize, sizeof(unsigned char));


 for (int i = 0; i < 3; i++) {
 memcpy(cpy, buf, isize);
 for (int x = 0; x < width; x++) {
  for (int y = 0; y < height; y++) {
   unsigned char rgb[3];
   gauss_filter_rgb(x, y, buf, width, height, rgb);
   plot(x, y, cpy, width, height, 3, rgb);
  }
 }
 memcpy(buf, cpy, isize);
 }

	GLuint texnum = make_rgb_texture(buf, width, height);

	free(cpy);
	free(buf);

	return texnum;
}

GLuint make_worley_texture(int width, int height)
{
	unsigned char *buf = gen_worley_map(width, height);
	GLuint texnum = make_r_texture(buf, width, height);

	free(buf);

	return texnum;
}

GLuint make_perlin_texture(int width, int height)
{
	unsigned char *buf = gen_perlin_map(width, height);
	GLuint texnum = make_r_texture(buf, width, height);

	free(buf);

	return texnum;
}

static void apply_filter_rgb(const unsigned char *image, int width, int height, int x, int y, int xoff, int yoff, float *r, float *g, float *b)
{
 if( x+xoff < 0 || y+yoff < 0 || x+xoff > (width-1) || y+yoff > (height-1) )
         return;

     int index = (y+yoff) * width * 3 + (x+xoff) * 3;

 //int radius = 80;
 //float stdd = sqrt(-(radius*radius) / (2.0*log10f(1.0 / 255.0)));
 /* good sigma value: 8.5 */
 //float gauss = gaussian(xoff, yoff, 8.5);
 //printf("%f\n", gauss);
 //
 float boxk = 1.0 / 448.0;;

 *r += (image[index]/255.f) * boxk;
 *g += (image[index+1]/255.f) * boxk;
 *b += (image[index+2]/255.f) * boxk;
}

void gauss_filter_rgb(int x, int y, const unsigned char *image, int width, int height, unsigned char rgb[3])
{
 float red = 0.0;
 float green = 0.0;
 float blue = 0.0;

 /* center pixel */
 /* surrounding pixels */
 /* good kernel value: 51 , 6 times larger than standard deviation*/
 const int KERNEL_SIZE = 11;

 for (int kx = -KERNEL_SIZE; kx < KERNEL_SIZE+1; kx++) {
  for (int ky = -KERNEL_SIZE; ky < KERNEL_SIZE+1; ky++) {
   apply_filter_rgb(image, width, height, x, y, kx, ky, &red, &green, &blue);
  }
 }

 red = clamp(red, 0.0, 1.0);
 green = clamp(green, 0.0, 1.0);
 blue = clamp(blue, 0.0, 1.0);
 rgb[0] = red * 255.0;
 rgb[1] = green * 255.0;
 rgb[2] = blue * 255.0;
}


static GLuint make_rgb_texture(unsigned char *buf, int width, int height)
{
	GLuint texnum;

	rgb *image = calloc(width*height, sizeof(rgb));
	rgbchannel(image, buf, width, height);

	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, width, height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
	
	/* smooth mag filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	free(image);
	return texnum;
}

static void rgbchannel(rgb *image, unsigned char *buf, int width, int height)
{
	size_t len = width * height;
	int nchannel = 0;
	for (int i = 0; i < len; i++) {
		image[i][0] = buf[nchannel++];
		image[i][1] = buf[nchannel++];
		image[i][2] = buf[nchannel++];
	}
}

static unsigned char *gen_worley_map(int size_x, int size_y)
{
	unsigned char *buf = calloc(size_x*size_y, sizeof(unsigned char));

	int nbuf = 0;
	for(int x = 0; x < size_x; x++) {
		for(int y = 0; y < size_y; y++) {
			float z = sqrt(worley_noise(0.1*x, 0.1*y));
			//z[i] = 1.0 - sqrt(z[i]); //if you want steep mountains
			//z = 1.0 - z; //if you want normal mountains
			buf[nbuf++] = 255*z;
		}
	}

	return buf;
}

static unsigned char *gen_perlin_map(int size_x, int size_y)
{
	const size_t len = size_x * size_y;
	unsigned char *buf = calloc(len, sizeof(unsigned char));

	int nbuf = 0;
	for(int x = 0; x < size_x; x++) {
		for(int y = 0; y < size_y; y++) {
			float z = fbm_noise(0.5*x, 0.5*y, 0.005, 2.5, 2.0);
			buf[nbuf++] = z * 255.0;
		}
	}

	return buf;
}

static void push(vec_int_t *stack, int x, int y)
{
	vec_push(stack, x);
	vec_push(stack, y);
}

static int pop(vec_int_t *stack, int *x, int *y)
{
	if(stack->length < 2)
		return 0; // it's empty

	*y = vec_pop(stack);
	*x = vec_pop(stack);

	return 1;
}

int floodfill(int x, int y, unsigned char *image, int width, int height, unsigned char old, unsigned char new)
{
 if(old == new) {
  return 1;
 }

 int x1;
 int above, below;
 int size = 0;

 vec_int_t stack;
 vec_init(&stack);
 push(&stack, x, y);

 while(pop(&stack, &x, &y)) {
  x1 = x;

  while(x1 >= 0 && image[y * width + x1] == old) {
   x1--;
  }

  x1++;
  above = below = 0;
  while(x1 < width && image[y * width + x1] == old) {
   image[y * width + x1] = new;
   size++;

   if(!above && y > 0 && image[(y - 1) * width + x1] == old) {
    push(&stack, x1, y - 1);
    above = 1;
   } else if(above && y > 0 && image[(y - 1) * width + x1] != old) {
    above = 0;
   }

   if(!below && y < height - 1 && image[(y + 1) * width + x1] == old) {
    push(&stack, x1, y + 1);
    below = 1;
   }
   else if(below && y < height - 1 && image[(y + 1) * width + x1] != old) {
    below = 0;
   }

   x1++;
  }
 }

 vec_deinit(&stack);
 return size;
}

