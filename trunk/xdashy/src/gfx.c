/*
This file is part of xdashy, free xbox dashboad.

Copyright (c) 2005 Mihalis Georgoulopoulos <msamurai@freemail.gr>

xdashy is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

xdashy is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with xdashy; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* gfx.c
 */

#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "mini3d.h"
#include "mini3d_rasterizer.h"
#include "xdashy.h"
#include "color_bits.h"

void draw_lord(void);

SDL_Surface *pscreen=0;
SDL_Surface *pbackground=0;

TTF_Font *font;

SDL_Joystick *joystick=0;

unsigned int effect_width, effect_height;

int init_gfx(const char *bg_file, const char *font_file, int font_height,
				int fx_w, int fx_h)
{
    /* Initialize the SDL library */
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0 ) 
	{
        return 0;
    }

    /* Clean up on exit */
    atexit(SDL_Quit);

#ifdef ENABLE_XBOX
	SDL_JoystickOpen(0);
#endif /* ENABLE_XBOX */
	
	SDL_JoystickEventState(SDL_ENABLE);
	
	if (pscreen)
		SDL_FreeSurface(pscreen);

	pbackground = IMG_Load(bg_file);

#ifdef ENABLE_XBOX
    pscreen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE | SDL_FULLSCREEN);
#else
	pscreen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
#endif /* ENABLE_XBOX */
	if ( pscreen == NULL ) 
	{
        fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
        return 0;
    }

	TTF_Init();
	if(!(font = TTF_OpenFont(font_file, font_height))) {
		fprintf(stderr, "failed to open font: %s\n", font_file);
		return 0;
	}
	
	xdashy_set_font_height(TTF_FontHeight(font));

	/* initialize mini3d */
	m3d_init();
	m3d_viewport(0, 0, fx_w, fx_h);

	effect_width = fx_w;
	effect_height = fx_h;
	
	m3d_clear_color(0.0, 0.0, 0.0, 0.0);
	m3d_clear_depth(1.0);

	m3d_enable(M3D_DEPTH_TEST);
	m3d_enable(M3D_LIGHTING);
	m3d_enable(M3D_LIGHT0);

	m3d_matrix_mode(M3D_PROJECTION);
	m3d_load_identity();
	m3d_perspective(45.0, 1.33333, 100.0, 1000.0);

	return 1;
}

void close_gfx()
{
	m3d_destroy();
	
#ifdef ENABLE_XBOX
	SDL_JoystickClose(joystick);
#endif /* ENABLE_XBOX */
	SDL_FreeSurface(pscreen);

	TTF_CloseFont(font);
	TTF_Quit();
}

void gfx_flip()
{
	SDL_Flip(pscreen);
}

void render_background()
{
	/* render background */
	SDL_BlitSurface(pbackground, 0, pscreen, 0);
}

void render_text(int x, int y, char *text, float r, float g, float b)
{
	SDL_Color clr;
	SDL_Rect rect;
	SDL_Surface *text_surf;
	
	clr.r = (int) (r * 255);
	clr.g = (int) (g * 255);
	clr.b = (int) (b * 255);
	
	rect.x = x;
	rect.y = y;

	text_surf = TTF_RenderText_Blended(font, text, clr);
	if (!text_surf) return;

	SDL_BlitSurface(text_surf, 0, pscreen, &rect);
	
	SDL_FreeSurface(text_surf);
}

void render_quad(int x, int y, int w, int h, float r, float g, float b)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	
	SDL_FillRect(pscreen, &rect, SDL_MapRGB(pscreen->format, 
				(int)(r * 255), (int)(g * 255), (int)(b * 255)));
}

void set_clip_rect(int x, int y, int w, int h)
{
	SDL_Rect rect;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	SDL_SetClipRect(pscreen, &rect);
}

