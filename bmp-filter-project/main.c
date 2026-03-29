#include <stdio.h>
#include "bmp.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    FILE *input_file;
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    Image image;

    image.data = NULL;

    if (argc != 4) {
        printf("Usage: ./bmpfilter <input.bmp> <script.txt> <output.bmp>\n");
        return 1;
    }

    input_file = fopen(argv[1], "rb");
    if (input_file == NULL) {
        printf("Error: cannot open input BMP file.\n");
        return 1;
    }

    if (!parse_bmp_with_fsm(input_file, &file_header, &info_header)) {
        fclose(input_file);
        return 1;
    }

    print_bmp_info(file_header, info_header);

    if (!load_bmp_image(input_file, file_header, info_header, &image)) {
        printf("Error: failed to load BMP pixel data.\n");
        fclose(input_file);
        return 1;
    }

    if (!apply_script(argv[2], &image)) {
        free_image(&image);
        fclose(input_file);
        return 1;
    }

    if (!save_bmp_image(argv[3], file_header, info_header, &image)) {
        printf("Error: failed to save output BMP file.\n");
        free_image(&image);
        fclose(input_file);
        return 1;
    }

    printf("Filtered image saved to %s\n", argv[3]);

    free_image(&image);
    fclose(input_file);
    return 0;
}