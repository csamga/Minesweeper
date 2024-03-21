#include "minesweeper.h"

#include "input.h"

#include <stdio.h>

struct minesweeper ms;

struct aes_color_rgb digit_colors[8] = {
    {0, 0, 255},
    {0, 128, 0},
    {255, 0, 0},
    {0, 0, 128},
    {128, 0, 0},
    {0, 128, 128},
    {128, 128, 0},
    {128, 0, 128}
};

static void minesweeper_init(void);
static void minesweeper_new_game(void);
static void minesweeper_clean(void);

void minesweeper_run(void) {
    minesweeper_init();

    do {
        minesweeper_new_game();
    } while (!ms.quit);

    minesweeper_clean();
}

static void minesweeper_init(void) {
    short width, height, n_mines;

    prompt_difficulty(&width, &height, &n_mines);

    ms.grid = grid_create(width, height, n_mines);
    ms.buffer = aes_buffer_alloc(width, height, true);

    aes_term_setup(
        AES_HIDE_CURSOR |
        AES_IMMEDIATE_INPUT |
        AES_NO_ECHO |
        AES_SWITCH_BUFFERS
    );
}

static void minesweeper_new_game(void) {
    grid_init(ms.grid);

    ms.over = false;
    ms.win = false;

    while (!ms.over && !ms.win && !ms.quit) {
        grid_draw(ms.grid, ms.buffer);
        process_input(ms.grid, ms.buffer);
    }

    if (!ms.quit) {
        reveal_mines(ms.grid->mines, ms.grid->n_mines);
        grid_draw(ms.grid, ms.buffer);

        if (ms.win) {
            puts("you won");
        } else {
            puts("you lost");
        }

        prompt_replay(&ms.quit);
    }

    aes_clear_screen();
}

static void minesweeper_clean(void) {
    grid_destroy(ms.grid);
    aes_buffer_free(ms.buffer);
    aes_term_reset();
}