void draw_cube(void) {	
	m3d_begin(M3D_QUADS);
	float a = 0.65f;

	/* face +Z */
	m3d_normal(0, 0, 1);
	m3d_color4(1, 0, 0, a);
	m3d_vertex(1, 1, 1);
	m3d_vertex(-1, 1, 1);
	m3d_vertex(-1, -1, 1);
	m3d_vertex(1, -1, 1);

	/* face -X */
	m3d_normal(-1, 0, 0);
	m3d_color4(0, 1, 0, a);
	m3d_vertex(-1, 1, 1);
	m3d_vertex(-1, 1, -1);
	m3d_vertex(-1, -1, -1);
	m3d_vertex(-1, -1, 1);

	/* face -Z */
	m3d_normal(0, 0, -1);
	m3d_color4(0, 0, 1, a);
	m3d_vertex(1, 1, -1);
	m3d_vertex(1, -1, -1);
	m3d_vertex(-1, -1, -1);
	m3d_vertex(-1, 1, -1);

	/* face +X */
	m3d_normal(1, 0, 0);
	m3d_color4(1, 1, 0, a);
	m3d_vertex(1, 1, 1);
	m3d_vertex(1, -1, 1);
	m3d_vertex(1, -1, -1);
	m3d_vertex(1, 1, -1);

	/* face +Y */
	m3d_normal(0, 1, 0);
	m3d_color4(0, 1, 1, a);
	m3d_vertex(1, 1, -1);
	m3d_vertex(-1, 1, -1);
	m3d_vertex(-1, 1, 1);
	m3d_vertex(1, 1, 1);

	/* face -Y */
	m3d_normal(0, -1, 0);
	m3d_color4(1, 0, 1, a);
	m3d_vertex(1, -1, -1);
	m3d_vertex(1, -1, 1);
	m3d_vertex(-1, -1, 1);
	m3d_vertex(-1, -1, -1);
	
	m3d_end();
}

void blit_effect_normal(int x, int y)
{
	int i, xspan, yspan;
	uint32_t *rbuf, *fb;
	
	rbuf = m3d_get_pixel_data();

	/* copy to SDL surface */
	if(SDL_MUSTLOCK(pscreen)) {
		SDL_LockSurface(pscreen);
	}

	xspan = pscreen->w - x;
	xspan = xspan > effect_width ? effect_width : xspan;

	yspan = pscreen->h - y;
	yspan = yspan > effect_height ? effect_height : yspan;

	fb = (uint32_t*)pscreen->pixels + y * pscreen->w + x;
	for(i=0; i<yspan; i++) {		
		memcpy(fb, rbuf, xspan * sizeof(uint32_t));
		fb += pscreen->w;
		rbuf += effect_width;
	}

	if(SDL_MUSTLOCK(pscreen)) {
		SDL_UnlockSurface(pscreen);
	}
}
void blit_effect_alpha_test(int x, int y, unsigned char ref)
{
	int i, j, xspan, yspan;
	uint32_t *rbuf, *fb;
	
	rbuf = m3d_get_pixel_data();

	/* copy to SDL surface */
	if(SDL_MUSTLOCK(pscreen)) {
		SDL_LockSurface(pscreen);
	}

	xspan = pscreen->w - x;
	xspan = xspan > effect_width ? effect_width : xspan;

	yspan = pscreen->h - y;
	yspan = yspan > effect_height ? effect_height : yspan;

	fb = (uint32_t*)pscreen->pixels + y * pscreen->w + x;
	for(j=0; j<yspan; j++) {
		for (i=0; i<xspan; i++) {
			if ((*rbuf >> ALPHA_SHIFT32) > ref) {
				*fb = *rbuf;
			}
			fb ++;
			rbuf ++;	
		}
		fb += pscreen->w - xspan;
		rbuf += effect_width - xspan;
	}

	if(SDL_MUSTLOCK(pscreen)) {
		SDL_UnlockSurface(pscreen);
	}
}

