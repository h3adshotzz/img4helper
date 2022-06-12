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
#include <getopt.h>

#include <libhelper.h>
#include <libhelper-logger.h>

#include "img4helper.h"
#include "extract.h"
#include "image4.h"

/* control debugging code */
#define IMG4HELPER_DEBUG 1

/**
 *  Img4helper Option definitions.
 */
static struct option standardopts[] = {
    { "help",           no_argument,            NULL,   'h' },
    { "version",        no_argument,            NULL,   'v' },

    { "print-all",      no_argument,            NULL,   'a' },
    { "print-im4p",     no_argument,            NULL,   'p' },
    { "print-im4m",     no_argument,            NULL,   'm' },
    { "print-im4r",     no_argument,            NULL,   'r' },

    { "extract",        no_argument,            NULL,   'e' },
    { "outfile",        required_argument,      NULL,   'o' },

    { "iv",             required_argument,      NULL,   'I' },
    { "key",            required_argument,      NULL,   'K' },

    { "test",           no_argument,            NULL,   't' },

    { NULL,             0,                      NULL,   0   }
};

/* htool usage output */
static void general_usage (int argc, char *argv[], int err, int ex)
{
    char *name = strchr (argv[0], '/');
    fprintf ((err) ? stderr : stdout,
        "Usage: %s [OPTIONS] PATH\n" \
        "\n" \
        "Application Options:\n" \
        "  -h, --help           Print usage for Img4helper.\n" \
        "  -v, --version        Print version for Img4helper.\n" \
        "\n"\
        "Image4 Options:\n" \
        "  -a, --print-all      Print everything from an Image4.\n" \
        "  -p, --print-im4p     Print the IM4P Payload.\n" \
        "  -m, --print-im4m     Print the IM4M Manifest.\n" \
        "  -r, --print-im4r     Print the IM4R Restore Info Nonce.\n" \
        "\n"
        "Extract/Decrypt/Decompress:\n" \
        "  -e, --extract        Extract a payload from an IMG4 or IM4P.\n" \
        "  -o, --outfile        Specify output path/name for extracted payload.\n" \
        "  -I, --iv             Specify IV for decrypting payload. (use with -e).\n" \
        "  -K, --key            Specify Key for decrypting payload. (use with -e).\n" \

        "\n",
        (name ? name + 1 : argv[0]));

    exit(ex);
}

///////////////////////////////////////////////////////////////////////////////

int img4helper_extract (image4_t *image4, img4helper_client_t *client)
{
    debugf("img4helper_extract\n");
    int res = extract_payload_from_image (image4, client);
}

///////////////////////////////////////////////////////////////////////////////

/* TODO: Move to print.c */

static int img4helper_print_payload_encryption_compression (image4_t *image4)
{
    /* if the image is encrypted, we can't tell if it's compressed yet */
    if (image4->im4p->flags & IMAGE4_FILE_ENCRYPTED) {
        hlog_print_list_item ("     ", "Payload", "Encrypted");
    } else {
        if (image4->im4p->flags & IMAGE4_FILE_COMPRESSED_LZSS)
            hlog_print_list_item ("     ", "Payload", "LZSS");
        else if (image4->im4p->flags & IMAGE4_FILE_COMPRESSED_BVX2)
            hlog_print_list_item ("     ", "Payload", "BVX2");
        else
            hlog_print_list_item ("     ", "Payload", "Compressed");
    }
    return 1; 
}

int img4helper_print (image4_t *image, img4helper_client_t *client);
int img4helper_print_all (image4_t *image4);
int img4helper_print_im4p (im4p_t *im4p, char *indent);

int img4helper_print (image4_t *image4, img4helper_client_t *client)
{
    /* print the filename, type and component name */
    hlog_print_list_header ("Image4 Contents");
    hlog_print_list_item ("      ", "Loaded", image4->path);
    hlog_print_list_item (" ", "Image4 Type", image4_get_file_type_string (image4->type));
    hlog_print_list_item ("   ", "Component", img4_get_component_name (image4));

    /* now, print based on the type requested */
    if (client->flags & FLAG_IMG4_PRINT_ALL) {
        img4helper_print_all (image4);
    } else if (client->flags & FLAG_IMG4_PRINT_IM4P) {

        img4helper_print_payload_encryption_compression (image4);
        int ret = img4helper_print_im4p (image4->im4p, "\t");

        /* if there was an issue, 'ret' should be 0. */
        if (!ret) errorf ("Could not print IM4P from file: %s\n", client->filename);

    }
}


