#include "filters.h"

static unsigned char clamp_to_byte(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > 255) {
        return 255;
    }
    return (unsigned char)value;
}

static void apply_grayscale(Pixel *pixels, int width, int height)
{
    int i;
    int total;
    int count;
    unsigned char gray;

    count = width * height;

    for (i = 0; i < count; i++) {
        total = (int)pixels[i].r + (int)pixels[i].g + (int)pixels[i].b;
        gray = (unsigned char)(total / 3);
        pixels[i].r = gray;
        pixels[i].g = gray;
        pixels[i].b = gray;
    }
}

static void apply_invert(Pixel *pixels, int width, int height)
{
    int i;
    int count;

    count = width * height;

    for (i = 0; i < count; i++) {
        pixels[i].r = (unsigned char)(255 - pixels[i].r);
        pixels[i].g = (unsigned char)(255 - pixels[i].g);
        pixels[i].b = (unsigned char)(255 - pixels[i].b);
    }
}

static void apply_brightness(Pixel *pixels, int width, int height, int amount)
{
    int i;
    int count;

    count = width * height;

    for (i = 0; i < count; i++) {
        pixels[i].r = clamp_to_byte((int)pixels[i].r + amount);
        pixels[i].g = clamp_to_byte((int)pixels[i].g + amount);
        pixels[i].b = clamp_to_byte((int)pixels[i].b + amount);
    }
}

static void apply_flip_horizontal(Pixel *pixels, int width, int height)
{
    int row;
    int left;
    int right;
    Pixel temp;

    for (row = 0; row < height; row++) {
        left = 0;
        right = width - 1;

        while (left < right) {
            temp = pixels[row * width + left];
            pixels[row * width + left] = pixels[row * width + right];
            pixels[row * width + right] = temp;
            left++;
            right--;
        }
    }
}

static void apply_flip_vertical(Pixel *pixels, int width, int height)
{
    int top;
    int bottom;
    int col;
    Pixel temp;

    top = 0;
    bottom = height - 1;

    while (top < bottom) {
        for (col = 0; col < width; col++) {
            temp = pixels[top * width + col];
            pixels[top * width + col] = pixels[bottom * width + col];
            pixels[bottom * width + col] = temp;
        }
        top++;
        bottom--;
    }
}

void apply_filters(Pixel *pixels,
                   int width,
                   int height,
                   FilterCommand commands[],
                   int command_count)
{
    int i;

    for (i = 0; i < command_count; i++) {
        switch (commands[i].type) {
            case FILTER_GRAYSCALE:
                apply_grayscale(pixels, width, height);
                break;
            case FILTER_INVERT:
                apply_invert(pixels, width, height);
                break;
            case FILTER_BRIGHTNESS:
                apply_brightness(pixels, width, height, commands[i].param);
                break;
            case FILTER_FLIP_H:
                apply_flip_horizontal(pixels, width, height);
                break;
            case FILTER_FLIP_V:
                apply_flip_vertical(pixels, width, height);
                break;
            default:
                break;
        }
    }
}