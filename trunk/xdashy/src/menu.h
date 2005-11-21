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

/* menu.h
 */

#ifndef _MENU_HEADER_
#define _MENU_HEADER_

enum MenuAction
{
	MA_NONE,
	MA_LAUNCH,
	MA_NEW_MENU
};

typedef struct MenuItem_t{
	struct MenuItem_t *mother;
	enum MenuAction action;
	unsigned int num_children;
	struct MenuItem_t **children;
	char launch[512];
	char title[512];
}MenuItem;

MenuItem *new_MenuItem(const char *title);
void del_MenuItem(MenuItem *item);

void MenuItem_add_item(MenuItem *item, MenuItem *item_to_add);
void MenuItem_set_launcher(MenuItem *item, const char *filename);

MenuItem *MenuItem_get_item(MenuItem *item, const char *title);

MenuItem *MenuItem_load(const char *filename);
void MenuItem_save(MenuItem *item, const char *filename);

#endif /* ndef _MENU_HEADER_ */
