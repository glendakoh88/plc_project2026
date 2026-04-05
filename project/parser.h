#ifndef PARSER_H
#define PARSER_H

#include "filters.h"

#define MAX_FILTERS 256
#define MAX_LINE_LENGTH 256

typedef struct
{
    FilterCommand commands[MAX_FILTERS];
    int count;
} FilterScript;

int parse_filter_script(const char *filename, FilterScript *script);
void free_filter_script(FilterScript *script);

#endif