#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <stdbool.h>

void process_input(struct grid *grid, struct aes_buffer *buffer);
void prompt_difficulty(short *width, short *height, short *n_mines);
void prompt_replay(bool *quit);

#endif
