#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "mini3d_rasterizer.h"
#include "mini3d.h"
#include "color_bits.h"

#define GET_R(p)	(((p) & RED_MASK32) >> RED_SHIFT32)
#define GET_G(p)	(((p) & GREEN_MASK32) >> GREEN_SHIFT32)
#define GET_B(p)	(((p) & BLUE_MASK32) >> BLUE_SHIFT32)
#define GET_A(p)	(((p) & ALPHA_MASK32) >> ALPHA_SHIFT32)

//#define ROUND(x) (int)((x) >= 0 ? (x) + 0.5 : (x) - 0.5)
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define CLAMP(x, l, h)	MAX((l), MIN((h), (x)))

#define swap(type, a, b)	{type tmp = a; a = b; b = tmp;}

struct edge {
	int x;
	fixed r, g, b;
	fixed u, v;
	fixed z;
};

static inline void scan_int(fixed x1, fixed y1, fixed x2, fixed y2, int elem_offset);
static inline void scan_fixed(fixed x1, fixed y1, fixed x2, fixed y2, int elem_offset);
static inline void fill_scanlines(int starty, int endy);

// ---------- rasterizer state variables and edge tables ---------
static unsigned int *state;

static struct frame_buffer *fb;
static int xres, yres;

//static struct tex2d *tex;

static int *scanline_offset;
struct edge *left_edge, *right_edge;

int m3d_rasterizer_setup(struct frame_buffer *fbuf) {
	int i;

	fb = fbuf;
	xres = fb->x;
	yres = fb->y;

	free(left_edge);
	free(right_edge);
	free(scanline_offset);

	left_edge = malloc(yres * sizeof(struct edge));
	right_edge = malloc(yres * sizeof(struct edge));
	scanline_offset = malloc(yres * sizeof(int));

	if(!left_edge || !right_edge || !scanline_offset) {
		return -1;
	}

	for(i=0; i<yres; i++) {
		scanline_offset[i] = i * xres;
	}

	return 0;
}

void m3d_rstate_sptr(unsigned int *sptr) {
	state = sptr;
}

void m3d_draw_line(struct vertex *points) {
	int i;
	for(i=0; i<2; i++) {
		int x = fixed_int(points[i].x);
		int y = fixed_int(points[i].y);

		fb->color_buffer[y * fb->x + x] = 0xff0000;
	}
}


/* draw_polygon()
 * function to rasterize a convex polygon with gouraud color
 * interpolation, texture coordinate interpolation and zbuffering
 *
 * note: can be easily extended to interpolate the normals as well
 * in order to calculate the illumination per pixel
 */

#define INTERP_COL
#define INTERP_Z
//#define INTERP_TEX

void m3d_draw_polygon(struct vertex *points, int count) {
	int i;
	int starty = yres;
	int endy = 0;

	struct vertex *v1 = points - 1;
	struct vertex *v2 = points;

	count--;
	for(i=0; i<count; i++) {
		v1 = v2;
		v2++;

		int iy1 = fixed_round(v1->y);
		if(iy1 < starty) starty = iy1;
		if(iy1 > endy) endy = iy1;

		scan_int(v1->x, v1->y, v2->x, v2->y, offsetof(struct edge, x));
#ifdef INTERP_Z
		scan_fixed(v1->z, v1->y, v2->z, v2->y, offsetof(struct edge, z));
#endif
#ifdef INTERP_COL
		// interpolate color
		scan_fixed(v1->r, v1->y, v2->r, v2->y, offsetof(struct edge, r));
		scan_fixed(v1->g, v1->y, v2->g, v2->y, offsetof(struct edge, g));
		scan_fixed(v1->b, v1->y, v2->b, v2->y, offsetof(struct edge, b));
#endif
#ifdef INTERP_TEX
		// interpolate texture coordinates
		scan_fixed(v1->u, v1->y, v2->u, v2->y, offsetof(struct edge, u));
		scan_fixed(v1->v, v1->y, v2->v, v2->y, offsetof(struct edge, v));
#endif
	}

	// unrolled the last loop to avoid the check to setup v2 correctly
	v1 = v2;
	v2 = points;

	int iy1 = fixed_round(v1->y);
	if(iy1 < starty) starty = iy1;
	if(iy1 > endy) endy = iy1;

	scan_int(v1->x, v1->y, v2->x, v2->y, offsetof(struct edge, x));
#ifdef INTERP_Z
	scan_fixed(v1->z, v1->y, v2->z, v2->y, offsetof(struct edge, z));
#endif
#ifdef INTERP_COL
	// interpolate color
	scan_fixed(v1->r, v1->y, v2->r, v2->y, offsetof(struct edge, r));
	scan_fixed(v1->g, v1->y, v2->g, v2->y, offsetof(struct edge, g));
	scan_fixed(v1->b, v1->y, v2->b, v2->y, offsetof(struct edge, b));
#endif
#ifdef INTERP_TEX
	// interpolate texture coordinates
	scan_fixed(v1->u, v1->y, v2->u, v2->y, offsetof(struct edge, u));
	scan_fixed(v1->v, v1->y, v2->v, v2->y, offsetof(struct edge, v));
#endif
	
	// fill the scanlines
	
	starty = MAX(starty, 0);
	endy = MIN(endy, yres);

#ifndef INTERP_COL
	{
		int y;
		for(y=starty; y<endy; y++) {
			right_edge[y].r = left_edge[y].r = points->r;
			right_edge[y].g = left_edge[y].g = points->g;
			right_edge[y].b = left_edge[y].b = points->b;
		}
	}
#endif

	fill_scanlines(starty, endy);
}


