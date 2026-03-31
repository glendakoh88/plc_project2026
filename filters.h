#ifndef FILTERS_H
#define FILTERS_H

#include "bmp.h"
#include "parser.h"

void apply_filters(Pixel *pixels,
                   int width,
                   int height,
                   FilterCommand commands[],
                   int command_count);

#endif