/**
 *     img4helper
 *     Copyright (C) 2019, @h3adsh0tzz
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

// Testing git config on new computer

#ifndef IMG4_H
#define IMG4_H

#include <glib.h>
#include <openssl/aes.h>
#include <lzfse.h>

#include "asn1.h"
#include "lzss.h"
#include "config.h"

typedef enum {
	IMG4_PRINT_ALL,
	IMG4_PRINT_IM4P,
	IMG4_PRINT_IM4M
} Img4PrintType;


typedef enum {
	IMG4_TYPE_IMG4,
	IMG4_TYPE_IM4M,
	IMG4_TYPE_IM4P,
	IMG4_TYPE_IM4R
} Img4Type;

typedef struct imgfile_t {
	char *buf;
	size_t size;
} imgfile_t;


/**
 * 	This is not a struct for the file structure, its to hold some
 * 	info, the size and a loaded buffer of the img4/im4p/im4m/im4r
 * 	file.
 */
typedef struct img4_t {

	/* File buffer and size */
	char *buf;
	size_t size;

	/* Image4 variant */
	Img4Type type;

} img4_t;


// Img4 printing
void print_img4(Img4PrintType type, char* filename);

img4_t *read_img (char *path);
char *string_for_img4type (Img4Type type);
char *img4_get_component_name (char *buf);
char *img4_check_compression_type (char *buf);
img4_t *img4_decompress_bvx2 (img4_t *file);

void img4_extract_im4p (char *im4p, char* outfile);
void img4_extract_test (char *file);

#endif
