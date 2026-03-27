#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "fsm.h"

/*validation functions*/
int check_B(BMPHeader *h, DIBHeader *d, long size) { return h->B == 'B'; }
int check_M(BMPHeader *h, DIBHeader *d, long size) { return h->M == 'M'; }
int check_file_size(BMPHeader *h, DIBHeader *d, long size) { return h->size == size; }
int check_dib_size(BMPHeader *h, DIBHeader *d, long size) {return d->header_size == 12 || d->header_size == 40 || d->header_size == 52 ||d->header_size == 56 || d->header_size == 108 || d->header_size == 124;}
int check_planes(BMPHeader *h, DIBHeader *d, long size) { return d->planes == 1; }
int check_bit_depth(BMPHeader *h, DIBHeader *d, long size) {return d->bits == 1 || d->bits == 4 || d->bits == 8 ||d->bits == 16 || d->bits == 24 || d->bits == 32;}
int check_compression(BMPHeader *h, DIBHeader *d, long size) { return d->compression <= 3; }



Transition table[] = {
        {check_B, B_OK, INVALID}, /*START*/
        {check_M, M_OK, INVALID},/*B_OK*/
        {check_file_size, FILE_SIZE_OK, INVALID},/*M_OK*/
        {check_dib_size, DIB_SIZE_OK, INVALID},/*FILE_SIZE_OK*/
        {check_planes, PLANES_OK, INVALID},/*DIB_SIZE_OK*/
        {check_bit_depth, BIT_DEPTH_OK, INVALID},/*PLANES_OK*/
        {check_compression,COMPRESSION_OK, INVALID},/*BIT_DEPTH_OK*/
        {NULL, COMPRESSION_OK, INVALID},/*COMPRESSION_OK*/
    };

void update_state(FSM * fsm, BMPHeader * bmp_header, DIBHeader * dib_header,long file_size){
    if(table[fsm->state].check(bmp_header,dib_header,file_size)){
        fsm->state = table[fsm->state].next_if_ok;
    }else {fsm->state = table[fsm->state].next_if_fail;}

}

int main(void) {
    /* Open BMP file */
    FILE *bmp_file = fopen("./test.bmp", "rb");
    BMPHeader *header = (BMPHeader*)malloc(sizeof(BMPHeader));
    DIBHeader *dib_header = (DIBHeader*)malloc(sizeof(DIBHeader));
    long file_size;
    FSM fsm;

    if (!bmp_file) { perror("Failed to open file"); return 1; }
    
    fseek(bmp_file, 0, SEEK_END);
    file_size = ftell(bmp_file);
    rewind(bmp_file);

    /* read BMP and DIB headers*/
    fread(header, sizeof(BMPHeader), 1, bmp_file);
    fread(dib_header, sizeof(DIBHeader), 1, bmp_file);

    /* FSM validation using transition table*/
    fsm.state = START;
    while (fsm.state != INVALID && fsm.state != COMPRESSION_OK){
        update_state(&fsm, header, dib_header,file_size);
    }

    if (fsm.state != COMPRESSION_OK){
        printf("Corrupted bmp file \n");
        fclose(bmp_file);
        free(header);
        free(dib_header);
        exit(1);
    }

    printf("Valid bmp\n");
    fclose(bmp_file);
    free(header);
    free(dib_header);
    return 0;
}
