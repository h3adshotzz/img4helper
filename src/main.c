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

#include <glib.h>
#include "img4.h"


/**
 * 	Main loop
 */
static GMainLoop *loop = NULL;

/**
 * 	Option menu
 */
static char *print_all = 0;
static char *print_im4p = 0;
static char *print_im4m = 0;
static char *print_im4r = 0;

static char *extract = 0;

static char *ivkey = 0;
static char *outfile = 0;

static int version = 0;

static GOptionEntry entries [] =
{
	/* Print options */
	{ "print-all", 'a', 0, G_OPTION_ARG_STRING, &print_all, "Print everything from the Image4 file.", NULL },
	{ "print-im4p", 'i', 0, G_OPTION_ARG_STRING, &print_im4p, "Print everything from the im4p (Providing there is one).", NULL },
	{ "print-im4m", 'm', 0, G_OPTION_ARG_STRING, &print_im4m, "Print everything from the im4m (Providing there is one).", NULL, },
	{ "print-im4r", 'r', 0, G_OPTION_ARG_STRING, &print_im4r, "Print everything from the im4r (Providing there is one).", NULL, },

	/* File operations */
	{ "extract", 'e', 0, G_OPTION_ARG_STRING, &extract, "Extract a payload from an IMG4 or IM4P (Use with --ivkey and --outfile).", NULL },

	/* Other options */
	{ "ivkey", 'k', 0, G_OPTION_ARG_STRING, &ivkey, "Specify an IVKEY pair to decrypt an im4p (Use with --extract and --outfile).", NULL },
	{ "outfile", 'o', 0, G_OPTION_ARG_STRING, &outfile, "Specify a file to write output too (Default outfile.raw, use with --extract", NULL },

	/* Check build info */
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &version, "View build info.", NULL },

	{ NULL, 0, 0, NULL, NULL, NULL }

};


/**
 * 	img4helper version notice. (Will also show beta tag)
 * 
 */
void version_tag ()
{
	
}


/**
 * 	Main
 *
 */
int main (int argc, char* argv[])
{
	GError *error = NULL;
	GOptionContext *context = NULL;

	/* Create a new loop */
	loop = g_main_loop_new (NULL, FALSE);

	/* Generate the help menu */
	context = g_option_context_new ("FILE");
	g_option_context_add_main_entries (context, entries, NULL);

	/* Parse command line args */
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_critical ("Failed: %s\n", error->message);
		exit(0);
	}

	/* Print banner */
	g_print ("----------------------------------------\n");
	g_print ("img4helper %s (c) @h3adsh0tzz 2019\n", VERSION_STRING);
	g_print ("----------------------------------------\n\n");


	/* Check if we are printing data */
	if (print_all) {
		img4_print_with_type (IMG4_TYPE_ALL, print_all);
		exit(1);
	} else if (print_im4p) {
		img4_print_with_type (IMG4_TYPE_IM4P, print_im4p);
		exit(1);
	} else if (print_im4m) {
		img4_print_with_type (IMG4_TYPE_IM4M, print_im4m);
		exit(1);
	} else if (print_im4r) {
		img4_print_with_type (IMG4_TYPE_IM4R, print_im4r);
		exit(1);
	}

	/* Check if we are extracting the payload */
	if (extract) {

		/* Check that an outfile name was specified, if not set out and notify */
		if (!outfile) {
			g_print ("[Error] No outfile specified. Will extract payload to outfile.raw\n");
			outfile = "outfile.raw";
		}

		/* Call the im4p extract function */
		img4_extract_im4p (extract, outfile, ivkey);

		exit (1);
	}

	/* We can only get to this point if nothing is set */
	g_print ("%s\n", g_option_context_get_help (context, TRUE, NULL));

	return 0;
}