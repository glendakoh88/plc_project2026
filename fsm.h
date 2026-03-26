typedef enum {
    START,
    B_OK,
    M_OK,
    FILE_SIZE_OK,
    DIB_SIZE_OK,
    PLANES_OK,
    BIT_DEPTH_OK,
    COMPRESSION_OK,
    INVALID
} State;

typedef struct {
    State state;
} FSM;

typedef int (*CheckFunc)(BMPHeader*, DIBHeader*, long);

typedef struct {
    CheckFunc check;
    State next_if_ok;
    State next_if_fail;
} Transition;
