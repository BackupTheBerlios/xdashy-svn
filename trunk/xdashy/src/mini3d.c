#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include "mini3d.h"
#include "mini3d_rasterizer.h"
#include "color_bits.h"

typedef struct vec3 {
	float x, y, z;
} vec3;

inline static vec3 add(vec3 a, vec3 b);
inline static vec3 sub(vec3 a, vec3 b);
inline static vec3 mul(vec3 v, float s);
inline static float dot(vec3 a, vec3 b);
inline static vec3 normalize(vec3 v);
inline static vec3 transform(vec3 v, float *m);
static void mult_matrix(float *t, float *m1, float *m2);

#define DEG_TO_RAD(a) (((float)a) * (3.1415926535897932 / 180.0))


struct state {
	unsigned int s;
	float matrices[2][16];
	float *mat;
	float msave[2][16];
	
	float mvp_mat[16];
	int mvp_valid;

	unsigned int prim;
	struct frame_buffer fb;
	float clear_r, clear_g, clear_b, clear_a;
	float clear_depth;

	vec3 lpos[4];
	vec3 ambient, diffuse, specular;
	float shininess;

	/* vertex state */
	fixed r, g, b, a;
	fixed nx, ny, nz;
	fixed tu, tv;
	struct vertex v[4];	/* max polygon: quad */
	int cur_vert;
} state;

void m3d_init(void) {
	state.s = 0;
	state.s |= M3D_DEPTH_WRITE;
	m3d_matrix_mode(M3D_PROJECTION);
	m3d_load_identity();
	m3d_perspective(45.0, 1.333333, 1.0, 1000.0);
	m3d_matrix_mode(M3D_MODELVIEW);
	m3d_load_identity();
	
	state.prim = M3D_TRIANGLES;

	state.mvp_valid = 0;

	state.r = state.g = state.b = 0;
	state.a = fixedi(1);
	state.nx = state.ny = state.nz = 0;
	state.tu = state.tv = 0;

	state.lpos[0].x = -50.0f;
	state.lpos[0].y = 100.0f;
	state.lpos[0].z = -100.0f;

	state.ambient.x = state.ambient.y = state.ambient.z = 0.0f;
	state.diffuse.x = state.diffuse.y = state.diffuse.z = 0.5f;
	state.specular.x = state.specular.y = state.specular.z = 0.0f;
	state.shininess = 1.0f;

	state.fb.color_buffer = state.fb.depth_buffer = 0;

	m3d_rstate_sptr(&state.s);
}

void m3d_destroy(void) {
	free(state.fb.color_buffer);
	free(state.fb.depth_buffer);
}

void m3d_viewport(int x, int y, int w, int h) {
	/* NOTE: for the moment ignore x/y */
	state.fb.x = w;
	state.fb.y = h;

	free(state.fb.color_buffer);
	free(state.fb.depth_buffer);
	
	if(!(state.fb.color_buffer = malloc(w * h * sizeof(uint32_t)))) {
		return;
	}

	if(!(state.fb.depth_buffer = malloc(w * h * sizeof(uint32_t)))) {
		free(state.fb.color_buffer);
		state.fb.color_buffer = 0;
		return;
	}

	m3d_rasterizer_setup(&state.fb);
}

uint32_t *m3d_get_pixel_data(void) {
	return state.fb.color_buffer;
}

/* clear */
void m3d_clear_color(float r, float g, float b, float a) {
	state.clear_r = r;
	state.clear_g = g;
	state.clear_b = b;
	state.clear_a = a;
}

void m3d_clear_depth(float d) {
	state.clear_depth = d;
}

void m3d_clear(unsigned int what) {
	int i;
	int sz = state.fb.x * state.fb.y;
	uint32_t *cptr = state.fb.color_buffer;
	uint32_t *zptr = state.fb.depth_buffer;
	uint32_t col = PACK_COLOR32(state.clear_a * 255.0, state.clear_r * 255.0, state.clear_g * 255.0, state.clear_b * 255.0);
	uint32_t zval = (uint32_t)fixedf(state.clear_depth);
	
	for(i=0; i<sz; i++) {
		if(what & M3D_COLOR_BUFFER_BIT) {
			*cptr++ = col;
		}

		if(what & M3D_DEPTH_BUFFER_BIT) {
			*zptr++ = zval;
		}
	}
}


