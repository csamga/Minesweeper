#include "../../AESLib/include/aes.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct cell {
    bool is_mine;
    bool revealed;
    bool marked;
    short n_adj_mines;
    short x, y;
};

struct grid {
    short width, height;
    struct cell **cells;
    short n_mines;
    struct cell **mines;
};

static bool over;
static bool win;
static bool quit;
static short empty;
static struct aes_color_rgb colors[8] = {
    {0, 0, 255},
    {0, 128, 0},
    {255, 0, 0},
    {0, 0, 128},
    {128, 0, 0},
    {0, 128, 128},
    {128, 128, 0},
    {128, 0, 128}
};

void init_grid(struct grid *grid) {
    static bool seeded = false;
    short x, y, m, i, j, adj_x, adj_y;
    struct cell c;
    bool valid;

    for (y = 0; y < grid->height; y++) {
        for (x = 0; x < grid->width; x++) {
            c.is_mine = false;
            c.marked = false;
            c.revealed = false;
            c.n_adj_mines = 0;
            c.x = x;
            c.y = y;

            grid->cells[y][x] = c;
        }
    }

    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
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

void draw_grid(
    struct grid *grid,
    struct cell select,
    struct aes_buffer *buffer)
{
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
                    if (!win && select.x == x && select.y == y) {
                        p_mine.bg = red;
                    } else {
                        p_mine.bg = grey;
                    }

                    aes_set_pixel(x, y, p_mine, buffer);
                } else {
                    if (c.n_adj_mines) {
                        p_reveal.c = '0' + c.n_adj_mines;
                        p_reveal.fg = colors[c.n_adj_mines - 1];
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

            if (x == select.x && y == select.y && !over) {
                p_select.c = buffer->pixels[
                    grid->width * select.y + select.x
                ].c;
                p_select.fg = buffer->pixels[
                    grid->width * select.y + select.x
                ].fg;
                aes_set_pixel(x, y, p_select, buffer);
            }
        }
    }

    aes_set_cursor_home();
    aes_present(buffer);
}

void reveal_cell(
    struct grid *grid,
    short x,
    short y,
    struct cell *select,
    struct aes_buffer *buffer)
{
    if (grid->cells[y][x].revealed) {
        return;
    } else {
        grid->cells[y][x].revealed = true;
        empty--;

        if (grid->cells[y][x].marked) {
            grid->cells[y][x].marked = false;
        }

        draw_grid(grid, *select, buffer);
        delay(1);

        if (grid->cells[y][x].n_adj_mines > 0) {
            return;
        } else {
            if (y < grid->height - 1) {
                reveal_cell(grid, x, y + 1, select, buffer);
            }
            if (x < grid->width - 1) {
                reveal_cell(grid, x + 1, y, select, buffer);
            }
            if (y > 0) {
                reveal_cell(grid, x, y - 1, select, buffer);
            }
            if (x > 0) {
                reveal_cell(grid, x - 1, y, select, buffer);
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

void process_input(
    struct grid *grid,
    struct cell *select,
    struct aes_buffer *buffer)
{
    char c;
    struct cell *selected;

    selected = &grid->cells[select->y][select->x];

    c = getchar();

    switch (c) {
    case 'z':
    case 'Z':
        select->y++;
        select->y %= grid->height;
        break;
    case 's':
    case 'S':
        select->y--;
        select->y += grid->height * (select->y < 0);
        break;
    case 'q':
    case 'Q':
        select->x--;
        select->x += grid->width * (select->x < 0);
        break;
    case 'd':
    case 'D': 
        select->x++;
        select->x %= grid->width;
        break;
    case 'm':
    case 'M':
        selected->marked = !selected->revealed && !selected->marked;
        break;
    case '\x0a':
        if (selected->is_mine) {
            over = true;
        } else {
            reveal_cell(grid, select->x, select->y, select, buffer);
        }
        break;
    case '\x1b':
        quit = true;
        break;
    }
}

int main(void) {
    struct grid grid;
    struct cell select;
    struct aes_buffer *buffer;
    short i;
    char c;

    puts("Choose difficulty :");
    printf("1: %*s, %*s mines\n", 5, "8x8", 3, "10");
    printf("2: %*s, %*s mines\n", 5, "16x16", 3, "40");
    printf("3: %*s, %*s mines\n", 5, "32x32", 3, "100");

    c = getchar();

    switch (c) {
    case '1':
        grid.width = 8;
        grid.height = 8;
        grid.n_mines = 10;
        break;
    case '2':
        grid.width = 16;
        grid.height = 16;
        grid.n_mines = 40;
        break;
    case '3':
        grid.width = 32;
        grid.height = 32;
        grid.n_mines = 100;
        break;
    default:
        grid.width = 16;
        grid.height = 16;
        grid.n_mines = 40;
    };

    getchar();

    aes_term_setup(
        AES_HIDE_CURSOR |
        AES_IMMEDIATE_INPUT |
        AES_NO_ECHO |
        AES_SWITCH_BUFFERS
    );

    buffer = aes_buffer_alloc(grid.width, grid.height, true);

    grid.cells = malloc(grid.height * sizeof grid.cells);

    for (i = 0; i < grid.height; i++) {
        grid.cells[i] = malloc(grid.width * sizeof **(grid.cells));
    }

    grid.mines = malloc(grid.n_mines * sizeof grid.mines);

    do {
        init_grid(&grid);
        select = grid.cells[0][0];

        empty = grid.width * grid.height - grid.n_mines;
        over = false;
        win = false;

        while (!over && !win && !quit) {
            draw_grid(&grid, select, buffer);
            process_input(&grid, &select, buffer);

            if (empty == 0) {
                win = true;
            }
        }

        if (!quit) {
            reveal_mines(grid.mines, grid.n_mines);
            draw_grid(&grid, select, buffer);

            if (win) {
                puts("you won");
            } else {
                puts("you lost");
            }

            puts("Replay ? [Y/n]");
            c = getchar();

            switch (c) {
            case 'y':
            case 'Y':
            case '\x0a':
                quit = false;
                break;
            case 'n':
            case 'N':
            case '\x1b':
                quit = true;
                break;
            }
        }

        aes_clear_screen();
    } while (!quit);

    for (i = 0; i < grid.height; i++) {
        free(grid.cells[i]);
    }

    free(grid.cells);
    free(grid.mines);
    aes_buffer_free(buffer);

    aes_term_reset();

    return 0;
}
