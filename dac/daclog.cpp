#include "daclog.h"
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <errno.h>
#include <ctype.h>

static int gDAClogInit = 0;
static pthread_mutex_t *gDAClogMutex = NULL;
static char gDAClogFile[MAX_LOG_FILENAME_SIZE+128];
FILE *gDAClogFd = NULL;
DAClogConfig *gDAClogOpt = NULL;

static const char *gDACllstr[] = 
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

int DAClogInit(DAClogConfig *config)
{	
    if (gDAClogInit) {
	printf("log module has been initialized\n");
	return -1;
    }
    int len;
    char  temp[256] = {0};
    char *logpath = config->path;

    if (config->logcb)
    {
	openlog(config->ident?config->ident:"", 0, config->facility);
	goto End;
    }

    if (!config)
	return -1;
    if (config->path == NULL || *config->path == 0)
    {
	getcwd(temp, sizeof(temp));
	logpath = temp;
    }

    if (config->filename == NULL || *config->filename == 0)
	return -3;

    if ((config->option & LogOpt_ByDay) && (config->option & LogOpt_ByHour))
    {
	printf("conflicting log option\n");
	return -1;
    }

    if (access(logpath, F_OK))
    {
	printf("path '%s' not exists\n", logpath);
	return -1;
    }

    len = strlen(logpath) + strlen(config->filename);
    if (len > MAX_LOG_FILENAME_SIZE)
    {
	printf("path and filename too long : %d\n", len);
	return -1;
    }

    if ((config->option & LogOpt_ByDay) || (config->option & LogOpt_ByHour))
    {
	gDAClogMutex = (pthread_mutex_t *) calloc(sizeof(pthread_mutex_t), 1);
	pthread_mutex_init(gDAClogMutex, NULL);
	time_t t = time(NULL);
	struct tm tmt;
	localtime_r(&t, &tmt);

	if ((config->option & LogOpt_ByDay))
	{
	    snprintf(gDAClogFile, sizeof(gDAClogFile), "%s/%s-%04d%02d%02d.log",
		     logpath, config->filename, 1900+tmt.tm_year, tmt.tm_mon+1, tmt.tm_mday);
	}
	else
	{
	    snprintf(gDAClogFile, sizeof(gDAClogFile), "%s/%s-%04d%02d%02d%02d.log",
		     logpath, config->filename, 1900+tmt.tm_year, tmt.tm_mon+1, tmt.tm_mday, tmt.tm_hour); 
	}
    }
    else
    {
	snprintf(gDAClogFile, sizeof(gDAClogFile), "%s/%s.log", logpath, config->filename);
    }

    if ((gDAClogFd = fopen(gDAClogFile, "a+")) == NULL)
    {
	printf("failed to open '%s': %s\n", gDAClogFile, strerror(errno));
	return -7;
    }

End:
    gDAClogOpt = (DAClogConfig *)calloc(sizeof(DAClogConfig), 1);
    if (logpath) gDAClogOpt->path = strdup(logpath);
    if (config->filename) gDAClogOpt->filename = strdup(config->filename);
    if (config->ident) gDAClogOpt->ident = strdup(config->ident);
    gDAClogOpt->facility = config->facility;
    gDAClogOpt->prefix = config->prefix;
    gDAClogOpt->option = config->option;
    gDAClogOpt->level = config->level;
    gDAClogOpt->logcb = config->logcb;
    gDAClogInit = 1;

    return 0;
}

int logDestroy()
{
    if (gDAClogMutex) {
	pthread_mutex_lock(gDAClogMutex);
    }
	
    if (gDAClogOpt) {
	fclose(gDAClogFd);
	gDAClogFd = NULL;
    }
	
    if (gDAClogOpt)
    {
	if (gDAClogOpt->path)
	    free(gDAClogOpt->path);
	if (gDAClogOpt->filename)
	    free(gDAClogOpt->filename);
	if (gDAClogOpt->ident)
	    free (gDAClogOpt->ident);
	free(gDAClogOpt);
	gDAClogOpt = NULL;
    }

    if (gDAClogMutex)
    {
	pthread_mutex_unlock(gDAClogMutex);
	pthread_mutex_destroy(gDAClogMutex);
	free(gDAClogMutex);
	gDAClogMutex = NULL;
    }
    memset(gDAClogFile, 0, sizeof(gDAClogFile));
    gDAClogInit = 0;
    return 0;
}

