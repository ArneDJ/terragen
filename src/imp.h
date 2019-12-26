/* image manipulation library */

struct image {
	int width; /* in pixels */
	int height; /* in pixels */
	unsigned char *data; /* raw image data */
	int nchannels; /* amount of channels, example: rgb has 3 */
	size_t size; /* size in bytes */
};

void plot(int x, int y, unsigned char *image, int width, int height, int nchannels, unsigned char *color);

int floodfill(int x, int y, unsigned char *image, int width, int height, unsigned char old, unsigned char new);

void draw_line(int x0, int y0, int x1, int y1, unsigned char* image, int width, int height, int nchannels, unsigned char* color);

void draw_thick_line(int x0, int y0, int x1, int y1, unsigned char* image, int width, int height, unsigned char *color, float wd);

void draw_triangle(float x0, float y0, float x1, float y1, float x2, float y2, unsigned char *image, int width, int height, unsigned char *color);

void draw_dist_triangle(float centerx, float centery, float x1, float y1, float x2, float y2, unsigned char *image, int width, int height);

/* voronoi diagram */
void do_voronoi(int width, int height, unsigned char *image);
void voronoi_rivers(int width, int height, unsigned char *image);
void voronoi_mountains(int width, int height, unsigned char *image);

float fbm_noise(float x, float y, float freq, float lacun, float gain);
float worley_noise(float x, float y);
