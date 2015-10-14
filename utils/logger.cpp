#include "logger.h"
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>

#define gettid() syscall(__NR_gettid)
void* thread_writelog(void* param)
{
  logger* logd = (logger* ) param;
  if (logd != NULL)
    logd->logdaemon();

  return NULL;
}


// logger
logger* logger::m_inst = NULL;
logger::logger()
:m_queue(200)
,m_logdir(NULL)
,m_file(NULL)
{
}


logger::~logger()
{
}


logger* logger::instance()
{
  if (m_inst == NULL) {
    m_inst = new logger;
  }
  return m_inst;
}


bool logger::init(const char* logdir, bool console)
{
  if (logdir == NULL) {
    m_logdir = new char[strlen(DEFAULT_LOGDIRECTORY)+1];
    strcpy (m_logdir, DEFAULT_LOGDIRECTORY);
    m_logdir[strlen(DEFAULT_LOGDIRECTORY)] = 0;
  }
  else {
    m_logdir = new char[strlen(logdir)+1];
    strcpy (m_logdir, logdir);
    m_logdir[strlen(logdir)] = 0;
  }

  m_console_on = console;
  if ((m_file = fopen ("/tmp/default.log", "w")) == NULL) {
    fprintf (stderr, "fopen() error: %d %s", errno, strerror(errno));
    return 1;
  }

  int queue_capacity = m_queue.getcapacity() - 8;

  pthread_cond_init(&m_qCond, NULL);
  pthread_mutex_init(&m_qMutex, NULL);
  pthread_mutex_init(&m_semMutex, NULL);
  sem_init (&m_qSemaphore, 0, queue_capacity-1);

  int value = 0;
  sem_getvalue(&m_qSemaphore, &value);

  pthread_t      th;
  pthread_attr_t thattr;
  pthread_attr_init(&thattr);
  pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);

  if (pthread_create(&th, &thattr, thread_writelog, this) != 0) {
    fprintf (stderr, "pthread_create() error: %d %s", errno, strerror(errno));
    return 1;
  }

  return 0;
}


void logger::log_debug(const char* fmt, ...)
{
  int  prefixlen = 0;
  char buffer[MAX_LOGBUFFER_LENGTH_FINAL] = "\0";
  addprefix(LOG_DEBUG, buffer, &prefixlen);

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer+prefixlen, MAX_LOGBUFFER_LENGTH_FINAL-2-prefixlen, fmt, args);
  va_end(args);

  logbuffer(DEFAULT_CATEGORY, LOG_DEBUG, buffer);
}


void logger::log_info(const char* fmt, ...)
{
  int  prefixlen = 0;
  char buffer[MAX_LOGBUFFER_LENGTH_FINAL] = "\0";
  addprefix(LOG_INFO, buffer, &prefixlen);

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer+prefixlen, MAX_LOGBUFFER_LENGTH_FINAL-2-prefixlen, fmt, args);
  va_end(args);

  logbuffer(DEFAULT_CATEGORY, LOG_INFO, buffer);
}


void logger::log_warn(const char* fmt, ...)
{
  int  prefixlen = 0;
  char buffer[MAX_LOGBUFFER_LENGTH_FINAL] = "\0";
  addprefix(LOG_WARNING, buffer, &prefixlen);

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer+prefixlen, MAX_LOGBUFFER_LENGTH_FINAL-2-prefixlen, fmt, args);
  va_end(args);

  logbuffer(DEFAULT_CATEGORY, LOG_WARNING, buffer);
}


void logger::log_error(const char* fmt, ...)
{
  int  prefixlen = 0;
  char buffer[MAX_LOGBUFFER_LENGTH_FINAL] = "\0";
  addprefix(LOG_ERROR, buffer, &prefixlen);

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer+prefixlen, MAX_LOGBUFFER_LENGTH_FINAL-2-prefixlen, fmt, args);
  va_end(args);

  logbuffer(DEFAULT_CATEGORY, LOG_ERROR, buffer);
}


