float fbm_noise(float x, float y, float freq, float lacun, float gain);

float worley_noise(float x, float y);

unsigned char *gen_voronoi_map();
unsigned char *gen_worley_map();

float fbm_map_value(float x, float y, float freq, float lacun, float gain);
