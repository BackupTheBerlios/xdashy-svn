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

#include "xdashy.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gfx.h"
#include "input.h"
#ifdef ENABLE_XBOX
#include <hal/xbox.h>
#endif /* ENABLE_XBOX */

/* Settings */
typedef struct rect_t
{
	int x,y;
	unsigned int w,h;
} rect;

struct settings
{
	char bg_file[512];
	char font_file[512];
	int font_height;
	rect client_area;
	rect title_area;
	rect effect_area;

	enum TextAlign text_align;
	
	float text_r;
	float text_g;
	float text_b;

	float selected_text_r;
	float selected_text_g;
	float selected_text_b;

	float bar_r;
	float bar_g;
	float bar_b;

	int fx_alpha_test;
	unsigned char fx_alpha_ref;
	int fx_alpha_blend;
	
} settings;


/* State */
struct xstate
{
	unsigned int selected_item;
	int menu_scroll;
	unsigned int items_in_client_area;
	MenuItem *root_menu;
} xstate;

// This will only affect the font height in xdashy state. Not the actual
// font height of the generated font.
void xdashy_set_font_height(unsigned int h)
{
	settings.font_height = h;
}

static void get_rect(MenuItem *item, rect *r)
{
	MenuItem *item2;
	
	item2 = MenuItem_get_item(item, "x");
	if (item2) r->x = atoi(item2->launch);
	
	item2 = MenuItem_get_item(item, "y");
	if (item2) r->y = atoi(item2->launch);
	
	item2 = MenuItem_get_item(item, "width");
	if (item2) r->w = atoi(item2->launch);
	
	item2 = MenuItem_get_item(item, "height");
	if (item2) r->h = atoi(item2->launch);	
}

static int get_bool(const char *str)
{
	if (!strcmp(str, "true") || !strcmp(str, "yes"))
		return 1;
	return 0;
}

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

static enum TextAlign get_text_align(MenuItem *item)
{
	if (!item) return TA_LEFT;

	if (!strcmp(item->launch, "left")) return TA_LEFT;
	if (!strcmp(item->launch, "center")) return TA_CENTER;
	if (!strcmp(item->launch, "right")) return TA_RIGHT;

	return TA_LEFT;
}

int xdashy_load_settings()
{
	MenuItem *item=0, *item2=0, *config=0;
	
	MenuItem *settings_file;
	
	if (xstate.root_menu)
		del_MenuItem(xstate.root_menu);
	
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
	xstate.root_menu = MenuItem_load(item->launch);
	if (! xstate.root_menu)
	{
		fprintf(stderr, "Failed to open \"%s\"\n", item->launch);
		return 0;
	}

	settings_file = MenuItem_load(item2->launch);
	if (! settings_file)
	{
		fprintf(stderr, "Failed to open \"%s\"\n", item2->launch);
		return 0;
	}
	
	del_MenuItem(config);

	item = MenuItem_get_item(settings_file, "background");
	if (!item) strcpy(settings.bg_file, "back.png");
	else strcpy(settings.bg_file, item->launch);

	item = MenuItem_get_item(settings_file, "font");
	strcpy(settings.font_file, "FreeSansBold.ttf");
	settings.font_height = 16;
	if (item)
	{
		item2 = MenuItem_get_item(item, "file");
		if (item2) strcpy(settings.font_file, item2->launch);

		item2 = MenuItem_get_item(item, "height");
		if (item2) settings.font_height = atoi(item2->launch);
	}

	item = MenuItem_get_item(settings_file, "client area");
	settings.client_area.x = 0;
	settings.client_area.y = 0;
	settings.client_area.w = 640;
	settings.client_area.h = 480;
	if (item) get_rect(item, &settings.client_area);
	
	xstate.items_in_client_area = settings.client_area.h / settings.font_height;
	
	item = MenuItem_get_item(settings_file, "title area");
	settings.title_area.x = 0;
	settings.title_area.y = 0;
	settings.title_area.w = 640;
	settings.title_area.h = 480;
	if (item) get_rect(item, &settings.title_area);

	item = MenuItem_get_item(settings_file, "effect area");
	settings.effect_area.x = 0;
	settings.effect_area.y = 0;
	settings.effect_area.w = 640;
	settings.effect_area.h = 480;
	if (item) get_rect(item, &settings.effect_area);
	
	/* effect blit options */
	item = MenuItem_get_item(settings_file, "effect blit");
	settings.fx_alpha_test = 0;
	settings.fx_alpha_ref = 0;
	settings.fx_alpha_blend = 0;
	if (item)
	{
		item2 = MenuItem_get_item(item, "alpha test");
		if (item2) settings.fx_alpha_test = get_bool(item2->launch);
		item2 = MenuItem_get_item(item, "alpha ref");
		if (item2) settings.fx_alpha_ref = (int)(255 * atof(item2->launch));
		item2 = MenuItem_get_item(item, "alpha blend");
		if (item2) settings.fx_alpha_blend = get_bool(item2->launch);
	}

	/* loa text align */
	settings.text_align = get_text_align(MenuItem_get_item(settings_file, 
		"text align"));
	
	/* load colors */
	get_color(MenuItem_get_item(settings_file, "text color"),
					&settings.text_r, 
					&settings.text_g, 
					&settings.text_b);
	get_color(MenuItem_get_item(settings_file, "selected text color"),
					&settings.selected_text_r, 
					&settings.selected_text_g, 
					&settings.selected_text_b);
	get_color(MenuItem_get_item(settings_file, "bar color"),
					&settings.bar_r, 
					&settings.bar_g, 
					&settings.bar_b);
	
	del_MenuItem(settings_file);
	
	return 1;
}

