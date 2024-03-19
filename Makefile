CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c89
LDFLAGS = -L../AESLib/bin
LDLIBS = -laes -lm

src_dir = src
tmp_dir = tmp
bin_dir = bin

src = $(wildcard $(src_dir)/*.c)
obj = $(patsubst $(src_dir)/%.c,$(tmp_dir)/%.o,$(src))
minesweeper = $(bin_dir)/minesweeper

$(minesweeper): $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(tmp_dir)/%.o: $(src_dir)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: run
run: $(minesweeper)
	./$(minesweeper)

.PHONY: clean
clean:
	-$(RM) $(minesweeper) $(filter-out %.c,$(obj))
