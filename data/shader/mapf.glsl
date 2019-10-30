#version 460 core

uniform sampler2D voronoi;

in vec2 uv;

vec4 permute(vec4 x)
{
	return mod ((34.0 * x + 1.0) * x , 289.0) ;
}
vec2 cellular2x2(vec2 P)
{
	const float K = 1.0/7.0;
	const float K2 = 0.5/7.0;
	const float jitter = 0.8; // jitter 1.0 makes F1 wrong more often
	vec2 Pi = mod ( floor ( P ) , 289.0) ;
	vec2 Pf = fract ( P ) ;
	vec4 Pfx = Pf . x + vec4 ( -0.5 , -1.5 , -0.5 , -1.5) ;
	vec4 Pfy = Pf . y + vec4 ( -0.5 , -0.5 , -1.5 , -1.5) ;
	vec4 p = permute ( Pi . x + vec4 (0.0 , 1.0 , 0.0 , 1.0) ) ;
	p = permute ( p + Pi . y + vec4 (0.0 , 0.0 , 1.0 , 1.0) ) ;
	vec4 ox = mod (p , 7.0) * K + K2 ;
	vec4 oy = mod ( floor ( p * K ) ,7.0) * K + K2 ;
	vec4 dx = Pfx + jitter * ox ;
	vec4 dy = Pfy + jitter * oy ;
	vec4 d = dx * dx + dy * dy ; // d i s t a n c e s squared
	// Cheat and pick only F1 for the return value
	d.xy = min ( d.xy , d.zw ) ;
	d.x = min ( d.x , d.y ) ;
	return d.xx ; // F1 duplicated , F2 not computed
}

void main(void)
{
	const float penis = 0.015625;
	vec4 ftexture = texture(voronoi, penis * uv);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	/*
	vec2 F = cellular2x2(uv);
	float n = 1.0 - 1.5 * F.x;
	float blobs = 1.0 - sqrt(F.x);
	*/
	gl_FragColor = ftexture;
}
