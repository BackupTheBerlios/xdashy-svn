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

/* menuparser.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "autoadd.h"

static char buffer1[512];

static int get_text(char *buffer)
{
	int len = 0;
	if (buffer[0] != '"') return -1;
	while (buffer[len+1] != '"')
	{
		buffer1[len] = buffer[len+1];
		len++;
	}

	buffer1[len] = 0;
	return len;
}

static int error_check(char *buffer, unsigned int len)
{
	/* TODO: replace "agkyles" with the english name for char '{' */
	int quotes=0, agkyles=0;
	unsigned int read_pos;

	for (read_pos=0; read_pos<len; read_pos++)
	{
		char c=buffer[read_pos];

		if (c == '{') agkyles ++;
		if (c == '}') agkyles --;

		if (agkyles < 0) return 0; 

		if (c == '"') quotes++;
	}

	if (quotes % 2) return 0;

	return 1;
}

static unsigned int remove_comments(char *buffer, unsigned int len)
{
	unsigned int read_pos, write_pos=0;
	char c;
	int comment = 0;
	
	for (read_pos=0; read_pos<len; read_pos++)
	{
		c = buffer[read_pos];
		if (c == '#') comment = 1;
		if (c == '\n') comment = 0;

		if (!comment)
		{
			buffer[write_pos] = c;
			write_pos ++;
		}
	}

	return write_pos;
}

static unsigned int remove_spaces(char *buffer, unsigned int len)
{
	unsigned int read_pos, write_pos=0;
	char c;
	int quote = 0;
	
	for (read_pos=0; read_pos<len; read_pos++)
	{
		c = buffer[read_pos];
		if (c == '"') quote = !quote;

		if (quote)
		{
			buffer[write_pos] = c;
			write_pos ++;
		}
		else
		{
			if (c!='\t' && c!='\n' && c!=' ')
			{
				buffer[write_pos] = c;
				write_pos ++;
			}
		}
	}

	return write_pos;
}

static void load(char *buffer, unsigned int len, MenuItem *root)
{
	MenuItem *new_item=0;
	int title_len;
	
	if (!len) return;
	
	if (buffer[0] == '}')
	{
		load(buffer + 1, len - 1, root->mother);
	}
	
	title_len = get_text(buffer);
	buffer += title_len + 2;
	len -= title_len + 2;

	if (!strcmp(buffer1, "AUTO ADD"))
	{
		if (buffer[0] == '=')
		{
			buffer++;
			len--;
			title_len = get_text(buffer);
			buffer += title_len + 2;
			len -= title_len + 2;
			auto_add_items(root, buffer1);
			load(buffer, len, root);
		}
	}
	
	else
	{
		new_item = new_MenuItem(buffer1);
	
		if (buffer[0] == '=')
		{
			buffer++;
			len--;
			title_len = get_text(buffer);
			buffer += title_len + 2;
			len -= title_len + 2;
			MenuItem_set_launcher(new_item, buffer1);
			MenuItem_add_item(root, new_item);
			load(buffer, len, root);
		}
		else if (buffer[0] == '{')
		{
			buffer++;
			len--;
			MenuItem_add_item(root, new_item);
			load(buffer, len, new_item);
		}
	}
}

MenuItem *MenuItem_load(const char *filename)
{
	FILE *fp;
	char *buffer;
	unsigned int len;
	MenuItem *root = 0;
	
	fp = fopen(filename, "r");
	if (!fp) return 0;
	
	/* get file size */
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
		
	/* allocate buffer */
	buffer = malloc(len);
	
	/* copy file to memory */
	fread(buffer, 1, len, fp);
	
	/* process script */
	if (! error_check(buffer, len))
	{
		fprintf(stderr, "%s: syntax error\n", filename);
		return 0;
	}
	len = remove_comments(buffer, len);
	len = remove_spaces(buffer, len);
	root = new_MenuItem("Main menu");
	
	load(buffer, len, root);
	
	/* clean up */
	free(buffer);
	fclose(fp);
	
	return root;
}

static void tabs(FILE *fp, int num)
{
	unsigned int i=0;
	
	for (i=0; i<num; i++)
	{
		fprintf(fp, "\t");
	}
}

static void save(MenuItem *item, FILE *fp, int indent)
{
	unsigned int i;
	if (! item) return;
	tabs(fp, indent);
	fprintf(fp, "\"%s\"", item->title);
	
	switch (item->action)
	{
		case MA_NONE:
			fprintf(fp, "\n");
			tabs(fp, indent);
			fprintf(fp, "{\n");
			tabs(fp, indent);
			fprintf(fp, "}\n");
			break;
		case MA_NEW_MENU:
			fprintf(fp, "\n");
			tabs(fp, indent);
			fprintf(fp, "{\n");
			for (i=0; i<item->num_children; i++)
			{
				save(item->children[i], fp, indent+1);
			}
			tabs(fp, indent);
			fprintf(fp, "}\n");
			break;
		case MA_LAUNCH:
			fprintf(fp, "=\"%s\"\n", item->launch);
			break;
		default:
			break;
	}
}

void MenuItem_save(MenuItem *item, const char *filename)
{
	unsigned int i=0;
	FILE *fp;

	fp = fopen(filename, "w");
	if (!fp) return;

	for (i=0; i<item->num_children; i++)
		save(item->children[i], fp, 0);
	
	fclose(fp);
}
