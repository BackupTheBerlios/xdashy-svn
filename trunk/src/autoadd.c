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

/* autoadd.c
 */

#include "menu.h"
#ifdef ENABLE_XBOX
#include <hal/fileio.h>
#endif /* ENBLE_XBOX */
#include <stdio.h>
#include <string.h>

char text_buffer[1024];

void auto_add_items(MenuItem *root, char *path)
{
#ifndef ENABLE_XBOX
	return;
#else

	unsigned int dir_handle;
	int len;
	FILE *fp;
	MenuItem *item;

	if (!root) return;
	
	/* get the first file */
	XBOX_FIND_DATA file_data;
	dir_handle = XFindFirstFile(path, "*", &file_data);
	
	if (dir_handle == ERROR_INVALID_HANDLE) return;
	
	do
	{
		/* search each subdirectory for a "default.xbe" */
		if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			/* create full file name */
			strcpy(text_buffer, path);
			if (text_buffer[strlen(text_buffer) - 1] != '/')
				strcat(text_buffer, "/");
			strcat(text_buffer, file_data.cFileName);
			strcat(text_buffer, "/default.xbe");

			fp = fopen(text_buffer, "r");
			if (fp)
			{
				fclose(fp);
				item = new_MenuItem(file_data.cFileName);
				MenuItem_set_launcher(item, text_buffer);
				MenuItem_add_item(root, item);
			}
		}
	} while(!XFindNextFile(dir_handle, &file_data));
	
	XFindClose(dir_handle);
#endif /* ndef ENABLE_XBOX */
}
