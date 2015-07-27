#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include "protocol/base64.h"

struct soption {
	unsigned char decoding;
	unsigned char encoding;
	FILE* fp_input;
	FILE* fp_output;
} ;
struct soption o;

int main(int argc, char *argv[])
{
	o.encoding = 1;
	o.fp_input	= stdin;
	o.fp_output = stdout;

	static struct option long_options[] = {
		{"help",       0, 0, 'h'}
	};

	const char* optlist = "def:o:h";

	while (1){
		int index = 0;
		int c = getopt_long(argc, argv, optlist, long_options, &index);
		if (c == EOF) break;
		switch (c) {
		case 'd':
			o.decoding = 1;
			break;
		case 'e':
			o.encoding = 1;
			break;
		case 'f':
			o.fp_input = fopen(optarg, "rb");
			break;
		case 'o':
			o.fp_output = fopen(optarg, "wb");
			break;
		case 'h':
		default:
			printf("usage:\n  %s [ -de ] [ -f input ] [ -o output ]\n", argv[0]);
			break;
		}
	}

	if (o.decoding) o.encoding = 0;

	int i=0;
	int offset=0;
	int buflen=4096;
	int iread, totallen = 0;

	int output_length = 0;
	char *output_buffer = NULL;
	char *input_buffer = malloc(buflen);

	char buffer[4096];
	while((iread = fread(buffer, 1, sizeof(buffer), o.fp_input)) > 0)
	{
		totallen += iread;
		if (totallen > buflen) {
			buflen += 200*1024;
			input_buffer = realloc(input_buffer, buflen);
			printf("realloc memory %-8d bytes\n", buflen);
		}
		memcpy(input_buffer+offset, buffer, iread);
		offset += iread;
	}
	for (i=offset-1; i>0 && (input_buffer[i] == 0x0a || input_buffer[i] == 0x0d); --i)
		input_buffer[i] = 0;
	if (o.decoding) 
	{
		output_buffer = base64_decode(input_buffer, strlen(input_buffer), &output_length);		
	}
	else if (o.encoding)
	{
		output_buffer = base64_encode(input_buffer, strlen(input_buffer), &output_length);
	}
	if (output_buffer && output_length > 0) {
		int w = 76;
		for (i=0; i<output_length; ) {
			if (i+76 > output_length)
				w = output_length - i;
			fwrite(output_buffer+i, w, 1, o.fp_output);
			fprintf(o.fp_output, "\n");
			i += w;
		}
		
		fflush(o.fp_output);
	}

	return 0;

}
