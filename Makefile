CFLAGS = -Wall -Wextra -pedantic

all: decompress compress accurate-compress

decompress: decompress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

compress: compress.c clownlzss/common.c clownlzss/memory_stream.c clownlzss/rage.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

accurate-compress: accurate-compress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)
