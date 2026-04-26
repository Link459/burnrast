LDFLAGS = -lSDL3 -lm
CC = gcc
SRCS = src/main.c src/model.c

build:
	$(CC) $(SRCS) $(LDFLAGS) -o burnrast

release:
	$(CC) $(SRCS) $(LDFLAGS) -O2 -o burnrast

debug:
	$(CC) $(SRCS) $(LDFLAGS) -g -o burnrast

run: build
	./burnrast


.PHONY: build release run
