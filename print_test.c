#include <stdio.h>
#include <stdlib.h>

#define ANSI_COLOR_WHITE            "\x1b[38;5;254m"
#define ANSI_COLOR_DARK_WHITE       "\x1b[38;5;251m"
#define ANSI_COLOR_DARK_GREY        "\x1b[38;5;243m"
#define ANSI_COLOR_YELLOW           "\x1b[38;5;214m"
#define ANSI_COLOR_DARK_YELLOW      "\x1b[38;5;94m"
#define ANSI_COLOR_RED              "\x1b[38;5;88m"
#define ANSI_COLOR_BLUE             "\x1b[38;5;32m"
#define ANSI_COLOR_BOLD             "\x1b[1m"
#define ANSI_COLOR_GREEN            "\x1b[32m"
#define ANSI_COLOR_MAGENTA          "\x1b[35m"
#define ANSI_COLOR_CYAN             "\x1b[36m"
#define ANSI_COLOR_RESET            "\x1b[0m"


void print_list_header (char *header)
{
    printf (ANSI_COLOR_RED "%s:\n" ANSI_COLOR_RESET, header);
}

void print_list_item (char *padding, char *descriptor, char *text)
{
    printf ("%s" ANSI_COLOR_BOLD ANSI_COLOR_DARK_WHITE "%s: " ANSI_COLOR_RESET
            ANSI_COLOR_DARK_GREY "%s\n" ANSI_COLOR_RESET, padding, descriptor, text);
}

int main()
{
    print_list_header ("Testing");
    print_list_item ("    ", "Item1", "data1");
    print_list_item ("    ", "Item2", "data2");
    print_list_item ("    ", "Item3", "data3");

    return 0;
}