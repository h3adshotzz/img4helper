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
 *	Main loop
 */
static GMainLoop *loop = NULL;


/**
 *	Option menu
 */
static char *print_im4p = 0;
static char *print_im4m = 0;
static char *print_all = 0;

static int show_beta_notice = 0;

static GOptionEntry entries[] =
{
	/* Print parts of an img4 */
	{ "print-all", 'a', 0, G_OPTION_ARG_STRING, &print_all, "Print everything from the Image4 file", NULL },
	{ "print-im4p", 'i', 0, G_OPTION_ARG_STRING, &print_im4p, "Print only the im4p", NULL },
	{ "print-im4m", 'm', 0, G_OPTION_ARG_STRING, &print_im4m, "Print only the im4m", NULL },

	/* Beta notice */
	{ "beta-notes", 0, 0, G_OPTION_ARG_NONE, &show_beta_notice, "Show notes for the current beta version", NULL },

	{ NULL, 0, 0, NULL, NULL, NULL, NULL }
};


void beta_notice ()
{
	g_print ("Please read the Github README for more information.\n");
	g_print ("Latest version can be downloaded from https://s3.cloud-itouk.org/dnlds/releases/img4helper/img4helper-darwinx86-latest.zip")
}


int main (int argc, char* argv[])
{
	GError *error = NULL;
	GOptionContext *context = NULL;

	int unknown_opt = 0;

	loop = g_main_loop_new (NULL, FALSE);

	// Create the help menu
	context = g_option_context_new ("FILE");
	g_option_context_add_main_entries (context, entries, NULL);

#if DEBUG
	// test
	for (int i = 0; i < argc; i++) {
		g_print("[%d]: %s\n", i, argv[i]);
	}
	g_print ("=========================== \n");
#endif 

	// Parse any args passed
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_critical ("Failed: %s\n", error->message);
		exit(1);
	}

	// Print version info
	g_print ("----------------------------------------\n");
	g_print ("img4helper %s (c) @h3adsh0tzz 2019\n", VERSION_STRING);
	g_print ("----------------------------------------\n\n");

	if (show_beta_notice) {
		beta_notice ();
		exit(1);
	}

	// Check for print args
	if (print_all) {
		print_img4(IMG4_PRINT_ALL, print_all);
		exit(1);
	} else if (print_im4p) {
		print_img4(IMG4_PRINT_IM4P, print_im4p);
		exit(1);
	} else if (print_im4m) {
        print_img4(IMG4_PRINT_IM4M, print_im4m);
		exit(1);
	}

	// If nothing is set, or what is given is not recognised, print the help
	g_option_context_get_help (context, TRUE, NULL);
	g_print ("Please run with --help to see the list of options\n");

	return 0;

}













