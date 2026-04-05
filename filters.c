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

static void apply_sepia(Pixel *pixels, int width, int height)
{
    int i;
    int count;
    int new_r;
    int new_g;
    int new_b;
    unsigned char old_r;
    unsigned char old_g;
    unsigned char old_b;

    count = width * height;

    for (i = 0; i < count; i++) {
        old_r = pixels[i].r;
        old_g = pixels[i].g;
        old_b = pixels[i].b;

        new_r = (393 * (int)old_r + 769 * (int)old_g + 189 * (int)old_b) / 1000;
        new_g = (349 * (int)old_r + 686 * (int)old_g + 168 * (int)old_b) / 1000;
        new_b = (272 * (int)old_r + 534 * (int)old_g + 131 * (int)old_b) / 1000;

        pixels[i].r = clamp_to_byte(new_r);
        pixels[i].g = clamp_to_byte(new_g);
        pixels[i].b = clamp_to_byte(new_b);
    }
}

static void apply_threshold(Pixel *pixels, int width, int height, int threshold)
{
    int i;
    int count;
    int gray;
    unsigned char value;

    count = width * height;

    for (i = 0; i < count; i++) {
        gray = ((int)pixels[i].r + (int)pixels[i].g + (int)pixels[i].b) / 3;
        value = gray >= threshold ? 255 : 0;

        pixels[i].r = value;
        pixels[i].g = value;
        pixels[i].b = value;
    }
}

static void apply_contrast(Pixel *pixels, int width, int height, int amount)
{
    int i;
    int count;
    int factor_num;
    int factor_den;

    count = width * height;
    factor_num = 259 * (amount + 255);
    factor_den = 255 * (259 - amount);

    for (i = 0; i < count; i++) {
        pixels[i].r = clamp_to_byte((factor_num * ((int)pixels[i].r - 128)) / factor_den + 128);
        pixels[i].g = clamp_to_byte((factor_num * ((int)pixels[i].g - 128)) / factor_den + 128);
        pixels[i].b = clamp_to_byte((factor_num * ((int)pixels[i].b - 128)) / factor_den + 128);
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
            case FILTER_SEPIA:
                apply_sepia(pixels, width, height);
                break;
            case FILTER_THRESHOLD:
                apply_threshold(pixels, width, height, commands[i].param);
                break;
            case FILTER_CONTRAST:
                apply_contrast(pixels, width, height, commands[i].param);
                break;
            default:
                break;
        }
    }
}
