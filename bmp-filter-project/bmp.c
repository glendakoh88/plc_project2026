#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

static unsigned short read_u16_le(FILE *fp)
{
    int b0;
    int b1;

    b0 = fgetc(fp);
    b1 = fgetc(fp);

    if (b0 == EOF || b1 == EOF) {
        return 0;
    }

    return (unsigned short)((unsigned int)b0 | ((unsigned int)b1 << 8));
}

static unsigned int read_u32_le(FILE *fp)
{
    int b0;
    int b1;
    int b2;
    int b3;

    b0 = fgetc(fp);
    b1 = fgetc(fp);
    b2 = fgetc(fp);
    b3 = fgetc(fp);

    if (b0 == EOF || b1 == EOF || b2 == EOF || b3 == EOF) {
        return 0;
    }

    return (unsigned int)((unsigned int)b0 |
                         ((unsigned int)b1 << 8) |
                         ((unsigned int)b2 << 16) |
                         ((unsigned int)b3 << 24));
}

int parse_bmp_with_fsm(FILE *fp, BMPFileHeader *file_header, BMPInfoHeader *info_header)
{
    BMPState state;

    if (fp == NULL || file_header == NULL || info_header == NULL) {
        return 0;
    }

    state = BMP_STATE_START;

    while (state != BMP_STATE_DONE && state != BMP_STATE_ERROR) {
        switch (state) {
            case BMP_STATE_START:
                state = BMP_STATE_READ_SIGNATURE;
                break;

            case BMP_STATE_READ_SIGNATURE:
                file_header->bfType = read_u16_le(fp);

                if (ferror(fp) || feof(fp)) {
                    printf("Error: failed to read BMP signature.\n");
                    state = BMP_STATE_ERROR;
                } else if (file_header->bfType != 0x4D42) {
                    printf("Error: not a BMP file.\n");
                    state = BMP_STATE_ERROR;
                } else {
                    state = BMP_STATE_READ_FILE_HEADER;
                }
                break;

            case BMP_STATE_READ_FILE_HEADER:
                file_header->bfSize = read_u32_le(fp);
                file_header->bfReserved1 = read_u16_le(fp);
                file_header->bfReserved2 = read_u16_le(fp);
                file_header->bfOffBits = read_u32_le(fp);

                if (ferror(fp) || feof(fp)) {
                    printf("Error: failed to read BMP file header.\n");
                    state = BMP_STATE_ERROR;
                } else {
                    state = BMP_STATE_READ_INFO_HEADER;
                }
                break;

            case BMP_STATE_READ_INFO_HEADER:
                info_header->biSize = read_u32_le(fp);
                info_header->biWidth = (LONG)read_u32_le(fp);
                info_header->biHeight = (LONG)read_u32_le(fp);
                info_header->biPlanes = read_u16_le(fp);
                info_header->biBitCount = read_u16_le(fp);
                info_header->biCompression = read_u32_le(fp);
                info_header->biSizeImage = read_u32_le(fp);
                info_header->biXPelsPerMeter = (LONG)read_u32_le(fp);
                info_header->biYPelsPerMeter = (LONG)read_u32_le(fp);
                info_header->biClrUsed = read_u32_le(fp);
                info_header->biClrImportant = read_u32_le(fp);

                if (ferror(fp) || feof(fp)) {
                    printf("Error: failed to read BMP info header.\n");
                    state = BMP_STATE_ERROR;
                } else if (info_header->biSize != 40) {
                    printf("Error: unsupported BMP header format.\n");
                    state = BMP_STATE_ERROR;
                } else {
                    state = BMP_STATE_VALIDATE_PLANES;
                }
                break;

            case BMP_STATE_VALIDATE_PLANES:
                if (info_header->biPlanes != 1) {
                    printf("Error: invalid BMP planes value.\n");
                    state = BMP_STATE_ERROR;
                } else {
                    state = BMP_STATE_VALIDATE_BPP;
                }
                break;

            case BMP_STATE_VALIDATE_BPP:
                if (info_header->biBitCount != 24) {
                    printf("Error: only 24-bit BMP files are supported.\n");
                    state = BMP_STATE_ERROR;
                } else {
                    state = BMP_STATE_VALIDATE_COMPRESSION;
                }
                break;

            case BMP_STATE_VALIDATE_COMPRESSION:
                if (info_header->biCompression != 0) {
                    printf("Error: compressed BMP files are not supported.\n");
                    state = BMP_STATE_ERROR;
                } else {
                    state = BMP_STATE_VALIDATE_OFFSET;
                }
                break;

            case BMP_STATE_VALIDATE_OFFSET:
                if (file_header->bfOffBits < 54) {
                    printf("Error: invalid pixel data offset.\n");
                    state = BMP_STATE_ERROR;
                } else {
                    state = BMP_STATE_DONE;
                }
                break;

            case BMP_STATE_DONE:
            case BMP_STATE_ERROR:
            default:
                break;
        }
    }

    return (state == BMP_STATE_DONE);
}

