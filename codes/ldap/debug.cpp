#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <errno.h>
#include <ctype.h>
#include "debug.h"

static const char *Levels[] = 
{
    "!!",
    "E",
    "W",
    "D",
    "4",
    "5",
    "6",
    NULL	
};

char *Debug::filename = NULL;
int Debug::initialized = 0;
int Debug::prefix = Debug::Prefix_Default;
int Debug::level = 9;
int Debug::rotate = 0;
int Debug::init(const char *options)
{
    if (options == NULL || *options == 0) {
	filename = strdup(DEFAULT_FILENAME);
	initialized=1;	// use default options
	return 0;
    }

    initialized=1;
    return 0;
}

int Debug::print(int l, const char* module, const char* file, int line,
		 const char* func, const char* format, ...)
{
    if (!initialized) {
	init(NULL);
    }

    if (l < LOG_LEVEL_ERROR)
	return 0;

    if (l > LOG_LEVEL_6)
	l = LOG_LEVEL_6;

    if (l > level)
	return 0;

    struct timeval tv;
    struct tm tmt;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tmt);

    char buffer[MAX_LOG_BUFFSIZE+128] = {0};
    int buflen = 0, offset = 0;

    // prefix : [time] [level] [pid:tid] [modulename] [filename(fileline):function] 
    if ((prefix & Prefix_Time)) {
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset,
			   "%02d/%02d %02d:%02d:%02d.%03d ", 
			   tmt.tm_mon+1, tmt.tm_mday, tmt.tm_hour, tmt.tm_min,
			   (int)tmt.tm_sec, (int)tv.tv_usec/1000);
    }
    if ((prefix & Prefix_Level)) {
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "%s ", Levels[l]);
    }
    if ((prefix & Prefix_Proc)) {
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset,
			   "[%d,%d] ", (int)getpid(), (int)syscall(__NR_gettid));
    }
    if ((prefix & Prefix_Module) && module) {
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "[%s] ", module);
    }
    if ((prefix & Prefix_Detail)) {
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset,
			   "[%s(%d):%s] ", file, line, func);
    }
    va_list args;
    va_start(args, format);
    offset += vsnprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, format, args);
    va_end(args);

    if (buffer[offset-1] != '\n')
	buffer[offset++] = '\n';
    buflen = offset;
    fwrite(buffer, buflen, 1, stdout);
    fflush(stdout);

    return buflen;
}

// utilities
static void PrintHexAsciiLine(const u_char *payload, int len, int offset, FILE* fp)
{
    int i;
    int gap;
    const u_char *ch;

    // offset
    fprintf(fp, "%05d   ", offset);

    // hex
    ch = payload;
    for(i = 0; i < len; i++)
    {
        fprintf(fp, "%02x ", *ch);
        ch++;
        // print extra space after 8th byte for visual aid
        if (i == 7)     fprintf(fp, " ");
    }
    
    // print space to handle line less than 8 bytes
    if (len < 8) fprintf(fp, " ");
    
    // fill hex gap with spaces if not full line 
    if (len < 16) 
    {
        gap = 16 - len;
        for (i = 0; i < gap; i++)
                fprintf(fp, "   ");
    }
    fprintf(fp, "   ");

    // ascii (if printable)
    ch = payload;
    for(i = 0; i < len; i++) 
    {
        if (isprint(*ch)) fprintf(fp, "%c", *ch);
        else              fprintf(fp, ".");
        ch++;
    }
    fprintf(fp, "\n");
}

void DPrintPayload(const u_char *payload, int len, FILE* fp)
{
    int len_rem = len;
    int line_width = 16;// number of bytes per line
    int line_len;
    int offset = 0;     // zero-based offset counter
    const u_char *ch = payload;

    if (len <= 0)   return;

    // data fits on one line
    if (len <= line_width) 
    {
        PrintHexAsciiLine(ch, len, offset, fp);
        fflush(fp);
        return;
    }

    // data spans multiple lines
    for ( ;; ) 
    {
        // compute current line length
        line_len = line_width % len_rem;
        PrintHexAsciiLine(ch, line_len, offset, fp);
        // compute total remaining 
        len_rem = len_rem - line_len;
        ch = ch + line_len; // shift pointer to remaining bytes to print
        offset = offset + line_width;

        if (len_rem <= line_width) 
        {
            // print last line and get out
            PrintHexAsciiLine(ch, len_rem, offset, fp);
            break;
        }
    }
    fflush(fp);

    return ;
}

