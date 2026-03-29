#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "filters.h"

int apply_script(const char *script_filename, Image *image)
{
    FILE *fp;
    char line[256];
    int line_number;

    fp = fopen(script_filename, "r");
    if (fp == NULL) {
        printf("Error: cannot open script file.\n");
        return 0;
    }

    line_number = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_number++;

        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0') {
            continue;
        }

        if (strcmp(line, "invert") == 0) {
            apply_invert(image);
        } else if (strcmp(line, "grayscale") == 0) {
            apply_grayscale(image);
        } else {
            printf("Error in script at line %d: unknown command '%s'\n", line_number, line);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}