all: decompress compress

decompress: decompress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) -Wall -Wextra -pedantic -ggdb3

compress: compress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) -Wall -Wextra -pedantic -ggdb3
