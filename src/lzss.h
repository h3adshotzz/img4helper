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
//
//  lzssdec.h
//  img4tool
//
//  Code borrowed from: http://newosxbook.com/src.jl?tree=listings&file=joker.c
//  Coded by Jonathan Levin (a.k.a @Morpheus______), http://newosxbook.com

#ifndef lzss_h
#define lzss_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

char *tryLZSS(char *compressed, size_t *filesize);
char *lzssDecompress(char *src, int size);

#endif /* lzss_h */