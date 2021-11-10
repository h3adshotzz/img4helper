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

/**
 *  This file is a small ASN.1 parser heavily based on that written by Tihmstar
 *  for his img4tool.
 *
 */

#ifndef __IMAGE4_ASN1_H__
#define __IMAGE4_ASN1_H__

#ifdef __cplusplus
extern {
#endif

#include <stdio.h>
#include <libhelper-logger.h>

#include "image4.h"


/**
 *  ASN.1 Tag Class
 */
#define kASN1TagClassUniversal		0
#define kASN1TagClassApplication	1
#define kASN1TagClassContextSpecific	2
#define kASN1TagClassPrivate		3

/**
 *  ASN.1 Primitive
 */
#define kASN1Primitive			0
#define kASN1Constructed		1

/**
 *  ASN.1 Tag Number
 */
#define kASN1TagEnd_of_Content  	0
#define kASN1TagBOOLEAN         	1
#define kASN1TagINTEGER         	2
#define kASN1TagBIT             	3
#define kASN1TagOCTET           	4
#define kASN1TagNULL            	5
#define kASN1TagOBJECT          	6
#define kASN1TagObject          	7
#define kASN1TagEXTERNAL        	8
#define kASN1TagREAL            	9
#define kASN1TagENUMERATED      	10 		//0x0A
#define kASN1TagEMBEDDED        	11 		//0x0B
#define kASN1TagUTF8String      	12 		//0x0C
#define kASN1TagRELATIVE_OID    	13 		//0x0D
#define kASN1TagReserved        	(14 | 15) 	//(0x0E | 0x0F)
#define kASN1TagSEQUENCE        	16 		//0x10
#define kASN1TagSET             	17 		//0x11
#define kASN1TagNumericString   	18 		//0x12
#define kASN1TagPrintableString 	19 		//0x13
#define kASN1TagT61String       	20 		//0x14
#define kASN1TagVideotexString  	21 		//0x15
#define kASN1TagIA5String       	22 		//0x16
#define kASN1TagUTCTime         	23 		//0x17
#define kASN1TagGeneralizedTime 	24 		//0x18
#define kASN1TagGraphicString   	25 		//0x19
#define kASN1TagVisibleString   	26 		//0x1A
#define kASN1TagGeneralString   	27 		//0x1B
#define kASN1TagUniversalString 	28 		//0x1C
#define kASN1TagCHARACTER       	29 		//0x1D
#define kASN1TagBMPString       	30 		//0x1E
#define kASN1TagPrivate   		(char)0xff


/**
 *
 */
typedef struct {
    unsigned char 	tag_number : 5;
    unsigned char 	is_constructed : 1;
    unsigned char 	tag_class : 2;
} asn1_tag_t;

typedef struct {
    unsigned char 	len : 7;
    unsigned char 	is_long : 1;
} asn1_length_t;

typedef struct {
    size_t 	data_len;
    size_t 	size_bytes;
} asn1_elem_len_t;

typedef struct {
    unsigned char	num : 7;
    unsigned char	more : 1;
} asn1_private_tag_t;


/**
 *
 */
asn1_elem_len_t
asn1_len (const char *buf);

uint64_t
asn1_get_number_from_tag (asn1_tag_t *tag);

size_t
asn1_get_private_tagnum (asn1_tag_t *tag, size_t *size_bytes);

char *
asn1_get_string (char *buf, char **outstring, size_t *strlen);

char *
asn1_element_at_index (const char *buf, int index);

int
asn1_elements_in_object (const char *buf);

int
asn1_element_at_index_with_counter (const char *buf, int index, asn1_tag_t **tag_ret);

int
asn1_get_sequence_name (const char *buf, char **name, size_t *namelen);

//test
int __TEST_GET_SEQUENCE_NAME (const char *buf, char **name, size_t *namelen);



#ifdef __cplusplus
}
#endif

#endif /* __image4_asn1_h__ */

