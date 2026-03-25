#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "fsm.h"



int check_B(BMPHeader *h, DIBHeader *d, long size) { return h->B == 'B'; }
int check_M(BMPHeader *h, DIBHeader *d, long size) { return h->M == 'M'; }
int check_file_size(BMPHeader *h, DIBHeader *d, long size) { return h->size == size; }
int check_dib_size(BMPHeader *h, DIBHeader *d, long size) {return d->header_size == 12 || d->header_size == 40 || d->header_size == 52 ||d->header_size == 56 || d->header_size == 108 || d->header_size == 124;}
int check_planes(BMPHeader *h, DIBHeader *d, long size) { return d->planes == 1; }
int check_bit_depth(BMPHeader *h, DIBHeader *d, long size) {return d->bits == 1 || d->bits == 4 || d->bits == 8 ||d->bits == 16 || d->bits == 24 || d->bits == 32;}
int check_compression(BMPHeader *h, DIBHeader *d, long size) { return d->compression <= 3; }


int process_fsm(BMPHeader *header, DIBHeader *dib_header, long file_size) {
    FSM machine;

    Transition fsm[] = {
        {START, NULL, CHECK_B, INVALID},
        {CHECK_B, check_B, CHECK_M, INVALID},
        {CHECK_M, check_M, CHECK_FILE_SIZE, INVALID},
        {CHECK_FILE_SIZE, check_file_size, CHECK_DIB_SIZE, INVALID},
        {CHECK_DIB_SIZE, check_dib_size, CHECK_PLANES, INVALID},
        {CHECK_PLANES, check_planes, CHECK_BIT_DEPTH, INVALID},
        {CHECK_BIT_DEPTH, check_bit_depth, CHECK_COMPRESSION, INVALID},
        {CHECK_COMPRESSION, check_compression, VALID, INVALID}
    };

    machine.state = START;
    while (machine.state != VALID && machine.state != INVALID) {
        Transition t = fsm[machine.state];
        if (!t.check || t.check(header, dib_header, file_size)) {
            machine.state = t.next_if_ok;
        } else {
            machine.state = t.next_if_fail;
        }
    }

    return machine.state == VALID;
}

int main(void) {
    long file_size;
    int valid;
    BMPHeader *header = (BMPHeader*)malloc(sizeof(BMPHeader));
    DIBHeader *dib_header = (DIBHeader*)malloc(sizeof(DIBHeader));

    /* Open BMP file */
    FILE *bmp_file = fopen("./test.bmp", "rb");
    if (!bmp_file) { perror("Failed to open file"); return 1; }
    
    fseek(bmp_file, 0, SEEK_END);
    file_size = ftell(bmp_file);
    rewind(bmp_file);

    if (!header || !dib_header) { perror("malloc failed"); return 1; }

    if (fread(header, sizeof(BMPHeader), 1, bmp_file) != 1 ||
        fread(dib_header, sizeof(DIBHeader), 1, bmp_file) != 1) {
        printf("Failed to read headers\n");
        fclose(bmp_file);
        free(header);
        free(dib_header);
        return 1;
    }

    
    /* FSM validation */
    valid = process_fsm(header, dib_header, file_size);

    /* Print result */
    if (valid) {
        printf("BMP file is valid\n");
    } else {
        printf("Corrupted BMP file\n");
    }


    fclose(bmp_file);
    free(header);
    free(dib_header);
    return 0;
}
