#include <math.h>
#include "gmath.h"
#include "vec.h"

static void push(vec_int_t *stack, int x, int y);
static int pop(vec_int_t *stack, int *x, int *y);
static inline int orient(float x0, float y0, float x1, float y1, float x2, float y2);
static inline int min3(int a, int b, int c);
static inline int max3(int a, int b, int c);
static void boxfilter(const unsigned char *image, int width, int height, int x, int y, int xoff, int yoff, float *r, float *g, float *b);

void plot(int x, int y, unsigned char *image, int width, int height, int nchannels, unsigned char color[3])
{
	if (x < 0 || y < 0 || x > (width-1) || y > (height-1)) {
		return;
	}

	int index = y * width * nchannels + x * nchannels;

	for (int i = 0; i < nchannels; i++) {
		image[index+i] = color[i];
	}
}


// http://members.chello.at/~easyfilter/bresenham.html
void draw_line(int x0, int y0, int x1, int y1, unsigned char *image, int width, int height, int nchannels, unsigned char* color)
{
	int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = dx+dy, e2; // error value e_xy

	for(;;) {
		plot(x0,y0, image, width, height, 3, color);
		if (x0==x1 && y0==y1)
			break;
		e2 = 2*err;
		if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0
		if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0
	}
}

void draw_thick_line(int x0, int y0, int x1, int y1, unsigned char *image, int width, int height, unsigned char *color, float wd)
{
	int dx = abs(x1-x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0 < y1 ? 1 : -1;
	int err = dx-dy, e2, x2, y2;                          /* error value e_xy */
	float ed = dx+dy == 0 ? 1 : sqrt((float)dx*dx+(float)dy*dy);

	for (wd = (wd+1)/2; ; ) {                                   /* pixel loop */
		plot(x0,y0, image, width, height, 3, color);
		//plot(x0,y0, image, width, height, nchannels, max(0,255*(abs(err-dx+dy)/ed-wd+1)));
		e2 = err; x2 = x0;
		if (2*e2 >= -dx) {                                           /* x step */
			for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
				plot(x0, y2 += sy, image, width, height, 3, color);
			if (x0 == x1) 
				break;
			e2 = err; err -= dy; x0 += sx;
		}
		if (2*e2 <= dy) {                                            /* y step */
			for (e2 = dx-e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
				plot(x2 += sx, y0, image, width, height, 3, color);
			if (y0 == y1) 
				break;
			err += dx; y0 += sy;
		}
	}
}

void draw_triangle(float x0, float y0, float x1, float y1, float x2, float y2, unsigned char *image, int width, int height, unsigned char *color)
{
	int area = orient(x0, y0, x1, y1, x2, y2);
	if (area == 0)
		return;

	// Compute triangle bounding box
	int minX = min3((int)x0, (int)x1, (int)x2);
	int minY = min3((int)y0, (int)y1, (int)y2);
	int maxX = max3((int)x0, (int)x1, (int)x2);
	int maxY = max3((int)y0, (int)y1, (int)y2);

	// Clip against screen bounds
	minX = max(minX, 0);
	minY = max(minY, 0);
	maxX = min(maxX, width - 1);
	maxY = min(maxY, height - 1);

	// Rasterize
	float px, py;
	for (py = minY; py <= maxY; py++) {
		for (px = minX; px <= maxX; px++) {
			// Determine barycentric coordinates
			int w0 = orient(x1, y1, x2, y2, px, py);
			int w1 = orient(x2, y2, x0, y0, px, py);
			int w2 = orient(x0, y0, x1, y1, px, py);

			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				plot((int)px, (int)py, image, width, height, 3, color);
			}
		}
	}
}

void draw_dist_triangle(float centerx, float centery, float x1, float y1, float x2, float y2, unsigned char *image, int width, int height)
{
	int area = orient(centerx, centery, x1, y1, x2, y2);
	if (area == 0)
		return;

	// Compute triangle bounding box
	int minX = min3((int)centerx, (int)x1, (int)x2);
	int minY = min3((int)centery, (int)y1, (int)y2);
	int maxX = max3((int)centerx, (int)x1, (int)x2);
	int maxY = max3((int)centery, (int)y1, (int)y2);

	// Clip against screen bounds
	minX = max(minX, 0);
	minY = max(minY, 0);
	maxX = min(maxX, width - 1);
	maxY = min(maxY, height - 1);

	// Rasterize
	float px, py;
	for (py = minY; py <= maxY; py++) {
		for (px = minX; px <= maxX; px++) {
			// Determine barycentric coordinates
			int w0 = orient(x1, y1, x2, y2, px, py);
			int w1 = orient(x2, y2, centerx, centery, px, py);
			int w2 = orient(centerx, centery, x1, y1, px, py);

			//unsigned char dist_color = vec2_dist(
			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				vec2 a = {px, py};
				vec2 b = {centerx, centery};
				float dist = 1.0 - (vec2_dist(a, b) / 150.0);
				unsigned char dist_color[3] = {255.0*dist, 255.0*dist, 255.0*dist};
				plot((int)px, (int)py, image, width, height, 3, dist_color);
			}
		}
	}
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

	while (pop(&stack, &x, &y)) {
		x1 = x;

		while (x1 >= 0 && image[y * width + x1] == old) {
			x1--;
		}

		x1++;
		above = below = 0;
		while (x1 < width && image[y * width + x1] == old) {
			image[y * width + x1] = new;
			size++;

			if (!above && y > 0 && image[(y - 1) * width + x1] == old) {
				push(&stack, x1, y - 1);
				above = 1;
			} else if(above && y > 0 && image[(y - 1) * width + x1] != old) {
				above = 0;
			}

			if (!below && y < height - 1 && image[(y + 1) * width + x1] == old) {
				push(&stack, x1, y + 1);
				below = 1;
			} else if (below && y < height - 1 && image[(y + 1) * width + x1] != old) {
				below = 0;
			}

			x1++;
		}
	}

	vec_deinit(&stack);
	return size;
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
			boxfilter(image, width, height, x, y, kx, ky, &red, &green, &blue);
		}
	}

	red = clamp(red, 0.0, 1.0);
	green = clamp(green, 0.0, 1.0);
	blue = clamp(blue, 0.0, 1.0);
	rgb[0] = red * 255.0;
	rgb[1] = green * 255.0;
	rgb[2] = blue * 255.0;
}

static void boxfilter(const unsigned char *image, int width, int height, int x, int y, int xoff, int yoff, float *r, float *g, float *b)
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

// http://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
static inline int orient(float x0, float y0, float x1, float y1, float x2, float y2)
{
	return ((int)x1 - (int)x0)*((int)y2 - (int)y0) - ((int)y1 - (int)y0)*((int)x2 - (int)x0);
}

static inline int min3(int a, int b, int c)
{
	return min(a, min(b, c));
}
static inline int max3(int a, int b, int c)
{
	return max(a, max(b, c));
}
