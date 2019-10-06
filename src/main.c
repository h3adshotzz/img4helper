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

static GOptionEntry entries[] =
{
	/* Print parts of an img4 */
	{ "print-all", 'a', 0, G_OPTION_ARG_STRING, &print_all, "Print everything from the Image4 file", NULL },
	{ "print-im4p", 'i', 0, G_OPTION_ARG_STRING, &print_im4p, "Print only the im4p", NULL },
	{ "print-im4m", 'm', 0, G_OPTION_ARG_STRING, &print_im4m, "Print only the im4m", NULL },

	{ NULL, 0, 0, NULL, NULL, NULL, NULL }
};


int main (int argc, char* argv[])
{
	GError *error = NULL;
	GOptionContext *context = NULL;

	int unknown_opt = 0;

	loop = g_main_loop_new (NULL, FALSE);

	// Create the help menu
	context = g_option_context_new ("FILE");
	g_option_context_add_main_entries (context, entries, NULL);

	// test
	for (int i = 0; i < argc; i++) {
		g_print("[%d]: %s\n", i, argv[i]);
	}

	// Parse any args passed
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_critical ("Failed: %s\n", error->message);
		exit(1);
	}

	// Check for print args
	if (print_all) {
		print_img4(IMG4_PRINT_ALL, print_all);
	} else if (print_im4p) {
		print_img4(IMG4_PRINT_IM4P, print_im4p);
	} else if (print_im4m) {
        print_img4(IMG4_PRINT_IM4M, print_im4m);
	}

	// If nothing is set, or what is given is not recognised, print the help
	if (!argc || unknown_opt) {
		g_option_context_get_help (context, TRUE, NULL);
	}

	// Run the main loop
	//g_main_loop_run (loop);

	return 0;

}













