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
// Add in meson to check if we can use CommonCrypto instead.
//#include <openssl/aes.h>
#include <lzfse.h>

#include "asn1.h"
#include "lzss.h"
#include "config.h"

/* Image types */
// TODO: Add all of them
#define IMAGE_TYPE_DIAG         'diag' // diagnostics
#define IMAGE_TYPE_IBOOT_LLB    'illb' // iboot (darwin) first-stage loader
#define IMAGE_TYPE_IBOOT        'ibot' // iboot (darwin) loader
#define IMAGE_TYPE_KERNEL		'krnl' // kernel cache
#define IMAGE_TYPE_DEVTREE      'dtre' // darwin device tree
#define IMAGE_TYPE_LOGO         'logo' // boot logo image
#define IMAGE_TYPE_RECMODE      'recm' // recovery mode image
#define IMAGE_TYPE_NEEDSERVICE  'nsrv' // recovery mode image
#define IMAGE_TYPE_BATTERYLOW0  'batl' // battery low image - empty
#define IMAGE_TYPE_BATTERYLOW1  'batL' // battery low image - red
#define IMAGE_TYPE_BATTERYCHRG  'batC' // battery charge image
#define IMAGE_TYPE_ENV          'ienv' // environment vars
#define IMAGE_TYPE_TSYS         'tsys' // tsys tester
#define IMAGE_TYPE_CHIME        'beep' // boot chime

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
