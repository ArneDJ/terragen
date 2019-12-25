/* image manipulation library */

void plot(int x, int y, unsigned char *image, int width, int height, int nchannels, unsigned char color[3]);

int floodfill(int x, int y, unsigned char *image, int width, int height, unsigned char old, unsigned char new);

void draw_line(int x0, int y0, int x1, int y1, unsigned char* image, int width, int height, int nchannels, unsigned char* color);

void draw_thick_line(int x0, int y0, int x1, int y1, unsigned char* image, int width, int height, unsigned char *color, float wd);

void draw_triangle(float x0, float y0, float x1, float y1, float x2, float y2, unsigned char *image, int width, int height, unsigned char *color);

void draw_dist_triangle(float centerx, float centery, float x1, float y1, float x2, float y2, unsigned char *image, int width, int height);

void gauss_filter_rgb(int x, int y, const unsigned char *image, int width, int height, unsigned char rgb[3]);

/* voronoi diagram */
unsigned char *do_voronoi(int width, int height);
unsigned char *voronoi_rivers(int width, int height);
unsigned char *voronoi_mountains(int width, int height);
