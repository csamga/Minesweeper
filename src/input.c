#include "grid.h"

#include "minesweeper.h"

#include <stdio.h>

void process_input(struct grid *grid, struct aes_buffer *buffer) {
    char c;
    struct cell *selected;

    selected = &grid->cells[grid->select_y][grid->select_x];

    c = getchar();

    switch (c) {
    case 'z':
    case 'Z':
        grid->select_y++;
        grid->select_y %= grid->height;
        break;
    case 's':
    case 'S':
        grid->select_y--;
        grid->select_y += grid->height * (grid->select_y < 0);
        break;
    case 'q':
    case 'Q':
        grid->select_x--;
        grid->select_x += grid->width * (grid->select_x < 0);
        break;
    case 'd':
    case 'D': 
        grid->select_x++;
        grid->select_x %= grid->width;
        break;
    case 'm':
    case 'M':
        selected->marked = !selected->revealed && !selected->marked;
        break;
    case '\x0a':
        if (selected->is_mine) {
            ms.over = true;
        } else {
            reveal_cell(grid, grid->select_x, grid->select_y, buffer);

            if (grid->empty == 0) {
                ms.win = true;
            }
        }
        break;
    case '\x1b':
        ms.quit = true;
        break;
    }
}

void prompt_difficulty(short *width, short *height, short *n_mines) {
    char c;

    puts("Choose difficulty :");
    printf("1: %*s, %*s mines\n", 5, "8x8", 3, "10");
    printf("2: %*s, %*s mines\n", 5, "16x16", 3, "40");
    printf("3: %*s, %*s mines\n", 5, "32x32", 3, "100");

    c = getchar();

    switch (c) {
    case '1':
        *width = 8;
        *height = 8;
        *n_mines = 10;
        break;
    case '2':
        *width = 16;
        *height = 16;
        *n_mines = 40;
        break;
    case '3':
        *width = 32;
        *height = 32;
        *n_mines = 100;
        break;
    default:
        *width = 16;
        *height = 16;
        *n_mines = 40;
    };

    getchar();
}

void prompt_replay(bool *quit) {
    char c;

    puts("Replay ? [Y/n]");
    c = getchar();

    switch (c) {
    case 'y':
    case 'Y':
    case '\x0a':
        *quit = false;
        break;
    case 'n':
    case 'N':
    case '\x1b':
        *quit = true;
        break;
    }
}
