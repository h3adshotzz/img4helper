/**
 * 	img4helper
 * 	Copyright (C) 2019, @h3adsh0tzz
 *
 * 	This program is free software: you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 * 	(at your option) any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include "lzss.h"

/*
 *  LZSS.C -- A data compression program for decompressing lzss compressed objects
 *  4/6/1989 Haruhiko Okumura
 *  Use, distribute, and modify this program freely.
 *  Please send me your improved versions.
 *  PC-VAN      SCIENCE
 *  NIFTY-Serve PAF01022
 *  CompuServe  74050,1022
 *
 *  Copyright (c) 2003 Apple Computer, Inc.
 *  DRI: Josh de Cesare
 */

#define N           4096    /* Size of the ring buffer - must be power of 2 */
#define F           18      /* Upper limit for match_length */
#define THRESHOLD   2       /* Encode string into position and length if match_length is greater than this */
#define NIL         N       /* Index for root of binary search trees */

void *lz_memmem (const void *b1, const void *b2, size_t len1, size_t len2)
{
    char *sp = (char *) b1;     // Initialise the search pointer
    char *pp = (char *) b2;     // Initialise the pattern pointer
    char *eos = (char *) sp + len1 + len2;  // Initiaise end of search address space pointer

    /* Sanity check */
    if(!(b1 && b2 && len1 && len2))
        return NULL;
    
    while (sp <= eos) {
        if (*sp == *pp)
            if (memcmp(sp, pp, len2) == 0)
                return sp;
        
        sp++;
    }

    return NULL;
}


int decompressed_lzss(u_int8_t *dst, u_int8_t *src, u_int32_t srclen){
    /* ring buffer of size N, with extra F-1 bytes to aid string comparison */
    u_int8_t text_buf[N + F - 1];
    u_int8_t *dststart = dst;
    u_int8_t *srcend = src + srclen;
    int  i, j, k, r, c;
    unsigned int flags;
    
    dst = dststart;
    srcend = src + srclen;
    for (i = 0; i < N - F; i++)
        text_buf[i] = ' ';
    r = N - F;
    flags = 0;
    for ( ; ; ) {
        if (((flags >>= 1) & 0x100) == 0) {
            if (src < srcend) c = *src++; else break;
            flags = c | 0xFF00;  /* uses higher byte cleverly */
        }   /* to count eight */
        if (flags & 1) {
            if (src < srcend) c = *src++; else break;
            *dst++ = c;
            text_buf[r++] = c;
            r &= (N - 1);
        } else {
            if (src < srcend) i = *src++; else break;
            if (src < srcend) j = *src++; else break;
            i |= ((j & 0xF0) << 4);
            j  =  (j & 0x0F) + THRESHOLD;
            for (k = 0; k <= j; k++) {
                c = text_buf[(i + k) & (N - 1)];
                *dst++ = c;
                text_buf[r++] = c;
                r &= (N - 1);
            }
        }
    }
    
    return (int)(dst - dststart);
}

struct compHeader {
    char        sig[8] ; // "complzss"
    uint32_t    unknown; // Likely CRC32. But who cares, anyway?
    uint32_t    uncompressedSize;
    uint32_t    compressedSize;
    uint32_t    unknown1; // 1
};

char *tryLZSS(char *compressed, size_t *filesize){
    struct compHeader *compHeader = (struct compHeader*)compressed;
    if (!compHeader) return NULL;
    int sig[2] = { 0xfeedfacf, 0x0100000c };
    int sig2[2] = { 0xfeedface, 0x0000000c };
    
    char *decomp = malloc (ntohl(compHeader->uncompressedSize));
    char *feed = lz_memmem(compressed+64, 1024, sig, sizeof(sig));
    
    if (!feed){
        feed = lz_memmem(compressed+64, 1024, sig2, sizeof(sig2));
        if (!feed)
            return NULL;
    }
    
    feed--;
    int rc = decompressed_lzss((void*)decomp, (void*)feed, ntohl(compHeader->compressedSize));

    if (rc != ntohl(compHeader->uncompressedSize)) {
        return NULL;
    }
    
    *filesize = rc;
    return (decomp);
    
}

char *lzssDecompress(char *src, int size) {
  unsigned char flags = 0;
  unsigned char mask  = 0;
  char *dst = NULL;
  unsigned int  len;
  unsigned int  disp;

  while(size > 0) {
    if(mask == 0) {
      // read in the flags data
      // from bit 7 to bit 0:
      //     0: raw byte
      //     1: compressed block
      flags = *src++;
      mask  = 0x80;
    }

    if(flags & mask) { // compressed block
      // disp: displacement
      // len:  length
      len  = (((*src)&0xF0)>>4)+3;
      disp = ((*src++)&0x0F);
      disp = disp<<8 | (*src++);

      size -= len;

      // for len, copy data from the displacement
      // to the current buffer position
      memcpy(dst, dst-disp-1, len);
      dst += len;
    }
    else { // uncompressed block
      // copy a raw byte from the input to the output
      *dst++ = *src++;
      size--;
    }

    mask >>= 1;
  }

  return dst;
}