typedef enum {
    START,
    CHECK_B,
    CHECK_M,
    CHECK_FILE_SIZE,
    CHECK_DIB_SIZE,
    CHECK_PLANES,
    CHECK_BIT_DEPTH,
    CHECK_COMPRESSION,
    VALID,
    INVALID
} State;

typedef struct {
    State state;
} FSM;

typedef int (*CheckFunc)(BMPHeader*, DIBHeader*, long);

typedef struct {
    State current;
    CheckFunc check;
    State next_if_ok;
    State next_if_fail;
} Transition;
