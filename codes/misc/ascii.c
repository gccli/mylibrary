#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ascii
{
	char	value;
	char*	symble;
	char*	desc;
} ascii_table[] = {
    {0x00, "NUL", "null"},
    {0x01, "SOH", "start of heading"},
    {0x02, "STX", "start of text"},
    {0x03, "ETX", "end of text"},
    {0x04, "EOT", "end of transmission"},
    {0x05, "ENQ", "enquiry"},
    {0x06, "ACK", "acknowledge"},
    {0x07, "BEL", "bell"},
    {0x08, "BS",  "backspace"},
    {0x09, "TAB", "horizontal tab"},
    {0x0A, "LF",  "NL line feed, new line"},
    {0x0B, "VT",  "vertical tab"},
    {0x0C, "FF",  "NP form feed, new page"},
    {0x0D, "CR",  "carriage return"},
    {0x0E, "SO",  "shift out"},
    {0x0F, "SI",  "shift in"},
    {0x10, "DLE", "data link escape"},
    {0x11, "DC1", "device control 1"},
    {0x12, "DC2", "device control 2"},
    {0x13, "DC3", "device control 3"},
    {0x14, "DC4", "device control 4"},
    {0x15, "NAK", "negative acknowledge"},
    {0x16, "SYN", "synchronous idle"},
    {0x17, "ETB", "end of trans. block"},
    {0x18, "CAN", "cancel"},
    {0x19, "EM",  "end of medium"},
    {0x1A, "SUB", "substitute"},
    {0x1B, "ESC", "escape"},
    {0x1C, "FS",  "file separator"},
    {0x1D, "GS",  "group separator"},
	{0x1E, "RS",  "record separator"},
    {0x1F, "US",  "unit separator"},
    {0x20, "SP",  "space"},
    {0x21, NULL,  NULL}
};

#define BOLDWRITE "\033[47;1m"
#define RED       "\033[31m"
#define END       "\033[0m"


int main(int argc, char *argv[])
{
    int  i, j, value, offset, width;
    char buffer[1024];

    for (j=0; j<4; ++j)
	if (j == 0)
	    printf(BOLDWRITE"Dec  Hex  Symble  Description               "END);
	else
	    printf(BOLDWRITE"Dec  Hex  Char  "END);
    printf("\n");
    for (i=0; i<32; ++i)
    {
	memset(buffer, 0, sizeof(buffer));
	for (offset = 0, j=0; j<4; ++j) {
	    value = j*32+i;
	    if (value < 32) {
		sprintf(buffer, "%-3d  %02x   %-6s  %s", value, value, ascii_table[i].symble, ascii_table[i].desc);
	    }
	    else {
		width = 28+16*j;
		if (offset < width)
		    memset(buffer+offset, ' ', width-offset);
		sprintf(buffer+width, "| %-3d  %02x   %c", value, value, value);
	    }
	    offset = strlen(buffer);
	}
	printf("%s\n", buffer);
    }
    
    return 0;
}

