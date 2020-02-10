all: a view comp

a: main.c
	$(CC) $^ -o $@ -Wall -Wextra -pedantic -ggdb3

view: view.c
	$(CC) $^ -o $@ -Wall -Wextra -pedantic -ggdb3 `sdl2-config --cflags --libs`

comp: comp.c
	$(CC) $^ -o $@ -Wall -Wextra -pedantic -ggdb3
