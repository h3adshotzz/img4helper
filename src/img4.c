/**
 * 	img4helper
 * 	Copyright (C) 2019, @h3adsh0tzz
 *
 * 	This program is free software: you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 * 	(at your option) any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include "img4.h"
#include "asn1.h"
#include <stdio.h>

char* read_from_file (const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *ret = (char*)malloc(size);
	if (ret) fread(ret, size, 1, f);
	fclose(f);

	return ret;
}

char* read_bytes (char* fn)
{
	char *buf = read_from_file(fn);

	g_print ("ASN1 Elements: %d\n", asn1ElementsInObject(buf));

	for (int i = (int)sizeof(char) * 7; i < (int)sizeof(char) * 11; i++) {
		g_print("%c", buf[i]);
	}
	g_print("\n");

	return buf;
}

void print_img4 (Img4PrintType type, char* filename)
{
	switch (type) {
		case IMG4_PRINT_ALL:
			g_print("Print all!\n");
			break;
		case IMG4_PRINT_IM4P:
			g_print("Print IM4P\n");
			break;
		case IMG4_PRINT_IM4M:
			g_print("Print IM4M\n");
			break;
		default:
			exit(1);
	}

	g_print ("Filename: %s\n", filename);
	if (filename) {
		char *tmp = read_bytes(filename);
	}
}
