#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#define MAX_COMMANDS 100
#define MAX_LINE_LENGTH 256

typedef enum {
    FILTER_GRAYSCALE,
    FILTER_INVERT,
    FILTER_BRIGHTNESS,
    FILTER_FLIP_H,
    FILTER_FLIP_V
} FilterType;

typedef struct {
    FilterType type;
    int param;
} FilterCommand;

int parse_script(FILE *script_file, FilterCommand commands[], int *command_count);
void print_command(const FilterCommand *command, int index);

#endif