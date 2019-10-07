/**
 *     img4helper
 *     Copyright (C) 2019, @h3adsh0tzz
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

/**
 *  About:
 *
 *  This file is a small ASN.1 Parser/Decoder based on that used by Tihmstar
 *  for img4tool.
 *
 */

#include "asn1.h"

asn1ElemLen_t asn1Len (const char buf[4])
{
	asn1Length_t *tmp = (asn1Length_t *)buf;
	size_t outsize = 0;
	int bytes = 0;

	unsigned char *sbuf = (unsigned char *)buf;

	if (!tmp->isLong) {
		outsize = tmp->len;
	} else {
		bytes = tmp->len;
		for (int i = 0; i < bytes; i++) {
			outsize *= 0x100;
			outsize += sbuf[1+i];
		}
	}

	asn1ElemLen_t ret;
	ret.dataLen = outsize;
	ret.sizeBytes = bytes + 1;

	return ret;
}


char *asn1GetString (char *buf, char **outstring, size_t *strlen)
{
	asn1Tag_t *tag = (asn1Tag_t *)buf;

	if (!(tag->tagNumber | kASN1TagIA5String)) {
		g_print("[Error] not a string\n");
		return 0;
	}

	asn1ElemLen_t len = asn1Len(++buf);
	*strlen = len.dataLen;
	buf += len.sizeBytes;
	if (outstring) *outstring = buf;

	return buf +* strlen;
}


int asn1ElementAtIndexWithCounter(const char *buf, int index, asn1Tag_t **tagret){
    int ret = 0;

    if (!((asn1Tag_t *)buf)->isConstructed) return 0;
    asn1ElemLen_t len = asn1Len(++buf);

    buf +=len.sizeBytes;

	/* TODO: add length and range checks */
    while (len.dataLen) {
        if (ret == index && tagret) {
            *tagret = (asn1Tag_t *)buf;
            return ret;
        }

        if (*buf == kASN1TagPrivate) {

            size_t sb = 0;
            //asn1GetPrivateTagnum((asn1Tag_t *)buf,&sb);
            buf += sb;
            len.dataLen -= sb;

        } else if (*buf == (char)0x9F) {

            //buf is element in set and it's value is encoded in the next byte
            asn1ElemLen_t l = asn1Len(++buf);
            if (l.sizeBytes > 1) l.dataLen += 0x80;
            buf += l.sizeBytes;
            len.dataLen -= 1 + l.sizeBytes;

        } else {
            buf++,len.dataLen--;
		}

        asn1ElemLen_t sublen = asn1Len(buf);
        size_t toadd = sublen.dataLen + sublen.sizeBytes;
        len.dataLen -= toadd;
        buf += toadd;
        ret ++;
    }

    return ret;
}


int asn1ElementsInObject (const char *buf)
{
	return asn1ElementAtIndexWithCounter (buf, -1, NULL);
}

char* asn1ElementAtIndex (const char *buf, int index)
{
	asn1Tag_t *ret;
	asn1ElementAtIndexWithCounter(buf, index, &ret);
	return (char*) ret;
}



////////////////////

/* These functions are not ASN.1 specific but are most appropriate in asn1.c */

int getSequenceName (const char *buf, char **name, size_t *namelen)
{
	int err = 0;

	if (((asn1Tag_t *) buf)->tagNumber != kASN1TagSEQUENCE) {
		g_print("not a sequence\n");
		return err;
	}

	int elems = asn1ElementsInObject(buf);
	if (!elems) {
		g_print("no elements in sequence\n");
		return err;
	}

	size_t len;
	asn1GetString(asn1ElementAtIndex(buf, 0), name, &len);
	if (namelen) *namelen = len;

	return err;
}


// Printing functions

void printHex (asn1Tag_t *str)
{
	if (str->tagNumber != kASN1TagOCTET) {
		g_print ("[Error] not an OCTET string\n");
		exit(1);
	}

	asn1ElemLen_t len = asn1Len((char *) str + 1);
	unsigned char *string = (unsigned char *) str + len.sizeBytes + 1;
	while (len.dataLen--) g_print ("%02x", *string++);
}

void printStringWithKey (char* key, asn1Tag_t *string)
{
	char *str = 0;
	size_t len;

	asn1GetString((char *)string, &str, &len);

	printf("%s: ", key);
	printf("%.*s", (int)len, str);
	printf("\n");
}

void printKBAG (char* octet)
{

	// Check if the given octet is in fact a kASN1TagOCTET
	if (((asn1Tag_t *) octet)->tagNumber != kASN1TagOCTET) {
		g_print ("[Error] not an OCTET\n");
		return;
	}

	// Get the Octet length
	asn1ElemLen_t octetlen = asn1Len(++octet);
	octet += octetlen.sizeBytes;

	// Parse the KBAG value(s)
	int subseqs = asn1ElementsInObject(octet);
	for (int i = 0; i < subseqs; i++) {

		// Pick the first/next KBAG value
		char *s = (char *)asn1ElementAtIndex(octet, i);
		int elems = asn1ElementsInObject(s);

		// Try to parse it
		if (elems--) {
			asn1Tag_t *num = (asn1Tag_t *) asn1ElementAtIndex(s, 0);
			if (num->tagNumber != kASN1TagINTEGER) {
				g_print ("[Warning] Skipping unexpected tag\n");
			} else {
				char n = *(char *)(num + 2);
				g_print ("num: %d\n", n);
			}
		}

		// Print the hex strings
		if (elems--) {
			printHex((asn1Tag_t *) asn1ElementAtIndex(s, 1));
			printHex((asn1Tag_t *) asn1ElementAtIndex(s, 2));
			putchar('\n');
		}
	}
}





















