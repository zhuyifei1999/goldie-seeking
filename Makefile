CFLAGS ?= -O3 -g -lm

strats: strats.c
	$(CC) $(CFLAGS) $< -o $@
