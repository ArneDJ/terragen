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

GLuint make_rgb_texture(rgb *image, int width, int height);

GLuint make_voronoi_texture(void);

GLuint make_worley_texture(void);

GLuint make_river_texture(void);
