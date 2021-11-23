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

#ifndef __IMG4HELPER_IMAGE4_H__
#define __IMG4HELPER_IMAGE4_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Determine which library to use for cryptographic operations. If we are on
 *  macOS, use CommonCrypto. Otherwise we should use OpenSSL.
 */
#if defined(__APPLE__) && defined(__MACH__)
#   define IMG4HELPER_CRYPTO_LIB_COMMONCRYPTO
#   include <CommonCrypto/CommonCrypto.h>
#elif
#   define IMG4HELPER_CRYPTO_LIB_OPENSSL
#   include <openssl/aes.h>
#   include <openssl/sha.h>
#endif

#include <libhelper-hlibc.h>
#include "asn1.h"


/**
 *  Image4 component types.
 */
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

/**
 *  \brief      Image 4 component types.
 */
typedef enum __img4helper_image4_type     image4_type_t;
enum __img4helper_image4_type {
    IMAGE4_TYPE_UNKNOWN,
    IMAGE4_TYPE_IMG4,
    IMAGE4_TYPE_IM4P,
    IMAGE4_TYPE_IM4M,
    IMAGE4_TYPE_IM4R
};

/**
 *  \brief      KBAG types.
 */
typedef enum __img4helper_kbag_type         kbag_type_t;
enum __img4helper_kbag_type {
    IMAGE4_KBAG_TYPE_PRODUCTION,
    IMAGE4_KBAG_TYPE_DEVELOPMENT,
    IMAGE4_KBAG_TYPE_UNKNOWN
};


/**
 *  IM4P file property flags.
 */
#define IMAGE4_FILE_ENCRYPTED           (1 << 1)
#define IMAGE4_FILE_NOT_ENCRYPTED       (1 << 2)
#define IMAGE4_FILE_COMPRESSED_LZSS     (1 << 3)
#define IMAGE4_FILE_COMPRESSED_BVX2     (1 << 4)
#define IMAGE4_FILE_COMPRESSED_NONE     (1 << 5)

/**
 * Image4 property flags.
 */
#define IMAGE4_FILE_INCLUDES_IM4P       (1 << 15)
#define IMAGE4_FILE_INCLUDES_IM4M       (1 << 16)
#define IMAGE4_FILE_INCLUDES_IM4R       (1 << 17)
#define IMAGE4_FILE_SINGLE_COMP         (1 << 18)


/**
 *
 */
typedef struct __img4helper_kbag            kbag_t;
struct __img4helper_kbag
{
    kbag_type_t      type;          /* PRODUCTION, DEVLOPMENT or UNKNOWN */
    uint8_t          iv[16];        /* KBAG IV ASN1 tag */
    uint8_t          key[32];       /* KBAG KEY ASN1 tag */
};

/**
 *
 */
typedef struct __img4helper_image4_im4p     im4p_t;
struct __img4helper_image4_im4p 
{
    uint32_t         offset;
    uint32_t         size;
    char            *comp;
    char            *desc;

    uint32_t         flags;
    HSList          *kbags;      /* collection of KBAGS in header */
};

/**
 *  \brief      Parsed Image4 file structure...
 */
typedef struct __img4helper_image4          image4_t;
struct __img4helper_image4 
{
    /* File properties */
    unsigned char   *data;      /* raw file data */
    uint32_t         size;      /* size of file */
    char            *path;      /* filepath */

    /* Flags */
    uint32_t         flags;     /* flags */

    /* Image4 properties */
    im4p_t          *im4p;      /* im4p (if included) */
    //im4m_t          *im4m;      /* im4m (if included) */
    //im4r_t          *im4r;      /* im4r (if included) */

    image4_type_t    type;      /* image4 type */
};

/**
 *  \brief
 */
image4_t *
image4_create ();

/**
 *  \brief
 */
image4_t *
image4_load_file (const char *path);

/**
 *  \brief
 */
image4_t *
image4_load (const char *path);

/**
 *  \brief
 */
image4_type_t
image4_get_file_type (image4_t *image4);

/**
 *  \brief
 */
char *
image4_get_file_type_string (image4_type_t type);

/**
 *  \brief
 **/
char *
img4_get_component_name (image4_t *image4);

/**
 *  \brief
 */
char *
image4_get_component_type (image4_t *image4);

//////

/**
 *  \brief
 */
im4p_t *
image4_parse_im4p (unsigned char *buf);


#ifdef __cplusplus
}
#endif

#endif /* __img4helper_image4_h__ */
