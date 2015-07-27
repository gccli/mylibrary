#include <unistd.h>
#include <sys/syscall.h> 
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "comm.h"

shared_ptr<CommClass> CommClass::m_cc;

void OnExit()
{
    Mylog("** use count %ld\n", CommClass::m_cc.use_count());
}

CommClass *CommClass::GetInstance()
{
    if (m_cc.get() == NULL)
    {
	Mylog("BEGIN Create new instance, use count %d\n", m_cc.use_count());
	m_cc = shared_ptr<CommClass>(new CommClass); 
	Mylog("END Create new instance, use count %d, pointer %p\n", m_cc.use_count(), m_cc.get());
	atexit(OnExit);
    }

    return m_cc.get();
}


#define MAX_LOG_BUFFSIZE 1024
void printlog(int level,
	      const char *filename,
	      const char *funcname,
	      int line,
	      const char *tag,
	      const char *format, ...)
{    
    char buffer[MAX_LOG_BUFFSIZE+128] = {0};
    int buflen = 0, offset = 0;


    if (0) {
	struct timeval tv;
	struct tm tmt;
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tmt);
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset,
			   "%02d/%02d %02d:%02d:%02d.%03d ", 
			   tmt.tm_mon+1, tmt.tm_mday, tmt.tm_hour,
			   tmt.tm_min, (int)tmt.tm_sec, (int)tv.tv_usec/1000);
    }
    if (1) {
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, 
			   "[%d,T:%d,P:%d] ",
			   (int)getpid(), (int)syscall(__NR_gettid), (int)getppid());
    }
    if (0) {
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "[%s] ", tag);
    }

    if (0) {
	offset += snprintf(buffer+offset, MAX_LOG_BUFFSIZE-offset, "[%s(%d):%s] ", filename, line, funcname);
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
}
