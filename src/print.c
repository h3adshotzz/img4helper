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

#include <libhelper.h>
#include <libhelper-logger.h>

#include "img4helper.h"
#include "print.h"

/* print detailed version info of img4helper */
void print_version_detail (int opt)
{
		if (opt == 1) {
				printf (BOLD "Copyright (C) Is This On? Holdings Ltd: Img4helper version %s (%s)\n\n",
						RESET IMG4HELPER_VERSION_NUMBER, libhelper_get_version_string());

				printf (BOLD RED " Debug Information:\n" RESET);
				printf (BOLD DARK_WHITE "      Build Time:  " RESET DARK_GREY "%s\n", __TIMESTAMP__);
				printf (BOLD DARK_WHITE "      Build Type:  " RESET DARK_GREY "%s\n", IMG4HELPER_VERSION_TAG);

				printf (BOLD DARK_WHITE "  Default Target:  " RESET DARK_GREY "%s-%s\n", BUILD_TARGET, BUILD_ARCH);
				printf (BOLD DARK_WHITE "        Platform:  " RESET DARK_GREY);

#if IMG4HELPER_MACOS_PLATFORM_TYPE == IMG4HELPER_PLATFORM_TYPE_APPLE_SILICON
				printf ("apple-silicon-aarch64 (Apple Silicon)");
#else
				printf ("intel-genuine-x86 (Intel Genuine)");
#endif
		} else {
				printf ("-----------------------------------------------------\n");
				printf ("   Img4helper %s - Built " __TIMESTAMP__ "\n", IMG4HELPER_VERSION_NUMBER);
				printf ("-----------------------------------------------------\n");
		}
}


static int print_payload_encryption_compression (image4_t *image4)
{
    if (image4->im4p->flags & IM4P_FLAG_FILE_ENCRYPTED) {
        hlog_print_list_item ("     ", "Payload", "Encrypted");
    } else {
        if (image4->im4p->flags & IM4P_FLAG_FILE_COMPRESSED_LZSS)
            hlog_print_list_item ("     ", "Payload", "LZSS-Compressed");
        else if (image4->im4p->flags & IM4P_FLAG_FILE_COMPRESSED_BVX2)
            hlog_print_list_item ("     ", "Payload", "BVX2-Compressed");
        else
            hlog_print_list_item ("     ", "Payload", "Unknown-Compressed");
    }
    return 1;
}


int print_image4 (image4_t *image4, img4helper_client_t *client)
{
    /* Print the image filename, type and component name */
    hlog_print_list_header ("Image4 Contents");
    hlog_print_list_item ("      ", "Loaded", image4->path);
    hlog_print_list_item (" ", "Image4 Type", image4_get_file_type_description (image4));
    hlog_print_list_item ("   ", "Component", image4_get_component_type_name (image4));

    debugf ("client->flags: 0x%08x\n", client->flags);

    /* Print further info based on the CLI flags */
    if (client->flags & FLAG_IMG4_PRINT_ALL) {

        /* print the compression and encryption status */
        if (image4->flags & IMAGE4_FLAG_INCLUDES_IM4P || image4->type == IMAGE4_COMP_TYPE_IM4P)
            print_payload_encryption_compression (image4);

        if (image4->type == IMAGE4_COMP_TYPE_IM4P || image4->type == IMAGE4_COMP_TYPE_IMG4)
            warningf ("IM4P parsing not yet implemented\n");
            print_header_im4p (image4->im4p, "\t");

        if (image4->type == IMAGE4_COMP_TYPE_IM4M || image4->type == IMAGE4_COMP_TYPE_IMG4)
            warningf ("IM4M parsing not yet implemented\n");

        if (image4->type == IMAGE4_COMP_TYPE_IM4R || image4->type == IMAGE4_COMP_TYPE_IMG4)
            warningf ("IM4R parsing not yet implemented\n");


    } else if (client->flags & FLAG_IMG4_PRINT_IM4P) {

        print_payload_encryption_compression (image4);
        int ret = print_header_im4p (image4->im4p, "\t");

        /* if there was an issue, 'ret' should be 0 */
        if (!ret) errorf ("Could not print IM4P from file: %s\n", client->filename);
    }
}

/**
 * This prints out the properties of an IM4P
 */
int print_header_im4p (im4p_t *im4p, char *indent)
{
    /* verify im4p is valid */
    if (im4p == NULL)
        return 0;

    /* print IM4P banner */
    printf ("\n");
    hlog_print_list_subheader ("  ", "IM4P: -----");

    /* print the type, descriptor and size */
    hlog_print_list_item (indent, "Type", im4p->comp);
    hlog_print_list_item (indent, "Desc", im4p->desc);
    hlog_print_list_item (indent, "Size", "0x%08x", im4p->size);

    /* Check if there are KBAGs to print */
    int kbag_count = h_slist_length (im4p->kbags);
    if (kbag_count) {

        printf (ANSI_COLOR_YELLOW "\n%sKBAGs:\n" ANSI_COLOR_RESET, indent);
        for (int i = 0; i < kbag_count; i++) {
            kbag_t *kbag = (kbag_t *) h_slist_nth_data (im4p->kbags, i);
            char *kbag_type;

            if (kbag->type == IMAGE4_KBAG_PRODUCTION) kbag_type = "Production";
            else if (kbag->type == IMAGE4_KBAG_DEVELOPMENT) kbag_type = "Development";
            else kbag_type = "Unknown:";

            printf (ANSI_COLOR_BOLD ANSI_COLOR_DARK_WHITE "%s  %s:\t" ANSI_COLOR_RESET ANSI_COLOR_DARK_GREY, indent, kbag_type);

            for (int j = 0; j < 16; j++) printf ("%02X", kbag->iv[j]);
            for (int j = 0; j < 32; j++) printf ("%02X", kbag->key[j]);

            printf (ANSI_COLOR_RESET "\n");

        }

    } else {
        printf (ANSI_COLOR_BLUE "%sNo KBAGs present.\n" ANSI_COLOR_RESET, indent);
    }

    printf ("\n");
    return 1;
}
