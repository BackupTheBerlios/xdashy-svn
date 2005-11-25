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

/* xdashy.c
 */

#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gfx.h"
#include "input.h"
#ifdef ENABLE_XBOX
#include <hal/xbox.h>
#endif /* ENABLE_XBOX */

MenuItem *root_menu;
MenuItem *settings;

/* Settings */
char settings_background[512];
char settings_font_file[512];
int settings_font_height;
int settings_client_area_x;
int settings_client_area_y;
int settings_client_area_width;
int settings_client_area_height;

int settings_title_area_x;
int settings_title_area_y;
int settings_title_area_width;
int settings_title_area_height;

float settings_text_r;
float settings_text_g;
float settings_text_b;

float settings_selected_text_r;
float settings_selected_text_g;
float settings_selected_text_b;

float settings_bar_r;
float settings_bar_g;
float settings_bar_b;

/* rendering */
unsigned int selected_item=0;
int menu_scroll=0;
unsigned int items_in_client_area=0;

static void get_color(MenuItem *item, float *r, float *g, float *b)
{
	MenuItem *item2;
	float R=1,G=1,B=1;
	if (!item)
		return;

	item2 = MenuItem_get_item(item, "r");
	if (item2) R = (float) atof(item2->launch);

	item2 = MenuItem_get_item(item, "g");
	if (item2) G = (float) atof(item2->launch);
	
	item2 = MenuItem_get_item(item, "b");
	if (item2) B = (float) atof(item2->launch);

	*r = R;
	*g = G;
	*b = B;
}

int xdashy_load_settings()
{
	MenuItem *item=0, *item2=0, *config=0;
	
	if (root_menu)
		del_MenuItem(root_menu);
	if (settings)
		del_MenuItem(settings);
	
	config = MenuItem_load("data/xdashy.config");
	if (! config)
	{
		fprintf(stderr, "Failed to open \"data/xdashy.config\"\n");
		return 0;
	}

	item = MenuItem_get_item(config, "menu");
	item2 = MenuItem_get_item(config, "skin");

	if (! item || ! item2)
	{
		fprintf(stderr, "Bad \"xdashy.config\" file\n");
		return 0;
	}
	
	/* load settings */
	root_menu = MenuItem_load(item->launch);
	if (! root_menu)
	{
		fprintf(stderr, "Failed to open \"%s\"\n", item->launch);
		return 0;
	}

	settings = MenuItem_load(item2->launch);
	if (! settings)
	{
		fprintf(stderr, "Failed to open \"%s\"\n", item2->launch);
		return 0;
	}
	
	del_MenuItem(config);

	item = MenuItem_get_item(settings, "background");
	if (!item) strcpy(settings_background, "back.png");
	else strcpy(settings_background, item->launch);

	item = MenuItem_get_item(settings, "font");
	strcpy(settings_font_file, "FreeSansBold.ttf");
	settings_font_height = 16;
	if (item)
	{
		item2 = MenuItem_get_item(item, "file");
		if (item2) strcpy(settings_font_file, item2->launch);

		item2 = MenuItem_get_item(item, "height");
		if (item2) settings_font_height = atoi(item2->launch);
	}

	item = MenuItem_get_item(settings, "client area");
	settings_client_area_x = 0;
	settings_client_area_y = 0;
	settings_client_area_width = 640;
	settings_client_area_height = 480;
	if (item)
	{
		item2 = MenuItem_get_item(item, "x");
		if (item2) settings_client_area_x = atoi(item2->launch);

		item2 = MenuItem_get_item(item, "y");
		if (item2) settings_client_area_y = atoi(item2->launch);

		item2 = MenuItem_get_item(item, "width");
		if (item2) settings_client_area_width = atoi(item2->launch);

		item2 = MenuItem_get_item(item, "height");
		if (item2) settings_client_area_height = atoi(item2->launch);
	}

	items_in_client_area = settings_client_area_height / settings_font_height;
	
	item = MenuItem_get_item(settings, "title area");
	settings_title_area_x = 0;
	settings_title_area_y = 0;
	settings_title_area_width = 640;
	settings_title_area_height = 480;
	if (item)
	{
		item2 = MenuItem_get_item(item, "x");
		if (item2) settings_title_area_x = atoi(item2->launch);

		item2 = MenuItem_get_item(item, "y");
		if (item2) settings_title_area_y = atoi(item2->launch);

		item2 = MenuItem_get_item(item, "width");
		if (item2) settings_title_area_width = atoi(item2->launch);

		item2 = MenuItem_get_item(item, "height");
		if (item2) settings_title_area_height = atoi(item2->launch);
	}

	/* load colors */
	get_color(MenuItem_get_item(settings, "text color"),
					&settings_text_r, 
					&settings_text_g, 
					&settings_text_b);
	get_color(MenuItem_get_item(settings, "selected text color"),
					&settings_selected_text_r, 
					&settings_selected_text_g, 
					&settings_selected_text_b);
	get_color(MenuItem_get_item(settings, "bar color"),
					&settings_bar_r, 
					&settings_bar_g, 
					&settings_bar_b);
	return 1;
}

