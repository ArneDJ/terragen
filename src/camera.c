#include <SDL2/SDL.h>
#include "gmath.h"
#include "camera.h"

struct camera init_camera(float x, float y, float z, float fov, float sensitivity)
{
	struct camera cam = {
		0.0, 0.0, fov, sensitivity, 1.0,
		{0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{x, y, z},
	};

	return cam;
}

void update_free_camera(struct camera *cam, float delta)
{
	int x, y;
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);
	SDL_GetRelativeMouseState(&x, &y);

	cam->yaw += (float)x * cam->sensitivity * delta;
	cam->pitch -= (float)y * cam->sensitivity * delta;

	if (cam->pitch > 1.57f)	
		cam->pitch = 1.57f;
	if (cam->pitch < -1.57f)	
		cam->pitch = -1.57f;

	/* point the camera in a direction based on mouse input */
	cam->center.x = cos(cam->yaw) * cos(cam->pitch);
	cam->center.y = sin(cam->pitch);
	cam->center.z = sin(cam->yaw) * cos(cam->pitch);		
	cam->center = vec3_normalize(cam->center);

	if(keystates[SDL_SCANCODE_W]) cam->eye = vec3_sum(cam->eye, vec3_scale(cam->speed * delta , cam->center));
	if(keystates[SDL_SCANCODE_S]) cam->eye = vec3_sub(cam->eye, vec3_scale(cam->speed * delta, cam->center));
	if(keystates[SDL_SCANCODE_D]) cam->eye = vec3_sum(cam->eye, vec3_scale(cam->speed * delta, vec3_crossn(cam->center, cam->up)));
	if(keystates[SDL_SCANCODE_A]) cam->eye = vec3_sub(cam->eye, vec3_scale(cam->speed * delta, vec3_crossn(cam->center, cam->up)));
}

void update_strategy_camera(struct camera *cam, float delta)
{
	int x, y;
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);
	SDL_GetRelativeMouseState(&x, &y);

	cam->yaw += (float)x * cam->sensitivity * delta;
	cam->pitch -= (float)y * cam->sensitivity * delta;

	if (cam->pitch > 1.57f)	
		cam->pitch = 1.57f;
	if (cam->pitch < -1.57f)	
		cam->pitch = -1.57f;

	cam->pitch = -0.78f;

	/* point the camera in a direction based on mouse input */
	cam->center.x = cos(cam->yaw) * cos(cam->pitch);
	cam->center.y = sin(cam->pitch);
	cam->center.z = sin(cam->yaw) * cos(cam->pitch);		
	cam->center = vec3_normalize(cam->center);

	vec3 tmp = cam->center;
	tmp.y = 0.0;

	if(keystates[SDL_SCANCODE_W]) cam->eye = vec3_sum(cam->eye, vec3_scale(cam->speed * delta , tmp));
	if(keystates[SDL_SCANCODE_S]) cam->eye = vec3_sub(cam->eye, vec3_scale(cam->speed * delta, tmp));
	if(keystates[SDL_SCANCODE_D]) cam->eye = vec3_sum(cam->eye, vec3_scale(cam->speed * delta, vec3_crossn(tmp, cam->up)));
	if(keystates[SDL_SCANCODE_A]) cam->eye = vec3_sub(cam->eye, vec3_scale(cam->speed * delta, vec3_crossn(tmp, cam->up)));
}