int img4helper_print_all (image4_t *image4)
{  
    /* print the compression & encryption status */
    if (image4->type == IMAGE4_TYPE_IMG4 && (image4->flags & IMAGE4_FILE_INCLUDES_IM4P) ||
        image4->type == IMAGE4_TYPE_IM4P)
        img4helper_print_payload_encryption_compression (image4);

    if (image4->type == IMAGE4_TYPE_IMG4) {
    
        /**
         *  Print im4p, im4m and im4r. If an item doesn't exist, print a simple message
         *  to say that the .img4 doesn't contain it.
         */

    } else if (image4->type == IMAGE4_TYPE_IM4P) {

        // print im4p data and kBAGs
        img4helper_print_im4p (image4->im4p, "\t");
    }

    return 1;
}

int img4helper_print_im4p (im4p_t *im4p, char *indent)
{
    /* verify im4p is valid */
    if (im4p == NULL)
        return 0;

    /* print IM4P banner */
    hlog_print_list_subheader ("  ", "IM4P: -----");

    /* print the type, descriptor and size */
    hlog_print_list_item (indent, "Type", im4p->comp);
    hlog_print_list_item (indent, "Desc", im4p->desc);
    hlog_print_list_item (indent, "Size", "0x%08x", im4p->size);

    /* Check if there are KBAGs to print */
    int kbag_count = h_slist_length (im4p->kbags);
    if (kbag_count) {

        printf (ANSI_COLOR_YELLOW "%sKBAGs:\n" ANSI_COLOR_RESET, indent);
        for (int i = 0; i < kbag_count; i++) {
            kbag_t *kbag = (kbag_t *) h_slist_nth_data (im4p->kbags, i);
            char *kbag_type;

            if (kbag->type == IMAGE4_KBAG_TYPE_PRODUCTION) kbag_type = "Production";
            else if (kbag->type == IMAGE4_KBAG_TYPE_DEVELOPMENT) kbag_type = "Development";
            else kbag_type = "Unknown:";

            printf (ANSI_COLOR_BOLD ANSI_COLOR_DARK_WHITE "%s%s:\t" ANSI_COLOR_RESET ANSI_COLOR_DARK_GREY, indent, kbag_type);
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

///////////////////////////////////////////////////////////////////////////////

/* print detailed version/build info */
void print_version_detail (int opt)
{
    if (opt == 1) {
        printf (BOLD "Copyright (C) Is This On? Holdings: Img4helperl version %s (%s)\n\n" RESET, IMG4HELPER_VERSION_NUMBER, libhelper_get_version_string ());
        
        printf (BOLD RED "  Debug Information:\n" RESET);
        printf (BOLD DARK_WHITE "      Build time:  " RESET DARK_GREY "%s\n", __TIMESTAMP__);
        printf (BOLD DARK_WHITE "      Build type:  " RESET DARK_GREY "%s\n", IMG4HELPER_VERSION_TAG);
        
        printf (BOLD DARK_WHITE "  Default Target:  " RESET DARK_GREY "%s-%s\n", BUILD_TARGET, BUILD_ARCH);
        printf (BOLD DARK_WHITE "        Platform:  " RESET DARK_GREY);
#if IMG4HELPER_MACOS_PLATFORM_TYPE == IMG4HELPER_PLATFORM_TYPE_APPLE_SILICON
        printf ("apple-silicon (Apple Silicon)\n" RESET);
#else   
        printf ("intel-genuine (Intel Genuine)\n" RESET);
#endif
    } else {
        printf ("-----------------------------------------------------\n");
        printf ("  Img4helper %s - Built " __TIMESTAMP__ "\n", IMG4HELPER_VERSION_NUMBER);
        printf ("-----------------------------------------------------\n");
    }
}


/* main */
int main(int argc, char *argv[])
{
    printf ("argc: %d\n", argc);
    for (int i = 0; i < argc; i++)
        printf ("argv[%d]: %s\n", i, argv[i]);

    /* always print version info */
    print_version_detail (0);

    /* create new client */
    img4helper_client_t *client = calloc (1, sizeof (img4helper_client_t));

    /* parse the options */
    int opt = 0;
    int optindex = 0;
    while ((opt = getopt_long (argc, argv, "hvapmreo:I:K:t", standardopts, &optindex)) > 0) {
        switch (opt) {
            
            /**
             *  Application Options
             */

            /* -h, --help */
            case 'h':
                general_usage (argc, argv, 0, EXIT_SUCCESS);
                break;

            /* -v, --version */
            case 'v':
                print_version_detail (1);
                return EXIT_SUCCESS;

            /**
             *  Image4 Options
             */

            /* -a, --print-all */
            case 'a':
                client->flags |= FLAG_IMG4_PRINT_ALL;
                break;

            /* -p, --print-im4p */
            case 'p':
                client->flags |= FLAG_IMG4_PRINT_IM4P;
                break;

            /* -m, --print-im4m */
            case 'm':
                client->flags |= FLAG_IMG4_PRINT_IM4M;
                break;

            /* -r, --print-im4r */
            case 'r':
                client->flags |= FLAG_IMG4_PRINT_IM4R;
                break;

            /**
             *  Extract/Decrypt/Decompress Options
             */

            /* -e, --extract */
            case 'e':
                client->flags |= FLAG_IMG4_EXTRACT_PAYLOAD;
                break;

            /* -o, --outfile */
            case 'o':
                client->flags |= FLAG_OUTFILE_SET;
                client->outfile = strdup ((const char *) optarg);
                break;

            /* -I, --iv */
            case 'I':
                client->flags |= FLAG_DECRYPT_IV_SET;
                client->iv = strdup ((const char *) optarg);
                break;

            /* -K, --key */
            case 'K':
                client->flags |= FLAG_DECRYPT_KEY_SET;
                client->key = strdup ((const char *) optarg);
                break;
            
            default:
                general_usage (argc, argv, 0, EXIT_FAILURE);
                break;
        }
    }

    /* check the input was valid */
    if (!client->flags) general_usage (argc, argv, 0, EXIT_FAILURE);

    /* set the file name */
    if (argc - optind == 1) {
        argc -= optind;
        argv += optind;
        client->filename = strdup (argv[0]);
    }

    /* verify the filename */
    if (!client->filename) {
        errorf ("Error: Invalid filename.\n");
        general_usage (argc, argv, 0, EXIT_FAILURE);
    }

    debugf ("flags: 0x%08x, filename: %s\n", client->flags, client->filename);

    /* create and load the file */
    image4_t *image4 = image4_load (client->filename);
    if (!image4) {
        errorf ("Error: could nto load bianry from filepath: %s.\n", client->filename);
        return EXIT_FAILURE;
    }

    /**
     *  Option:         -a, -p, -m, -r. --print-<all/im4p/im4m/im4r>
     *  Description:    Print out information from the Image4 file.
     */
    if (client->flags & FLAG_IMG4_PRINT_ALL ||
        client->flags & FLAG_IMG4_PRINT_IM4M ||
        client->flags & FLAG_IMG4_PRINT_IM4P ||
        client->flags & FLAG_IMG4_PRINT_IM4R) {

        /* this will check the client->flags again to determine what to print */
        img4helper_print (image4, client);
    }

    /**
     *  Option:         -e, --extract
     *  Description:    Extract the payload from the Image4 file.
     */
    if (client->flags & FLAG_IMG4_EXTRACT_PAYLOAD) {

        /**
         * Extract the IM4P payload. If the payload is encrypted, but --key and --iv
         * were not set, an error will be printed.
         */
        img4helper_extract (image4, client);
    }


    return 0;
}

















