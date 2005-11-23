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

/* input.c
 */

#include <SDL.h>
#include "xdashy.h"

int process_input()
{
	SDL_Event event;

	while( SDL_PollEvent( &event ) ) 
	{
		switch( event.type ) 
		{
			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					return 0;
					break;

				case SDLK_UP:
					xdashy_move_up();
					break;
				
				case SDLK_DOWN:
					xdashy_move_down();
					break;
				
				case SDLK_LEFT:
					xdashy_move_back();
					break;
				
				case SDLK_RIGHT:
				case SDLK_RETURN:
					xdashy_execute();
					break;

				default:
					break;
				}
				break;
			case SDL_JOYHATMOTION:
				switch (event.jhat.value)
				{
				case SDL_HAT_UP:
					xdashy_move_up();
					break;
				
				case SDL_HAT_DOWN:
					xdashy_move_down();
					break;
				
				case SDL_HAT_LEFT:
					xdashy_move_back();
					break;
				
				case SDL_HAT_RIGHT:
					xdashy_execute();
					break;

				default:
					break;
				}
				break;
		}
	}

	return 1;
}

