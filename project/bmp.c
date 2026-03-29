#include "bmp.h"
#include <stdlib.h>
#include <string.h>

/* BMP compression value for uncompressed RGB */
#define BMP_COMPRESSION_RGB 0UL

static U16 read_u16_le(FILE *fp);
static U32 read_u32_le(FILE *fp);
static S32 read_s32_le(FILE *fp);
static int write_u16_le(FILE *fp, U16 value);
static int write_u32_le(FILE *fp, U32 value);
static int write_s32_le(FILE *fp, S32 value);
static int read_pixels(FILE *fp, BMPImage *image);
static int write_header(FILE *fp, const BMPHeader *header);
static int write_pixels(FILE *fp, const BMPImage *image);

void bmp_print_error(const char *message)
{
    fprintf(stderr, "BMP Error: %s\n", message);
}

void bmp_free(BMPImage *image)
{
    if (image == NULL)
    {
        return;
    }

    if (image->pixels != NULL)
    {
        free(image->pixels);
        image->pixels = NULL;
    }

    image->width = 0;
    image->height = 0;
    image->row_padding = 0;
}

Pixel *bmp_get_pixel(BMPImage *image, int x, int y)
{
    if (image == NULL || image->pixels == NULL)
    {
        return NULL;
    }

    if (x < 0 || x >= image->width || y < 0 || y >= image->height)
    {
        return NULL;
    }

    return &image->pixels[y * image->width + x];
}

int bmp_is_supported(const BMPHeader *header)
{
    if (header == NULL)
    {
        return 0;
    }

    if (header->bfType != 0x4D42)
    {
        return 0;
    }

    if (header->biSize != 40UL)
    {
        return 0;
    }

    if (header->biPlanes != 1)
    {
        return 0;
    }

    if (header->biBitCount != 24)
    {
        return 0;
    }

    if (header->biCompression != BMP_COMPRESSION_RGB)
    {
        return 0;
    }

    if (header->biWidth <= 0 || header->biHeight == 0)
    {
        return 0;
    }

    if (header->bfOffBits < 54UL)
    {
        return 0;
    }

    return 1;
}

int bmp_load(const char *filename, BMPImage *image)
{
    FILE *fp;
    BMPParseState state;

    if (filename == NULL || image == NULL)
    {
        bmp_print_error("invalid arguments to bmp_load");
        return 0;
    }

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        bmp_print_error("could not open input BMP file");
        return 0;
    }

    memset(image, 0, sizeof(BMPImage));
    state = BMP_STATE_READ_SIGNATURE;

    while (state != BMP_STATE_DONE && state != BMP_STATE_ERROR)
    {
        switch (state)
        {
            case BMP_STATE_READ_SIGNATURE:
                image->header.bfType = read_u16_le(fp);
                if (ferror(fp) || feof(fp))
                {
                    bmp_print_error("failed to read BMP signature");
                    state = BMP_STATE_ERROR;
                }
                else
                {
                    state = BMP_STATE_READ_FILE_HEADER;
                }
                break;

            case BMP_STATE_READ_FILE_HEADER:
                image->header.bfSize = read_u32_le(fp);
                image->header.bfReserved1 = read_u16_le(fp);
                image->header.bfReserved2 = read_u16_le(fp);
                image->header.bfOffBits = read_u32_le(fp);

                if (ferror(fp) || feof(fp))
                {
                    bmp_print_error("failed to read BMP file header");
                    state = BMP_STATE_ERROR;
                }
                else
                {
                    state = BMP_STATE_READ_INFO_HEADER;
                }
                break;

            case BMP_STATE_READ_INFO_HEADER:
                image->header.biSize = read_u32_le(fp);
                image->header.biWidth = read_s32_le(fp);
                image->header.biHeight = read_s32_le(fp);
                image->header.biPlanes = read_u16_le(fp);
                image->header.biBitCount = read_u16_le(fp);
                image->header.biCompression = read_u32_le(fp);
                image->header.biSizeImage = read_u32_le(fp);
                image->header.biXPelsPerMeter = read_s32_le(fp);
                image->header.biYPelsPerMeter = read_s32_le(fp);
                image->header.biClrUsed = read_u32_le(fp);
                image->header.biClrImportant = read_u32_le(fp);

                if (ferror(fp) || feof(fp))
                {
                    bmp_print_error("failed to read BMP info header");
                    state = BMP_STATE_ERROR;
                }
                else
                {
                    state = BMP_STATE_VALIDATE;
                }
                break;

            case BMP_STATE_VALIDATE:
                if (!bmp_is_supported(&image->header))
                {
                    bmp_print_error("unsupported BMP format (must be 24-bit uncompressed)");
                    state = BMP_STATE_ERROR;
                }
                else
                {
                    image->width = (int)image->header.biWidth;
                    image->height = (int)((image->header.biHeight < 0)
                                          ? -image->header.biHeight
                                          : image->header.biHeight);
                    image->row_padding = (4 - ((image->width * 3) % 4)) % 4;
                    state = BMP_STATE_DONE;
                }
                break;

            case BMP_STATE_DONE:
            case BMP_STATE_ERROR:
            default:
                break;
        }
    }

    if (state == BMP_STATE_ERROR)
    {
        fclose(fp);
        bmp_free(image);
        return 0;
    }

    if (!read_pixels(fp, image))
    {
        fclose(fp);
        bmp_free(image);
        return 0;
    }

    fclose(fp);
    return 1;
}

