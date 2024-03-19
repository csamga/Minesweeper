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


void init_grid(
    struct cell *grid[],
    short width,
    short height,
    short n_mines,
    struct cell **mines)
{
    static bool seeded = false;
    short x, y, m, i, j, adj_x, adj_y;
    struct cell c;
    bool valid;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            c.is_mine = false;
            c.marked = false;
            c.revealed = false;
            c.n_adj_mines = 0;
            c.x = x;
            c.y = y;

            grid[y][x] = c;
        }
    }

    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    for (m = 0; m < n_mines; m++) {
        do {
            x = rand() % width;
            y = rand() % height;

            valid = !grid[y][x].is_mine;
        } while (!valid);

        grid[y][x].is_mine = true;
        mines[m] = &grid[y][x];

        for (i = -1; i <= 1; i++) {
            adj_x = x + i;

            for (j = -1; j <= 1; j++) {
                adj_y = y + j;

                if (adj_x >= 0 &&
                    adj_x < width &&
                    adj_y >= 0 &&
                    adj_y < height &&
                    (i != 0 || j != 0))
                {
                    if (!grid[adj_y][adj_x].is_mine) {
                        grid[adj_y][adj_x].n_adj_mines++;
                    }
                }
            }
        }
    }
}

void draw_grid(
    struct cell **grid,
    short width,
    short height,
    struct cell select,
    struct aes_buffer *buffer)
{
    struct cell c;
    struct aes_pixel p_empty, p_mine, p_select, p_reveal;
    struct aes_color_rgb black, light_grey, grey, white, red, green;
    short w, h;

    black = aes_make_color(0, 0, 0);
    grey = aes_make_color(192, 192, 192);
    light_grey = aes_make_color(223, 223, 223);
    white = aes_make_color(255, 255, 255);
    red = aes_make_color(255, 0, 0);
    green = aes_make_color(0, 255, 0);

    p_empty.fg = white;
    p_empty.bg = light_grey;

    p_mine.c = '*';
    p_mine.fg = white;

    p_select.bg = white;
    p_select.fg = black;

    p_reveal.bg = grey;

    for (h = 0; h < height; h++) {
        for (w = 0; w < width; w++) {
            c = grid[h][w];

            if (c.revealed) {
                if (c.is_mine) {
                    if (win) {
                        p_mine.bg = green;
                    } else {
                        p_mine.bg = red;
                    }

                    aes_set_pixel(w, h, p_mine, buffer);
                } else {
                    if (c.n_adj_mines) {
                        p_reveal.c = '0' + c.n_adj_mines;
                        p_reveal.fg = colors[c.n_adj_mines - 1];
                    } else {
                        p_reveal.c = ' ';
                        p_reveal.fg = white;
                    }

                    aes_set_pixel(w, h, p_reveal, buffer);
                }
            } else {
                if (c.marked) {
                    p_empty.c = '!';
                } else {
                    p_empty.c = ' ';
                }

                aes_set_pixel(w, h, p_empty, buffer);
            }

            if (w == select.x && h == select.y && !over) {
                p_select.c = buffer->pixels[width * select.y + select.x].c;
                p_select.fg = buffer->pixels[width * select.y + select.x].fg;
                aes_set_pixel(w, h, p_select, buffer);
            }
        }
    }

    aes_set_cursor_home();
    aes_present(buffer);
}

void reveal_cell(
    struct cell **grid,
    short width,
    short height,
    short x,
    short y,
    struct aes_buffer *buffer)
{
    if (grid[y][x].revealed) {
        return;
    } else {
        grid[y][x].revealed = true;
        empty--;

        if (grid[y][x].marked) {
            grid[y][x].marked = false;
        }

        if (grid[y][x].n_adj_mines > 0) {
            return;
        } else {
            if (y < height - 1) {
                reveal_cell(grid, width, height, x, y + 1, buffer);
            }
            if (x < width - 1) {
                reveal_cell(grid, width, height, x + 1, y, buffer);
            }
            if (y > 0) {
                reveal_cell(grid, width, height, x, y - 1, buffer);
            }
            if (x > 0) {
                reveal_cell(grid, width, height, x - 1, y, buffer);
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
    struct cell **grid,
    struct cell *select,
    short width,
    short height,
    struct aes_buffer *buffer)
{
    char c;

    c = getchar();

    switch (c) {
    case 'z':
    case 'Z':
        select->y++;
        select->y %= height;
        break;
    case 's':
    case 'S':
        select->y--;
        select->y += height * (select->y < 0);
        break;
    case 'q':
    case 'Q':
        select->x--;
        select->x += width * (select->x < 0);
        break;
    case 'd':
    case 'D': 
        select->x++;
        select->x %= width;
        break;
    case 'm':
    case 'M':
        grid[select->y][select->x].marked = !grid[select->y][select->x].marked;
        break;
    case '\x0a':
        if (grid[select->y][select->x].is_mine) {
            over = true;
        } else {
            reveal_cell(grid, width, height, select->x, select->y, buffer);
        }
        break;
    case '\x1b':
        quit = true;
        break;
    }
}

int main(void) {
    struct cell **grid, select, **mines;
    struct aes_buffer *buffer;
    short i, n_mines, size;
    char c;

    puts("Choose difficulty :");
    printf("1: %*s, %*s mines\n", 5, "8x8", 3, "10");
    printf("2: %*s, %*s mines\n", 5, "16x16", 3, "40");
    printf("3: %*s, %*s mines\n", 5, "32x32", 3, "100");

    c = getchar();

    switch (c) {
    case '1':
        size = 8;
        n_mines = 10;
        break;
    case '2':
        size = 16;
        n_mines = 40;
        break;
    case '3':
        size = 32;
        n_mines = 100;
        break;
    default:
        size = 16;
        n_mines = 40;
    };

    getchar();

    aes_term_setup(
        AES_HIDE_CURSOR |
        AES_IMMEDIATE_INPUT |
        AES_NO_ECHO |
        AES_SWITCH_BUFFERS
    );

    buffer = aes_buffer_alloc(size, size, true);

    grid = malloc(size * sizeof grid);

    for (i = 0; i < size; i++) {
        *(grid + i) = malloc(size * sizeof **grid);
    }

    mines = malloc(n_mines * sizeof mines);

    do {
        init_grid(grid, size, size, n_mines, mines);
        select = grid[0][0];

        empty = size * size - n_mines;
        over = false;
        win = false;

        while (!over && !win && !quit) {
            draw_grid(grid, size, size, select, buffer);
            process_input(grid, &select, size, size, buffer);

            if (empty == 0) {
                win = true;
            }
        }

        if (!quit) {
            reveal_mines(mines, n_mines);
            draw_grid(grid, size, size, select, buffer);

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

    for (i = 0; i < size; i++) {
        free(*(grid + i));
    }

    free(grid);
    free(mines);
    aes_buffer_free(buffer);

    aes_term_reset();

    return 0;
}
