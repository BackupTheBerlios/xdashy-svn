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

/* gfx.h
 */

#ifndef _GFX_HEADER_
#define _GFX_HEADER_

int init_gfx(const char *bg_file, const char *font_file, int font_height, 
				int fx_w, int fx_h);
void render_background();
void close_gfx();
void gfx_flip();
void render_text(int x, int y, char *text, float r, float g, float b);
void render_quad(int x, int y, int w, int h, float r, float g, float b);
void set_clip_rect(int x, int y, int w, int h);

void render_effect(int x, int y, int alpha_test, unsigned char alpha_ref, int alpha_blend);

#endif /* ndef _GFX_HEADER_ */
