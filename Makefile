CC=gcc
CFLAGS=-lm -lSDL2 -lGL -lGLEW

src = $(wildcard src/*.c)

main : $(src)
	$(CC) -o terra $(src) $(CFLAGS)

