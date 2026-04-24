build:
	gcc src/main.c -lm -l SDL3 -o burnrast

run: build
	./burnrast
