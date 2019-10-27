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

// Draft Kernelcache parser and analysis. This should eventually
// be moved to something like libdarwinhelper where it can be
// used in other tools more easily.

/**
 *  libhelper v0.1
 *  (c) h3adsh0tzzz
 */

#include "darwin.h"
#include <mach-o/loader.h>

void parse_header (FILE *f)
{
    fseek (f, 0, SEEK_END);
    size_t *fsize = ftell (f);
    fseek (f, 0, SEEK_SET);

    char *buf = malloc (fsize);
    if (buf) fread (buf, fsize, 1, f);

    g_print ("Loaded file %d bytes long.\n", fsize);


    /* */

    uint32_t magic;
    fseek (f, 0, SEEK_SET);
    fread(&magic, sizeof(uint32_t), 1, f);

    int is_64 = 0;
    if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64) {
        g_print ("64bit\n");
        is_64 = 1;
    } else if (magic == MH_MAGIC || magic == MH_CIGAM) {
        g_print ("32bit. Not interested yet!\n");
        exit (0);
    } else {
        exit (0);
    }

    struct mach_header_64 *mh_header = buf;

    //g_print ("magic: %s\n", (char *)mh_header->magic);

    switch (mh_header->cputype) {
        case CPU_TYPE_X86:
            g_print ("CPU type: x86\n");
            break;
        case CPU_TYPE_X86_64:
            g_print ("CPU type: x86_64\n");
            break;
        case CPU_TYPE_ARM:
            g_print ("CPU type: ARM\n");
            break;
        case CPU_TYPE_ARM64:
            g_print ("CPU type: ARM64\n");
            break;
        default:
            g_print ("CPU type: Unknown\n");
            break;
    }

}

void darwin_helper_test (char *path)
{
    /**
     *  Sorta just learning how to handle Mach-O files here, so
     *  these are my notes so i can start parsing them properly.
     *  
     *  Who knows, this might end up a blog post eventually? :P
     * 
     *  -----------------------------------------------
     * 
     *  Mach-O files are streams of bytes grouped into data chunks. There are three
     *  major regions, Header, Load Commands and Data.
     * 
     *  Header:
     *     
     *      The header contains file type information like architecture and flags that affect
     *      the interpretation of the rest of the file. The header, obviously, also identifies 
     *      the binary as a Mach-O file.
     * 
     *  Load Commands:
     * 
     *      The load commands directly follow the header. These load commands are variable in size
     *      and specify the layout and characteristics of a file. They specify initial virtual memory
     *      layout, symbol table location, initial execution state and any shared libraries.
     * 
     *      There can be more load commands. The kernelcache has `LC_SOURCE_VERSION` as an example.
     * 
     *  
     *      
     * 
     */

    // Load the file
    FILE *f = fopen (path, "rb");
    if (!f) {
        g_print ("Could not load file at all\n");
        exit(0);
    }

    parse_header (f);

    fclose (f);
}