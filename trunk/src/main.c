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

/* main.c
 */

#include <stdio.h>
#include "menu.h"
#include "gfx.h"
#include "xdashy.h"
#include "input.h"
#include "autoadd.h"

#ifdef ENABLE_XBOX
# include <hal/xbox.h>
void XBoxStartup()
{
	freopen("stdout.txt", "a", stdout);
	freopen("stderr.txt", "a" ,stderr);

#else
int main(int argc, char *argv[])
{
#endif /* ENABLE_XBOX */

	if (! xdashy_init()) 
#ifndef ENABLE_XBOX
			return -1;
#else
			return;
#endif /* ndef ENABLE_XBOX */
		
	while (process_input())
	{		
		/* render */
		xdashy_render();
		
		gfx_flip();
	}

	xdashy_close();

#ifndef ENABLE_XBOX
	return 0;
#endif /* ndef ENABLE_XBOX */
}
