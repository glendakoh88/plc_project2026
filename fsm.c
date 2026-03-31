#include "fsm.h"

static int check_B(BMPHeader *h, DIBHeader *d, long size)
{
    (void)d;
    (void)size;
    return h->B == 'B';
}

static int check_M(BMPHeader *h, DIBHeader *d, long size)
{
    (void)d;
    (void)size;
    return h->M == 'M';
}

static int check_file_size(BMPHeader *h, DIBHeader *d, long size)
{
    (void)d;
    return (long)h->size == size;
}

static int check_dib_size(BMPHeader *h, DIBHeader *d, long size)
{
    (void)h;
    (void)size;
    return d->header_size == 40 ||
           d->header_size == 52 ||
           d->header_size == 56 ||
           d->header_size == 108 ||
           d->header_size == 124;
}

static int check_planes(BMPHeader *h, DIBHeader *d, long size)
{
    (void)h;
    (void)size;
    return d->planes == 1;
}

static int check_bit_depth(BMPHeader *h, DIBHeader *d, long size)
{
    (void)h;
    (void)size;
    return d->bits == 24;
}

static int check_compression(BMPHeader *h, DIBHeader *d, long size)
{
    (void)h;
    (void)size;
    return d->compression == 0;
}

static int check_offset(BMPHeader *h, DIBHeader *d, long size)
{
    long min_offset;

    (void)size;
    min_offset = (long)sizeof(BMPHeader) + (long)d->header_size;
    return (long)h->offset >= min_offset;
}

static int check_dimensions(BMPHeader *h, DIBHeader *d, long size)
{
    (void)h;
    (void)size;
    return d->width > 0 && d->height != 0;
}

static Transition table[] = {
    {check_B, B_OK, INVALID},
    {check_M, M_OK, INVALID},
    {check_file_size, FILE_SIZE_OK, INVALID},
    {check_dib_size, DIB_SIZE_OK, INVALID},
    {check_planes, PLANES_OK, INVALID},
    {check_bit_depth, BIT_DEPTH_OK, INVALID},
    {check_compression, COMPRESSION_OK, INVALID},
    {check_offset, OFFSET_OK, INVALID},
    {check_dimensions, DIMENSIONS_OK, INVALID},
    {0, VALID, INVALID}
};

static void update_state(FSM *fsm, BMPHeader *header, DIBHeader *dib_header, long file_size)
{
    if (table[fsm->state].check == 0) {
        fsm->state = table[fsm->state].next_if_ok;
        return;
    }

    if (table[fsm->state].check(header, dib_header, file_size)) {
        fsm->state = table[fsm->state].next_if_ok;
    } else {
        fsm->state = table[fsm->state].next_if_fail;
    }
}

int validate_bmp(BMPHeader *header, DIBHeader *dib_header, long file_size)
{
    FSM fsm;

    fsm.state = START;

    while (fsm.state != INVALID && fsm.state != VALID) {
        update_state(&fsm, header, dib_header, file_size);
    }

    return fsm.state == VALID;
}