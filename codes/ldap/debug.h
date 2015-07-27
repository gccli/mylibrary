#ifndef COMM_DEBUG_H__
#define COMM_DEBUG_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <syslog.h>

#define MAX_LOG_FILESIZE 1024
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

typedef enum __DLogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_4,
    LOG_LEVEL_5,
    LOG_LEVEL_6,
} DLogLevel;

void PrintPayload(const u_char *payload, int len, FILE* fp);


#define DEFAULT_FILENAME  "debug.log"

class Debug
{
public:
    typedef enum __PrefixOption
    {
	Prefix_None         = 0x00,
	Prefix_Time         = 0x01, // datetime prefix: mm/dd hh:mm:ss.ms e.g. 10/10 10:58:07.027
	Prefix_Level        = 0x02, // log level tag: e.g. DEBUG
	Prefix_Proc         = 0x04, // process info prefix: pid and tid
	Prefix_Module       = 0x08, // module name
	Prefix_Detail       = 0x80, // detail debug info: file(line):func 
	Prefix_Default      = Prefix_Time|Prefix_Level|Prefix_Proc|Prefix_Module|Prefix_Detail,
    } PrefixOpt;

    typedef enum __RotateOption
    {
	Rotate_None      = 0,
	Rotate_Daily     = 1,
	Rotate_SizeLimit = 2,
    } RotateOpt;

public:
    static int init(const char *options);
    static int print(int level,
		     const char* module,
		     const char* file,
		     int line,
		     const char* func,
		     const char* format,
		     ...);


    static int initialized;
    static int prefix;
    static int level;
    static int rotate;   // default 0 no rotate
    static int syslog;
    static char *filename;
};

#define debugs(LEVEL,...)					\
    Debug::print(LEVEL,NULL,__FILE__,__LINE__,__FUNCTION__,	\
		 ##__VA_ARGS__)














#endif

