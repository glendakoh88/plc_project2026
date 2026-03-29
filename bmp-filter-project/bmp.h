#ifndef BMP_H
#define BMP_H

#include <stdio.h>

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int   DWORD;
typedef int            LONG;

typedef struct {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BMPFileHeader;

typedef struct {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BMPInfoHeader;

typedef struct {
    BYTE b;
    BYTE g;
    BYTE r;
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel *data;
} Image;

typedef enum {
    BMP_STATE_START,
    BMP_STATE_READ_SIGNATURE,
    BMP_STATE_READ_FILE_HEADER,
    BMP_STATE_READ_INFO_HEADER,
    BMP_STATE_VALIDATE_PLANES,
    BMP_STATE_VALIDATE_BPP,
    BMP_STATE_VALIDATE_COMPRESSION,
    BMP_STATE_VALIDATE_OFFSET,
    BMP_STATE_DONE,
    BMP_STATE_ERROR
} BMPState;

int parse_bmp_with_fsm(FILE *fp, BMPFileHeader *file_header, BMPInfoHeader *info_header);
void print_bmp_info(BMPFileHeader file_header, BMPInfoHeader info_header);

int load_bmp_image(FILE *fp, BMPFileHeader file_header, BMPInfoHeader info_header, Image *image);
int save_bmp_image(const char *filename, BMPFileHeader file_header, BMPInfoHeader info_header, Image *image);
void free_image(Image *image);

#endif