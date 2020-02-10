#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(int argc, char *argv[])
{
	const char *in_filename = argc > 1 ? argv[1] : "comp_in";
	const char *out_filename = argc > 2 ? argv[2] : "comp_out";

	FILE *in_file = fopen(in_filename, "rb");
	fseek(in_file, 0, SEEK_END);
	size_t in_size = ftell(in_file);
	rewind(in_file);
	unsigned char *in_buffer = malloc(in_size);
	fread(in_buffer, 1, in_size, in_file);
	fclose(in_file);

	unsigned char *out_buffer = malloc(1024 * 64);
	size_t out_index = 2;

	unsigned char uncompressed_queue[0x1FFF];
	size_t uncompressed_queue_index = 0;

	size_t current_index = 0;
	while (current_index < in_size)
	{
		size_t rle_length;

		for (rle_length = 1; rle_length <= MIN(0xFFF + 4, in_size - current_index); ++rle_length)
			if (in_buffer[current_index] != in_buffer[current_index + rle_length])
				break;

		size_t best_match_length = 0;
		size_t best_match_backsearch = 0;

		for (size_t backsearch = 1; backsearch <= MIN(0x1FFF, current_index); ++backsearch)
		{
			for (size_t length = 0; length < in_size - current_index; ++length)
			{
				if (in_buffer[current_index + length] != in_buffer[current_index - backsearch + length])
					break;

				if (length + 1 >= best_match_length)	// Should be '>'
				{
					best_match_length = length + 1;
					best_match_backsearch = backsearch;
				}
			}
		}

		if (rle_length < 4 && best_match_length < 4)
		{
			uncompressed_queue[uncompressed_queue_index++] = in_buffer[current_index];

			if (uncompressed_queue_index > 0x1FFF)
			{
				printf("Doing 0x%zX uncompressed bytes\n", uncompressed_queue_index);

				if (uncompressed_queue_index > 0x1F)
				{
					out_buffer[out_index++] = 0x20 | ((uncompressed_queue_index >> 8) & 0x1F);
					out_buffer[out_index++] = uncompressed_queue_index & 0xFF;
				}
				else
				{
					out_buffer[out_index++] = uncompressed_queue_index;
				}

				memcpy(&out_buffer[out_index], uncompressed_queue, uncompressed_queue_index);
				out_index += uncompressed_queue_index;

				uncompressed_queue_index = 0;
			}

			++current_index;
		}
		else if (rle_length >= best_match_length)
		{
			if (uncompressed_queue_index != 0)
			{
				printf("Doing 0x%zX uncompressed bytes\n", uncompressed_queue_index);

				if (uncompressed_queue_index > 0x1F)
				{
					out_buffer[out_index++] = 0x20 | ((uncompressed_queue_index >> 8) & 0x1F);
					out_buffer[out_index++] = uncompressed_queue_index & 0xFF;
				}
				else
				{
					out_buffer[out_index++] = uncompressed_queue_index;
				}

				memcpy(&out_buffer[out_index], uncompressed_queue, uncompressed_queue_index);
				out_index += uncompressed_queue_index;

				uncompressed_queue_index = 0;
			}

			unsigned char value = in_buffer[current_index];

			printf("Doing RLE match (value 0x%.2X, 0x%zX bytes)\n", value, rle_length);

			current_index += rle_length;

			rle_length -= 4;

			if (rle_length > 0xF)
			{
				out_buffer[out_index++] = 0x40 | 0x10 | ((rle_length >> 8) & 0xF);
				out_buffer[out_index++] = rle_length & 0xFF;
			}
			else
			{
				out_buffer[out_index++] = 0x40 | (rle_length & 0xF);
			}

			out_buffer[out_index++] = value;
		}
		else
		{
			if (uncompressed_queue_index != 0)
			{
				printf("Doing 0x%zX uncompressed bytes\n", uncompressed_queue_index);

				if (uncompressed_queue_index > 0x1F)
				{
					out_buffer[out_index++] = 0x20 | ((uncompressed_queue_index >> 8) & 0x1F);
					out_buffer[out_index++] = uncompressed_queue_index & 0xFF;
				}
				else
				{
					out_buffer[out_index++] = uncompressed_queue_index;
				}

				memcpy(&out_buffer[out_index], uncompressed_queue, uncompressed_queue_index);
				out_index += uncompressed_queue_index;

				uncompressed_queue_index = 0;
			}

			current_index += best_match_length;

			best_match_length -= 4;

			size_t thing = best_match_length > 3 ? 3 : best_match_length;

			printf("Doing dictionary match (0x%zX bytes)\n", thing + 4);

			out_buffer[out_index++] = 0x80 | (thing << 5) | ((best_match_backsearch >> 8) & 0x1F);
			out_buffer[out_index++] = best_match_backsearch & 0xFF;

			best_match_length -= thing;

			while (best_match_length != 0)
			{
				printf("Doing dictionary match (0x%zX bytes)\n", best_match_length & 0x1F);

				out_buffer[out_index++] = 0x60 | (best_match_length & 0x1F);
				best_match_length -= best_match_length & 0x1F;
			}
		}
	}

	out_buffer[0] = out_index & 0xFF;
	out_buffer[1] = (out_index >> 8) & 0xFF; 

	out_buffer[out_index++] = 0;

	FILE *out_file = fopen(out_filename, "wb");
	fwrite(out_buffer, 1, out_index, out_file);
	fclose(out_file);
}
