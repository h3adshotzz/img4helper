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
		handle_im4p (buf, 1);
	} else if (!strcmp(magic, "IM4M")) {
		handle_im4m (buf, 1);
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

void img4_extract_test (char *file)
{
	// Because we need the size of the payload and \0 fucks us over,
	// we won't use the read_from_file function
	FILE *f = fopen (file, "rb");
	if (!f) {
		g_print ("[Error] Could not read file: %s\n", file);
		exit(1);
	}

	// Calc the size of the file
	fseek (f, 0, SEEK_END);
	size_t size = ftell (f);
	fseek (f, 0, SEEK_SET);

	// Buffer for the file and reading the bytes
	char *buf = malloc(size);
	if (buf) {
		fread (buf, size, 1, f);
	}

	// Close the file
	fclose (f);

	// Check the files magic to see how to approach this
	char *magic = getImageFileType ((char *) buf);
	g_print ("magic: %s\n", magic);


	char *im4p;
	if (!strcmp (magic, "IMG4")) {
		im4p = getIM4PFromIMG4 (buf);
	} else if (!strcmp (magic, "IM4P")) {
		im4p = buf;
		free(buf);
	} else {
		g_print ("[Error] Provided file was not an IMG4 nor IM4P.");
		exit(1);
	}

	int elm_count = asn1ElementsInObject (im4p);
	g_print ("elms: %d\n", elm_count);

	if (elm_count < 4) {
		g_print ("not enough elems: %d", elm_count);
		exit(1);
	}

	// By adding N to the string, it removes N bytes from the start. 
	char *tag = asn1ElementAtIndex (im4p, 3) + 1;
	asn1ElemLen_t len = asn1Len(tag);
	char *data = tag + len.sizeBytes;

	for (int i = 0; i < 5; i++) {
		g_print ("%c ", data[i]);
	}
	g_print ("\n\n");

	char *outdata;
	if (!strncmp (data, "complzss", 8)) {
		g_print ("Found LZSS Compression\n");
		//outdata = tryLZSS(data, 0);		/* This library is fucked and doesn't work */
	} else if (!strncmp(data, "bvx2", 4)) {
		g_print ("Found bvx2\n");

		char *compTag = data + len.sizeBytes;
		char *fakeCompSizeTag = asn1ElementAtIndex (compTag, 0);
		char *uncompSizeTag = asn1ElementAtIndex (compTag, 1);

		size_t fake_src_size = asn1GetNumberFromTag ((asn1Tag_t *)fakeCompSizeTag);
		size_t dst_size = asn1GetNumberFromTag ((asn1Tag_t *)uncompSizeTag);

		size_t src_size = len.sizeBytes;

		if (fake_src_size != 1) {
			g_print ("[Error] fake_src_size not 1 but 0x%zx!\n", fake_src_size);
		}

		outdata = malloc (dst_size);

#error "We need LZFSE, and I'm starting to get annoyed with meson so I'm going to try and switch to cmake first".
		size_t uncomp_size = lzfse_decode_buffer ((uint8_t *) outdata, dst_size, (uint8_t *) data, src_size, NULL);

		if (uncomp_size != dst_size) {
			g_print ("[Error] Expected to decompress %zu bytes but only got %zu\n", dst_size, uncomp_size);
			exit(1);
		}

		size = dst_size;

		FILE *test = fopen ("test.raw", "wb");
		fwrite (outdata, size, 1, test);
		fclose (test);

	} else {
		g_print ("Shit, not bvx2 and lzss is broken\n");
	}


	/*FILE *test = fopen ("test.raw", "wb");
	fwrite (data, size, 1, test);
	fclose (test);*/
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
	// TODO: IM4R Parsing


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

	// Get the files type
	char *magic = getImageFileType (buf);
	if (!magic) {
		g_print ("[Error] Input file format not recognised\n");
		exit(1);
	}

	// Print the loaded file name and the size
	g_print ("Loaded: %s\n", filename);
	g_print ("Image4 Type: %s\n\n", magic);

	// Switch through the possible PRINT operations
	switch (type) {
		case IMG4_PRINT_ALL:

			// Check what image we're dealing with, then handle as appropriate
			if (!strcmp (magic, "IMG4")) {

				// Because this in a full img4, we have to extract everything from it.
				char *im4p = getIM4PFromIMG4 (buf);
				char *im4m = getIM4MFromIMG4 (buf);
				//char *im4r = getIM4RFromIMG4 (buf);

				// Print the IMG4 banner
				g_print ("IMG4: ------\n");

				// Handle the payload first
				handle_im4p (im4p, 1);

				// Make some space between the two
				g_print ("\n");

				// Handle the IM4M next
				handle_im4m (im4m, 1);

				// More space
				g_print ("\n");

				// Lastly, handle the IM4R
				//handle_imr4 (im4r, 1);

			} else if (!strcmp(magic, "IM4P")) {
				handle_im4p (buf, 1);
			} else if (!strcmp(magic, "IM4M")) {
				handle_im4m (buf, 1);
			} else if (!strcmp(magic, "IM4R")) {
				//
			} else {
				g_print ("[Error] Unrecognised image\n");
				exit(1);
			}

			break;
		case IMG4_PRINT_IM4P:

			// Verify the image is an IM4P or an IMG4
			if (!strcmp (magic, "IMG4")) {
				handle_im4p (getIM4PFromIMG4 (buf), 1);
			} else if (!strcmp (magic, "IM4P")) {
				handle_im4p (buf, 1);
			} else {
				g_print ("[Error] The file does not contain an IM4P (%s)\n", magic);
				exit(1);
			}

			break;
		case IMG4_PRINT_IM4M:

			// Verify the image is an IM4M or an IMG4
			if (!strcmp (magic, "IMG4")) {
				handle_im4m (getIM4MFromIMG4 (buf), 1);
			} else if (!strcmp (magic, "IM4M")) {
				handle_im4m (buf, 1);
			} else {
				g_print ("[Error] The file does not contain an IM4M (%s)\n", magic);
				exit(1);
			}

			break;
		default:
			exit(1);
	}
}
