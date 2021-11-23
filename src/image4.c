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

int
img4_check_compression_type (char *buf);

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
    if (image4->type == IMAGE4_TYPE_IMG4) {
        
        /**
         *  If we have a full IMG4, we want to look for all three components,
         *  im4p, im4m and im4r. Once each comp is found, we can set the flag
         *  and call the relevant parse function to get the correct struct.
         */
        char *magic;
        size_t l;

        /* try im4p first */
        char *im4p_raw = (char *) asn1_element_at_index (image4->data, 1);
        asn1_get_sequence_name (im4p_raw, &magic, &l);

        if (!strncmp (magic, "IM4P", 4)) {
            
            /* set the flag and im4p struct */
            image4->flags |= IMAGE4_FILE_INCLUDES_IM4P;
            image4->im4p = image4_parse_im4p (im4p_raw);

            if (image4->im4p == NULL) {
                warningf ("image4_load: could not load IM4P from image marked IMG4.\n");
            }
        }


    } else if (image4->type == IMAGE4_TYPE_IM4P) {
        
        /* set the flags */
        image4->flags |= IMAGE4_FILE_SINGLE_COMP;
        image4->im4p = image4_parse_im4p (image4->data);

    }



    /* check and set the image4 component type */
    //image4->comp = image4_get_component_type (image4);
    //printf ("comp: %s\n", image4->comp);

FAIL:
    return (image4) ? image4 : NULL;
}

im4p_t *
image4_parse_im4p (unsigned char *buf)
{
    im4p_t *im4p = calloc (1, sizeof (im4p_t));

    /* verify the magic is an im4p */
    char *magic;
    size_t l;

    asn1_get_sequence_name (buf, &magic, &l);
    if (strncmp (magic, "IM4P", 4)) {
        warningf ("image4_parse_im4p: provided buffer is not an IM4P.\n");
        return NULL;
    }

    int elems = asn1_elements_in_object (buf);

    /* get and set the component type */
    if (--elems > 0)
        im4p->comp = asn1_get_string_from_tag ((asn1_tag_t *) asn1_element_at_index (buf, 1));

    /* get and set the descriptor */
    if (--elems > 0)
        im4p->desc = asn1_get_string_from_tag ((asn1_tag_t *) asn1_element_at_index (buf, 2));

    /* get and set the payload size */
    asn1_tag_t *data = (asn1_tag_t *) asn1_element_at_index (buf, 3);
    if (data->tag_number != kASN1TagOCTET) {
        warningf ("image4_parse_im4p: skipped an unexpected tag where an OCTETSTRING was expected\n");
    } else {
	im4p->size = asn1_len((char *) data + 1).data_len;
    }

    /* check if the payload is compressed (if it's encrypted, the flag can be set during decryption */
    im4p->flags |= img4_check_compression_type (buf);    

    /* check, get and set the KBAG value */
    char *kbag_octet = (char *) asn1_element_at_index (buf, 4);
    if (kbag_octet != NULL && ((asn1_tag_t *) kbag_octet)->tag_number == kASN1TagOCTET) {

        /* set the im4p flag that the payload is encrypted */
        im4p->flags |= IMAGE4_FILE_ENCRYPTED;

        /* get the length of the kbag tag octet */
        asn1_elem_len_t octet_len = asn1_len (++kbag_octet);
        kbag_octet += octet_len.size_bytes;

        /* parse the kbag */
        for (int i = 0; i < asn1_elements_in_object (kbag_octet); i++) {

            /* create the kbag struct */
            kbag_t *kbag = calloc (1, sizeof (kbag_t));
            
            /* pick the value */
            char *seq = (char *) asn1_element_at_index (kbag_octet, i);
            int elems = asn1_elements_in_object (seq);
    
            /**
             *  With KBAGs, we often get multiple types: PRODUCTION and DEVELOPMENT. Typically the
             *  KBAG marked PRODUCTION is the first one. This code will parse however many KBAGs
             *  are contained here, and mark the first two accordingly. Any extras are marked as
             *  UNKNOWN.
             */
            if (elems--) {
                asn1_tag_t *num = (asn1_tag_t *) asn1_element_at_index (seq, 0);

                if (num->tag_number == kASN1TagINTEGER) {
                    int j;

                    /* set the kbag type, depending on i */
                    if (i == 0) kbag->type = IMAGE4_KBAG_TYPE_PRODUCTION;
                    else if (i == 1) kbag->type = IMAGE4_KBAG_TYPE_DEVELOPMENT;
                    else kbag->type = IMAGE4_KBAG_TYPE_UNKNOWN;

                    /**
                     *  Parse the KBAG IV. Load the tag from the KBAG sequence, get the length
                     *  and string for the IV, and write byte-by-byte into the iv buffer in the
                     *  kbag struct. 
                     */
                    asn1_tag_t *tag_iv = asn1_element_at_index (seq, 1);
                    asn1_elem_len_t tag_iv_len = asn1_len ((char *) tag_iv + 1);

                    j = 0;
                    unsigned char *tag_iv_str = (unsigned char *) tag_iv + tag_iv_len.size_bytes + 1;
                    while (tag_iv_len.data_len--) kbag->iv[j] = (uint8_t) *tag_iv_str++, j++;


                    /**
                     *  Parse the KBAG Key. This is the same process as with the IV. The only difference
                     *  between the two is that the key is 32 bytes and the IV is only 16 bytes.
                     */
                    asn1_tag_t *tag_key = asn1_element_at_index (seq, 2);
                    asn1_elem_len_t tag_key_len = asn1_len ((char *) tag_key + 1);
                    
                    j = 0;
                    unsigned char *tag_key_str = (unsigned char *) tag_key + tag_key_len.size_bytes + 1;
                    while (tag_key_len.data_len--) kbag->key[j] = (uint8_t) *tag_key_str++, j++;

                } else {
                    /* we didn't expect this tag here */
                    warningf ("image4_parse_im4p: skipping unexpected tag.\n");
                }
            }

            /* Add the KBAG to the im4p */
            im4p->kbags = h_slist_append (im4p->kbags, kbag);
        }

    } else {
        // no kbag
    }

    return im4p;
}

