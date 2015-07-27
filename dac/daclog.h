#ifndef _COMMON_LOGGER_H__
#define _COMMON_LOGGER_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <syslog.h>

#define MAX_LOG_FILENAME_SIZE 1024
#define MAX_LOG_BUFFSIZE 4096

// Colour Control
#define FMTEND        "\033[0m"
#define FMTBLACK      "\033[30m"
#define FMTRED        "\033[31m"
#define FMTGREEN      "\033[32m"
#define FMTYELLOW     "\033[33m"
#define FMTBLUE       "\033[34m"
#define FMTMAGENTA    "\033[35m"
#define FMTCYAN       "\033[36m"
#define FMTWRITE      "\033[37m"

// Kill Line, Moves the cursor to Begining Of Line
#define TCKL          "\033[0G\033[0K"

typedef int (*DAClogCallback)(int level, const char* file, int line, const char* func, const char* format, va_list ap);

typedef enum __DLogLevel {
	LOG_LEVEL_NONE = 0,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARN,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_4,
	LOG_LEVEL_5,
	LOG_LEVEL_6,
} DLogLevel;

typedef enum __DLogPrefix
{
	LogPrefix_None         = 0x00,
	LogPrefix_Time         = 0x01, // datetime prefix: mm/dd hh:mm:ss.ms e.g. 10/10 10:58:07.027
	LogPrefix_Level        = 0x02, // log level tag: e.g. DEBUG
	LogPrefix_Proc         = 0x04, // process info prefix: pid and tid
	LogPrefix_Module       = 0x08, // module name
	LogPrefix_Detail       = 0x80, // detail debug info: file(line):func 
	LogPrefix_Default      = 0xff,
} DLogPrefix;

typedef enum __DLogOpt
{
	LogOpt_Default         = 0x00,
	LogOpt_Stderr          = 0x01,
	LogOpt_WithColor       = 0x02,
	LogOpt_ByHour          = 0x04,
	LogOpt_ByDay           = 0x08,
} DLogOpt;

typedef struct __DAClogConfig
{
	char  *path;
	char  *filename;
	char  *ident;   // syslog identity
	int    facility;// syslog facility
	int    prefix;
	int    option;
	int    level;

	DAClogCallback logcb;

	__DAClogConfig()
		:path(NULL)
		,filename(NULL)
		,ident(NULL)
		,facility(LOG_LOCAL0)
		,prefix(LogPrefix_Default)
		,option(LogOpt_Default)
		,level(0)
		,logcb(NULL)
	{}		
} DAClogConfig;


//////////////////////////////////////////////////////////////////////
extern FILE *gDAClogFd;
extern DAClogConfig *gDAClogOpt;

int DAClogInit(DAClogConfig *config);

int DAClog(int level,
	   const char* module,
	   const char* file,
	   int line,
	   const char* func,
	   const char* format,
	   ...);
int DAClogDestroy();
void PrintPayload(const u_char *payload, int len, FILE* fp);

#define dacerror(...)\
    DAClog(LOG_LEVEL_ERROR,NULL,__FILE__,__LINE__,__FUNCTION__, \
	    ##__VA_ARGS__)
#define debugs(...)\
    DAClog(LOG_LEVEL_DEBUG,NULL,__FILE__,__LINE__,__FUNCTION__, \
	    ##__VA_ARGS__)
#define debug4(...)\
    DAClog(LOG_LEVEL_4,NULL,__FILE__,__LINE__,__FUNCTION__, \
	    ##__VA_ARGS__)
#define debug5(...)\
    DAClog(LOG_LEVEL_5,NULL,__FILE__,__LINE__,__FUNCTION__, \
	    ##__VA_ARGS__)
#define debug6(...)\
    DAClog(LOG_LEVEL_6,NULL,__FILE__,__LINE__,__FUNCTION__, \
	    ##__VA_ARGS__)

#endif

