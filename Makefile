CFLAGS = -Wall -Wextra -pedantic

all: decompress compress compress-accurate

decompress: decompress.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

compress: compress.c clownlzss/common.c clownlzss/memory_stream.c clownlzss/rage.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)

compress-accurate: compress-accurate.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LIBS)
