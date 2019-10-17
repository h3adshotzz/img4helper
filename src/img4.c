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

//////////////////////////////////////////////////////////////////
/*				   IM4P/IM4M Specific Funcs						*/
//////////////////////////////////////////////////////////////////


/**
 * 	img4_read_image_from_path ()
 * 
 * 	The Image4 situated at path will be read in and used to construct an
 * 	image4_t type, which contains a few properties about an Image4 file
 * 	that is useful for other functions and operations.
 * 
 * 	Args:
 * 		char *path			-	The path to the Image4 that should be loaded
 * 	
 * 	Returns:
 * 		image4_t			-	The constructed image4_t type.
 * 
 */
image4_t *img4_read_image_from_path (char *path)
{
	/* Create our image4_t to populate and return */
	image4_t *image = malloc (sizeof (image4_t));

	/* The path should have already been verified by the caller */
	FILE *f = fopen (path, "rb");
	if (!f) {
		g_print ("[Error] Could not read file!\n");
		exit(0);
	}

	/* Check the files size in bytes */
	fseek (f, 0, SEEK_END);
	image->size = ftell (f);
	fseek (f, 0, SEEK_SET);

	/* Start reading bytes into the buffer */
	image->buf = malloc (image->size);
	if (image->buf) fread (image->buf, image->size, 1, f);
	
	/* Close the file */
	fclose (f);

	/* Calculate the image file type */
	char *magic = getImageFileType (image->buf);
	if (!magic) {
		g_print ("[Error] Input file could not be recognised as an Image4\n");
		exit (0);
	}

	/* Go through and check each of the magics */
	if (!strncmp (magic, "IMG4", 4)) {
		image->type = IMG4_TYPE_IMG4;
	} else if (!strncmp (magic, "IM4P", 4)) {
		image->type = IMG4_TYPE_IM4P;
	} else if (!strncmp (magic, "IM4M", 4)) {
		image->type = IMG4_TYPE_IM4M;
	} else if (!strncmp (magic, "IM4R", 4)) {
		image->type = IMG4_TYPE_IM4R;
	} else {
		g_print ("[Error] Input file could not be recognised as an Image4\n");
		exit (0);
	}

	/* Return the newly created image */
	return image;

}


/**
 * 	img4_string_for_image_type ()
 * 
 * 	Takes an Image4Type enum and translates into a string, as enums are
 * 	stored as integers.
 * 
 * 	Args:
 * 		Image4Type type 	-	The type to translate
 * 
 * 	Returns:
 * 		char *				-	The string translation of the type.
 * 
 */
char *img4_string_for_image_type (Image4Type type)
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


/**
 * 	TODO: THIS NEEDS TO BE FINISHED!
 * 
 * 	img4_get_component_name ()
 * 
 * 	Takes an image buffer and determines which firmware file is has been taken
 * 	from. For example: 'krnl' == 'KernelCache', 'ibot' == 'iBoot'.
 * 
 * 	Args:
 * 		char *		-	The payload bytes
 * 
 * 	Returns:
 * 		char *		-	The component type string.
 * 
 */
char *img4_get_component_name (char *buf)
{
	char *magic;
	size_t l;
	getSequenceName(buf, &magic, &l);

	char *raw = asn1ElementAtIndex(buf, 1) + 2;

	if (!strncmp (raw, "ibot", 4)) {
		return "iBoot";
	} else if (!strncmp (raw, IMAGE_TYPE_IBEC, 4)) {
		return "iBoot Epoch Change (iBEC)";
	} else if (!strncmp (raw, IMAGE_TYPE_IBSS, 4)) {
		return "iBoot Single Stage (iBSS)";
	} else if (!strncmp (raw, IMAGE_TYPE_LLB, 4)) {
		return "Low Level Bootloader (LLB)";
	} else if (!strncmp (raw, IMAGE_TYPE_SEP_OS, 4)) {
		return "Secure Enclave OS (SEP OS)";
	} else if (!strncmp (raw, IMAGE_TYPE_SEP_OS_RESTORE, 4)) {
		return "Secure Enclave OS Restore (SEP OS RESTORE)";
	} else if (!strncmp (raw, IMAGE_TYPE_DEVTREE, 4)) {
		return "Device Tree";
	} else if (!strncmp (raw, IMAGE_TYPE_RAMDISK, 4)) {
		return "Update/Restore Ramdisk";
	} else if (!strncmp (raw, IMAGE_TYPE_KERNELCACHE, 4)) {
		return "Darwin Kernel Cache";
	} else if (!strncmp (raw, IMAGE_TYPE_LOGO, 4)) {
		return "Boot Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_RECMODE, 4)) {
		return "Recovery Mode Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_NEEDSERVICE, 4)) {
		return "Need Service Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_GLYPHCHRG, 4)) {
		return "Glyph Charge Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_GLYPHPLUGIN, 4)) {
		return "Glyph Plugin Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYCHARGING0, 4)) {
		return "Battery Charging 0 Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYCHARGING1, 4)) {
		return "Battery Charging 1 Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYLOW0, 4)) {
		return "Battery Charging Low 0 Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYLOW1, 4)) {
		return "Battery Charging Low 1 Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_BATTERYFULL, 4)) {
		return "Battery Full Logo";
	} else if (!strncmp (raw, IMAGE_TYPE_OS_RESTORE, 4)) {
		return "iOS Restore Image";
	} else if (!strncmp (raw, IMAGE_TYPE_HAMMER, 4)) {
		return "Hammer";
	} else {
		return "Unknown Component";
	}
}


