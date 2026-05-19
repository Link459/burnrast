LDFLAGS = -lSDL3 -lm
CC = gcc
SRCS = src/main.c src/model.c src/image.c src/rasterization_pipeline.c

build:
	$(CC) $(SRCS) $(LDFLAGS) -o burnrast

release:
	$(CC) $(SRCS) $(LDFLAGS) -O2 -o burnrast

debug:
	$(CC) $(SRCS) $(LDFLAGS) -g -o burnrast

asan:
	$(CC) $(SRCS) $(LDFLAGS) -g -fsanitize=address -o burnrast

ubsan:
	$(CC) $(SRCS) $(LDFLAGS) -fsanitize=undefined -o burnrast

run: build
	./burnrast


.PHONY: build release run
