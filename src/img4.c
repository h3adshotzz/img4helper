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

img4_t *read_img (char *path)
{
	/* Create an img4_t to hold the img data */
	img4_t *ret = malloc (sizeof (img4_t));

	/* Load the file and size into buf and size */
	FILE *f = fopen (path, "rb");
	if (!f) return NULL;
	
	fseek (f, 0, SEEK_END);
	ret->size = ftell (f);
	fseek (f, 0, SEEK_SET);

	ret->buf = malloc(ret->size);
	if (ret->buf) fread (ret->buf, ret->size, 1, f);

	fclose (f);

	/* Load the file type */
	char *magic = getImageFileType(ret->buf);
	if (!magic) {
		g_print ("[Error] Input file not recognised\n");
		exit(1);
	}

	if (!strcmp(magic, "IMG4")) {
		ret->type = IMG4_TYPE_IMG4;
	} else if (!strcmp(magic, "IM4P")) {
		ret->type = IMG4_TYPE_IM4P;
	} else if (!strcmp(magic, "IM4M")) {
		ret->type = IMG4_TYPE_IM4M;
	} else if (!strcmp(magic, "IM4R")) {
		ret->type = IMG4_TYPE_IM4R;
	} else {
		g_print ("[Error] Unrecognised image\n");
		exit(1);
	}

	return ret;
}

char *string_for_img4type (Img4Type type)
{
	if (type == IMG4_TYPE_IMG4) {
		return "IMG4";
	} else if (type == IMG4_TYPE_IM4P) {
		return "IM4P";
	} else if (type == IMG4_TYPE_IM4M) {
		return "IM4M";
	} else if (type == IMG4_TYPE_IM4R) {
		return "IM4R";
	} else {
		return "Unknown";
	}
}

char *img4_get_component_name (char *buf)
{
	asn1Tag_t *t = (asn1Tag_t *) asn1ElementAtIndex(buf, 1);
	char *comp = 0;
	size_t len;
	asn1GetString((char *)t, &comp, &len); 

	/* Return a full string for the component name */
	if (!strncmp(comp, "krnl", 4)) {
		return "KernelCache";
	} else if (!strncmp(comp, "ibot", 4)) {
		return "iBoot";
	} else {
		return comp - (strlen(comp) + 4);
	}
}

char *img4_check_compression_type (char *buf)
{
	/* Get an element count to ensure we are using an IM4P */
	int c = asn1ElementsInObject (buf);
	if (c < 4) {
		g_print ("[Error] Not enough elements in payload\n");
		exit(1);
	}

	/* Try to select the payload tag from the buf */
	char *tag = asn1ElementAtIndex (buf, 3) + 1;
	asn1ElemLen_t len = asn1Len (tag);
	char *data = tag + len.sizeBytes;

	/* Check for either LZSS or LZFSE/BVX2 */
	if (!strncmp (data, "complzss", 8)) {
		return "complzss";
	} else if (!strncmp (data, "bvx2", 4)) {
		return "bvx2";
	} else {
		/* Either the image is encrypted, or corrupt. Be optimistic and guess encrypted */
		return "encrypted";
	}
}

img4_t *img4_decompress_bvx2 (img4_t *file)
{
	img4_t *ret = file;

	char *tag = asn1ElementAtIndex (file->buf, 3) + 1;
	asn1ElemLen_t len = asn1Len (tag);
	char *data = tag + len.sizeBytes;

	char *compTag = data + len.dataLen;
	char *fakeCompSizeTag = asn1ElementAtIndex (compTag, 0);
	char *uncompSizeTag = asn1ElementAtIndex (compTag, 1);

	size_t fake_src_size = asn1GetNumberFromTag ((asn1Tag_t *) fakeCompSizeTag);
	size_t dst_size = asn1GetNumberFromTag ((asn1Tag_t *) uncompSizeTag);

	size_t src_size = len.dataLen;

	if (fake_src_size != 1) {
		g_print ("[Error] fake_src_size not 1 but 0x%zx!\n", fake_src_size);
	}

	ret->buf = malloc (dst_size);

	size_t uncomp_size = lzfse_decode_buffer ((uint8_t *) ret->buf, dst_size,
											  (uint8_t *) data, src_size,
											  NULL);

	if (uncomp_size != dst_size) {
		g_print ("[Error] Expected to decompress %zu bytes but only got %zu bytes\n", dst_size, uncomp_size);
		exit(1);
	}			

	file->size = dst_size;
	
	return file;
}

