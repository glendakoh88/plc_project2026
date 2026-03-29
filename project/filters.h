#ifndef FILTERS_H
#define FILTERS_H

#include "bmp.h"

typedef enum
{
    FILTER_GRAYSCALE = 0,
    FILTER_INVERT,
    FILTER_BRIGHTNESS,
    FILTER_FLIP_HORIZONTAL,
    FILTER_FLIP_VERTICAL
} FilterType;

typedef struct
{
    FilterType type;
    int int_value;
} FilterCommand;

int apply_filter(BMPImage *image, const FilterCommand *command);

#endif