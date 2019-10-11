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
void handle_im4p (char *buf, int tabs)
{
	char *padding = malloc(sizeof(char) * tabs);
	for (int i = 0; i < tabs; i++) {
		padding[i] = '\t';
	}

	// Print IM4P
	g_print ("  IM4P: ------\n");

	// Get buf magic
	char *magic;
	size_t l;
	getSequenceName(buf, &magic, &l);

	// Check if the magic matches IM4P
	if (strncmp("IM4P", magic, l)) {
		g_print ("%s[Error] Expected \"IM4P\", got \"%s\"\n", padding, magic);
	}

	// Grab all the elements in the file magic.
	// TODO: counting more elemnts than it should. Images without kbags are still being parsed
	int elems = asn1ElementsInObject(buf);

	// Print the Image type, desc and size
	if (--elems > 0) {
		printStringWithKey("Type", (asn1Tag_t *) asn1ElementAtIndex(buf, 1), padding);
	}
	if (--elems > 0) {
		printStringWithKey("Desc", (asn1Tag_t *) asn1ElementAtIndex(buf, 2), padding);
	}
	if (--elems > 0) {

		// Check the data size and print it.
		asn1Tag_t *data = (asn1Tag_t *) asn1ElementAtIndex(buf, 3);
		if (data->tagNumber != kASN1TagOCTET) {
			g_print ("%s[Warning] Skipped an unexpected tag where an OCTETSTRING was expected\n", padding);
		} else {
			g_print ("%sSize: 0x%08zx\n\n", padding, asn1Len((char *) data + 1).dataLen);
		}
	}

	// Check for KBAG values. There should be two on encrypted images, with the first being PRODUCT
	//      and the seccond being DEVELOPMENT.
	if (--elems > 0) {
		img4PrintKeybag ((char *) asn1ElementAtIndex(buf, 4), padding);
	} else {
		g_print ("%sThis IM4P is not encrypted, no KBAG values\n", padding);
	}

}


/**
 *
 *	handle_im4m()
 *
 *	Handle a given file's im4m section. This will simply print the data contained
 *	within the IM4M.
 *
 */
void handle_im4m(char *buf, int tabs)
{

	char *padding = malloc(sizeof(char) * tabs);
	for (int i = 0; i < tabs; i++) {
		padding[i] = '\t';
	}

	// Print IM4M
	g_print("  IM4M: ------\n");

	// Get buf magic
	char *magic;
	size_t l;
	getSequenceName(buf, &magic, &l);

	// Check the magic matches IM4M
	if (strncmp("IM4M", magic, l)) {
		g_print ("%sFile does not contain an IM4M\n", padding);
		return;
	}

	// Check the IM4M has at least two elems
	int elems = asn1ElementsInObject (buf);
	if (elems < 2) {
		g_print ("%s[Error] Expecting at least 2 elements\n", padding);
		exit(1);
	}

	// Print the IM4M Version
	if (--elems > 0) {
		g_print("%sVersion: ", padding);
		asn1PrintNumber((asn1Tag_t *) asn1ElementAtIndex(buf, 1), "");
		putchar('\n');
		putchar('\n');
	}

	// Go through each IM4M element and print it
	if (--elems > 0) {

		// Manifest Body parse
		asn1Tag_t *manbset = (asn1Tag_t *) asn1ElementAtIndex(buf, 2);
		if (manbset->tagNumber != kASN1TagSET) {
			g_print ("%s[Error] Expecting SET\n", padding);
			exit(1);
		}

		asn1Tag_t *privtag = manbset + asn1Len((char *) manbset + 1).sizeBytes + 1;

		size_t sb;
		asn1PrintPrivtag(asn1GetPrivateTagnum(privtag++, &sb), padding);
		g_print(":\n");

		char *manbseq = (char *)privtag + sb;
		manbseq += asn1Len(manbseq).sizeBytes + 1;

		img4PrintManifestBody(manbseq, padding);
	}

}


/**
 *
 *
 *
 *
 */
void handle_img4 (char *buf)
{
	// Format the output correctly
	g_print ("IMG4: ------\n");

	// Handle the IM4P first, tabbing set to 1.
	handle_im4p (getIM4PFromIMG4 (buf), 1);

	// Make some space between the IM4M and IM4P
	g_print ("\n");

	// Handle the IM4M
	handle_im4m (getIM4MFromIMG4 (buf), 1);
}


void handle_all (char *buf)
{
	char *magic = getImageFileType(buf);
	if (!magic) {
		g_print ("[Error] Input file not recognised\n");
		exit(1);
	}

	g_print ("Image type: %s\n\n", magic);

	if (!strcmp(magic, "IMG4")) {
		handle_img4 (buf);
	} else if (!strcmp(magic, "IM4P")) {
		handle_im4p (buf, 0);
	} else if (!strcmp(magic, "IM4M")) {
		handle_im4m (buf, 0);
	} else if (!strcmp(magic, "IM4R")) {
		//
	} else {
		g_print ("[Error] Unrecognised image\n");
		exit(1);
	}

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
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *ret = malloc(size);
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
	// TODO: Add parsing for IM4M
	// TODO: Idk, think of something else to add.


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

	// Print the loaded file name and the size
	g_print ("Loaded: %s\n", filename);

	// Switch through the possible PRINT operations
	switch (type) {
		case IMG4_PRINT_ALL:

			// Handle everything
			handle_all (buf);

			break;
		case IMG4_PRINT_IM4P:

			// Handle the IM4P
			handle_im4p(buf, 0);

			break;
		case IMG4_PRINT_IM4M:

			// Handle the IM4M
			handle_im4m(buf, 0);

			break;
		default:
			exit(1);
	}
}
