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

#ifndef IMG4_H
#define IMG4_H

/* Headers */
#include <glib.h>
#include <lzfse.h>
#include "asn1.h"
#include "lzss.h"
#include "config.h"

/* These are different types of Image4 */
#define IMAGE_TYPE_DIAG					"diag"	// diagnostics
#define IMAGE_TYPE_LLB					"illb"	// iboot first-stage loader
#define IMAGE_TYPE_IBOOT				"ibot"	// iboot second-stage loader
#define IMAGE_TYPE_IBSS					"ibss"	// iboot single stage
#define IMAGE_TYPE_IBEC					"ibec"	// iboot epoch change
#define IMAGE_TYPE_DEVTREE				"dtre"	// darwin device tree
#define IMAGE_TYPE_RAMDISK				"rdsk"	// darwin ram disk for restore
#define IMAGE_TYPE_KERNELCACHE			"krnl"	// darwin kernel cache
#define IMAGE_TYPE_LOGO					"logo"	// boot logo image
#define IMAGE_TYPE_RECMODE				"recm"	// recovery mode image
#define IMAGE_TYPE_NEEDSERVICE			"nsrv"	// need service image
#define IMAGE_TYPE_GLYPHCHRG			"glyC"	// glyph charge image
#define IMAGE_TYPE_GLYPHPLUGIN			"glyP"	// glyph plug in image
#define IMAGE_TYPE_BATTERYCHARGING0		"chg0"  // battery charging image - bright
#define IMAGE_TYPE_BATTERYCHARGING1		"chg1"  // battery charging image - dim
#define IMAGE_TYPE_BATTERYLOW0			"bat0"	// battery low image - empty
#define IMAGE_TYPE_BATTERYLOW1			"bat1"	// battery low image - red (composed onto empty)
#define IMAGE_TYPE_BATTERYFULL			"batF"	// battery full image list
#define IMAGE_TYPE_OS_RESTORE			"rosi"	// OS image for restore
#define IMAGE_TYPE_SEP_OS				"sepi"	// SEP OS image
#define IMAGE_TYPE_SEP_OS_RESTORE		"rsep"	// SEP OS image for restore
#define IMAGE_TYPE_HAMMER				"hmmr"	// PE's Hammer test


/* Image type */
typedef enum {
	IMG4_TYPE_ALL,
	IMG4_TYPE_IMG4,
	IMG4_TYPE_IM4P,
	IMG4_TYPE_IM4M,
	IMG4_TYPE_IM4R
} Image4Type;

/**
 * 	This is not a struct for the file structure, its to hold some
 * 	info, the size and a loaded buffer of the img4/im4p/im4m/im4r
 * 	file.
 */
typedef struct image4_t {

	/* File buffer and size */
	char *buf;
	size_t size;

	/* Image4 variant and component (krnl, ibot, etc...) */
	Image4Type type;
	char *component;

} image4_t;


/**
 * 	Functions for handling and manipulating image4_t's
 * 
 */
image4_t 	*img4_read_image_from_path (char *path);
char 		*img4_string_for_image_type (Image4Type print_type);
char 		*img4_get_component_name (char *buf);


/**
 * 	Functions for handling specific types of Image4 files.
 * 
 * 	Note: Full .img4 files should call handle_img4(), which will then
 * 	split the file and call the respective handle_x functions for the
 * 	part of the file being worked on.
 * 
 * 	All functions take the same arguments. The buf is the loaded
 * 	image file, and the tabs are how many indentations to print the
 * 	data in.
 * 
 * 	The handle() function will take an image_t and use the correct
 * 	function.
 * 
 */
void img4_handle (image4_t *image);

void img4_handle_img4 (char *buf, int tabs);
void img4_handle_im4p (char *buf, int tabs);
void img4_handle_im4m (char *buf, int tabs);
void img4_handle_im4r (char *buf, int tabs);


/**
 * 	Functions for printing Image4's.
 *
 * 	One passes the Image4 type and the filename to load to this function,
 * 	the filename is that loaded and an image4_t is constructed and passed
 * 	to img4_handle.
 * 
 */
void img4_print_with_type (Image4Type type, char *filename);


#endif /* IMG4_H */