/**
 * 
 * 
 */
void img4_extract_im4p (char *im4p, char *outfile)
{

	/*
Loaded: kernelcache.release.iphone11
Image4: IM4P

[*] Detecting compression type... bvx2
[*] Decoding...
[*] Done!


		TODO: Fix component type detection
		TODO: Fix LZSS decompression
		TODO: Tidy up code

	*/

	// Check that a valid file was loaded. 
	img4_t *file = read_img (im4p);
	if (!file->size || !file->buf) {
		g_print ("[Error] There was an issue loading the file\n");
		exit(1);
	}

	// Print some file information
	g_print ("Loaded: \t%s\n", im4p);
	g_print ("Image4 Type: \t%s\n", string_for_img4type(file->type));

	/*asn1Tag_t *t = (asn1Tag_t *) asn1ElementAtIndex(file->buf, 1);
	char *comp = 0;
	size_t len;
	asn1GetString((char *)t, &comp, &len); 
	g_print ("Component: \t%.*s\n\n", (int)len, comp);
*/
	char *a = img4_get_component_name (file->buf);
	g_print ("Component: \t%s\n\n", a);

	/* We can only decrypt full IMG4s or IM4Ps, so ignore if anything else */
	if (file->type == IMG4_TYPE_IM4M || file->type == IMG4_TYPE_IM4R) {
		g_print ("[Error] Only .img4 and .im4p file payloads can be extracted\n");
		exit (1);
	}

	/* Try to detect a compression type, if there is not luck, the file could be encrypted */
	char *comp_type = img4_check_compression_type (file->buf);

	/* Create a new img4_t as we are decrpyting/decompressing the buf */
	img4_t *nimg = malloc (sizeof(img4_t));
	nimg->type = file->type;

	/* First check if its encrypted */
	if (!strcmp (comp_type, "encrypted")) {

		g_print ("Encryption: True\n");

/*
#error This doesn't quite work just yet, I just need to commit so i can use my laptop
		char *iv = "0afa50a07d119e6ed70cb5d072a3d0d6";
		char *key = "1a72479271ce1f8e9625c15c1e05e3e681d6408971a1ddc0d2b0c6a937a10d3e";
		uint8_t fullkey[] = "0afa50a07d119e6ed70cb5d072a3d0d61a72479271ce1f8e9625c15c1e05e3e681d6408971a1ddc0d2b0c6a937a10d3e";

		unsigned char *enc_out = malloc (file->size);

		AES_KEY aes_key;
		AES_set_decrypt_key (fullkey, 256, &aes_key);
		AES_decrypt ((unsigned char *)file->buf, enc_out, &aes_key);

		g_print ("Size: %lu\n", strlen((char *) enc_out));
		for (int i = 0; i < 10; i++) {
			g_print ("%02x ", enc_out[i]);
		}

		FILE *test = fopen (outfile, "wb");
		fwrite (enc_out, file->size, 1, test);
		fclose (test);*/
	} else {

		/* Set the new image with the contents of the old */
		nimg->size = file->size;
		nimg->buf = file->buf;
	}

	/* Start banner */
	g_print ("== Start\n");

	/* Check for a form of compression, and whether the image was decrypted */
	comp_type = img4_check_compression_type (nimg->buf);
	g_print ("[*] Detecting compression type...");

	if (!strcmp (comp_type, "complzss")) {


	} else if (!strcmp (comp_type, "bvx2")) {

		/* There is BVX2 compression */
		g_print (" bvx2\n");

		/* Decode/Decompress the payload */
		g_print ("[*] Decompressing...\n");
		nimg = img4_decompress_bvx2 (nimg);
		g_print ("[*] Done\n");

		g_print ("[*] Writing to file: %s\n", outfile);
		FILE *test = fopen (outfile, "wb");
		fwrite (nimg->buf, nimg->size, 1, test);
		fclose (test);

	} else {

		/* There was no compression */
		g_print ("None!\n");

	}

	if (!strcmp(a, "KernelCache")) {
		g_print ("\nPlease run img4helper --analyse-kernel to analyse decompressed KernelCache.\n");
	} else if (!strcmp(a, "iBoot")) {
		g_print ("\nPlease run img4helper --analyse-iboot to analyse decompressed iBoot.\n");
	}


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
