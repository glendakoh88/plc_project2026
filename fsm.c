#include <limits.h>
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
    return (long)h->offset >= min_offset && (long)h->offset <= size;
}

static int check_dimensions(BMPHeader *h, DIBHeader *d, long size)
{
    (void)h;
    (void)size;
    return d->width > 0 && d->height != 0;
}

static int check_pixel_bounds(BMPHeader *h, DIBHeader *d, long file_size)
{
    long width;
    long height;
    long abs_height;
    long row_size;
    long pixel_bytes;
    long offset;

    width = (long)d->width;
    height = (long)d->height;
    abs_height = height > 0 ? height : -height;
    offset = (long)h->offset;

    if (width <= 0 || abs_height <= 0) {
        return 0;
    }

    if (width > (LONG_MAX - 3L) / 3L) {
        return 0;
    }

    row_size = ((width * 3L + 3L) / 4L) * 4L;

    if (abs_height > 0 && row_size > LONG_MAX / abs_height) {
        return 0;
    }

    pixel_bytes = row_size * abs_height;

    if (offset < 0 || offset > file_size) {
        return 0;
    }

    if (pixel_bytes < 0) {
        return 0;
    }

    if (offset > file_size - pixel_bytes) {
        return 0;
    }

    return 1;
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
    {check_pixel_bounds, PIXEL_BOUNDS_OK, INVALID},
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