/**
 * 	img4_handle_im4p ()
 * 
 * 	Takes the byte stream of an .im4p, or the im4p section of an .img4, and
 * 	a tab count. The im4p is parsed and each element is printed in a formatted
 * 	way according to the tab count. If there are any errors, the program should
 * 	gracefully exit as there are multiple error checks
 * 
 * 	Args:
 * 		char *buf 		-	The im4p buffer
 * 		int tabs 		-	Amount of whitespace to print before the elements
 * 
 */
void img4_handle_im4p (char *buf, int tabs)
{
	/* Generate a padding string for tabs */
	char *padding = malloc(sizeof(char) * tabs);
	for (int i = 0; i < tabs; i++) {
		padding[i] = '\t';
	}

	/* Print the IM4P banner */
	g_print ("  IM4P: ------\n");

	/* We can't really use image4_t here, so we have to calculate everything */
	char *magic;
	size_t l;
	getSequenceName(buf, &magic, &l);

	/* Verify that we have been given an im4p buffer */
	if (strncmp("IM4P", magic, l)) {
		g_print ("%s[Error] Expected \"IM4P\", got \"%s\"\n", padding, magic);
	}

	// TODO: counting more elemnts than it should. Images without kbags are still being parsed
	/* Grab the amount of ASN1 elements in the buffer */
	int elems = asn1ElementsInObject(buf);

	/* Print the Image type, desc and size */
	if (--elems > 0) {
		printStringWithKey("Type", (asn1Tag_t *) asn1ElementAtIndex(buf, 1), padding);
	}
	if (--elems > 0) {
		printStringWithKey("Desc", (asn1Tag_t *) asn1ElementAtIndex(buf, 2), padding);
	}
	if (--elems > 0) {

		/* Check the data size and print it. */
		asn1Tag_t *data = (asn1Tag_t *) asn1ElementAtIndex(buf, 3);
		if (data->tagNumber != kASN1TagOCTET) {
			g_print ("%s[Warning] Skipped an unexpected tag where an OCTETSTRING was expected\n", padding);
		} else {
			g_print ("%sSize: 0x%08zx\n\n", padding, asn1Len((char *) data + 1).dataLen);
		}
	}

	// Check for KBAG values. There should be two on encrypted images, with the first being PRODUCT
	//      and the seccond being DEVELOPMENT.
	//
	//	THIS DOES NOT WORK AND NEEDS TO BE FIXED
	// 	REWRITE WHOLE FUNCTION
	//
	if (elems < 1) {
		g_print ("%sThis IM4P is not encrypted, no KBAG values\n", padding);
		exit(1);
	}
	img4PrintKeybag ((char *) asn1ElementAtIndex(buf, 4), padding);

}


/**
 * 	img4_handle_im4m ()
 * 
 * 	Similar to img4_handle_im4p (), takes the .im4m or im4m section buffer and
 * 	a tab count, then prints formatted information contained within the manifest.
 * 
 * 	Args:
 * 		char *buf 		-	The im4m buffer
 * 		int tags		-	 Amount of whitespace to print before the elements
 * 
 */
void img4_handle_im4m (char *buf, int tabs)
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
 * 	img4_handle_im4r ()
 * 
 * 	Similar to img4_handle_im4p (), takes the .im4r or im4r section buffer and
 * 	a tab count, then prints formatted information contained within the im4r.
 * 
 * 	Args:
 * 		char *buf 		-	The im4r buffer
 * 		int tags		-	Amount of whitespace to print before the elements
 * 
 */
