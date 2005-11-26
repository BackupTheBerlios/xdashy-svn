/* This isn't supposed to be used directly by the user.
 * It's an internal mini3d interface.
 */
#ifndef _MINI3D_RASTERIZER_H_
#define _MINI3D_RASTERIZER_H_

#include "fixed_point.h"

struct frame_buffer {
	uint32_t *color_buffer;
	uint32_t *depth_buffer;
	/*uint8_t *stencil_buffer;*/
	int x, y;
};

struct vertex {
	fixed x, y, z, w;
	fixed nx, ny, nz;
	fixed r, g, b, a;
	fixed u, v;
};

int m3d_rasterizer_setup(struct frame_buffer *fbuf);

void m3d_rstate_sptr(unsigned int *sptr);
/*void m3d_rstate_tptr(... some texture pointer pointer ...);*/

void m3d_draw_line(struct vertex *points);
void m3d_draw_polygon(struct vertex *points, int count);

#endif	/* _MINI3D_RASTERIZER_H_ */
