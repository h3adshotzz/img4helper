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

#ifndef IMG4_H
#define IMG4_H

#include <glib.h>
#include "asn1.h"
#include "config.h"

typedef enum {
	IMG4_PRINT_ALL,
	IMG4_PRINT_IM4P,
	IMG4_PRINT_IM4M
} Img4PrintType;


typedef enum {
	IMG4_TYPE_IMG4,
	IMG4_TYPE_IM4M,
	IMG4_TYPE_IM4P,
	IMG4_TYPE_IM4R
} Img4Type;

// Img4 printing
void print_img4(Img4PrintType type, char* filename);

#endif
