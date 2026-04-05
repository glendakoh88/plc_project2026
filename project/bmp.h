#ifndef BMP_H
#define BMP_H

#include <stdio.h>

/* ANSI C compatible basic integer aliases */
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned long  U32;
typedef signed long    S32;

typedef struct
{
    U8 b;
    U8 g;
    U8 r;
} Pixel;

typedef struct
{
    /* BITMAPFILEHEADER fields */
    U16 bfType;
    U32 bfSize;
    U16 bfReserved1;
    U16 bfReserved2;
    U32 bfOffBits;

    /* BITMAPINFOHEADER fields */
    U32 biSize;
    S32 biWidth;
    S32 biHeight;
    U16 biPlanes;
    U16 biBitCount;
    U32 biCompression;
    U32 biSizeImage;
    S32 biXPelsPerMeter;
    S32 biYPelsPerMeter;
    U32 biClrUsed;
    U32 biClrImportant;
} BMPHeader;

typedef struct
{
    BMPHeader header;
    Pixel *pixels;      /* width * height pixels */
    int width;
    int height;
    int row_padding;
} BMPImage;

typedef enum
{
    BMP_STATE_READ_SIGNATURE = 0,
    BMP_STATE_READ_FILE_HEADER,
    BMP_STATE_READ_INFO_HEADER,
    BMP_STATE_VALIDATE,
    BMP_STATE_DONE,
    BMP_STATE_ERROR
} BMPParseState;

/* Loads a 24-bit uncompressed BMP from disk */
int bmp_load(const char *filename, BMPImage *image);

/* Saves a BMP image back to disk */
int bmp_save(const char *filename, const BMPImage *image);

/* Frees allocated pixel memory */
void bmp_free(BMPImage *image);

/* Helper to access a pixel by x,y */
Pixel *bmp_get_pixel(BMPImage *image, int x, int y);

/* Validation helper */
int bmp_is_supported(const BMPHeader *header);

/* Prints a BMP-related error */
void bmp_print_error(const char *message);

#endif