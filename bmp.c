#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

long get_file_size(FILE *fp)
{
    long current_pos;
    long size;

    current_pos = ftell(fp);
    if (current_pos == -1L) {
        return -1L;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        return -1L;
    }

    size = ftell(fp);
    if (size == -1L) {
        return -1L;
    }

    if (fseek(fp, current_pos, SEEK_SET) != 0) {
        return -1L;
    }

    return size;
}

int read_bmp_headers(FILE *fp, BMPHeader *header, DIBHeader *dib_header)
{
    if (fp == NULL || header == NULL || dib_header == NULL) {
        return 0;
    }

    rewind(fp);

    if (fread(header, sizeof(BMPHeader), 1, fp) != 1) {
        return 0;
    }

    if (fread(dib_header, sizeof(DIBHeader), 1, fp) != 1) {
        return 0;
    }

    return 1;
}

unsigned char *read_header_block(FILE *fp, BMPHeader *header)
{
    unsigned char *buffer;
    size_t bytes_read;

    if (fp == NULL || header == NULL) {
        return NULL;
    }

    if (header->offset == 0) {
        return NULL;
    }

    buffer = (unsigned char *)malloc((size_t)header->offset);
    if (buffer == NULL) {
        return NULL;
    }

    rewind(fp);
    bytes_read = fread(buffer, 1, (size_t)header->offset, fp);
    if (bytes_read != (size_t)header->offset) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

unsigned char *read_trailer_block(FILE *fp,
                                  BMPHeader *header,
                                  DIBHeader *dib_header,
                                  long file_size,
                                  long *trailer_size)
{
    int width;
    int height;
    int abs_height;
    int row_size;
    long pixel_bytes;
    long trailer_offset;
    unsigned char *buffer;
    size_t bytes_read;

    if (fp == NULL || header == NULL || dib_header == NULL || trailer_size == NULL) {
        return NULL;
    }

    width = (int)dib_header->width;
    height = (int)dib_header->height;
    abs_height = height > 0 ? height : -height;
    row_size = ((width * 3 + 3) / 4) * 4;
    pixel_bytes = (long)row_size * (long)abs_height;
    trailer_offset = (long)header->offset + pixel_bytes;

    if (file_size < trailer_offset) {
        return NULL;
    }

    *trailer_size = file_size - trailer_offset;

    if (*trailer_size == 0) {
        return NULL;
    }

    buffer = (unsigned char *)malloc((size_t)(*trailer_size));
    if (buffer == NULL) {
        return NULL;
    }

    if (fseek(fp, trailer_offset, SEEK_SET) != 0) {
        free(buffer);
        return NULL;
    }

    bytes_read = fread(buffer, 1, (size_t)(*trailer_size), fp);
    if (bytes_read != (size_t)(*trailer_size)) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

Pixel *read_pixels(FILE *fp, BMPHeader *header, DIBHeader *dib_header)
{
    Pixel *pixels;
    int width;
    int height;
    int abs_height;
    int row_size;
    int padding;
    int row;
    int col;
    int file_row;
    unsigned char pad[3];

    if (fp == NULL || header == NULL || dib_header == NULL) {
        return NULL;
    }

    width = (int)dib_header->width;
    height = (int)dib_header->height;
    abs_height = height > 0 ? height : -height;

    pixels = (Pixel *)malloc((size_t)(width * abs_height) * sizeof(Pixel));
    if (pixels == NULL) {
        return NULL;
    }

    row_size = ((width * 3 + 3) / 4) * 4;
    padding = row_size - (width * 3);

    if (fseek(fp, (long)header->offset, SEEK_SET) != 0) {
        free(pixels);
        return NULL;
    }

    for (row = 0; row < abs_height; row++) {
        if (height > 0) {
            file_row = abs_height - 1 - row;
        } else {
            file_row = row;
        }

        for (col = 0; col < width; col++) {
            if (fread(&pixels[file_row * width + col], sizeof(Pixel), 1, fp) != 1) {
                free(pixels);
                return NULL;
            }
        }

        if (padding > 0) {
            if (fread(pad, 1, (size_t)padding, fp) != (size_t)padding) {
                free(pixels);
                return NULL;
            }
        }
    }

    return pixels;
}

int write_bmp(const char *output_path,
              const unsigned char *header_block,
              const unsigned char *trailer_block,
              long trailer_size,
              BMPHeader *header,
              DIBHeader *dib_header,
              Pixel *pixels)
{
    FILE *out;
    int width;
    int height;
    int abs_height;
    int row_size;
    int padding;
    int row;
    int col;
    int file_row;
    unsigned char pad[3];
    uint32_t new_image_size;
    uint32_t new_file_size;

    if (output_path == NULL || header_block == NULL || header == NULL ||
        dib_header == NULL || pixels == NULL) {
        return 0;
    }

    out = fopen(output_path, "wb");
    if (out == NULL) {
        return 0;
    }

    width = (int)dib_header->width;
    height = (int)dib_header->height;
    abs_height = height > 0 ? height : -height;

    row_size = ((width * 3 + 3) / 4) * 4;
    padding = row_size - (width * 3);

    pad[0] = 0;
    pad[1] = 0;
    pad[2] = 0;

    new_image_size = (uint32_t)(row_size * abs_height);
    new_file_size = (uint32_t)((long)header->offset + (long)new_image_size + trailer_size);

    header->size = new_file_size;
    dib_header->image_size = new_image_size;

    if (fwrite(header_block, 1, (size_t)header->offset, out) != (size_t)header->offset) {
        fclose(out);
        return 0;
    }

    if (fseek(out, 2L, SEEK_SET) != 0) {
        fclose(out);
        return 0;
    }
    if (fwrite(&header->size, sizeof(uint32_t), 1, out) != 1) {
        fclose(out);
        return 0;
    }

    if (fseek(out, 34L, SEEK_SET) != 0) {
        fclose(out);
        return 0;
    }
    if (fwrite(&dib_header->image_size, sizeof(uint32_t), 1, out) != 1) {
        fclose(out);
        return 0;
    }

    if (fseek(out, (long)header->offset, SEEK_SET) != 0) {
        fclose(out);
        return 0;
    }

    for (row = 0; row < abs_height; row++) {
        if (height > 0) {
            file_row = abs_height - 1 - row;
        } else {
            file_row = row;
        }

        for (col = 0; col < width; col++) {
            if (fwrite(&pixels[file_row * width + col], sizeof(Pixel), 1, out) != 1) {
                fclose(out);
                return 0;
            }
        }

        if (padding > 0) {
            if (fwrite(pad, 1, (size_t)padding, out) != (size_t)padding) {
                fclose(out);
                return 0;
            }
        }
    }

    if (trailer_size > 0 && trailer_block != NULL) {
        if (fwrite(trailer_block, 1, (size_t)trailer_size, out) != (size_t)trailer_size) {
            fclose(out);
            return 0;
        }
    }

    fclose(out);
    return 1;
}
