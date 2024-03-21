#include "grid.h"

#include "minesweeper.h"

#include <stdlib.h>
#include <time.h>

struct grid *grid_create(short width, short height, short n_mines) {
    static bool seeded = false;
    struct grid *grid;
    short i;
    
    grid = malloc(sizeof *grid);

    grid->cells = malloc(height * sizeof grid->cells);

    for (i = 0; i < height; i++) {
        grid->cells[i] = malloc(width * sizeof **(grid->cells));
    }

    grid->mines = malloc(n_mines * sizeof grid->mines);

    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    grid->width = width;
    grid->height = height;
    grid->n_mines = n_mines;

    return grid;
}

void grid_destroy(struct grid *grid) {
    short i;

    for (i = 0; i < grid->height; i++) {
        free(ms.grid->cells[i]);
    }

    free(ms.grid->cells);
    free(ms.grid->mines);
    free(ms.grid);

}

void grid_init(struct grid *grid) {
    short x, y, m, i, j, adj_x, adj_y;
    struct cell c;
    bool valid;

    grid->select_x = 0;
    grid->select_y = grid->height - 1;
    grid->empty = grid->width * grid->height - grid->n_mines;

    for (y = 0; y < grid->height; y++) {
        for (x = 0; x < grid->width; x++) {
            c.is_mine = false;
            c.marked = false;
            c.revealed = false;
            c.n_adj_mines = 0;

            grid->cells[y][x] = c;
        }
    }

    for (m = 0; m < grid->n_mines; m++) {
        do {
            x = rand() % grid->width;
            y = rand() % grid->height;

            valid = !grid->cells[y][x].is_mine;
        } while (!valid);

        grid->cells[y][x].is_mine = true;
        grid->mines[m] = &grid->cells[y][x];

        for (i = -1; i <= 1; i++) {
            adj_x = x + i;

            for (j = -1; j <= 1; j++) {
                adj_y = y + j;

                if (adj_x >= 0 &&
                    adj_x < grid->width &&
                    adj_y >= 0 &&
                    adj_y < grid->height &&
                    (i != 0 || j != 0))
                {
                    if (!grid->cells[adj_y][adj_x].is_mine) {
                        grid->cells[adj_y][adj_x].n_adj_mines++;
                    }
                }
            }
        }
    }
}

void grid_draw(struct grid *grid, struct aes_buffer *buffer) {
    struct cell c;
    struct aes_pixel p_empty, p_mine, p_select, p_reveal, p_marked;
    struct aes_color_rgb black, light_grey, grey, white, red;
    short x, y;

    black = aes_make_color(0, 0, 0);
    grey = aes_make_color(192, 192, 192);
    light_grey = aes_make_color(223, 223, 223);
    white = aes_make_color(255, 255, 255);
    red = aes_make_color(255, 0, 0);

    p_empty.c = ' ';
    p_empty.fg = white;
    p_empty.bg = light_grey;

    p_mine.c = '*';
    p_mine.fg = black;

    p_select.bg = white;
    p_select.fg = black;

    p_reveal.bg = grey;

    p_marked.c = '!';
    p_marked.fg = red;
    p_marked.bg = light_grey;

    for (y = 0; y < grid->height; y++) {
        for (x = 0; x < grid->width; x++) {
            c = grid->cells[y][x];

            if (c.revealed) {
                if (c.is_mine) {
                    if (!ms.win && grid->select_x == x && grid->select_y == y) {
                        p_mine.bg = red;
                    } else {
                        p_mine.bg = grey;
                    }

                    aes_set_pixel(x, y, p_mine, buffer);
                } else {
                    if (c.n_adj_mines) {
                        p_reveal.c = '0' + c.n_adj_mines;
                        p_reveal.fg = digit_colors[c.n_adj_mines - 1];
                    } else {
                        p_reveal.c = ' ';
                        p_reveal.fg = white;
                    }

                    aes_set_pixel(x, y, p_reveal, buffer);
                }
            } else if (c.marked) {
                aes_set_pixel(x, y, p_marked, buffer);
            } else {
                aes_set_pixel(x, y, p_empty, buffer);
            }

            if (x == grid->select_x && y == grid->select_y && !ms.over) {
                p_select.c = buffer->pixels[
                    grid->width * grid->select_y + grid->select_x
                ].c;
                p_select.fg = buffer->pixels[
                    grid->width * grid->select_y + grid->select_x
                ].fg;
                aes_set_pixel(x, y, p_select, buffer);
            }
        }
    }

    aes_set_cursor_home();
    aes_present(buffer);
}

void reveal_cell(struct grid *grid, short x, short y, struct aes_buffer *buffer) {
    if (grid->cells[y][x].revealed) {
        return;
    } else {
        grid->cells[y][x].revealed = true;
        grid->empty--;

        if (grid->cells[y][x].marked) {
            grid->cells[y][x].marked = false;
        }

        /* draw_grid(grid, *select, buffer);
        delay(1); */

        if (grid->cells[y][x].n_adj_mines > 0) {
            return;
        } else {
            if (y < grid->height - 1) {
                reveal_cell(grid, x, y + 1, buffer);
            }
            if (x < grid->width - 1) {
                reveal_cell(grid, x + 1, y, buffer);
            }
            if (y > 0) {
                reveal_cell(grid, x, y - 1, buffer);
            }
            if (x > 0) {
                reveal_cell(grid, x - 1, y, buffer);
            }
        }
    }
}

void reveal_mines(struct cell **mines, short n_mines) {
    short m;

    for (m = 0; m < n_mines; m++) {
        mines[m]->revealed = true;
    }
}