/* general state */
void m3d_enable(unsigned int what) {
	state.s |= 1 << what;
}

void m3d_disable(unsigned int what) {
	state.s &= ~(1 << what);
}

/* zbuffer state */
void m3d_depth_mask(int boolval) {
	if(boolval) {
		m3d_enable(M3D_DEPTH_WRITE);
	} else {
		m3d_disable(M3D_DEPTH_WRITE);
	}
}

/* lights and materials */
void m3d_light(unsigned int light, unsigned int pname, float *params) {
	int index = light - M3D_LIGHT0;
	vec3 *tmp = (vec3*)params;
	
	switch(pname) {
	case M3D_POSITION:
		state.lpos[index] = transform(*tmp, state.matrices[0]);
		break;

	case M3D_DIFFUSE:
	default:
		break;
	}
}

void m3d_material(unsigned int pname, float param) {
	/* this is only valid for pname == M3D_SHININESS */
	state.shininess = param;
}

void m3d_materialv(unsigned int pname, float *params) {
	switch(pname) {
	case M3D_AMBIENT:
		memcpy(&state.ambient, params, 3 * sizeof(float));
		break;

	case M3D_DIFFUSE:
		memcpy(&state.diffuse, params, 3 * sizeof(float));
		break;

	case M3D_SPECULAR:
		memcpy(&state.specular, params, 3 * sizeof(float));
		break;

	case M3D_EMISSION:
	default:
		break;
	}
}

/* matrix manipulation */
void m3d_matrix_mode(unsigned int mode) {
	state.mat = state.matrices[mode - M3D_MODELVIEW];
}

void m3d_load_identity(void) {
	memset(state.mat, 0, 16 * sizeof(float));
	state.mat[0] = state.mat[5] = state.mat[10] = state.mat[15] = 1.0f;
	state.mvp_valid = 0;
}

void m3d_load_matrix(float *mat) {
	memcpy(state.mat, mat, 16 * sizeof(float));
	state.mvp_valid = 0;
}

#define M(i, j)	((i << 2) + j)

void m3d_mult_matrix(float *m2) {
	mult_matrix(state.mat, state.mat, m2);
	state.mvp_valid = 0;
}

void m3d_push_matrix(void) {
	ptrdiff_t midx = state.mat - state.matrices[0];
	memcpy(state.msave[midx], state.mat, 16 * sizeof(float));
}

void m3d_pop_matrix(void) {
	ptrdiff_t midx = state.mat - state.matrices[0];
	memcpy(state.mat, state.msave[midx], 16 * sizeof(float));
}

void m3d_translate(float x, float y, float z) {
	float tmat[16] = {1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1};
	m3d_mult_matrix(tmat);
}

void m3d_rotate(float angle, float x, float y, float z) {
	float sina, cosa, invcosa, nxsq, nysq, nzsq, len;
	float rmat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	len = (float)sqrt(x * x + y * y + z * z);
	x /= len;
	y /= len;
	z /= len;

	sina = (float)sin(DEG_TO_RAD(angle));
	cosa = (float)cos(DEG_TO_RAD(angle));
	invcosa = 1.0f - cosa;
	nxsq = x * x;
	nysq = y * y;
	nzsq = z * z;
	
	rmat[M(0, 0)] = nxsq + (1.0f - nxsq) * cosa;
	rmat[M(0, 1)] = x * y * invcosa - z * sina;
	rmat[M(0, 2)] = x * z * invcosa + y * sina;
	rmat[M(1, 0)] = x * y * invcosa + z * sina;
	rmat[M(1, 1)] = nysq + (1.0f - nysq) * cosa;
	rmat[M(1, 2)] = y * z * invcosa - x * sina;
	rmat[M(2, 0)] = x * z * invcosa - y * sina;
	rmat[M(2, 1)] = y * z * invcosa + x * sina;
	rmat[M(2, 2)] = nzsq + (1.0f - nzsq) * cosa;

	m3d_mult_matrix(rmat);
}

