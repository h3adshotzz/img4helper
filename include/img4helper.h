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

#ifndef __IMG4HELPER_MAIN_H__
#define __IMG4HELPER_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "version.h"
#include "image4.h"

/**
 *
 */
typedef struct __img4helper_client          img4helper_client_t;
struct __img4helper_client
{
    /* file info */
    char        *filename;          /* filename to load */
    char        *outfile;           /* path/file to write extracted file */
    
    /* encryption keys */
    char        *iv;
    char        *key;

    /* flags */
    uint32_t     flags;

    /* Image4 */
    image4_t    *image4;
};


/**
 *  General flags for Img4helper client. These are set when parsing command line
 *  args given when running Img4helper. To check if a flag is set, we can use the
 *  function:
 *      
 *      img4helper_client_check_flags (client_flags, mask);
 */
#define FLAG_HELP                   (1 << 1)
#define FLAG_VERSION                (1 << 2)
#define FLAG_OUTFILE_SET            (1 << 3)
#define FLAG_IMG4_PRINT_ALL         (1 << 4)
#define FLAG_IMG4_PRINT_IM4P        (1 << 5)
#define FLAG_IMG4_PRINT_IM4M        (1 << 6)
#define FLAG_IMG4_PRINT_IM4R        (1 << 7)
#define FLAG_IMG4_EXTRACT_PAYLOAD   (1 << 8)

#define FLAG_DECRYPT_IV_SET         (1 << 9)
#define FLAG_DECRYPT_KEY_SET        (1 << 10)



/**
 *  These are some colour definitions to allow nicer looking output
 *  by img4helper.
 */
#define WHITE           "\x1b[38;5;254m"
#define DARK_WHITE      "\x1b[38;5;251m"
#define DARK_GREY       "\x1b[38;5;243m"
#define YELLOW          "\x1b[38;5;214m"
#define DARK_YELLOW     "\x1b[38;5;94m"
#define RED             "\x1b[38;5;88m"
#define BLUE            "\x1b[38;5;32m"
#define RESET           "\x1b[0m"
#define BOLD            "\x1b[1m"

/**
 *  Exit codes
 */
#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1


#ifdef __cplusplus
}
#endif

#endif /* __img4helper_main_h__ */


