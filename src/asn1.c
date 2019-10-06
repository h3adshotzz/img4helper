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


int asn1ElementAtIndexWithCounter (const char *buf, int index, asn1Tag_t **tag)
{
	int ret = 0;
	asn1ElemLen_t buflen = asn1Len(++buf);

	if (!((asn1Tag_t *)buf)->isConstructed) return 0;
	asn1ElemLen_t len = buflen;

	buf += len.sizeBytes;

	// TODO: Add length and range checks
	while (len.dataLen) {
		if (ret == index && tag) {
			*tag = (asn1Tag_t *)buf;
			g_print("Something happened\n");
			return ret;
		}

		if (*buf == kASN1TagPrivate) {

			g_print ("kASN1TagPrivate\n");
			size_t sb = 0;
			//asn1GetPrivateTagnum((asn1Tag_t *)buf, &sb);
			buf += sb;
			len.dataLen = sb;

		} else if (*buf == (char)0x9F) {

			g_print ("*buf == (char)0x9F\n");
			// buf is an element in the set and it's value is encoded in the next byte
			asn1ElemLen_t l = buflen;
			if (l.sizeBytes > 1) l.dataLen += 0x80;
			buf += l.sizeBytes;
			len.dataLen -= 1 + l.sizeBytes;

		} else {
			g_print ("else\n");
			buf++, len.dataLen--;
		}

		asn1ElemLen_t sublen = buflen;
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











