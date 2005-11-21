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

/* menu.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "menu.h"

/* Allocate a new MenuItem object
 * object's title is set to "title"
 * action is set to MA_NONE
 */
MenuItem *new_MenuItem(const char *title)
{
	MenuItem *item = malloc(sizeof(MenuItem));

	if (!item) return 0;
	
	item->mother = 0;
	item->action = MA_NONE;
	item->num_children = 0;
	item->children = 0;
	item->launch[0] = 0;
	strcpy(item->title, title);

	return item;
}

/* Recursively delete the item and its children
 */
void del_MenuItem(MenuItem *item)
{
	if (!item) return;
	
	unsigned int i;
	for (i=0; i < item->num_children; i++)
	{
		if (item->children[i]->num_children)
			del_MenuItem(item->children[i]);
	}
	free(item);
	item = 0;
}

/* add a new item. Action is automatically set to MA_NEW_MENU
 */
void MenuItem_add_item(MenuItem *item, MenuItem *item_to_add)
{
	if (!item || !item_to_add) return;
	
	/* allocate memory */
	item->children = realloc(item->children, 
		(item->num_children+1) * sizeof(MenuItem*));
	
	/* add item */
	item_to_add->mother = item;
	item->children[item->num_children] = item_to_add;
	item->num_children ++;
	item->action = MA_NEW_MENU;
}

/* set a file to execute. Action is automatically set to MA_LAUNCH
 */
void MenuItem_set_launcher(MenuItem *item, const char *filename)
{
	if (!item || !filename) return;
	strcpy(item->launch, filename);
	item->action = MA_LAUNCH;
}

/* get a child item by title
 */
MenuItem *MenuItem_get_item(MenuItem *item, const char *title)
{
	unsigned int i;
	for (i=0; i<item->num_children; i++)
	{
		if (!strcmp(item->children[i]->title, title))
			return item->children[i];
	}

	return 0;
}