int xdashy_init()
{	
	if (! xdashy_load_settings())
	{
		fprintf(stderr, "Failed to load xdashy settings\n");
		return 0;
	}
	
	return init_gfx(settings_background, settings_font_file, settings_font_height);
}

static void xdashy_perform_action(char *action)
{
	/* predefined actions */
	if (!strcmp(action, "REBOOT XBOX"))
	{
		fprintf(stdout, "Rebooting ...\n");
#ifdef ENABLE_XBOX
		XReboot();
#endif /* ENABLE_XBOX */
		return;
	}
	
	fprintf(stdout, "Launching %s ...\n", action);
#ifdef ENABLE_XBOX
	XLaunchXBE(action);
#endif /* ENABLE_XBOX */

}

void xdashy_move_up()
{
	if (selected_item > 0) 
	{
		selected_item --;
		int pos = selected_item * settings_font_height + menu_scroll;
		if (pos < 0) menu_scroll -= pos;
	}
}

void xdashy_move_down()	
{
	if (selected_item < root_menu->num_children-1)
	{
		selected_item ++;
		int pos = selected_item *settings_font_height + menu_scroll;
		pos += settings_font_height - 1;
		if (pos > settings_client_area_height)
		{
			menu_scroll -= pos - settings_client_area_height;
		}
	}
}

void xdashy_execute()
{
	switch (root_menu->children[selected_item]->action)
	{
		case MA_NEW_MENU:
			root_menu = root_menu->children[selected_item];
			selected_item = 0;
			menu_scroll = 0;
			break;
		case MA_LAUNCH:
			xdashy_perform_action(root_menu->children[selected_item]->launch);
			break;
		case MA_NONE:
			break;
	}
}

void xdashy_move_back()
{
	if (root_menu->mother)
	{
		root_menu = root_menu->mother;
		selected_item = 0;
		menu_scroll = 0;
	}
}


void xdashy_render()
{
	unsigned int i;

	render_background();
	
	/* set clip rect to title area */
	set_clip_rect(settings_title_area_x, settings_title_area_y,
			settings_title_area_width, settings_title_area_height);
	
	render_text(settings_title_area_x, settings_title_area_y,
				root_menu->title,
				settings_text_r,
				settings_text_g,
				settings_text_b);
	
	/* set clip rect to client area */
	set_clip_rect(settings_client_area_x, settings_client_area_y,
			settings_client_area_width, settings_client_area_height);

	render_quad(0, 
				settings_client_area_y + 
				selected_item * settings_font_height + 
				menu_scroll,
				640, settings_font_height,
				settings_bar_r,
				settings_bar_g,
				settings_bar_b);
	
	for (i=0; i<root_menu->num_children; i++)
	{
		if (i == selected_item)
		{
			render_text(settings_client_area_x, 
					settings_client_area_y + 
					settings_font_height * i + menu_scroll,
					root_menu->children[i]->title,
					settings_selected_text_r, 
					settings_selected_text_g, 
					settings_selected_text_b);
		}
		else
		{
			render_text(settings_client_area_x, 
					settings_client_area_y + 
					settings_font_height * i + menu_scroll,
					root_menu->children[i]->title,
					settings_text_r, 
					settings_text_g, 
					settings_text_b);
		}
	}
	
	/* remove clip rect */
	set_clip_rect(0, 0, 640, 480);

	/* render thingy */
	render_3d(320, 120);
}

void xdashy_close()
{
	if (root_menu)
		del_MenuItem(root_menu);
	if (settings)
		del_MenuItem(settings);
	
	close_gfx();
}
