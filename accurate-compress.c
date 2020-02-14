//  Copyright (C) 2020 Clownacy
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.

// This tool compresses files in the format used by 'Streets of Rage 2'.
// It should produce output identical to compressed data found in the original ROM.

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static unsigned char out_buffer[1024 * 64];	// 64KB - The size of the Mega Drive's RAM
static size_t out_index = 2;

static unsigned char uncompressed_queue[0x1FFF];
static size_t uncompressed_queue_index = 0;

static void FlushUncompressedQueue(void)
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

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		printf("\nStreets of Rage 2 compressor by Clownacy.\nThis version produces files that are accurate to the original compressor used in Streets of Rage 2's development.\n\nUsage:\n    %s [input file] [output file]\n\n", argv[0]);
	}
	else
	{
		const char *in_filename = argv[1];

		char *out_filename;

		if (argc > 2)
		{
			out_filename = argv[2];
		}
		else
		{
			out_filename = malloc(strlen(in_filename) + 6);
			sprintf(out_filename, "%s.sor2", in_filename);
		}

		FILE *in_file = fopen(in_filename, "rb");

		if (in_file == NULL)
		{
			puts("Couldn't open file");
		}
		else
		{
			fseek(in_file, 0, SEEK_END);
			size_t in_size = ftell(in_file);
			rewind(in_file);
			unsigned char *in_buffer = malloc(in_size);

			if (in_buffer != NULL)
			{
				fread(in_buffer, 1, in_size, in_file);
				fclose(in_file);

				// Begin compression
				size_t current_index = 0;
				while (current_index < in_size)
				{
					// Search for RLE-matches
					size_t rle_length;

					for (rle_length = 1; rle_length <= MIN(0xFFF + 4, in_size - current_index); ++rle_length)
						if (in_buffer[current_index] != in_buffer[current_index + rle_length])
							break;

					// Search for dictionary-matches
					size_t best_match_length = 0;
					size_t best_match_backsearch = 0;

					for (size_t backsearch = 1; backsearch <= MIN(0x1FFF, current_index); ++backsearch)
					{
						for (size_t length = 0; length < in_size - current_index; ++length)
						{
							if (in_buffer[current_index + length] != in_buffer[current_index - backsearch + length])
								break;

							if (length + 1 >= best_match_length)	// Should be '>' instead, but the original compressor did '>='
							{
								best_match_length = length + 1;
								best_match_backsearch = backsearch;
							}
						}
					}

					if (rle_length < 4 && best_match_length < 4)	// Matches must be 4 bytes or longer
					{
						// Add byte to the byte queue
						uncompressed_queue[uncompressed_queue_index++] = in_buffer[current_index];

						// If we're out of room in the queue, flush it
						if (uncompressed_queue_index > 0x1FFF)
							FlushUncompressedQueue();

						++current_index;
					}
					else if (rle_length >= best_match_length)
					{
						// Output RLE-match

						// If any uncompressed bytes need flushing, do it now
						if (uncompressed_queue_index != 0)
							FlushUncompressedQueue();

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
						// Output dictionary match

						// If any uncompressed bytes need flushing, do it now
						if (uncompressed_queue_index != 0)
							FlushUncompressedQueue();

						current_index += best_match_length;

						best_match_length -= 4;

						// The first match can only encode 7 bytes
						size_t thing = best_match_length > 3 ? 3 : best_match_length;

						printf("Doing dictionary match (0x%zX bytes)\n", thing + 4);

						out_buffer[out_index++] = 0x80 | (thing << 5) | ((best_match_backsearch >> 8) & 0x1F);
						out_buffer[out_index++] = best_match_backsearch & 0xFF;

						best_match_length -= thing;

						// If there are still more bytes in this match, do them in blocks of 0x1F bytes
						while (best_match_length != 0)
						{
							thing = best_match_length > 0x1F ? 0x1F : best_match_length;

							printf("Doing dictionary match (0x%zX bytes)\n", thing);

							out_buffer[out_index++] = 0x60 | thing;
							best_match_length -= thing;
						}
					}
				}

				// Flush any uncompressed bytes, since there's nothing left to do now
				if (uncompressed_queue_index != 0)
					FlushUncompressedQueue();

				// Write the header
				out_buffer[0] = out_index & 0xFF;
				out_buffer[1] = (out_index >> 8) & 0xFF; 

				// Write the terminator byte
				out_buffer[out_index++] = 0;

				// End of compression

				FILE *out_file = fopen(out_filename, "wb");

				if (out_file == NULL)
				{
					puts("Couldn't open output file");
				}
				else
				{
					fwrite(out_buffer, 1, out_index, out_file);
					fclose(out_file);
				}

				free(in_buffer);
			}
		}
	}

	return 0;
}
