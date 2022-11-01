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
#include "print.h"

#include <libhelper-lzfse.h>
#include <CommonCrypto/CommonCrypto.h>>


// TODO: create a fetch_im4p_payload_bytes func.

/**
 *  Fetch the im4p payload from a given image4_t, whether it's an
 *  IMG4 or IM4P.
 */
unsigned char *im4p_fetch_payload (image4_t *image4)
{
    unsigned char *data = image4->data;

    /**
     *  If the image is an IMG4, rather than an im4p, then set 'data' to
     *  the 2nd element in the ASN1.
     */
    if (image4->type == IMAGE4_COMP_TYPE_IMG4)
        data = asn1_element_at_index (image4->data, 1);

    /**
     *  Now, we have the either `data` pointing to the IM4P, or image4->data
     *  already points to it.
     */
    char *tag = asn1_element_at_index (data, 3) + 1;
    //asn1_elem_len_t len = asn1_len (tag);
    //data = tag + len.size_bytes;
    //debugf ("len.size_bytes: %d\n", len.size_bytes);
    
    return tag;
}


unsigned char *decompress_bvx2_payload_bytes (unsigned char *payload, size_t dst_size, size_t src_size, size_t fake_src_size)
{
    /* Verify that fake_src_size is 1 */
    if (fake_src_size != 1) {
        errorf ("decompress_bvx2_payload_bytes: fake_src_size: expected 1, got %d\n", fake_src_size);
        return NULL;
    }

    char *decompressed = calloc (1, dst_size);
    size_t decomp_size = lzfse_decode_buffer ((uint8_t *) decompressed, dst_size,
                                              (uint8_t *) payload, src_size,
                                              NULL);

    return decompressed;
}

unsigned char *decompress_lzss_payload_bytes ()
{
    return NULL;
}

unsigned char *decrypt_payload_bytes (unsigned char *payload, char *_iv, char *_key, size_t src_size, size_t img_size)
{
    /* Calculate a size that is a multiple of the block size */
    size_t tst = 0;
    if (img_size % BLOCK_SIZE) tst = img_size + (BLOCK_SIZE - (img_size % BLOCK_SIZE)) + BLOCK_SIZE;

    /* Create an uint8_t array for both key and iv of the correct size */
	uint8_t key[32] = { };
	uint8_t iv[16] = { };

	/* Copy the hex data for the IV */
	for (int i = 0; i < (int) sizeof (iv); i++) {
		unsigned int t;
		sscanf (_iv+i*2,"%02x",&t);
		iv[i] = t;
	}
	for (int i = 0; i < (int) sizeof (key); i++) {
		unsigned int t;
		sscanf (_key+i*2,"%02x",&t);
		key[i] = t;
	}

    /* Use CommonCrypto's decryption function */
    unsigned char *decrypted_data = calloc (1, src_size);
    CCCryptorStatus dec = CCCrypt (kCCDecrypt, kCCAlgorithmAES, 0, key, sizeof(key), iv, payload, src_size, decrypted_data, tst, NULL);

    return decrypted_data;
}


/* Extract the IM4P payload from a given image */
int img4helper_extract (image4_t *image4, img4helper_client_t *client)
{
    /* Print the image filename, type and component name */
    hlog_print_list_header ("Image4 Contents");
    hlog_print_list_item ("      ", "Loaded", image4->path);
    hlog_print_list_item (" ", "Image4 Type", image4_get_file_type_description (image4));
    hlog_print_list_item ("   ", "Component", image4_get_component_type_name (image4));
    hlog_print_list_item (" ", "Compression", image4_get_compression_description (image4));
    printf ("\n");

    printf (ANSI_COLOR_GREEN "[*] Preparing payload\n" ANSI_COLOR_RESET);

    /* This contains the ASN1 tag at the top */
    unsigned char *im4p_payload = im4p_fetch_payload (image4);

    /* This is the raw payload bytes */
    asn1_elem_len_t len = asn1_len (im4p_payload);
    unsigned char *payload_bytes = im4p_payload + len.size_bytes;
    size_t payload_size = len.data_len;

    /* Check if the payload is encrypted */
    if (image4->im4p->flags & IM4P_FLAG_FILE_ENCRYPTED) {

        /* Verify that the key and iv are set by CLI args */
        if ((client->key == NULL) || (client->iv == NULL)) {
            
            /* If a key and iv isn't set, then we just write the encrypted payload to a file */
            printf (ANSI_COLOR_GREEN "[*] No --iv or --key provided, writing encrypted payload to: %s\n" ANSI_COLOR_RESET, client->outfile);

            FILE *fp = fopen(client->outfile, "w+");
            fwrite(payload_bytes, payload_size, 1, fp);
            fclose(fp);

            return 1;

        } else {
            
            printf (ANSI_COLOR_GREEN "[*] IV:  " ANSI_COLOR_BOLD ANSI_COLOR_DARK_GREY "%s\n" ANSI_COLOR_RESET, client->iv);
            printf (ANSI_COLOR_GREEN "[*] Key: " ANSI_COLOR_BOLD ANSI_COLOR_DARK_GREY "%s\n" ANSI_COLOR_RESET, client->key);
            printf (ANSI_COLOR_GREEN "[*] Decrypting payload...\n" ANSI_COLOR_RESET);

            payload_bytes = decrypt_payload_bytes (payload_bytes, client->iv, client->key, payload_size, image4->size);

            if (payload_bytes == NULL) errorf ("There was an issue decrypting the payload\n");
        }
    }

    /* Handle any compression on the im4p_payload (that should now be decompressed) */
    if (image4->im4p->flags & IM4P_FLAG_FILE_COMPRESSED_BVX2) {

        printf (ANSI_COLOR_GREEN "[*] Decompressing bvx2 payload..." ANSI_COLOR_RESET);

        /* Calculate the decompressed payload size */
        asn1_tag_t *comp_info_tag = payload_bytes + len.data_len;

        size_t fake_src_size = asn1_get_number_from_tag ((asn1_tag_t *) asn1_element_at_index (comp_info_tag, 0));
        size_t dst_size = asn1_get_number_from_tag ((asn1_tag_t *) asn1_element_at_index (comp_info_tag, 1));
        size_t src_size = len.data_len;

        printf ("fake_src_size: %d\ndst_size: %d\nsrc_size: %d\nlen.data_len: %d\n",
                fake_src_size, dst_size, src_size, len.data_len);

        payload_bytes = decompress_bvx2_payload_bytes (payload_bytes, dst_size, src_size, fake_src_size);
        payload_size = dst_size;

        if (payload_bytes == NULL && payload_size == 0) errorf ("There was an issue decompressing the payload\n");

    }
  
    FILE *fp = fopen(client->outfile, "w+");
    fwrite(payload_bytes, payload_size, 1, fp);
    fclose(fp);

    printf (ANSI_COLOR_GREEN "[*] Extracted payload written to: %s\n" ANSI_COLOR_RESET, client->outfile);

    return 1;
}