static inline void scan_int(fixed x1, fixed y1, fixed x2, fixed y2, int elem_offset) {
	struct edge *edge = right_edge;

	if(y1 > y2) {
		edge = left_edge;
		swap(fixed, y1, y2);
		swap(fixed, x1, x2);
	}

	fixed x = x1;
	int y = fixed_round(y1);

	fixed dx = x2 - x1;
	fixed dy = y2 - y1;
	int idy = fixed_round(y2) - y;

	if(!idy) return;

	fixed slope = dy ? fixed_div(dx, dy) : dx;

	struct edge *ptr = edge + y;	// CAUTION: since this is only used for x which has offset 0, we skip it
	for(int j=0; j<idy; j++, y++, ptr++) {

		*(int*)ptr = fixed_round(x);
		x += slope;
	}
}

static inline void scan_fixed(fixed x1, fixed y1, fixed x2, fixed y2, int elem_offset) {
	struct edge *edge = right_edge;

	if(y1 > y2) {
		edge = left_edge;
		swap(fixed, y1, y2);
		swap(fixed, x1, x2);
	}

	fixed x = x1;
	int y = fixed_round(y1);

	fixed dx = x2 - x1;
	fixed dy = y2 - y1;
	int idy = fixed_round(y2) - y;

	if(!idy) return;

	fixed slope = dy ? fixed_div(dx, dy) : dx;

	struct edge *ptr = (struct edge*)((char*)(edge + y) + elem_offset);
	for(int j=0; j<=idy; j++, y++, ptr++) {

		*(fixed*)ptr = x;
		x += slope;
	}
}


static inline void fill_scanlines(int starty, int endy) {
	
	struct edge *left_ptr = left_edge + starty;
	struct edge *right_ptr = right_edge + starty;
	int *sptr = scanline_offset + starty;

	for(int y=starty; y<endy; y++, left_ptr++, right_ptr++) {
		uint32_t *cptr, *zptr;
		int startx = left_ptr->x;
		int endx = right_ptr->x;

		fixed dx = fixedi(endx - startx);
		
		// color, z, u, v interpolation
		fixed r = left_ptr->r;
		fixed g = left_ptr->g;
		fixed b = left_ptr->b;
#ifdef INTERP_COL
		fixed dr = right_ptr->r - r;
		fixed dg = right_ptr->g - g;
		fixed db = right_ptr->b - b;
#endif
#ifdef INTERP_Z
		fixed z = left_ptr->z;
		fixed dz = right_ptr->z - z;
#endif
#ifdef INTERP_TEX
		fixed u = left_ptr->u;
		fixed v = left_ptr->v;
		fixed du = right_ptr->u - u;
		fixed dv = right_ptr->v - v;
#endif
		
		fixed rslope, gslope, bslope, zslope, uslope, vslope;

		if(dx > 0) {
#ifdef INTERP_COL
			rslope = fixed_div(dr, dx);
			gslope = fixed_div(dg, dx);
			bslope = fixed_div(db, dx);
#endif
#ifdef INTERP_Z
			zslope = fixed_div(dz, dx);
#endif
#ifdef INTERP_TEX
			uslope = fixed_div(du, dx);
			vslope = fixed_div(dv, dx);
#endif
		} else {
			rslope = gslope = bslope = zslope = uslope = vslope = 0;
		}

		startx = MAX(startx, 0);
		endx = MIN(endx, xres);
		
		cptr = fb->color_buffer + *sptr + startx;
		zptr = fb->depth_buffer + *sptr++ + startx;
		
		for(int x=startx; x<endx; x++) {
#ifdef INTERP_Z
			uint32_t zval = (uint32_t)z;

			if(!(*state & M3D_DEPTH_TEST) || zval < *zptr) {
#endif
				
				static fixed fixed_255 = fixedi(255);
				int ir = fixed_int(fixed_mul(r, fixed_255));
				int ig = fixed_int(fixed_mul(g, fixed_255));
				int ib = fixed_int(fixed_mul(b, fixed_255));
#ifdef INTERP_TEX
				if(tex) {
					int tx = fixed_int(fixed_mul(u, fixedi(tex->x))) & tex->xmask;
					int ty = fixed_int(fixed_mul(v, fixedi(tex->y))) & tex->ymask;

					uint32_t texel = tex->pixels[(ty << tex->xpow) + tx];
					ir = (ir * GET_R(texel)) >> 8;
					ig = (ir * GET_G(texel)) >> 8;
					ib = (ir * GET_B(texel)) >> 8;
				} else {
					ir = fixed_int(fixed_mul(r, fixed_255));
					ig = fixed_int(fixed_mul(g, fixed_255));
					ib = fixed_int(fixed_mul(b, fixed_255));
				}
#endif
				
			
				*cptr = PACK_COLOR24(ir, ig, ib);
#ifdef INTERP_Z
				if(*state & M3D_DEPTH_WRITE) *zptr = zval;
			}
			zptr++;
			z += zslope;
#endif

			cptr++;
#ifdef INTERP_COL
			r += rslope;
			g += gslope;
			b += bslope;
#endif
#ifdef INTERP_TEX
			u += uslope;
			v += vslope;
#endif
		}

	}
}
