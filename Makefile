all: decompress compress view

decompress: decompress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) -Wall -Wextra -pedantic -ggdb3

compress: compress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) -Wall -Wextra -pedantic -ggdb3

view: view.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) -Wall -Wextra -pedantic -ggdb3 `sdl2-config --cflags --libs`