void print_bmp_info(BMPFileHeader file_header, BMPInfoHeader info_header)
{
    printf("BMP info:\n");
    printf("Signature: 0x%X\n", file_header.bfType);
    printf("File size: %u\n", file_header.bfSize);
    printf("Pixel data offset: %u\n", file_header.bfOffBits);
    printf("DIB header size: %u\n", info_header.biSize);
    printf("Width: %d\n", info_header.biWidth);
    printf("Height: %d\n", info_header.biHeight);
    printf("Planes: %u\n", info_header.biPlanes);
    printf("Bits per pixel: %u\n", info_header.biBitCount);
    printf("Compression: %u\n", info_header.biCompression);
}

int load_bmp_image(FILE *fp, BMPFileHeader file_header, BMPInfoHeader info_header, Image *image)
{
    int width;
    int height;
    int abs_height;
    int row;
    int col;
    int target_row;
    int index;
    int padding;
    unsigned char pad[3];

    if (fp == NULL || image == NULL) {
        return 0;
    }

    width = (int)info_header.biWidth;
    height = (int)info_header.biHeight;

    if (width <= 0 || height == 0) {
        return 0;
    }

    abs_height = (height < 0) ? -height : height;
    padding = (4 - (width * 3) % 4) % 4;

    image->width = width;
    image->height = abs_height;
    image->data = (Pixel *)malloc((size_t)(width * abs_height) * sizeof(Pixel));

    if (image->data == NULL) {
        return 0;
    }

    if (fseek(fp, (long)file_header.bfOffBits, SEEK_SET) != 0) {
        free(image->data);
        image->data = NULL;
        return 0;
    }

    for (row = 0; row < abs_height; row++) {
        if (height > 0) {
            target_row = abs_height - 1 - row;
        } else {
            target_row = row;
        }

        for (col = 0; col < width; col++) {
            index = target_row * width + col;

            if (fread(&image->data[index], sizeof(Pixel), 1, fp) != 1) {
                free(image->data);
                image->data = NULL;
                return 0;
            }
        }

        if (padding > 0) {
            if (fread(pad, 1, (size_t)padding, fp) != (size_t)padding) {
                free(image->data);
                image->data = NULL;
                return 0;
            }
        }
    }

    return 1;
}

static void write_u16_le(FILE *fp, unsigned short value)
{
    fputc(value & 0xFF, fp);
    fputc((value >> 8) & 0xFF, fp);
}

static void write_u32_le(FILE *fp, unsigned int value)
{
    fputc(value & 0xFF, fp);
    fputc((value >> 8) & 0xFF, fp);
    fputc((value >> 16) & 0xFF, fp);
    fputc((value >> 24) & 0xFF, fp);
}

int save_bmp_image(const char *filename, BMPFileHeader file_header, BMPInfoHeader info_header, Image *image)
{
    FILE *fp;
    int width;
    int height;
    int row;
    int col;
    int index;
    int padding;
    unsigned char pad[3];

    if (filename == NULL || image == NULL || image->data == NULL) {
        return 0;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;
    }

    width = image->width;
    height = image->height;
    padding = (4 - (width * 3) % 4) % 4;

    info_header.biWidth = width;
    info_header.biHeight = height;
    info_header.biSizeImage = (DWORD)((width * 3 + padding) * height);
    file_header.bfSize = file_header.bfOffBits + info_header.biSizeImage;

    pad[0] = 0;
    pad[1] = 0;
    pad[2] = 0;

    write_u16_le(fp, file_header.bfType);
    write_u32_le(fp, file_header.bfSize);
    write_u16_le(fp, file_header.bfReserved1);
    write_u16_le(fp, file_header.bfReserved2);
    write_u32_le(fp, file_header.bfOffBits);

    write_u32_le(fp, info_header.biSize);
    write_u32_le(fp, (unsigned int)info_header.biWidth);
    write_u32_le(fp, (unsigned int)info_header.biHeight);
    write_u16_le(fp, info_header.biPlanes);
    write_u16_le(fp, info_header.biBitCount);
    write_u32_le(fp, info_header.biCompression);
    write_u32_le(fp, info_header.biSizeImage);
    write_u32_le(fp, (unsigned int)info_header.biXPelsPerMeter);
    write_u32_le(fp, (unsigned int)info_header.biYPelsPerMeter);
    write_u32_le(fp, info_header.biClrUsed);
    write_u32_le(fp, info_header.biClrImportant);

    for (row = height - 1; row >= 0; row--) {
        for (col = 0; col < width; col++) {
            index = row * width + col;

            if (fwrite(&image->data[index], sizeof(Pixel), 1, fp) != 1) {
                fclose(fp);
                return 0;
            }
        }

        if (padding > 0) {
            if (fwrite(pad, 1, (size_t)padding, fp) != (size_t)padding) {
                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);
    return 1;
}

void free_image(Image *image)
{
    if (image != NULL && image->data != NULL) {
        free(image->data);
        image->data = NULL;
    }
}