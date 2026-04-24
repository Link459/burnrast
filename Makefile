LDFLAGS = -lSDL3 -lm
CC = gcc
SRCS = src/main.c

build:
	$(CC) $(SRCS) $(LDFLAGS) -o burnrast

release:
	$(CC) $(SRCS) $(LDFLAGS) -O2 -o burnrast

run: build
	./burnrast


.PHONY: build release run