//////////////////////////////////////////////////////////

image4_type_t
image4_get_file_type (image4_t *image4)
{
    char 	*magic;
    size_t 	 l;

    unsigned char *buffer = malloc (image4->size);
    memcpy (buffer, image4->data, image4->size);

    /**
     *	Get the Image4 magic sequence, and then compare that to the known
     *  types. Return the correct image4_type_t.
     */
    asn1_get_sequence_name(image4->data, &magic, &l);

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
image4_get_file_type_string (image4_type_t type)
{
    if (type == IMAGE4_TYPE_IMG4) return "IMG4";
    else if (type == IMAGE4_TYPE_IM4P) return "IM4P";
    else if (type == IMAGE4_TYPE_IM4M) return "IM4M";
    else if (type == IMAGE4_TYPE_IM4R) return "IM4R";
    else return "UNKNOWN";
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

char *
img4_get_component_name (image4_t *image4)
{
	char *magic;
	size_t l;

	char *raw = asn1_element_at_index (image4->data, 1) + 2;

	if (!strncmp (raw, "ibot", 4)) {
		return "iBoot";
	} else if (!strncmp (raw, IMAGE_TYPE_IBEC, 4)) {
		return "iBoot Epoch Change (iBEC)";
	} else if (!strncmp (raw, IMAGE_TYPE_IBSS, 4)) {
		return "iBoot Single Stage (iBSS)";
	} else if (!strncmp (raw, IMAGE_TYPE_LLB, 4)) {
		return "Low Level Bootloader (LLB)";
	} else if (!strncmp (raw, IMAGE_TYPE_SEP_OS, 4)) {
		return "Secure Enclave OS (SEP OS)";
	} else if (!strncmp (raw, IMAGE_TYPE_SEP_OS_RESTORE, 4)) {
		return "Secure Enclave OS Restore (SEP OS RESTORE)";
	} else if (!strncmp (raw, IMAGE_TYPE_DEVTREE, 4)) {
		return "Device Tree";
	} else if (!strncmp (raw, IMAGE_TYPE_RAMDISK, 4)) {
		return "Update/Restore Ramdisk";
	} else if (!strncmp (raw, IMAGE_TYPE_KERNELCACHE, 4)) {
		return "Darwin Kernel Cache";
	} else if (!strncmp (raw, IMAGE_TYPE_LOGO, 4)) {
		return "Boot Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_RECMODE, 4)) {
		return "Recovery Mode Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_NEEDSERVICE, 4)) {
		return "Need Service Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_GLYPHCHRG, 4)) {
		return "Glyph Charge Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_GLYPHPLUGIN, 4)) {
		return "Glyph Plugin Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYCHARGING0, 4)) {
		return "Battery Charging 0 Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYCHARGING1, 4)) {
		return "Battery Charging 1 Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYLOW0, 4)) {
		return "Battery Charging Low 0 Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYLOW1, 4)) {
		return "Battery Charging Low 1 Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYFULL, 4)) {
		return "Battery Full Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_OS_RESTORE, 4)) {
		return "iOS Restore Image";
	} else if (!strncmp (raw, IMAGE_TYPE_HAMMER, 4)) {
		return "Hammer";
	} else {
		return "Unknown Component";
	}
}

int 
img4_check_compression_type (char *buf)
{
        /* Get the element count and ensure buf is an im4p */
        int c = asn1_elements_in_object (buf);
        if (c < 4) {
                printf ("[Error] Not enough elements in given payload\n");
                exit (0);
        }

        /* Try to select the payload tag from the buffer */
        char *tag = asn1_element_at_index (buf, 3) + 1;
        asn1_elem_len_t len = asn1_len (tag);
        char *data = tag + len.size_bytes;

        /* Check for either lzss or bvx2/lzfse */
        if (!strncmp (data, "complzss", 8)) {
                return IMAGE4_FILE_COMPRESSED_LZSS;
        } else if (!strncmp (data, "bvx2", 4)) {
                return IMAGE4_FILE_COMPRESSED_BVX2;
        } else {
                return IMAGE4_FILE_COMPRESSED_NONE;
        }
}









