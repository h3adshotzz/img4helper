//===----------------------------------------------------------------------===//
//
//                              Img4helper
//
//  This  document  is the property of "Is This On?" It is considered to be
//  confidential and proprietary and may not be, in any form, reproduced or
//  transmitted, in whole or in part, without express permission of Is This
//  On?.
//
//  Copyright (C) 2021, Is This On? Holdings
//  
//  Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

#include "image4.h"
#include "asn1.h"

#include <libhelper-logger.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

/////////////////
image4_type_t
image4_get_file_type (image4_t *image4);

char *
image4_get_component_type (image4_t *image4);
//////////////////

image4_t *
image4_load_file (const char *path)
{
    image4_t *image4 = calloc (1, sizeof (image4_t));
    
    /* verify file path */
    if (!path) {
        errorf ("htool_image4ary_load: file path is invalid\n");
        return NULL;
    }
    image4->path = (char *) strdup (path);
    
    /* create the file descriptor */
    int fd = open (image4->path, O_RDONLY);
    
    /* calculate the file size */
    struct stat st;
    fstat (fd, &st);
    image4->size = st.st_size;
    
    /* mmap the file */
    image4->data = mmap (NULL, image4->size, PROT_READ, MAP_PRIVATE, fd, 0);
    close (fd);
    
    /* verify the map was successful */
    if (image4->data == MAP_FAILED) {
        errorf ("htool_image4ary_load: failed to map file at path: %s\n", image4->path);
        return NULL;
    }
    
    return (image4) ? image4 : NULL;
}

image4_t *
image4_load (const char *path)
{
    /* load the file at the path into a new struct */
    image4_t *image4 = image4_load_file (path);

    /* verify the file load */
    if (!image4)
        goto FAIL;

    /* check and set the image4 file type */
    image4->type = image4_get_file_type (image4);
    printf ("type: %d\n", image4->type);

    /* check and set the image4 component type */
    image4->comp = image4_get_component_type (image4);
    printf ("comp: %s\n", image4->comp);

FAIL:
    return (image4) ? image4 : NULL;
}

//////////////////////////////////////////////////////////

image4_type_t
image4_get_file_type (image4_t *image4)
{
    char 	*magic;
    size_t 	 l;

    unsigned char *buffer = malloc (image4->size);
    memcpy (buffer, image4->data, image4->size);

    if ((const char *) *image4->data == kASN1TagPrivate)
        printf("yest\n");
    else printf ("na\n");

    /**
     *	Get the Image4 magic sequence, and then compare that to the known
     *  types. Return the correct image4_type_t.
     */
    asn1_get_sequence_name(image4->data, &magic, &l);
    debugf ("magic: %s\n", magic);

    if (!strncmp("IMG4", magic, l)) return IMAGE4_TYPE_IMG4;
    else if (!strncmp("IM4P", magic, l)) return IMAGE4_TYPE_IM4P;
    else if (!strncmp("IM4M", magic, l)) return IMAGE4_TYPE_IM4M;
    else if (!strncmp("IM4R", magic, l)) return IMAGE4_TYPE_IM4R;
    else goto FAIL;

FAIL:
    warningf ("image4_get_file_type: unexpected tag, got: \"%s\"\n", magic);
    return IMAGE4_TYPE_UNKNOWN;
}

char *
image4_get_component_type (image4_t *image4)
{
    char *comp_name;
	size_t l;

	asn1_get_sequence_name (image4->data, &comp_name, &l);
	char *raw = asn1_element_at_index (image4->data, 1) + 2;

	if (!strncmp (raw, IMAGE_TYPE_IBOOT, 4))                    return IMAGE_TYPE_IBOOT;
	else if (!strncmp (raw, IMAGE_TYPE_IBEC, 4))                return IMAGE_TYPE_IBEC;
	else if (!strncmp (raw, IMAGE_TYPE_IBSS, 4))                return IMAGE_TYPE_IBSS;
	else if (!strncmp (raw, IMAGE_TYPE_LLB, 4))                 return IMAGE_TYPE_LLB;
	else if (!strncmp (raw, IMAGE_TYPE_SEP_OS, 4))              return IMAGE_TYPE_SEP_OS;
	else if (!strncmp (raw, IMAGE_TYPE_SEP_OS_RESTORE, 4))      return IMAGE_TYPE_SEP_OS_RESTORE;
	else if (!strncmp (raw, IMAGE_TYPE_DEVTREE, 4))             return IMAGE_TYPE_DEVTREE;
	else if (!strncmp (raw, IMAGE_TYPE_RAMDISK, 4))             return IMAGE_TYPE_RAMDISK;
	else if (!strncmp (raw, IMAGE_TYPE_KERNELCACHE, 4))         return IMAGE_TYPE_KERNELCACHE;
	else if (!strncmp (raw, IMAGE_TYPE_LOGO, 4))                return IMAGE_TYPE_LOGO;
	else if (!strncmp (raw, IMAGE_TYPE_RECMODE, 4))             return IMAGE_TYPE_RECMODE;
	else if (!strncmp (raw, IMAGE_TYPE_NEEDSERVICE, 4))         return IMAGE_TYPE_NEEDSERVICE;
    else if (!strncmp (raw, IMAGE_TYPE_GLYPHCHRG, 4))           return IMAGE_TYPE_GLYPHCHRG;
    else if (!strncmp (raw, IMAGE_TYPE_GLYPHPLUGIN, 4))         return IMAGE_TYPE_GLYPHPLUGIN;
	else if (!strncmp (raw, IMAGE_TYPE_BATTERYCHARGING0, 4))    return IMAGE_TYPE_BATTERYCHARGING0;
	else if (!strncmp (raw, IMAGE_TYPE_BATTERYCHARGING1, 4))    return IMAGE_TYPE_BATTERYCHARGING1;
	else if (!strncmp (raw, IMAGE_TYPE_BATTERYLOW0, 4))         return IMAGE_TYPE_BATTERYLOW0;
	else if (!strncmp (raw, IMAGE_TYPE_BATTERYLOW1, 4))         return IMAGE_TYPE_BATTERYLOW1;
	else if (!strncmp (raw, IMAGE_TYPE_BATTERYFULL, 4))         return IMAGE_TYPE_BATTERYFULL;
	else if (!strncmp (raw, IMAGE_TYPE_OS_RESTORE, 4))          return IMAGE_TYPE_OS_RESTORE;
	else if (!strncmp (raw, IMAGE_TYPE_HAMMER, 4))              return IMAGE_TYPE_HAMMER;
    
    /* otherwise, return the substring */
    char *comp = malloc (8);
    memcpy (comp, comp_name + 4, 8);

    return comp;
}












