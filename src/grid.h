#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

#include "../../AESLib/include/aes.h"

#include <stdbool.h>

extern bool win;
extern bool over;
extern bool quit;
extern struct aes_color_rgb digit_colors[8];

struct cell {
    bool is_mine;
    bool revealed;
    bool marked;
    short n_adj_mines;
};

struct grid {
    short width, height;
    struct cell **cells;
    short n_mines;
    struct cell **mines;
    short select_x, select_y;
    short empty;
};

struct grid *grid_create(short width, short height, short n_mines);
void grid_destroy(struct grid *grid);
void grid_init(struct grid *grid);
void grid_draw(struct grid *grid, struct aes_buffer *buffer);
void reveal_cell(struct grid *grid, short x, short y, struct aes_buffer *buffer);
void reveal_mines(struct cell **mines, short n_mines);

#endif
