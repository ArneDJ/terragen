typedef unsigned char rgb[3];

enum {
	FOURCC_DXT1 = 0x31545844,
	FOURCC_DXT3 = 0x33545844,
	FOURCC_DXT5 = 0x35545844,
};

/* DDS header */
struct dds {
	char identifier[4]; /* file type */
	uint32_t height; /* in pixels */
	uint32_t width; /* in pixels */
	uint32_t linear_size;
	uint32_t mip_levels;
	uint32_t dxt_codec; /* compression type */
};

GLuint load_dds_texture(const char *fpath);

GLuint make_r_texture(unsigned char *image, int width, int height);

GLuint make_voronoi_texture(int width, int height);

GLuint make_mountain_texture(int width, int height);

GLuint make_river_texture(int width, int height);

GLuint make_worley_texture(int width, int height);

GLuint make_perlin_texture(int width, int height);

void plot(int x, int y, unsigned char *image, int width, int height, int nchannels, unsigned char *color);

int floodfill(int x, int y, unsigned char *image, int width, int height, unsigned char old, unsigned char new);

void gauss_filter_rgb(int x, int y, const unsigned char *image, int width, int height, unsigned char rgb[3]);
