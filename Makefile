CFLAGS = -Wall -Wextra -pedantic

all: decompress compress

decompress: decompress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

compress: compress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)
