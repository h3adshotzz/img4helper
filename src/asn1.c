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
    
    if (!((asn1Tag_t *)buf)->isConstructed) {
		return 0;
	}
    asn1ElemLen_t len = asn1Len(++buf);
    
    buf += len.sizeBytes;
    
	/* TODO: add lenght and range checks */
    while (len.dataLen) {
        if (ret == index && tagret){
            *tagret = (asn1Tag_t *) buf;
            return ret;
        }
        
        if (*buf == kASN1TagPrivate) {
            
			size_t sb;
            asn1GetPrivateTagnum((asn1Tag_t *) buf, &sb);
            
			buf += sb;
            len.dataLen -= sb;

        } else if (*buf == (char)0x9F){
            
			//buf is element in set and it's value is encoded in the next byte
            asn1ElemLen_t l = asn1Len(++buf);
            if (l.sizeBytes > 1) l.dataLen += 0x80;
            buf += l.sizeBytes;
            len.dataLen -= 1 + l.sizeBytes;
        
		} else
            buf++,len.dataLen--;
        
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

size_t asn1GetPrivateTagnum (asn1Tag_t *tag, size_t *sizebytes)
{
	if (*(unsigned char *) tag != 0xff) {
		g_print ("[Error] Not a private TAG 0x%02x\n", *(unsigned int *) tag);
		exit(1);
	}

	size_t sb = 1;
	asn1ElemLen_t taglen = asn1Len((char *)++tag);
	taglen.sizeBytes -= 1;

	if (taglen.sizeBytes != 4) {
		/* 
         WARNING: seems like apple's private tag is always 4 bytes long
         i first assumed 0x84 can be parsed as long size with 4 bytes, 
         but 0x86 also seems to be 4 bytes size even if one would assume it means 6 bytes size.
         This opens the question what the 4 or 6 nibble means.
        */
		taglen.sizeBytes = 4;
	}

	size_t tagname = 0;
	do {
		tagname *= 0x100;
		tagname >>= 1;
		tagname += ((asn1PrivateTag_t *)tag)->num;
		sb++;
	} while (((asn1PrivateTag_t *) tag++)->more);

	if (sizebytes) *sizebytes = sb;
	return tagname;
}

void asn1PrintRecKeyVal (char *buf)
{
	if (((asn1Tag_t *) buf)->tagNumber == kASN1TagSEQUENCE) {
		int i;
		if ((i = asn1ElementsInObject(buf)) != 2) {
			g_print ("[Error] Expecting 2 elements, found %d\n", i);
			exit(1);
		}

		printI5AString((asn1Tag_t *) asn1ElementAtIndex(buf, 0));
		g_print(": ");

		asn1PrintRecKeyVal (asn1ElementAtIndex(buf, 1));
		g_print("\n");

		return;
	} else if (((asn1Tag_t *)buf)->tagNumber != kASN1TagSET) {
		asn1PrintValue ((asn1Tag_t *)buf);
		return;
	}

	// Must be a kASN1TagSET
	g_print("------------------------------\n");
	for (int i = 0; i < asn1ElementsInObject(buf); i++) {

		char *elem = (char*)asn1ElementAtIndex(buf, i);
		size_t sb;

		printPrivtag(asn1GetPrivateTagnum((asn1Tag_t *) elem, &sb));
		g_print (": ");

		elem += sb;
		elem += asn1Len(elem + 1).sizeBytes;
		asn1PrintRecKeyVal (elem);

	}
}

void asn1PrintIM4MVal (char *buf, char* padding)
{
	if (((asn1Tag_t *) buf)->tagNumber == kASN1TagSEQUENCE) {
		int i;
		if ((i = asn1ElementsInObject(buf)) != 2) {
			g_print ("[Error] Expecting 2 elements, found %d\n", i);
			exit(1);
		}

		asn1Tag_t *str = (asn1Tag_t *) asn1ElementAtIndex(buf, 0);
		if (str->tagNumber != kASN1TagIA5String) {
			g_print ("[Error] Not an IA5String\n");
			exit(1);
		}

		asn1ElemLen_t len = asn1Len((char *) ++str);
		g_print("%.*s: ", (int) len.dataLen, ((char *) str) + len.sizeBytes);

		asn1PrintIM4MVal (asn1ElementAtIndex(buf, 1), padding);
		g_print("\n");

		return;
	} else if (((asn1Tag_t *)buf)->tagNumber != kASN1TagSET) {
		asn1PrintPaddedValue ((asn1Tag_t *)buf, padding);
		return;
	}

	// Must be a kASN1TagSET
	g_print("------------------------------\n");
	for (int i = 0; i < asn1ElementsInObject(buf); i++) {

		char *elem = (char*)asn1ElementAtIndex(buf, i);
		size_t sb;

		//printPrivtag(asn1GetPrivateTagnum((asn1Tag_t *) elem, &sb));

		size_t tmp = asn1GetPrivateTagnum((asn1Tag_t *) elem, &sb);
		char *ptag = (char *) &tmp;
		int len = 0;
		g_print ("\t");
		while (*ptag) ptag++, len++;
		while (len--) putchar(*--ptag);

		g_print (": ");

		elem += sb;
		elem += asn1Len(elem + 1).sizeBytes;
		asn1PrintIM4MVal (elem, padding);

	}
}

void asn1PrintPaddedValue (asn1Tag_t *tag, char* padding)
{
	char *tmp = padding;
	if (tag->tagNumber == kASN1TagIA5String) {

		// Create a formatted printi5astring
		//printI5AString (tag);

		if (tag->tagNumber != kASN1TagIA5String) {
			g_print ("[Error] Not an IA5String\n");
			exit(1);
		}

		asn1ElemLen_t len = asn1Len((char *) ++tag);
		//putStr(((char*)str)+len.sizeBytes, len.dataLen);
		// This doesn't control the IM4M tags
		g_print ("%.*s", (int) len.dataLen, ((char *) tag) + len.sizeBytes);

	} else if (tag->tagNumber == kASN1TagOCTET) {
		//printHex (tag);

		if (tag->tagNumber != kASN1TagOCTET) {
			g_print ("[Error] not an OCTET string\n");
			exit(1);
		}

		asn1ElemLen_t len = asn1Len((char *) tag + 1);
		unsigned char *string = (unsigned char *) tag + len.sizeBytes + 1;
		while (len.dataLen--) g_print ("%02x", *string++);

	} else if (tag->tagNumber == kASN1TagINTEGER) {
		asn1ElemLen_t len = asn1Len ((char *) tag + 1);

		unsigned char *num = (unsigned char *) tag + 1 + len.sizeBytes;
		uint64_t pnum = 0;

		while (len.dataLen--) {
			pnum *= 0x100;
			pnum += *num++;
		}

		if (sizeof(uint64_t) == 8) {
			g_print("%llu", pnum);
		} else {
			g_print(" (hex)");
		}

	} else if (tag->tagNumber == kASN1TagBOOLEAN) {
		g_print ("%s", (*(char *) tag + 2 == 0) ? "false" : "true");
	} else {
		g_print ("[Error] Can't print unknown tag %02x\n", *(unsigned char *)tag);
	}
}

void asn1PrintValue (asn1Tag_t *tag)
{
	if (tag->tagNumber == kASN1TagIA5String) {
		printI5AString (tag);
	} else if (tag->tagNumber == kASN1TagOCTET) {
		printHex (tag);
	} else if (tag->tagNumber == kASN1TagINTEGER) {
		asn1ElemLen_t len = asn1Len ((char *) tag + 1);

		unsigned char *num = (unsigned char *) tag + 1 + len.sizeBytes;
		uint64_t pnum = 0;

		while (len.dataLen--) {
			pnum *= 0x100;
			pnum += *num++;
		}

		if (sizeof(uint64_t) == 8) {
			g_print("%llu", pnum);
		} else {
			g_print(" (hex)");
		}

	} else if (tag->tagNumber == kASN1TagBOOLEAN) {
		g_print ("%s", (*(char *) tag + 2 == 0) ? "false" : "true");
	} else {
		g_print ("[Error] Can't print unknown tag %02x\n", *(unsigned char *)tag);
	}
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

char *getIM4PFromIMG4 (char *buf)
{
	char *magic;
	size_t l;

	getSequenceName (buf, &magic, &l);

	if (strncmp("IMG4", magic, l)) {
		g_print ("[Error] Expected \"IMG4\", got \"%s\"\n", magic);
		exit(1);
	}

	if (asn1ElementsInObject (buf) < 2) {
		g_print ("[Error] Not enough elements in SEQUENCE\n");
		exit(1);
	}

	// The IM4P should be first, so get the element at index 1
	char *ret = (char *) asn1ElementAtIndex(buf, 1);
	getSequenceName (ret, &magic, &l);

	if (strncmp("IM4P", magic, 4) == 0) {
		return ret;
	} else {
		g_print ("[Error] Expected \"IM4P\", got \"%s\"\n", magic);
		exit(1);
	}

}

char *getIM4MFromIMG4 (char *buf)
{
	char *magic;
	size_t l;

	getSequenceName (buf, &magic, &l);

	if (strncmp("IMG4", magic, l)) {
		g_print ("[Error] Expected \"IMG4\", got \"%s\"\n", magic);
		exit(1);
	}

	if (asn1ElementsInObject (buf) < 3) {
		g_print ("[Error] Not enough elements in SEQUENCE\n");
		exit(1);
	}

	char *ret = (char*) asn1ElementAtIndex (buf, 2);
    if (((asn1Tag_t *) ret)->tagClass != kASN1TagClassContextSpecific) {
		g_print ("[Error] unexpected Tag 0x%02x, expected SET\n", *(unsigned char*) ret);
		exit(1);
	}

	ret += asn1Len (ret + 1).sizeBytes +1;
	getSequenceName (ret, &magic, &l);

	if (strncmp("IM4M", magic, 4) == 0) {
		return ret;
	} else {
		g_print ("[Error] Expected \"IM4P\", got \"%s\"\n", magic);
		exit(1);
	}
}


// This should return an array of element types [im4p, im4m, im4r]
void getElementsFromIMG4 (char *buf)
{
	char *magic;
	size_t l;

	//
	getSequenceName(buf, &magic, &l);
	if (strncmp("IMG4", magic, l)) {
		g_print ("[Error] Expected \"IMG4\", got \"%s\"\n", magic);
		return;
	}

	//
	int elemCnt = asn1ElementsInObject (buf);
	for (int i = 0; i < elemCnt; i++) {
		char *ret = (char *)asn1ElementAtIndex(buf, i);
		ret += asn1Len(ret + 1).sizeBytes + 1;
		char *tmp;
		getSequenceName (ret, &tmp, &l);
		g_print ("[%d]: %s\n", i, tmp);
	}
}

char* getImageFileType (char *buf)
{
	char *magic;
	size_t l;

	//
	getSequenceName(buf, &magic, &l);
	if (!strncmp("IMG4", magic, l)) {
		return "IMG4";
	} else if (!strncmp("IM4M", magic, l)) {
		return "IM4M";
	} else if (!strncmp("IM4P", magic, l)) {
		return "IM4P";
	} else if (!strncmp("IM4R", magic, l)) {
		return "IM4R";
	} else {
		g_print ("[Error] Unexpected tag, got \"%s\"\n", magic);
		exit(1);
	}
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

void printNumber (asn1Tag_t *tag)
{
	if (tag->tagNumber != kASN1TagINTEGER) {
		g_print("[Error] Tag not an Integer\n");
		return;
	}

	asn1ElemLen_t len = asn1Len((char *) ++tag);
	uint32_t num = 0;
	while (len.sizeBytes--) {
		num *= 0x100;
		num += *(unsigned char*) ++tag;
	}
	printf("%u", num);
}

void printI5AString (asn1Tag_t *str)
{
	if (str->tagNumber != kASN1TagIA5String) {
		g_print ("[Error] Not an IA5String\n");
		exit(1);
	}

	asn1ElemLen_t len = asn1Len((char *) ++str);
	putStr(((char*)str)+len.sizeBytes, len.dataLen);
}

void printPrivtag (size_t privTag)
{
	char *ptag = (char *) &privTag;
	int len = 0;
	while (*ptag) ptag++, len++;
	while (len--) putchar(*--ptag);
}

void printFormattedMANB (const char *buf, char *padding)
{
	// Get the buf magic
	char *magic;
	size_t l;
	getSequenceName(buf, &magic, &l);

	// Check that the magic contains MANB
	if (strncmp("MANB", magic, l)) {
		g_print ("[Error] Expected \"MANB\", got \"%s\"\n", magic);
		exit(1);
	}

	// Count the number of elements and make sure it is at least 2
	int manbElmCount = asn1ElementsInObject (buf);
	if (manbElmCount < 2) {
		g_print ("[Error] Not enough elements in MANB\n");
		exit(1);
	}

	// Fetch all the elements from the manifest body
	char *manbSeq = (char *)asn1ElementAtIndex(buf, 1);
	for (int i = 0; i < asn1ElementsInObject(manbSeq); i++) {

		// Cycle through each, parsing it and printing.
		asn1Tag_t *manbElm = (asn1Tag_t *)asn1ElementAtIndex(manbSeq, i);


		//This prints the property name
		size_t privTag = 0;
		if (*(char *) manbElm == kASN1TagPrivate) {

			size_t sb;
			g_print ("%s", padding);
			size_t privTag = asn1GetPrivateTagnum(manbElm, &sb);
			char *ptag = (char *) &privTag;
			int len = 0;
			while (*ptag) ptag++, len++;
			while (len--) putchar(*--ptag);

			g_print(": ");
			manbElm += sb;

		} else {
			manbElm++;
		}

		//This prints the property value.
		manbElm += asn1Len ((char *)manbElm).sizeBytes;

		// Bastard function
		asn1PrintIM4MVal ((char *)manbElm, padding);
		if (strncmp((char *)&privTag, "PNAM", 4) == 0) {
			break;
		}
	}

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

void printFormattedKBAG (char *octet, char *padding)
{
	// Check if the octet is a kASN1TagOCTET
	if (((asn1Tag_t *) octet)->tagNumber != kASN1TagOCTET) {
		g_print ("%s[Error] not an OCTET\n", padding);
		exit(1);
	}

	// Get the octet length
	asn1ElemLen_t octetLen = asn1Len(++octet);
	octet += octetLen.sizeBytes;

	// Parse the KBAG Values
	int subseqs = asn1ElementsInObject (octet);
	for (int i = 0; i < subseqs; i++) {

		// Pick the value
		char *s = (char *)asn1ElementAtIndex (octet, i);
		int elems = asn1ElementsInObject(s);

		// Parse it
		if (elems--) {
			asn1Tag_t *num = (asn1Tag_t *) asn1ElementAtIndex (s, 0);
			if (num->tagNumber != kASN1TagINTEGER) {
				g_print ("%s[Warning] Skipping unexpected tag\n", padding);
			} else {
				if (i == 0) {
					g_print ("%sKBAG Production [1]:\n", padding);
				} else if (i == 1) {
					g_print ("%sKBAG Development [2]:\n", padding);
				} else {
					char n = *(char *)(num + 2);
					g_print ("%sKBAG Unknown: [%d]:\n", padding, n);
				}
			}
		}

		// Print the KBAG Hex string
		if (elems--) {
			g_print("%s", padding);
			printHex((asn1Tag_t *) asn1ElementAtIndex(s, 1));
			printHex((asn1Tag_t *) asn1ElementAtIndex(s, 2));
			putchar('\n');
			putchar('\n');
		}
	}
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