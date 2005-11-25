#ifndef _MINI3D_H_
#define _MINI3D_H_

#include <stdint.h>

enum {
	M3D_SMOOTH,
	M3D_LIGHTING,
	M3D_LIGHT0,
	M3D_LIGHT1,
	M3D_LIGHT2,
	M3D_LIGHT3,
	M3D_TEXTURE,
	M3D_DEPTH_TEST,

	M3D_MODELVIEW = 100,
	M3D_PROJECTION,

	M3D_LINES = 200,
	M3D_TRIANGLES,
	M3D_QUADS,

	M3D_COLOR_BUFFER_BIT = 300,
	M3D_DEPTH_BUFFER_BIT,

	M3D_POSITION = 400,
	M3D_AMBIENT,
	M3D_DIFFUSE,
	M3D_SPECULAR,
	M3D_EMISSION,
	M3D_SHININESS
};

void m3d_init(void);
void m3d_destroy(void);
void m3d_viewport(int x, int y, int w, int h);
uint32_t *m3d_get_pixel_data(void);

/* clear */
void m3d_clear_color(float r, float g, float b);
void m3d_clear_depth(float d);
void m3d_clear(unsigned int what);

/* general state */
void m3d_enable(unsigned int what);
void m3d_disable(unsigned int what);

/* lights and materials */
void m3d_light(unsigned int light, unsigned int pname, float *params);
void m3d_material(unsigned int pname, float param);
void m3d_materialv(unsigned int pname, float *params);

/* matrix manipulation */
void m3d_matrix_mode(unsigned int mode);
void m3d_load_identity(void);
void m3d_load_matrix(float *mat);
void m3d_mult_matrix(float *mat);

void m3d_translate(float x, float y, float z);
void m3d_rotate(float angle, float x, float y, float z);
void m3d_scale(float x, float y, float z);

void m3d_perspective(float fovy, float aspect, float znear, float zfar);

/* rendering */
void m3d_begin(unsigned int primitive);
void m3d_end(void);

void m3d_vertex(float x, float y, float z);
void m3d_color3(float r, float g, float b);
void m3d_color4(float r, float g, float b, float a);
void m3d_normal(float x, float y, float z);
void n3d_tex_coord(float u, float v);

#endif	/* _MINI3D_H_ */
