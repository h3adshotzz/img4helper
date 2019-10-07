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

#include "img4.h"
#include "asn1.h"
#include <stdio.h>


//////////////////////////////////////////////////////////////////
/*				   IM4P/IM4M Specific Funs						*/
//////////////////////////////////////////////////////////////////


/**
 *	handle_im4p()
 *
 *	Handle a given file's im4p section. This will simply print the data contained
 *  within the IM4P header.
 *
 */
void handle_im4p ()
{

}


/**
 *
 *	handle_im4m()
 *
 *	Handle a given file's im4m section. This will simply print the data contained
 *	within the IM4M.
 *
 */
void handle_im4m()
{

}


//////////////////////////////////////////////////////////////////
/*					General IMG4 Funcs							*/
//////////////////////////////////////////////////////////////////


/**
 *	read_from_file()
 *
 *	Reads the given input file and returns an array buffer.
 *
 */
char* read_from_file (const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *ret = (char*)malloc(size);
	if (ret) fread(ret, size, 1, f);
	fclose(f);

	return ret;
}


/**
 *	print_img4
 *
 *	Handles the input file and a given Img4PrintType which specifies whether the
 *	user requested the entire file to be printed, the im4p or the im4m.
 *
 */
void print_img4 (Img4PrintType type, char* filename)
{
	// Check the filename given is not NULL
	if (!filename) {
		g_print("[Error] No filename given.\n");
		exit(1);
	}

	// Load the file into a buffer
	char *buf = read_from_file (filename);

	// Check that the buffer actually has some data
	if (!buf) {
		g_print("[Error] Blank file\n");
		exit(1);
	}

	// Switch through the possible PRINT operations
	switch (type) {
		case IMG4_PRINT_ALL:
			// This means we are printing both the IM4P and the IM4M.
			char *magic;
			size_t l;
			getSequenceName(buf, &magic, &l);

			// Check if the magic matches IM4P
			if (strncmp("IM4P", magic, l)) {
				g_print ("[Error] Expected \"IM4P\", got \"%s\"\n", magic);
			}

			// Grab all the elements in the file magic.
			int elems = asn1ElementsInObject(buf);
#ifdef DEBUG
			g_print("Elements in buffer: %d\n", elems);
#endif

			// Print Image type, description and size
			if (--elems > 0) printStringWithKey("Type", (asn1Tag_t *) asn1ElementAtIndex(buf, 1));
			if (--elems > 0) printStringWithKey("Desc", (asn1Tag_t *) asn1ElementAtIndex(buf, 2));
			if (--elems > 0) {

				// Check the data size and print it.
				asn1Tag_t *data = (asn1Tag_t *) asn1ElementAtIndex(buf, 3);
				if (data->tagNumber != kASN1TagOCTET) {
					g_print ("[Warning] Skipped an unexpected tag where an OCTETSTRING was expected\n");
				} else {
					g_print ("Size: 0x%08zx\n\n\n", asn1Len((char *) data + 1).dataLen);
				}

			}

			// Check for KBAG values. There should be two on encrypted images, with the first being PRODUCTION
			// 		and the seccond being DEVELOPMENT.
			if (--elems > 0) {
				g_print ("KBAG:\n");
				printKBAG ((char *) asn1ElementAtIndex(buf, 4));
			} else {
				g_print ("This IM4P does not contain any KBAG values\n");
			}

			break;
		case IMG4_PRINT_IM4P:
			g_print("Print IM4P\n");
			//print_im4p (
			break;
		case IMG4_PRINT_IM4M:
			g_print("Print IM4M\n");
			break;
		default:
			exit(1);
	}
}
