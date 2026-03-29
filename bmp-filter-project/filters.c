#include "filters.h"

void apply_invert(Image *image)
{
    int i;
    int total_pixels;

    if (image == NULL || image->data == NULL) {
        return;
    }

    total_pixels = image->width * image->height;

    for (i = 0; i < total_pixels; i++) {
        image->data[i].r = 255 - image->data[i].r;
        image->data[i].g = 255 - image->data[i].g;
        image->data[i].b = 255 - image->data[i].b;
    }
}

void apply_grayscale(Image *image)
{
    int i;
    int total_pixels;
    int gray;

    if (image == NULL || image->data == NULL) {
        return;
    }

    total_pixels = image->width * image->height;

    for (i = 0; i < total_pixels; i++) {
        gray = (image->data[i].r + image->data[i].g + image->data[i].b) / 3;
        image->data[i].r = (BYTE)gray;
        image->data[i].g = (BYTE)gray;
        image->data[i].b = (BYTE)gray;
    }
}