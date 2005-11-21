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

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

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
        fprintf(stderr, "Couldn't set video mode: %s\n",SDL_GetError());
        return 0;
    }

	TTF_Init();
	font = TTF_OpenFont(font_file, font_height);
	settings_font_height = TTF_FontHeight(font);
	
	return 1;
}

void close_gfx()
{
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
