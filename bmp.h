#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t B;
    uint8_t M;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_resolution;
    int32_t y_resolution;
    uint32_t colors_used;
    uint32_t important_colors;
} DIBHeader;
#pragma pack(pop)

typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Pixel;

long get_file_size(FILE *fp);
int read_bmp_headers(FILE *fp, BMPHeader *header, DIBHeader *dib_header);
unsigned char *read_header_block(FILE *fp, BMPHeader *header);
unsigned char *read_trailer_block(FILE *fp,
                                  BMPHeader *header,
                                  DIBHeader *dib_header,
                                  long file_size,
                                  long *trailer_size);
Pixel *read_pixels(FILE *fp, BMPHeader *header, DIBHeader *dib_header);
int write_bmp(const char *output_path,
              const unsigned char *header_block,
              const unsigned char *trailer_block,
              long trailer_size,
              BMPHeader *header,
              DIBHeader *dib_header,
              Pixel *pixels);

#endif