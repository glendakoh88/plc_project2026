#include <stdio.h>
#include <string.h>
#include "bmp.h"
#include "parser.h"
#include "filters.h"

static void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s input.bmp script.txt output.bmp\n", program_name);
}

int main(int argc, char *argv[])
{
    BMPImage image;
    FilterScript script;
    int i;

    if (argc != 4)
    {
        print_usage(argv[0]);
        return 1;
    }

    if (!bmp_load(argv[1], &image))
    {
        return 1;
    }

    if (!parse_filter_script(argv[2], &script))
    {
        bmp_free(&image);
        return 1;
    }

    for (i = 0; i < script.count; i++)
    {
        if (!apply_filter(&image, &script.commands[i]))
        {
            free_filter_script(&script);
            bmp_free(&image);
            return 1;
        }
    }

    if (!bmp_save(argv[3], &image))
    {
        free_filter_script(&script);
        bmp_free(&image);
        return 1;
    }

    free_filter_script(&script);
    bmp_free(&image);

    return 0;
}