int DAClog(int level, const char* module, const char* file, int line,
	   const char* func, const char* format, ...)
{
    int ret = 0;
    if (!gDAClogInit) {
	printf(FMTRED"log module not initialize, please call logInit first"FMTEND"\n");
	return 0;
    }

    if(gDAClogOpt->logcb)
    {	
	va_list ap;
	va_start(ap, format);
	ret = gDAClogOpt->logcb(level, file, line, func, format, ap);
	va_end(ap);
	return ret;
    }

    if (gDAClogOpt == NULL)
    {
	printf("invalid file descriptor\n");
	return 0;
    }

    if (gDAClogOpt->level < LOG_LEVEL_ERROR)
	return 0;

    if (level > LOG_LEVEL_6)
	level = LOG_LEVEL_6;

    if (level > gDAClogOpt->level)
	return 0;

    struct timeval tv;
    struct tm tmt;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tmt);

    char temp[256] = {0};
    if ((gDAClogOpt->option & LogOpt_ByDay))
    {
	snprintf(temp, sizeof(temp), "%s/%s-%04d%02d%02d.log",
		 gDAClogOpt->path, gDAClogOpt->filename, 1900+tmt.tm_year, tmt.tm_mon+1, tmt.tm_mday);
    }
    else if ((gDAClogOpt->option & LogOpt_ByHour))
    {
	snprintf(temp, sizeof(temp), "%s/%s-%04d%02d%02d%02d.log",
		 gDAClogOpt->path, gDAClogOpt->filename, 1900+tmt.tm_year, tmt.tm_mon+1, tmt.tm_mday, tmt.tm_hour); 
    }

    if (temp[0] != 0 && strcmp(temp, gDAClogFile) != 0)
    {
	pthread_mutex_lock(gDAClogMutex);
	fprintf(gDAClogFd, "change log file");
	fclose(gDAClogFd);
	gDAClogFd = NULL;
	
	if ((gDAClogFd = fopen(gDAClogFile, "a+")) == NULL)
	{
	    printf("failed to open '%s': %s\n", gDAClogFile, strerror(errno));
	    pthread_mutex_unlock(gDAClogMutex);
	    return 0;
	}
	
	pthread_mutex_unlock(gDAClogMutex);
    }

    char buffer[MAX_LOG_BUFFSIZE+128] = {0};
    int buflen = 0, offset = 0;

    // prefix : [time] [level] [pid:tid] [modulename] [filename(fileline):function] 

    if ((gDAClogOpt->prefix & LogPrefix_Time))
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "%02d/%02d %02d:%02d:%02d.%03d ", 
			   tmt.tm_mon+1, tmt.tm_mday, tmt.tm_hour, tmt.tm_min, (int)tmt.tm_sec, (int)tv.tv_usec/1000);

    if ((gDAClogOpt->prefix & LogPrefix_Level))
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "%s ", gDACllstr[level]);

    if ((gDAClogOpt->prefix & LogPrefix_Proc))
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "[%d,%d] ", (int)getpid(), (int)syscall(__NR_gettid));

    if ((gDAClogOpt->prefix & LogPrefix_Module) && module)
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "[%s] ", module);

    if ((gDAClogOpt->prefix & LogPrefix_Detail))
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "[%s(%d):%s] ", file, line, func);

    va_list args;
    va_start(args, format);
    offset += vsnprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, format, args);
    va_end(args);

    if (buffer[offset-1] != '\n')
	buffer[offset++] = '\n';
    buflen = offset;
    fwrite(buffer, buflen, 1, gDAClogFd);
    fflush(gDAClogFd);

    if ((gDAClogOpt->option & LogOpt_Stderr))
    {
	if((gDAClogOpt->option & LogOpt_WithColor) && level == LOG_LEVEL_ERROR)
	    fprintf(stderr, FMTRED"%s"FMTEND, buffer);
	else
	    fprintf(stderr, "%s", buffer);
    }

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