void m3d_rotate_euler(float x, float y, float z) {
	float mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	x = DEG_TO_RAD(x);
	y = DEG_TO_RAD(y);
	z = DEG_TO_RAD(z);

	mat[M(1,1)] = (float)cos(x);
	mat[M(1,2)] = -(float)sin(x);
	mat[M(2,1)] = (float)sin(x);
	mat[M(2,2)] = (float)cos(x);
	m3d_mult_matrix(mat);

	mat[M(1,1)] = 1.0f;
	mat[M(1,2)] = mat[M(2,1)] = 0.0f;
	mat[0] = (float)cos(y);
	mat[M(0,2)] = (float)sin(y);
	mat[M(2,0)] = -(float)sin(y);
	mat[M(2,2)] = (float)cos(y);
	m3d_mult_matrix(mat);

	mat[M(2,0)] = mat[M(0,2)] = 0.0f;
	mat[M(2,2)] = 1.0f;
	mat[0] = (float)cos(z);
	mat[1] = -(float)sin(z);
	mat[M(1,0)] = (float)sin(z);
	mat[M(1,1)] = (float)cos(z);
	m3d_mult_matrix(mat);
}

void m3d_scale(float x, float y, float z) {
	float smat[16] = {x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1};
	m3d_mult_matrix(smat);
}

void m3d_perspective(float fovy, float aspect, float znear, float zfar) {
	float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	float fovx = fovy * aspect;
	float w = 1.0f / tan(DEG_TO_RAD(fovx) * 0.5f);
	float h = 1.0f / tan(DEG_TO_RAD(fovy) * 0.5f);
	float q = zfar / (zfar - znear);

	m[M(0, 0)] = w;
	m[M(1, 1)] = h;
	m[M(2, 2)] = q;
	m[M(3, 2)] = 1.0f;
	m[M(2, 3)] = -q * znear;
	m3d_mult_matrix(m);
}



/* rendering */
void m3d_begin(unsigned int primitive) {
	state.prim = primitive;
	state.cur_vert = 0;

	if(!state.mvp_valid) {
		mult_matrix(state.mvp_mat, state.matrices[1], state.matrices[0]);
		state.mvp_valid = 1;
	}
}

void m3d_end(void) {
}


static vec3 shade(vec3 vcs_pos, vec3 vcs_n) {
	static const vec3 view = {0.0f, 0.0f, 1.0f};
	vec3 col = state.ambient;

	if(!(state.s & (1 << M3D_LIGHTING))) {
		return col;
	}

	for(int i=0; i<4; i++) {
		vec3 ldir, half;
		float ndotl, ndoth;
		vec3 dif, spec;
		
		if(!(state.s & (1 << (M3D_LIGHT0 + i)))) {
			continue;
		}

		ldir = sub(state.lpos[i], vcs_pos);
		ldir = normalize(ldir);
		
		ndotl = dot(vcs_n, ldir);
		if(ndotl < 0.0f) ndotl = 0.0f;
		dif = mul(state.diffuse, ndotl);
		
		half.x = (view.x + ldir.x) * 0.5f;
		half.y = (view.y + ldir.y) * 0.5f;
		half.z = (view.z + ldir.z) * 0.5f;
		
		ndoth = dot(vcs_n, half);
		if(ndoth < 0.0f) ndoth = 0.0f;
		spec = mul(state.specular, ndoth);

		col = add(col, dif);
		col = add(col, spec);
	}

	if(col.x > 1.0f) col.x = 1.0f;
	if(col.y > 1.0f) col.y = 1.0f;
	if(col.z > 1.0f) col.z = 1.0f;

	return col;
}

