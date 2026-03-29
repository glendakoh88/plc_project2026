#include "filters.h"
#include <stdio.h>

static int clamp_channel(int value);
static int apply_grayscale(BMPImage *image);
static int apply_invert(BMPImage *image);
static int apply_brightness(BMPImage *image, int amount);
static int apply_flip_horizontal(BMPImage *image);
static int apply_flip_vertical(BMPImage *image);

static int clamp_channel(int value)
{
    if (value < 0)
    {
        return 0;
    }

    if (value > 255)
    {
        return 255;
    }

    return value;
}

static int apply_grayscale(BMPImage *image)
{
    int x;
    int y;
    int gray;
    Pixel *pixel;

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            pixel = bmp_get_pixel(image, x, y);
            if (pixel == NULL)
            {
                return 0;
            }

            gray = ((int)pixel->r + (int)pixel->g + (int)pixel->b) / 3;
            pixel->r = (U8)gray;
            pixel->g = (U8)gray;
            pixel->b = (U8)gray;
        }
    }

    return 1;
}

static int apply_invert(BMPImage *image)
{
    int x;
    int y;
    Pixel *pixel;

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            pixel = bmp_get_pixel(image, x, y);
            if (pixel == NULL)
            {
                return 0;
            }

            pixel->r = (U8)(255 - pixel->r);
            pixel->g = (U8)(255 - pixel->g);
            pixel->b = (U8)(255 - pixel->b);
        }
    }

    return 1;
}

static int apply_brightness(BMPImage *image, int amount)
{
    int x;
    int y;
    Pixel *pixel;

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            pixel = bmp_get_pixel(image, x, y);
            if (pixel == NULL)
            {
                return 0;
            }

            pixel->r = (U8)clamp_channel((int)pixel->r + amount);
            pixel->g = (U8)clamp_channel((int)pixel->g + amount);
            pixel->b = (U8)clamp_channel((int)pixel->b + amount);
        }
    }

    return 1;
}

static int apply_flip_horizontal(BMPImage *image)
{
    int y;
    int left;
    int right;
    Pixel *p1;
    Pixel *p2;
    Pixel temp;

    for (y = 0; y < image->height; y++)
    {
        left = 0;
        right = image->width - 1;

        while (left < right)
        {
            p1 = bmp_get_pixel(image, left, y);
            p2 = bmp_get_pixel(image, right, y);

            if (p1 == NULL || p2 == NULL)
            {
                return 0;
            }

            temp = *p1;
            *p1 = *p2;
            *p2 = temp;

            left++;
            right--;
        }
    }

    return 1;
}

static int apply_flip_vertical(BMPImage *image)
{
    int top;
    int bottom;
    int x;
    Pixel *p1;
    Pixel *p2;
    Pixel temp;

    top = 0;
    bottom = image->height - 1;

    while (top < bottom)
    {
        for (x = 0; x < image->width; x++)
        {
            p1 = bmp_get_pixel(image, x, top);
            p2 = bmp_get_pixel(image, x, bottom);

            if (p1 == NULL || p2 == NULL)
            {
                return 0;
            }

            temp = *p1;
            *p1 = *p2;
            *p2 = temp;
        }

        top++;
        bottom--;
    }

    return 1;
}

int apply_filter(BMPImage *image, const FilterCommand *command)
{
    if (image == NULL || command == NULL)
    {
        fprintf(stderr, "Filter Error: invalid filter call\n");
        return 0;
    }

    switch (command->type)
    {
        case FILTER_GRAYSCALE:
            return apply_grayscale(image);

        case FILTER_INVERT:
            return apply_invert(image);

        case FILTER_BRIGHTNESS:
            return apply_brightness(image, command->int_value);

        case FILTER_FLIP_HORIZONTAL:
            return apply_flip_horizontal(image);

        case FILTER_FLIP_VERTICAL:
            return apply_flip_vertical(image);

        default:
            fprintf(stderr, "Filter Error: unknown filter type\n");
            return 0;
    }
}