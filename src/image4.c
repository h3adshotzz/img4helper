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

#include <libhelper-logger.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

image4_t *
image4_load_file (const char *path)
{
    image4_t *image4 = image4_create ();
    
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

    


FAIL:
    return (image4) ? image4 : NULL;
}

image4_t *
image4_create ()
{
    return (image4_t *) calloc (1, sizeof (image4_t));
}