void m3d_vertex(float x, float y, float z) {
	float *row;
	int prim_elem = state.prim - M3D_LINES + 2;
	struct vertex *v = state.v + state.cur_vert;

	v->r = state.r;
	v->g = state.g;
	v->b = state.b;
	v->a = state.a;
	v->nx = state.nx;
	v->ny = state.ny;
	v->nz = state.nz;
	v->u = state.tu;
	v->v = state.tv;
	
	/* if lighting is enabled, modify the color */
	if(state.s & (1 << M3D_LIGHTING)) {
		vec3 pos, normal, col;
		
		pos.x = x;
		pos.y = y;
		pos.z = z;
		pos = transform(pos, state.matrices[0]);

		normal.x = fixed_float(v->nx);
		normal.y = fixed_float(v->ny);
		normal.z = fixed_float(v->nz);
	
		col = shade(pos, normal);
		v->r = fixedf(col.x);
		v->g = fixedf(col.y);
		v->b = fixedf(col.z);
	}
	
	// transform into post-projective homogeneous clip-space
	row = state.mvp_mat;
	v->x = fixedf(row[0] * x + row[1] * y + row[2] * z + row[3]); row += 4;
	v->y = fixedf(row[0] * x + row[1] * y + row[2] * z + row[3]); row += 4;
	v->z = fixedf(row[0] * x + row[1] * y + row[2] * z + row[3]); row += 4;
	v->w = fixedf(row[0] * x + row[1] * y + row[2] * z + row[3]);

	// divide with W
	v->x = fixed_div(v->x, v->w);
	v->y = fixed_div(v->y, v->w);
	v->z = fixed_div(v->z, v->w);

	// viewport transformation
	fixed width = fixedi(state.fb.x);
	fixed height = fixedi(state.fb.y);
	v->x = fixed_mul(v->x, width) + fixed_mul(width, fixed_half);
	v->y = fixed_mul(height, fixed_half) - fixed_mul(v->y, height);

	state.cur_vert = (state.cur_vert + 1) % prim_elem;

	if(!state.cur_vert) {
		if(state.prim == M3D_LINES) {
			m3d_draw_line(state.v);
		} else {
			m3d_draw_polygon(state.v, prim_elem);
		}
	}
}

void m3d_color3(float r, float g, float b) {
	state.r = fixedf(r);
	state.g = fixedf(g);
	state.b = fixedf(b);
	state.a = fixedi(1);
}

void m3d_color4(float r, float g, float b, float a) {
	state.r = fixedf(r);
	state.g = fixedf(g);
	state.b = fixedf(b);
	state.a = fixedf(a);
}

void m3d_normal(float x, float y, float z) {
	float *row = state.matrices[0];	
	state.nx = fixedf(row[0] * x + row[1] * y + row[2] * z); row += 4;
	state.ny = fixedf(row[0] * x + row[1] * y + row[2] * z); row += 4;
	state.nz = fixedf(row[0] * x + row[1] * y + row[2] * z);
}

void n3d_tex_coord(float u, float v) {
	state.tu = fixedf(u);
	state.tv = fixedf(v);
}



/* --- math helpers --- */

inline static vec3 add(vec3 a, vec3 b) {
	vec3 res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	res.z = a.z + b.z;
	return res;
}

inline static vec3 sub(vec3 a, vec3 b) {
	vec3 res;
	res.x = a.x - b.x;
	res.y = a.y - b.y;
	res.z = a.z - b.z;
	return res;
}

inline static vec3 mul(vec3 v, float s) {
	vec3 res;
	res.x = v.x * s;
	res.y = v.y * s;
	res.z = v.z * s;
	return res;
}

inline static float dot(vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline static vec3 normalize(vec3 v) {
	float len = (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	v.x /= len;
	v.y /= len;
	v.z /= len;
	return v;
}

inline static vec3 transform(vec3 v, float *m) {
	vec3 out;
	float *row = m;
	out.x = row[0] * v.x + row[1] * v.y + row[2] * v.z + row[3]; row += 4;
	out.y = row[0] * v.x + row[1] * v.y + row[2] * v.z + row[3]; row += 4;
	out.z = row[0] * v.x + row[1] * v.y + row[2] * v.z + row[3];
	return out;
}


static void mult_matrix(float *t, float *m1, float *m2) {
	int i;
	float res[16];

	for(i=0; i<4; i++) {
		res[M(i,0)] = m1[M(i,0)] * m2[M(0,0)] + m1[M(i,1)] * m2[M(1,0)] + m1[M(i,2)] * m2[M(2,0)] + m1[M(i,3)] * m2[M(3,0)];
		res[M(i,1)] = m1[M(i,0)] * m2[M(0,1)] + m1[M(i,1)] * m2[M(1,1)] + m1[M(i,2)] * m2[M(2,1)] + m1[M(i,3)] * m2[M(3,1)];
		res[M(i,2)] = m1[M(i,0)] * m2[M(0,2)] + m1[M(i,1)] * m2[M(1,2)] + m1[M(i,2)] * m2[M(2,2)] + m1[M(i,3)] * m2[M(3,2)];
		res[M(i,3)] = m1[M(i,0)] * m2[M(0,3)] + m1[M(i,1)] * m2[M(1,3)] + m1[M(i,2)] * m2[M(2,3)] + m1[M(i,3)] * m2[M(3,3)];
	}

	memcpy(t, res, 16 * sizeof(float));
}
