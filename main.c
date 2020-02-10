#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *aaa_out_buffer;
static const unsigned char *orig_in_buffer;
static const unsigned char *in_buffer;
static unsigned char *out_buffer;

void DecompressBlock(void)
{
	const unsigned char *in_buffer_end = in_buffer + (in_buffer[0] | (in_buffer[1] << 8));

//	printf("End = %p\n", in_buffer_end - in_buffer);

	in_buffer += 2;

	for (;;)
	{
//		printf("Offset %p\n", in_buffer - orig_in_buffer);

		if (in_buffer == in_buffer_end)
			return;

		unsigned char command = *in_buffer++;

		if (command & 0x80)
		{
			// Dictionary-match
			unsigned int length = ((command & 0x60) >> 5) + 4;

			const size_t offset = ((command & 0x1F) << 8) | *in_buffer++;
			const unsigned char *dictionary = out_buffer - offset;

			for (;;)
			{
				printf("Doing dictionary-match of 0x%X bytes from offset 0x%zX to offset 0x%zX\n", length, offset, out_buffer - aaa_out_buffer);

				for (unsigned int i = 0; i < length; ++i)
					*out_buffer++ = *dictionary++;

				if ((*in_buffer & 0xE0) != 0x60)
					break;

//				printf("Offset %p\n", in_buffer - orig_in_buffer);

				if (in_buffer == in_buffer_end)
					return;

				length = *in_buffer++ & 0x1F;
			}
		}
		else if (command & 0x40)
		{
			// Byte-fill
			unsigned int length = command & 0x2F;

			if (command & 0x10)
				length = (length << 8) | *in_buffer++;

			length += 4;

			unsigned char byte = *in_buffer++;

			printf("Doing byte-fill of 0x%X for 0x%X bytes\n", byte, length);

			for (unsigned int i = 0; i < length; ++i)
				*out_buffer++ = byte;
		}
		else
		{
			// Uncompressed copy
			unsigned int length = command & 0x1F;

			if (command & 0x20)
				length = (length << 8) | *in_buffer++;

			printf("Doing uncompressed copy of 0x%X bytes\n", length);

			for (unsigned int i = 0; i < length; ++i)
				*out_buffer++ = *in_buffer++;
		}
	}
}

void Decompress(const unsigned char *_in_buffer, size_t in_size, unsigned char **_out_buffer, size_t *out_size)
{
	(void)in_size;

	aaa_out_buffer = malloc(1024 * 64);

	orig_in_buffer = _in_buffer;
	in_buffer = _in_buffer;
	out_buffer = aaa_out_buffer;

	for (;;)
	{
		DecompressBlock();

		if (*in_buffer++ == 0)
			break;
	}

	*_out_buffer = aaa_out_buffer;
	*out_size = out_buffer - aaa_out_buffer;
}

int main(int argc, char *argv[])
{
	const char *in_filename = argc > 1 ? argv[1] : "in";
	const char *out_filename = argc > 2 ? argv[2] : "out";

	FILE *in_file = fopen(in_filename, "rb");

	if (in_file != NULL)
	{
		fseek(in_file, 0, SEEK_END);
		size_t in_size = ftell(in_file);
		rewind(in_file);

		unsigned char *in_buffer = malloc(in_size);

		fread(in_buffer, 1, in_size, in_file);
		fclose(in_file);

		unsigned char *out_buffer;
		size_t out_size;
		Decompress(in_buffer, in_size, &out_buffer, &out_size);

		FILE *out_file = fopen(out_filename, "wb");

		if (out_file != NULL)
		{
			fwrite(out_buffer, 1, out_size, out_file);
			fclose(out_file);
		}
	}

	return 0;
}