int bmp_save(const char *filename, const BMPImage *image)
{
    FILE *fp;
    BMPHeader header;
    U32 row_size;
    U32 image_size;

    if (filename == NULL || image == NULL || image->pixels == NULL)
    {
        bmp_print_error("invalid arguments to bmp_save");
        return 0;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        bmp_print_error("could not open output BMP file");
        return 0;
    }

    header = image->header;

    row_size = (U32)(image->width * 3 + image->row_padding);
    image_size = row_size * (U32)image->height;

    header.bfType = 0x4D42;
    header.bfOffBits = 54UL;
    header.biSize = 40UL;
    header.biWidth = (S32)image->width;
    header.biHeight = (S32)image->height;
    header.biPlanes = 1;
    header.biBitCount = 24;
    header.biCompression = BMP_COMPRESSION_RGB;
    header.biSizeImage = image_size;
    header.bfSize = header.bfOffBits + image_size;

    if (!write_header(fp, &header))
    {
        fclose(fp);
        bmp_print_error("failed to write BMP header");
        return 0;
    }

    if (!write_pixels(fp, image))
    {
        fclose(fp);
        bmp_print_error("failed to write BMP pixel data");
        return 0;
    }

    fclose(fp);
    return 1;
}

static int read_pixels(FILE *fp, BMPImage *image)
{
    int x;
    int y;
    int file_y;
    Pixel *pixel;
    unsigned char padding[3];

    if (fseek(fp, (long)image->header.bfOffBits, SEEK_SET) != 0)
    {
        bmp_print_error("invalid pixel data offset");
        return 0;
    }

    image->pixels = (Pixel *)malloc((size_t)(image->width * image->height * sizeof(Pixel)));
    if (image->pixels == NULL)
    {
        bmp_print_error("memory allocation failed for pixel data");
        return 0;
    }

    memset(image->pixels, 0, (size_t)(image->width * image->height * sizeof(Pixel)));

    /*
     * BMP with positive height stores rows bottom-up.
     * If height were negative, rows would be top-down.
     */
    for (y = 0; y < image->height; y++)
    {
        if (image->header.biHeight > 0)
        {
            file_y = image->height - 1 - y;
        }
        else
        {
            file_y = y;
        }

        for (x = 0; x < image->width; x++)
        {
            pixel = &image->pixels[file_y * image->width + x];

            if (fread(&pixel->b, 1, 1, fp) != 1 ||
                fread(&pixel->g, 1, 1, fp) != 1 ||
                fread(&pixel->r, 1, 1, fp) != 1)
            {
                bmp_print_error("failed to read pixel data");
                return 0;
            }
        }

        if (image->row_padding > 0)
        {
            if (fread(padding, 1, (size_t)image->row_padding, fp) != (size_t)image->row_padding)
            {
                bmp_print_error("failed to read row padding");
                return 0;
            }
        }
    }

    return 1;
}

