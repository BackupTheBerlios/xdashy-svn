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

/* xdashy.h
 */

#ifndef _XDASHY_HEADER_
#define _XDASHY_HEADER_

int xdashy_init();

void xdashy_move_up();
void xdashy_move_down();
void xdashy_move_back();
void xdashy_execute();

void xdashy_render();
void xdashy_close();

// This will only affect the font height in xdashy state. Not the actual
// font height of the generated font.
void xdashy_set_font_height(unsigned int h);

#endif /* ndef _XDASHY_HEADER_ */
