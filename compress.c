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
// It uses graph-theory to produce the smallest files possible
// (...in theory, anyway - there could be some oversights).

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clownlzss/rage.h"

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		printf("\nStreets of Rage 2 compressor by Clownacy.\nThis version produces smaller files than the other one.\n\nUsage:\n    %s [input file] [output file]\n\n", argv[0]);
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

				size_t out_size;
				unsigned char *out_buffer = ClownLZSS_RageCompress(in_buffer, in_size, &out_size);

				FILE *out_file = fopen(out_filename, "wb");

				if (out_file == NULL)
				{
					puts("Couldn't open output file");
				}
				else
				{
					fwrite(out_buffer, 1, out_size, out_file);
					fclose(out_file);
				}

				free(in_buffer);
			}
		}
	}

	return 0;
}
