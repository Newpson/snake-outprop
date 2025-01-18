CFLAGS += -std=c23 -D_DEFAULT_SOURCE
LDFLAGS += -lm

all: snake

snake: snake.c

.PHONY: all clean
clean:
	rm -f *.o snake
