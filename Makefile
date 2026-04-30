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
	$(CC) $(SRCS) $(LDFLAGS) -fsanitize=address -o burnrast

run: build
	./burnrast


.PHONY: build release run