void img4_handle_im4r (char *buf, int tabs)
{
	/* Calculate padding */
	char *padding = malloc (sizeof (char) * tabs);
	for (int i = 0; i < tabs; i++) {
		padding[i] = '\t';
	}

	/* Print IM4R banner */
	g_print ("%sIM4R: ------\n", padding);

	/* Verify the buf magic */
	char *magic;
	size_t l;
	getSequenceName (buf, &magic, &l);

	/* Check the magic matches IM4R */
	if (strncmp (magic, "IM4R", 4)) {
		g_print ("%sFile does not contain an IM4R\n", padding);
		exit (0);
	}

	/* Verify the amount of elements in the IM4R */
	int elems = asn1ElementsInObject (buf);
	if (elems < 2) {
		g_print ("%s[Error] Expecting at least 2 elements\n", padding);
		exit (0);
	}

	/* The BNCN is contained within an ASN1 SET */
	asn1Tag_t *set = (asn1Tag_t *) asn1ElementAtIndex (buf, 1);
	if (set->tagNumber != kASN1TagSET) {
		g_print ("%s[Error] Expecting SET\n", padding);
		exit (0);
	}

	/* Remove a few bytes from the start of the SET so we get to the next elem */
	set += asn1Len ((char *) set + 1).sizeBytes + 1;
	
	/* Check if the tag is private */
	if (set->tagClass != kASN1TagClassPrivate) {
		g_print ("%s[Error] Expecting Tag of private type\n", padding);
		exit (0);
	}
	
	/* Print the private tag, which should be "BNCN" */
	asn1PrintPrivtag (asn1GetPrivateTagnum (set++, 0), padding);
	g_print ("\n");

	/* Advance to the next value */
	set += asn1Len ((char *) set).sizeBytes + 1;
	elems += asn1ElementsInObject ((char *) set);

	/* Check that there is still two values left in the set */
	if (elems < 2) {
		g_print ("%s[Error] Expecting at least 2 values\n", padding);
		exit (0);
	}

	/* Print the tag key */
	asn1PrintIA5String ((asn1Tag_t *) asn1ElementAtIndex ((char *) set, 0), padding);
	g_print (": ");

	/* Print it's value */
	asn1PrintOctet ((asn1Tag_t *) asn1ElementAtIndex ((char *) set, 1), padding);
	g_print ("\n");

}


//////////////////////////////////////////////////////////////////
/*				    Image Decomp/Decrypt						*/
//////////////////////////////////////////////////////////////////


/**
 * 	img4_check_compression_type ()
 * 
 * 	Takes a given buffer, hopefully verified to be an im4p, and checks
 * 	the compression or encryption state of that buffer. The return value
 * 	is of the Image4CompressionType type. 
 * 
 * 	Args:
 * 		char *buf 				-	The verified im4p buffer
 * 
 * 	Return:
 * 		Image4CompressionType	-	The type of compression
 * 
 */
Image4CompressionType img4_check_compression_type (char *buf)
{
	/* Get the element count and ensure buf is an im4p */
	int c = asn1ElementsInObject (buf);
	if (c < 4) {
		g_print ("[Error] Not enough elements in given payload\n");
		exit (0);
	}

	/* Try to select the payload tag from the buffer */
	char *tag = asn1ElementAtIndex (buf, 3) + 1;
	asn1ElemLen_t len = asn1Len (tag);
	char *data = tag + len.sizeBytes;

	/* Check for either lzss or bvx2/lzfse */
	if (!strncmp (data, "complzss", 8)) {
		return IMG4_COMP_LZSS;
	} else if (!strncmp (data, "bvx2", 4)) {
		return IMG4_COMP_BVX2;
	} else if (!strncmp (data, "TEXT", 4)) {
		return IMG4_COMP_NONE;
	} else {
		return IMG4_COMP_ENCRYPTED;
	}

}


//////////////////////////////////////////////////////////////////
/*				    Image Extracting Func						*/
//////////////////////////////////////////////////////////////////


/**
 * 
 */
image4_t *img4_decompress_bvx2 (image4_t *img)
{
	char *tag = asn1ElementAtIndex (img->buf, 3) + 1;
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

	img->buf = malloc (dst_size);

	size_t uncomp_size = lzfse_decode_buffer ((uint8_t *) img->buf, dst_size,
											  (uint8_t *) data, src_size,
											  NULL);

	if (uncomp_size != dst_size) {
		g_print ("[Error] Expected to decompress %zu bytes but only got %zu bytes\n", dst_size, uncomp_size);
		exit(1);
	}			

	img->size = dst_size;
	
	return img;

}

/**
 * 
 */
