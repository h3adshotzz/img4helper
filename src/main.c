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
#include <libhelper-image4.h>

#include "img4helper.h"
#include "extract.h"
//#include "image4.h"
#include "print.h"

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
    /* Check that the image was loaded properly */
    if (!image4->size || !image4->data) {
        errorf ("There was an issue loaded the specified Image4: %s\n", image4->path);
        return -1;
    }

    /*  */




    debugf("img4helper_extract\n");
    int res = extract_payload_from_image (image4, client);
}


///////////////////////////////////////////////////////////////////////////////


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
        print_image4 (image4, client);
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

