inline void logger::addprefix (int level, char* strprefix, int* prefixlen)
{
  struct timeval tv;
  gettimeofday (&tv, NULL);

  struct tm TM, *pTM;
  pTM = localtime_r(&tv.tv_sec, &TM);

  switch (level) {
    case LOG_DEBUG:
      sprintf (strprefix, "%02d/%02d %02d:%02d:%02d.%03ld %-7s",
        pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec,
        tv.tv_usec/1000, PREFIX_DEBUG);
      break;
    case LOG_INFO:
      sprintf (strprefix, "%02d/%02d %02d:%02d:%02d.%03ld %-7s",
        pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec,
        tv.tv_usec/1000, PREFIX_INFO);
      break;
    case LOG_WARNING:
      sprintf (strprefix, "%02d/%02d %02d:%02d:%02d.%03ld %-7s",
        pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec,
        tv.tv_usec/1000, PREFIX_WARNING);
      break;
    case LOG_ERROR:
      sprintf (strprefix, "%02d/%02d %02d:%02d:%02d.%03ld %-7s",
        pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec,
        tv.tv_usec/1000, PREFIX_ERROR);
      break;
    default:
      sprintf (strprefix, "%02d/%02d %02d:%02d:%02d.%03ld %-7s",
        pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec,
        tv.tv_usec/1000, "UNKNOWN");
      break;
  }

  *prefixlen = strlen (strprefix);
}


void logger::logbuffer (int category, int level, const char* buffer)
{
  enqueue(category, level, buffer);

  if (!m_console_on) return ;
// TODO: level color control

  if (level == LOG_WARNING) {
    fprintf (stderr, YELLOW"%s\n"END_FMT, buffer);
  }
  else if (level == LOG_ERROR) {
    fprintf (stderr, RED"%s\n"END_FMT, buffer);
  }
  else {
    fprintf (stdout, "%s\n", buffer);
  }
}


void logger::enqueue(int category, int level, const char* buffer)
{
  pthread_mutex_lock(&m_qMutex);

  int ret = m_queue.push(0, level, buffer);
  if (ret == QUEUE_FULL) {
    printf ("logqueue is full, please wait.\n");
  }
  pthread_mutex_unlock(&m_qMutex);
  pthread_cond_signal(&m_qCond);

  while (sem_wait(&m_qSemaphore) != 0) { }

  if (ret == QUEUE_UPLIMIT) {
    int value = 0;
    sem_getvalue(&m_qSemaphore, &value);
    printf ("warning<%ld>: log message is too busy, queue usage: %d%%, semaphore value: %d\n",
      gettid(), m_queue.getpercent(), value);
    fflush (stdout);
  }
}


int logger::dequeue (LogItem* Item)
{
  pthread_mutex_lock(&m_qMutex);
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 1;                                 // timeout: 1 second

  int ret = 0;
  while (m_queue.empty() && ret != ETIMEDOUT) {
    ret = pthread_cond_timedwait(&m_qCond, &m_qMutex, &ts);
  }

  if (ret == ETIMEDOUT) {
    pthread_mutex_unlock (&m_qMutex);
    return 1;
  }

  if (m_queue.pop(Item) != 0) {
    pthread_mutex_unlock (&m_qMutex);
    printf ("warning: empty queue.\n");
    return 2;
  }
  pthread_mutex_unlock(&m_qMutex);

  return 0;
}


int logger::logdaemon()
{
  int ret;
  while (true) {
// get data from queue
    LogItem item;
    if ((ret = dequeue(&item)) == 1) {
      check_date();
    }
    else if (ret == 0) {
      qflush(item.category, item.level, item.buffer);
      sem_post(&m_qSemaphore);
    }
    else {
      continue ;
    }
  }

  return 0;
}


void logger::check_date()
{
  struct timeval tv;
  gettimeofday (&tv, NULL);

  char strt[64];
  struct tm TM, *pTM;
  pTM = localtime_r(&tv.tv_sec, &TM);
  sprintf (strt, "%04d/%02d/%02d %02d:%02d:%02d.%03ld",
    pTM->tm_year+1900, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec, tv.tv_usec/1000);

//	printf ("current time: %s\n", strt);
}


int logger::qflush(int category, int level,const char * buffer)
{
  fprintf(m_file, "%s", buffer);
  fflush (m_file);

  return 0;
}
