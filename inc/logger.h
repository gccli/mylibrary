#ifndef LOGGER_H_
#define LOGGER_H_

#include "logqueue.h"
#include <pthread.h>
#include <semaphore.h>

typedef unsigned char u_char;

// [control] [timestamp] [optional prefix] actual message [control]
#define MAX_OPTIONAL_FIELD              256
#define MAX_LOGBUFFER_LENGTH_FINAL      MAX_LOGBUFFER_LENGTH+MAX_OPTIONAL_FIELD

#define CATEGORY_SYSTEM                 0
#define DEFAULT_CATEGORY                CATEGORY_SYSTEM
#define DEFAULT_LOGDIRECTORY            "/tmp"

#define PREFIX_DEBUG   "DEBUG:"
#define PREFIX_INFO    "INFO:"
#define PREFIX_WARNING "WARN:"
#define PREFIX_ERROR   "ERROR:"

#define RED           "\033[31m"
#define GREEN         "\033[32m"
#define YELLOW        "\033[33m"
#define BLUE          "\033[34m"
#define MAGENTA       "\033[35m"
#define ORCHID        "\033[36m"

#define B_RED         "\033[1;31m"
#define B_GREEN       "\033[1;32m"
#define B_YELLOW      "\033[1;33m"
#define B_BLUE        "\033[1;34m"
#define B_MAGENTA     "\033[1;35m"
#define B_ORCHID      "\033[1;36m"

#define END_FMT       "\033[0m"

#define logdebug(args...) logger::instance()->log_debug(args)
#define loginfo(args...)  logger::instance()->log_info(args)
#define logwarn(args...)  logger::instance()->log_warn(args)
#define logerror(args...) logger::instance()->log_error(args)

class logger
{
    friend void* thread_writelog(void* );
    public:
        enum LogLevel
        {
            LOG_DEBUG,
            LOG_INFO,
            LOG_WARNING,
            LOG_ERROR
        };

        logger();
        ~logger();

        bool init(const char* logdir = NULL, bool console = true);

        static logger* instance();

        void log_debug (const char* fmt, ...);
        void log_info  (const char* fmt, ...);
        void log_warn  (const char* fmt, ...);
        void log_error (const char* fmt, ...);

    private:
        void logbuffer (int category, int level, const char* buffer);
        void addprefix (int level, char* prefix, int* prefixlen);

        void enqueue (int category, int level, const char* buffer);
        int  dequeue (LogItem* Item);

    private:
        int  logdaemon();
        int  qflush(int category, int level, const char* buffer);
        void check_date();

    private:
        static logger*         m_inst;
        logqueue               m_queue;
        pthread_cond_t         m_qCond;
        pthread_mutex_t        m_qMutex;
        pthread_mutex_t        m_semMutex;
        sem_t                  m_qSemaphore;
        bool                   m_console_on;

        char                   *m_logdir;
        FILE                   *m_file;
};

extern void PrintPayload(const u_char *payload, int len, FILE* fp=stdout);
#endif                                            //LOGGER_H_
