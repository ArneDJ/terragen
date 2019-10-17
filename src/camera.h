struct camera {
	float yaw, pitch;
	float fov, sensitivity, speed;
	vec3 center, up, eye;
};

struct camera init_camera(float x, float y, float z, float fov, float sensitivity);

void update_free_camera(struct camera *cam, float delta);
void update_strategy_camera(struct camera *cam, float delta);

