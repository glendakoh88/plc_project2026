#ifndef FSM_H
#define FSM_H

#include "bmp.h"

typedef enum {
    START,
    B_OK,
    M_OK,
    FILE_SIZE_OK,
    DIB_SIZE_OK,
    PLANES_OK,
    BIT_DEPTH_OK,
    COMPRESSION_OK,
    OFFSET_OK,
    DIMENSIONS_OK,
    PIXEL_BOUNDS_OK,
    VALID,
    INVALID
} State;

typedef struct {
    State state;
} FSM;

typedef int (*CheckFunc)(BMPHeader *header, DIBHeader *dib_header, long file_size);

typedef struct {
    CheckFunc check;
    State next_if_ok;
    State next_if_fail;
} Transition;

int validate_bmp(BMPHeader *header, DIBHeader *dib_header, long file_size);

#endif