void img4_extract_im4p (char *infile, char *outfile, char *ivkey)
{
	/* Load the image */
	image4_t *image = img4_read_image_from_path (infile);

	/* Check that the image was loaded properly */
	if (!image->size || !image->buf) {
		g_print ("[Error] There was an issue loading the file\n");
		exit (0);
	}

	/* Print some file information */
	g_print ("Loaded: \t%s\n", infile);
	g_print ("Image4 Type: \t%s\n", img4_string_for_image_type (image->type));
	g_print ("Component: \t%s\n\n", img4_get_component_name (image->buf));

	/* Create an image to hold the decompressed one */
	image4_t *newimage = malloc (sizeof (image4_t));

	/* Start decompression process */
	g_print ("[*] Detecting compression type...");

	/* Get compression type */
	Image4CompressionType comp = img4_check_compression_type (image->buf);
	if (comp == IMG4_COMP_BVX2) {

		/* Complete the first log */
		g_print (" bvx2\n");

		/* Decompress the payload */
		g_print ("[*] Decompressing im4p...");
		newimage = img4_decompress_bvx2 (image);

		/* Check if that was successful */
		if (!newimage->buf) {
			g_print (" error. There was a problem decompressing the im4p\n");
			exit (0);
		}

		/* Print success */
		g_print (" done!\n");

	} else {
		g_print ("\n[*] Cannot handle compression type. Exiting...\n");
		exit (0);
	}

	/* Print that we are now writing to the file */
	g_print ("[*] Writing decompressed payload to file: %s\n", outfile);

	/* Write the buffer to the outfile */
	FILE *o = fopen (outfile, "wb");
	fwrite (newimage->buf, newimage->size, 1, o);
	fclose (o);

	//g_print ("\nPlease run img4helper with --analyse to analyse the decompressed image.\n");

}



//////////////////////////////////////////////////////////////////
/*				    Image Printing Funcs						*/
//////////////////////////////////////////////////////////////////


/**
 * 	img4_print_with_type ()
 * 
 * 	Loads and prints a given Image4 at the filename location. The part of the
 * 	file that is printed (--print-all, --print-im4p, etc...) is determined by
 * 	print_type, this should not be confused with image->type, which although
 * 	the same type, image->type describes what Image4 variant the image is, and
 * 	print_type describes what portion of an image to print.
 * 
 * 	Args:
 * 		Image4Type print_type		-	The part of the Image4 to print
 * 		char *filename				-	The filename of the Image4 to be loaded.
 * 
 */	
void img4_print_with_type (Image4Type print_type, char *filename)
{
	/* Check the given filename is not NULL */
	if (!filename) {
		g_print ("[Error] No filename given\n");
		exit(0);
	}

	/* Create an image4_t for the filename */
	image4_t *image = img4_read_image_from_path (filename);
	char *magic = img4_string_for_image_type (image->type);

	/* Print some info contained in the file header */
	g_print ("Loaded: \t%s\n", filename);
	g_print ("Image4 Type: \t%s\n", magic);

	/* Switch through different print options */
	switch (print_type) {
		case IMG4_TYPE_ALL:

			/* Check what type of image we are dealing with */
			if (!strcmp (magic, "IMG4")) {

				/* A full Image4 has an IM4P, IM4M and IM4R */
				char *im4p = getIM4PFromIMG4 (image->buf);
				char *im4m = getIM4MFromIMG4 (image->buf);
				char *im4r = getIM4RFromIMG4 (image->buf);

				/* Print the component name so it is inline with the loaded and type */
				g_print ("Component:  \t%s\n\n", img4_get_component_name (im4p));

				/* Print the IMG4 banner */
				g_print ("IMG4: ------\n");

				/* Handle the im4p first */
				img4_handle_im4p (im4p, 1);

				/* Make some space between the two */
				g_print ("\n");

				/* Handle the im4m next */
				img4_handle_im4m (im4m, 1);

				/* More space */
				g_print ("\n");

				/* Finally, the im4r */
				img4_handle_im4r (im4r, 1);
				

			} else if (!strcmp (magic, "IM4P")) {

				/* Print the component type */
				g_print ("Component: \t%s\n\n", img4_get_component_name (image->buf));

				/* Handle the im4p */
				img4_handle_im4p (image->buf, 1);

			} else if (!strcmp (magic, "IM4M")) {

				/* Handle the im4m */
				g_print ("\n");
				img4_handle_im4m (image->buf, 1);

			} else if (!strcmp (magic, "IM4R")) {

				/* Handle the im4r */
				g_print ("\n");
				img4_handle_im4r (image->buf, 1);

			} else {

				/* Error */
				g_print ("[Error] Could not find an IMG4, IM4P, IM4M or IM4R\n");

			}

			break;

		default:
			break;

	}

}