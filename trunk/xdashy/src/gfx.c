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

#define REND_XSZ	240
#define REND_YSZ	180

SDL_Surface *pscreen=0;
SDL_Surface *pbackground=0;

TTF_Font *font;

SDL_Joystick *joystick=0;

extern int settings_font_height;

int init_gfx(const char *bg_file, const char *font_file, int font_height)
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
	
	settings_font_height = TTF_FontHeight(font);

	/* initialize mini3d */
	m3d_init();
	m3d_viewport(0, 0, REND_XSZ, REND_YSZ);

	m3d_clear_color(0.0, 0.0, 0.0);
	m3d_clear_depth(1.0);

	//m3d_enable(M3D_DEPTH_TEST);
	//m3d_enable(M3D_LIGHTING);
	//m3d_enable(M3D_LIGHT0);

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

	/* face +Z */
	m3d_normal(0, 0, 1);
	m3d_color3(1, 0, 0);
	m3d_vertex(1, -1, 1);
	m3d_vertex(-1, -1, 1);
	m3d_vertex(-1, 1, 1);
	m3d_vertex(1, 1, 1);

	/* face -X */
	m3d_normal(-1, 0, 0);
	m3d_color3(0, 1, 0);
	m3d_vertex(-1, -1, 1);
	m3d_vertex(-1, -1, -1);
	m3d_vertex(-1, 1, -1);
	m3d_vertex(-1, 1, 1);

	/* face -Z */
	m3d_normal(0, 0, -1);
	m3d_color3(0, 0, 1);
	m3d_vertex(-1, 1, -1);
	m3d_vertex(-1, -1, -1);
	m3d_vertex(1, -1, -1);
	m3d_vertex(1, 1, -1);

	/* face +X */
	m3d_normal(1, 0, 0);
	m3d_color3(1, 1, 0);
	m3d_vertex(1, 1, -1);
	m3d_vertex(1, -1, -1);
	m3d_vertex(1, -1, 1);
	m3d_vertex(1, 1, 1);

	/* face +Y */
	m3d_normal(0, 1, 0);
	m3d_color3(0, 1, 1);
	m3d_vertex(1, 1, 1);
	m3d_vertex(-1, 1, 1);
	m3d_vertex(-1, 1, -1);
	m3d_vertex(1, 1, -1);

	/* face -Y */
	m3d_normal(0, -1, 0);
	m3d_color3(1, 0, 1);
	m3d_vertex(-1, -1, -1);
	m3d_vertex(-1, -1, 1);
	m3d_vertex(1, -1, 1);
	m3d_vertex(1, -1, -1);
	
	m3d_end();
}

void render_3d(int x, int y) {
	int i, xspan, yspan;
	uint32_t *rbuf, *fb;
	float t = SDL_GetTicks() / 10.0f;
	
	m3d_clear(M3D_COLOR_BUFFER_BIT | M3D_DEPTH_BUFFER_BIT);

	m3d_matrix_mode(M3D_MODELVIEW);
	m3d_load_identity();
	m3d_translate(0, 0, 10);
	m3d_rotate(t, 1, 0, 0);
	m3d_rotate(t, 0, 1, 0);

	draw_cube();
	
	rbuf = m3d_get_pixel_data();

	/* copy to SDL surface */
	if(SDL_MUSTLOCK(pscreen)) {
		SDL_LockSurface(pscreen);
	}

	xspan = pscreen->w - x;
	xspan = xspan > REND_XSZ ? REND_XSZ : xspan;

	yspan = pscreen->h - y;
	yspan = yspan > REND_YSZ ? REND_YSZ : yspan;

	fb = (uint32_t*)pscreen->pixels + y * pscreen->w + x;
	for(i=0; i<REND_YSZ; i++) {
		memcpy(fb, rbuf, xspan * sizeof(uint32_t));
		fb += pscreen->w;
		rbuf += REND_XSZ;
	}

	if(SDL_MUSTLOCK(pscreen)) {
		SDL_UnlockSurface(pscreen);
	}
}