int xdashy_init()
{	
	if (! xdashy_load_settings())
	{
		fprintf(stderr, "Failed to load xdashy settings\n");
		return 0;
	}
	
	return init_gfx(settings.bg_file, settings.font_file, settings.font_height,
					settings.effect_area.w, settings.effect_area.h);
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
	if (xstate.selected_item > 0) 
	{
		xstate.selected_item --;
		int pos = xstate.selected_item * settings.font_height + xstate.menu_scroll;
		if (pos < 0) xstate.menu_scroll -= pos;
	}
}

void xdashy_move_down()	
{
	if (xstate.selected_item < xstate.root_menu->num_children-1)
	{
		xstate.selected_item ++;
		int pos = xstate.selected_item * settings.font_height + xstate.menu_scroll;
		pos += settings.font_height - 1;
		if (pos > settings.client_area.h)
		{
			xstate.menu_scroll -= pos - settings.client_area.h;
		}
	}
}

void xdashy_execute()
{
	switch (xstate.root_menu->children[xstate.selected_item]->action)
	{
		case MA_NEW_MENU:
			xstate.root_menu = xstate.root_menu->children[xstate.selected_item];
			xstate.selected_item = 0;
			xstate.menu_scroll = 0;
			break;
		case MA_LAUNCH:
			xdashy_perform_action(xstate.root_menu->children[xstate.selected_item]->launch);
			break;
		case MA_NONE:
			break;
	}
}

void xdashy_move_back()
{
	if (xstate.root_menu->mother)
	{
		xstate.root_menu = xstate.root_menu->mother;
		xstate.selected_item = 0;
		xstate.menu_scroll = 0;
	}
}


void xdashy_render()
{
	unsigned int i;

	render_background();

	/* render thingy */
	render_effect(settings.effect_area.x, settings.effect_area.y,
		settings.fx_alpha_test, settings.fx_alpha_ref,
		settings.fx_alpha_blend);
	
	/* set clip rect to title area */
	set_clip_rect(settings.title_area.x, settings.title_area.y,
			settings.title_area.w, settings.title_area.h);
	
	render_text(settings.title_area.x, settings.title_area.y,
				settings.title_area.w,
				xstate.root_menu->title,
				settings.text_r,
				settings.text_g,
				settings.text_b,
				settings.text_align);
	
	/* set clip rect to client area */
	set_clip_rect(settings.client_area.x, settings.client_area.y,
			settings.client_area.w, settings.client_area.h);

	render_quad(0, 
				settings.client_area.y + 
				xstate.selected_item * settings.font_height + 
				xstate.menu_scroll,
				640, settings.font_height,
				settings.bar_r,
				settings.bar_g,
				settings.bar_b);
	
	for (i=0; i<xstate.root_menu->num_children; i++)
	{
		if (i == xstate.selected_item)
		{
			render_text(settings.client_area.x, 
					settings.client_area.y + 
					settings.font_height * i + xstate.menu_scroll,
					settings.client_area.w,
					xstate.root_menu->children[i]->title,
					settings.selected_text_r, 
					settings.selected_text_g, 
					settings.selected_text_b,
					settings.text_align);
		}
		else
		{
			render_text(settings.client_area.x, 
					settings.client_area.y + 
					settings.font_height * i + xstate.menu_scroll,
					settings.client_area.w,
					xstate.root_menu->children[i]->title,
					settings.text_r, 
					settings.text_g, 
					settings.text_b,
					settings.text_align);
		}
	}
	
	/* remove clip rect */
	set_clip_rect(0, 0, 640, 480);
}

void xdashy_close()
{
	if (xstate.root_menu)
		del_MenuItem(xstate.root_menu);
	
	close_gfx();
}
