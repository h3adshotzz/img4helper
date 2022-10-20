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

#include <stdio.h>
#include <string.h>

/* libhelper */
#include <libhelper.h>
#include <libhelper-logger.h>
#include <libhelper-file.h>
#include <libhelper-image4.h>

#include "img4helper.h"
#include "extract.h"

#include <libhelper-lzfse.h>
#include <CommonCrypto/CommonCrypto.h>>

image4_t *decompress_bvx2 (image4_t *img)
{
    char *tag = asn1_element_at_index (img->data, 3) + 1;
    asn1_elem_len_t len = asn1_len (tag);
    char *data = tag + len.size_bytes;

    char *comp_tag = data + len.data_len;
    char *fake_comp_size_tag = asn1_element_at_index (comp_tag, 0);
    char *uncomp_size_tag = asn1_element_at_index (comp_tag, 1);

    size_t fake_src_size = asn1_get_number_from_tag ((asn1_tag_t *) fake_comp_size_tag);
    size_t dst_size = asn1_get_number_from_tag ((asn1_tag_t *) uncomp_size_tag);

    size_t src_size = len.data_len;

    if (fake_src_size != 1) {
        printf ("error: fake_src_size not 1 but: 0x%zx\n", fake_src_size);
    }
    img->data = calloc (1, dst_size);
    size_t decomp_size = lzfse_decode_buffer ((uint8_t *) img->data, dst_size, (uint8_t *) data, src_size, NULL);

    if (img->data != dst_size) {
        printf("img->decomp_size != dst_size: %zu, %zu\n", dst_size, decomp_size);
        exit(1);
    }
    img->size = dst_size;
    img->flags |= IMAGE4_FLAG_MODIFIED_DATA;

    return img;
}

/* Extract the IM4P payload from a given image */
int extract_payload_from_image (image4_t *image4, img4helper_client_t *client)
{
    /* Check if the payload is encrypted */
    if (image4->im4p->flags & IM4P_FLAG_FILE_ENCRYPTED) {
        debugf ("payload is encrypted\n");

        /* Verify that the key and iv are set by CLI args */
        if ((client->key == NULL) || (client->iv == NULL)) {
            warningf ("Extracting payload without decrypting.\n");
        } else {
            /* decrypt */
        }
    } else {
        debugf ("payload is not encrypted\n");
    }

    /* Check if the payload is compressed */
    if (image4->im4p->flags & IM4P_FLAG_FILE_COMPRESSED_BVX2) {
        debugf("image_compressed_bvx2\n");
    } else if (image4->im4p->flags & IM4P_FLAG_FILE_COMPRESSED_LZSS) {
        debugf("image_compressed_lzss\n");
    } else if (image4->im4p->flags & IM4P_FLAG_FILE_COMPRESSED_NONE) {
        debugf("image_compressed_none\n");
    } else {
        warningf("image_compressed_unknown\n");
    }

    debugf ("extracting...\n");
    debugf ("image: 0x%08x\n", image4->data);
    image4_t *test = decompress_bvx2 (image4);

    printf("compressed: %d bytes, decompressed: %d bytes.\n", image4->size, test->size);
    
    FILE *fp = fopen("test.bin", "w+");
    fwrite(test->data, test->size, 1, fp);
    fclose(fp);

    return 1;
}

int extract_im4p_payload (image4_t *image4, img4helper_client_t *client)
{

    /* Check the image is valid */
    if (!image4->size || !image4->data) {
        errorf ("Invalid Image4 file: %s\n", client->filename);
        return -1;
    }

    /* print some image info here */


    /* Start decompression */
    printf ("[*] Detecting compression type...");



}
