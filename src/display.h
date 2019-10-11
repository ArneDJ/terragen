struct object {
	struct mesh m;
	GLuint texture;
	GLuint shader;
	vec3 translation;
	vec3 rotation;
	vec3 scale;
};

void display_static_object(struct object *obj);

void display_skybox(struct mesh m);