static int write_pixels(FILE *fp, const BMPImage *image)
{
    int x;
    int y;
    int file_y;
    const Pixel *pixel;
    unsigned char padding[3];

    padding[0] = 0;
    padding[1] = 0;
    padding[2] = 0;

    for (y = 0; y < image->height; y++)
    {
        file_y = image->height - 1 - y;

        for (x = 0; x < image->width; x++)
        {
            pixel = &image->pixels[file_y * image->width + x];

            if (fwrite(&pixel->b, 1, 1, fp) != 1 ||
                fwrite(&pixel->g, 1, 1, fp) != 1 ||
                fwrite(&pixel->r, 1, 1, fp) != 1)
            {
                return 0;
            }
        }

        if (image->row_padding > 0)
        {
            if (fwrite(padding, 1, (size_t)image->row_padding, fp) != (size_t)image->row_padding)
            {
                return 0;
            }
        }
    }

    return 1;
}

static int write_header(FILE *fp, const BMPHeader *header)
{
    if (!write_u16_le(fp, header->bfType)) return 0;
    if (!write_u32_le(fp, header->bfSize)) return 0;
    if (!write_u16_le(fp, header->bfReserved1)) return 0;
    if (!write_u16_le(fp, header->bfReserved2)) return 0;
    if (!write_u32_le(fp, header->bfOffBits)) return 0;

    if (!write_u32_le(fp, header->biSize)) return 0;
    if (!write_s32_le(fp, header->biWidth)) return 0;
    if (!write_s32_le(fp, header->biHeight)) return 0;
    if (!write_u16_le(fp, header->biPlanes)) return 0;
    if (!write_u16_le(fp, header->biBitCount)) return 0;
    if (!write_u32_le(fp, header->biCompression)) return 0;
    if (!write_u32_le(fp, header->biSizeImage)) return 0;
    if (!write_s32_le(fp, header->biXPelsPerMeter)) return 0;
    if (!write_s32_le(fp, header->biYPelsPerMeter)) return 0;
    if (!write_u32_le(fp, header->biClrUsed)) return 0;
    if (!write_u32_le(fp, header->biClrImportant)) return 0;

    return 1;
}

static U16 read_u16_le(FILE *fp)
{
    unsigned char b0;
    unsigned char b1;

    b0 = 0;
    b1 = 0;

    fread(&b0, 1, 1, fp);
    fread(&b1, 1, 1, fp);

    return (U16)(b0 | ((U16)b1 << 8));
}

static U32 read_u32_le(FILE *fp)
{
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;

    b0 = b1 = b2 = b3 = 0;

    fread(&b0, 1, 1, fp);
    fread(&b1, 1, 1, fp);
    fread(&b2, 1, 1, fp);
    fread(&b3, 1, 1, fp);

    return (U32)b0 |
           ((U32)b1 << 8) |
           ((U32)b2 << 16) |
           ((U32)b3 << 24);
}

static S32 read_s32_le(FILE *fp)
{
    return (S32)read_u32_le(fp);
}

static int write_u16_le(FILE *fp, U16 value)
{
    unsigned char bytes[2];

    bytes[0] = (unsigned char)(value & 0xFF);
    bytes[1] = (unsigned char)((value >> 8) & 0xFF);

    return (fwrite(bytes, 1, 2, fp) == 2);
}

static int write_u32_le(FILE *fp, U32 value)
{
    unsigned char bytes[4];

    bytes[0] = (unsigned char)(value & 0xFF);
    bytes[1] = (unsigned char)((value >> 8) & 0xFF);
    bytes[2] = (unsigned char)((value >> 16) & 0xFF);
    bytes[3] = (unsigned char)((value >> 24) & 0xFF);

    return (fwrite(bytes, 1, 4, fp) == 4);
}

static int write_s32_le(FILE *fp, S32 value)
{
    return write_u32_le(fp, (U32)value);
}