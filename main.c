#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "fsm.h"
#include "parser.h"
#include "filters.h"

int main(int argc, char *argv[])
{
    char *input_bmp_path;
    char *script_path;
    char *output_bmp_path;
    FILE *bmp_file;
    FILE *script_file;
    BMPHeader header;
    DIBHeader dib_header;
    long file_size;
    FilterCommand commands[MAX_COMMANDS];
    int command_count;
    unsigned char *header_block;
    unsigned char *trailer_block;
    long trailer_size;
    Pixel *pixels;
    int width;
    int height;
    int abs_height;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s input.bmp filters.txt output.bmp\n", argv[0]);
        return 1;
    }

    input_bmp_path = argv[1];
    script_path = argv[2];
    output_bmp_path = argv[3];

    bmp_file = fopen(input_bmp_path, "rb");
    if (bmp_file == NULL) {
        perror("Failed to open input BMP file");
        return 1;
    }

    file_size = get_file_size(bmp_file);
    if (file_size < 0) {
        fprintf(stderr, "Failed to determine BMP file size\n");
        fclose(bmp_file);
        return 1;
    }

    if (!read_bmp_headers(bmp_file, &header, &dib_header)) {
        fprintf(stderr, "Failed to read BMP headers\n");
        fclose(bmp_file);
        return 1;
    }

    if (!validate_bmp(&header, &dib_header, file_size)) {
        fprintf(stderr, "Invalid or unsupported BMP format\n");
        fclose(bmp_file);
        return 1;
    }

    printf("Valid BMP file\n");

    script_file = fopen(script_path, "r");
    if (script_file == NULL) {
        perror("Failed to open filter script");
        fclose(bmp_file);
        return 1;
    }

    command_count = 0;
    if (!parse_script(script_file, commands, &command_count)) {
        fclose(script_file);
        fclose(bmp_file);
        return 1;
    }

    printf("Parsed %d command(s) successfully\n", command_count);

    header_block = read_header_block(bmp_file, &header);
    if (header_block == NULL) {
        fprintf(stderr, "Failed to read BMP header block\n");
        fclose(script_file);
        fclose(bmp_file);
        return 1;
    }

    trailer_size = 0;
    trailer_block = read_trailer_block(bmp_file, &header, &dib_header, file_size, &trailer_size);

    pixels = read_pixels(bmp_file, &header, &dib_header);
    if (pixels == NULL) {
        fprintf(stderr, "Failed to read pixel data\n");
        free(header_block);
        if (trailer_block != NULL) {
            free(trailer_block);
        }
        fclose(script_file);
        fclose(bmp_file);
        return 1;
    }

    width = (int)dib_header.width;
    height = (int)dib_header.height;
    abs_height = height > 0 ? height : -height;

    apply_filters(pixels, width, abs_height, commands, command_count);

    if (!write_bmp(output_bmp_path,
                   header_block,
                   trailer_block,
                   trailer_size,
                   &header,
                   &dib_header,
                   pixels)) {
        fprintf(stderr, "Failed to write output BMP\n");
        free(pixels);
        free(header_block);
        if (trailer_block != NULL) {
            free(trailer_block);
        }
        fclose(script_file);
        fclose(bmp_file);
        return 1;
    }

    printf("Output written successfully to %s\n", output_bmp_path);

    free(pixels);
    free(header_block);
    if (trailer_block != NULL) {
        free(trailer_block);
    }
    fclose(script_file);
    fclose(bmp_file);

    return 0;
}
