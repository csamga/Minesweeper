#ifndef MINESWEEPER_H_INCLUDED
#define MINESWEEPER_H_INCLUDED

#include "grid.h"

#include "../../AESLib/include/aes.h"

#include <stdbool.h>

struct minesweeper {
    struct grid *grid;
    bool quit;
    bool over;
    bool win;
    struct aes_buffer *buffer;
};

extern struct minesweeper ms;
extern struct aes_color_rgb digit_colors[8];

void minesweeper_run(void);

#endif
