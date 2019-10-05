#define IDENTITY_MATRIX {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}

typedef float vec2[2];
typedef float vec3[3]; /* vector of 3 elements */
typedef float vec4[4]; /* vector of 4 elements */

typedef float mat2[4]; /* 2 by 2 matrix */
typedef float mat3[9]; /* 3 by 3 matrix */
typedef float mat4[16]; /* 4 by 4 matrix */

/* normalizes a vector vec into a unit vector */
/* param: vec - the vector to normalize */
void vec3_normalize(vec3 vec);

 /* calculate the cross product of vectors a and b */
 /* param: a - left operand */
 /* param: b - right operand */
 /* param: dest - the result is stored in vector dest */
void vec3_cross(vec3 a, vec3 b, vec3 dest);

/* normalizes the cross product of two products */
/* param: a - left operand vector */
/* param: b - right operand vector */
/* param: dest - the normalized cross product is stored in dest */
void vec3_crossn(vec3 a, vec3 b, vec3 dest);

/* scale a vector with a scalar */
/* param: src - the vector to scale */
/* param: scalar - the value to scale the vector with */
void vec3_scale(vec3 src, float scalar);

/* sum of two vectors */
/* param: a - left operand */
/* param: b - right operand */
/* param: deset - result is stored in dest */
void vec3_add(vec3 a, vec3 b, vec3 dest); 

/* substract a vector by another one */ 
/* param: a - vector to substract from */
/* param: b - vector to substract */
/* param: dest - result of the substraction is stored in dest */
void vec3_sub(vec3 a, vec3 b, vec3 dest); 

/* calculate the dot product of two vectors */
/* param: a - left operand vector */
/* param: b - right operand vector */
/* return: the calculated cross product */
float vec3_dot(vec3 a, vec3 b); 

/* transforms a 4 by 4 matrix by a vector translation */
/* param: matrix - the matrix to translate */
/* param: translation - translation vector */
void mat4_translate(mat4 matrix, vec3 translation);

/* scales a 4 by 4 matrix by a vector */
void mat4_scale(mat4 matrix, vec3 scale);

/* rotates a 4 by 4 matrix by a vector */
void mat4_rotate(mat4 matrix, vec3 rotation);

/* rotate the 4 by 4 matrix by a certain angle in radians */
void mat4_rotate_x(mat4 matrix, float angle);
void mat4_rotate_y(mat4 matrix, float angle);
void mat4_rotate_z(mat4 matrix, float angle);

/* creates a perspective projection matrix */
void make_projection_matrix(mat4 matrix, int angle, float aspect, float near, float far);

/* creates a viewing transform matrix, also known as lookat function */
void make_view_matrix(mat4 matrix, vec3 eye, vec3 center, vec3 up);