void blit_effect_alpha_blend(int x, int y, unsigned char ref)
{
	int i, j, xspan, yspan;
	uint32_t *rbuf, *fb;
	
	rbuf = m3d_get_pixel_data();

	/* copy to SDL surface */
	if(SDL_MUSTLOCK(pscreen)) {
		SDL_LockSurface(pscreen);
	}

	xspan = pscreen->w - x;
	xspan = xspan > effect_width ? effect_width : xspan;

	yspan = pscreen->h - y;
	yspan = yspan > effect_height ? effect_height : yspan;

	fb = (uint32_t*)pscreen->pixels + y * pscreen->w + x;
	for(j=0; j<yspan; j++) {
		for (i=0; i<xspan; i++) {
			int alpha = (*rbuf & ALPHA_MASK32) >> ALPHA_SHIFT32;
			if(alpha > ref) {
				int sr = (*rbuf & RED_MASK32) >> RED_SHIFT32;
				int sg = (*rbuf & GREEN_MASK32) >> GREEN_SHIFT32;
				int sb = (*rbuf & BLUE_MASK32) >> BLUE_SHIFT32;
				int dr = (*fb & RED_MASK32) >> RED_SHIFT32;
				int dg = (*fb & GREEN_MASK32) >> GREEN_SHIFT32;
				int db = (*fb & BLUE_MASK32) >> BLUE_SHIFT32;

				uint32_t col = 0;
				col |= (((sr * alpha) / 255 + (dr * (255 - alpha)) / 255) << RED_SHIFT32) & RED_MASK32;
				col |= (((sg * alpha) / 255 + (dg * (255 - alpha)) / 255) << GREEN_SHIFT32) & GREEN_MASK32;
				col |= (((sb * alpha) / 255 + (db * (255 - alpha)) / 255) << BLUE_SHIFT32) & BLUE_MASK32;
				col |= (alpha << ALPHA_SHIFT32) & ALPHA_MASK32;
				
				*fb = col;
			}
			fb++;
			rbuf++;
		}
		fb += pscreen->w - xspan;
		rbuf += effect_width - xspan;
	}

	if(SDL_MUSTLOCK(pscreen)) {
		SDL_UnlockSurface(pscreen);
	}
}


void render_effect(int x, int y, int alpha_test, unsigned char alpha_ref, int alpha_blend) {
	float t = SDL_GetTicks() / 10.0f;
	
	m3d_clear(M3D_COLOR_BUFFER_BIT | M3D_DEPTH_BUFFER_BIT);

	float dif[] = {0.2, 0.4, 1.0, 1.0};
	float spec[] = {1, 1, 1, 1};
	m3d_materialv(M3D_DIFFUSE, dif);
	m3d_materialv(M3D_SPECULAR, spec);
	m3d_material(M3D_SHININESS, 60.0f);

	m3d_matrix_mode(M3D_MODELVIEW);
	m3d_load_identity();
	m3d_translate(0, 0, 800);
	m3d_rotate(t, 0, 1, 0);
	//m3d_rotate(-90, 1, 0, 0);

	draw_lord();

	if (alpha_test)
	{
		blit_effect_alpha_test(x, y, alpha_ref);
		if (alpha_blend)
		{
			blit_effect_alpha_blend(x, y, alpha_ref);
		}
	}
	else
	{
		if (alpha_blend)
		{
			blit_effect_alpha_blend(x, y, 0);
		}
		else
		{
			blit_effect_normal(x, y);
		}
	}
	
}


/* ------------------------------------------------- */

#define MESH_NVERT	5535
#define MESH_NNORM	29124
#define MESH_NFACE	9708

extern float vertices[MESH_NVERT][3];
extern float normals[MESH_NNORM][3];
extern int triangles[MESH_NFACE][3];

void draw_lord(void) {
	int i, j;
	
	m3d_begin(M3D_TRIANGLES);
	for(i=0; i<MESH_NFACE; i++) {
		for(j=2; j>=0; j--) {
			int nidx = i * 3 + j;
			int vidx = triangles[i][j];
			
			m3d_normal(normals[nidx][0], normals[nidx][1], normals[nidx][2]);
			m3d_vertex(vertices[vidx][0], vertices[vidx][1], vertices[vidx][2]);
		}
	}
	m3d_end();
}
