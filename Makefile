build:
	gcc src/main.c -l SDL3 -o burnrast

run: build
	./burnrast
