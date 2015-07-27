////////////////////////////////////////////////////////////////////////////
//
//
//							PROPRIETARY MATERIALS
//
//	   Websense, Inc. (Websense) has prepared this material for use by
//	   Websense personnel, licensees, and customers.  The information
//	   contained herein is the property of Websense and shall not be
//	   reproduced in whole or part without the prior written consent
//	   of an authorized representative of.Websense, Inc.
//
//							RESTRICTED RIGHTS LEGEND
//
//	   Use, duplication  or disclosure	by the U.S. Government is subject
//	   to restrictions as set forth in subdivision (b)(3)(ii) of the Rights
//	   in Technical Data and Computer Software clause at 52.227-7013.  All
//	   other Government use , duplication or disclosure shall be governed
//	   exclusively by the terms of the Websense Subscription Agreement.
//
//								 Websense, Inc.
//						   Copyright (c) 1997 - 2009
//							10240 Sorrento Valley Rd
//							 San Diego, CA 92121
//								(858) 320-8000
//
/////////////////////////////////////////////////////////////////////////

#include "readwritelock.h"

#ifdef _WIN32

CWin32RWLock::CWin32RWLock()
{
	Init();
}
void CWin32RWLock::Init()
{
	if(m_InitFlag==false)
	{
		m_Mutex = ::CreateMutex(NULL, FALSE, NULL);
		m_ReadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		m_WriteEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		m_InitFlag=true;
	}

}

CWin32RWLock::~CWin32RWLock()
{
	Release();
}
void CWin32RWLock::Release()
{
	if(m_InitFlag==true)
	{
		::CloseHandle(m_Mutex);
		::CloseHandle(m_ReadEvent);
		::CloseHandle(m_WriteEvent);
		m_InitFlag=false;
	}
}

bool CWin32RWLock::ReadLock(unsigned long waitTime,const char* filename,int line)
{
	DWORD RetValue;
	::WaitForSingleObject(m_Mutex, waitTime);
	++m_ReadNum;
	if (m_LockType == WRITE_LOCK) 
	{
		RetValue = ::SignalObjectAndWait(m_Mutex, m_ReadEvent, waitTime, FALSE);
		if (RetValue == WAIT_OBJECT_0) 
		{
			return true;
		} 
		else 
		{
			if (RetValue == WAIT_TIMEOUT)
				::SetLastError(WAIT_TIMEOUT);
			return false;
		}
	}
	m_LockType = READ_LOCK;
	::ReleaseMutex(m_Mutex);
	return true;
}


void CWin32RWLock::ReadUnlock(const char* filename,int line)
{

	::WaitForSingleObject(m_Mutex, INFINITE);
	--m_ReadNum;
	if (m_ReadNum == 0) 
	{
		if (m_WriteNum > 0) 
		{        
			m_LockType = WRITE_LOCK;
			::SetEvent(m_WriteEvent);
		} 
		else 
		{
			m_LockType = LOCK_NONE;
		}
	}
	::ReleaseMutex(m_Mutex);
}


bool CWin32RWLock::WriteLock(unsigned long waitTime,const char* filename,int line)
{
	::WaitForSingleObject(m_Mutex, waitTime);
	++m_WriteNum;
	if (m_LockType != LOCK_NONE) 
	{
		DWORD RetValue = ::SignalObjectAndWait(m_Mutex, m_WriteEvent, waitTime, FALSE);
		if (RetValue == WAIT_OBJECT_0) 
		{
			return true;
		} 
		else 
		{
			if (RetValue == WAIT_TIMEOUT)
				::SetLastError(WAIT_TIMEOUT);
			return false;
		}
	}
	m_LockType = WRITE_LOCK;
	::ReleaseMutex(m_Mutex);
	return true;
}


void CWin32RWLock::WriteUnlock(const char* filename,int line)
{

	::WaitForSingleObject(m_Mutex, INFINITE);
	--m_WriteNum;

	if (m_WriteNum > 0) 
	{

		::SetEvent(m_WriteEvent);
	} 
	else if (m_ReadNum > 0) 
	{

		m_LockType = READ_LOCK;
		::PulseEvent(m_ReadEvent);
	} 
	else 
	{
		m_LockType = LOCK_NONE;
	}
	::ReleaseMutex(m_Mutex);
}

#else


CLinuxRWLock::CLinuxRWLock()
{		
	Init();
	
};
void CLinuxRWLock::Init()
{
	if(m_InitFlag==false)
	{
		pthread_mutex_init(&m_Mutex, NULL);
		pthread_cond_init(&m_WriteEvent, NULL);
		pthread_cond_init(&m_ReadEvent, NULL);	
		m_InitFlag=true;
	}
	
}

CLinuxRWLock::~CLinuxRWLock()
{
	Release();
}

void CLinuxRWLock::Release()
{
	if(m_InitFlag)
	{
		pthread_mutex_destroy(&m_Mutex);
		pthread_cond_destroy(&m_WriteEvent);
		pthread_cond_destroy(&m_ReadEvent);
		m_InitFlag=false;
	}
}
bool CLinuxRWLock::ReadLock(unsigned long Milliseconds,const char* filename,int line)
{
	static int abort_on_timeout = (getenv("CCA_ABORT_ON_LOCK_TIMEOUT") != NULL);
	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += Milliseconds/1000;
	pthread_mutex_lock(&m_Mutex);	
	m_QueueReadNum++;
	while (m_LockType==WRITE_LOCK)
	{
		int err=pthread_cond_timedwait(&m_ReadEvent,&m_Mutex,&timeout);
		if (err) {
			fprintf(stderr, "FATAL: %s:%d: CLinuxRWLock::ReadLock failed, error %d: %s\n",
				filename, line, err, strerror(err));
			if (abort_on_timeout) abort();
			return false;
		}
	}
	m_QueueReadNum--;
	m_ReadNum++;
	m_LockType=READ_LOCK;
	pthread_mutex_unlock(&m_Mutex);
	return true;
}

void CLinuxRWLock::ReadUnlock(const char* filename,int line)
{
	pthread_mutex_lock(&m_Mutex);
	m_ReadNum--;
	if (0 == m_ReadNum)
	{		
		m_LockType=LOCK_NONE;
		pthread_cond_signal(&m_WriteEvent);	
	}	
	pthread_mutex_unlock(&m_Mutex);
}

bool CLinuxRWLock::WriteLock(unsigned long Milliseconds,const char* filename,int line)
{
	static int abort_on_timeout = (getenv("CCA_ABORT_ON_LOCK_TIMEOUT") != NULL);
	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += Milliseconds/1000;

	pthread_mutex_lock(&m_Mutex);
	m_QueueWriteNum++;
	while(m_LockType!=LOCK_NONE)
	{
		int err=pthread_cond_timedwait(&m_WriteEvent,&m_Mutex,&timeout);
		if (err) {
			fprintf(stderr, "FATAL: %s:%d: CLinuxRWLock::WriteLock failed, error %d: %s\n",
				filename, line, err, strerror(err));
			if (abort_on_timeout) abort();
			return false;
		}
	}	
	m_QueueWriteNum--;
	m_WriteNum++;
	m_LockType=WRITE_LOCK;
	pthread_mutex_unlock(&m_Mutex);
	return true;
}

void CLinuxRWLock::WriteUnlock(const char* filename,int line)
{
	pthread_mutex_lock(&m_Mutex);
	m_LockType=LOCK_NONE;
	if(m_Priority==READ_PRIORITY)
	{
		if(m_QueueReadNum)
			pthread_cond_broadcast(&m_ReadEvent);	
		else		
			pthread_cond_signal(&m_WriteEvent);	
	}
	else if(m_Priority==WRITE_PRIORITY)
	{
		if(m_QueueWriteNum)
			pthread_cond_signal(&m_WriteEvent);				
		else		
			pthread_cond_broadcast(&m_ReadEvent);	
	} 
	else 
	{
		pthread_cond_signal(&m_WriteEvent);	
		pthread_cond_broadcast(&m_ReadEvent);	
	}
	pthread_mutex_unlock(&m_Mutex);
}



void CLinuxNativeRWLock::die_now(void)
{
	/* and now, intentionally... */
	*(char*)NULL = 0; /* ... THAT will not be missed by anybody, in
					  * any environment, debugger or otherwise */
	exit(2); /* huh? how did we get here? */
}

CLinuxNativeRWLock::CLinuxNativeRWLock()
{		
	Init();
};
void CLinuxNativeRWLock::Init()
{
	if(m_InitFlag==false)
	{
		int err = pthread_rwlock_init(&m_RWLock,NULL);
		if (err) {
			fprintf(stderr,
				"FATAL: %s:%d: CLinuxNativeRWLock::CLinuxNativeRWLock "
				"failed, error %d: %s\n",
				"", 0, err, strerror(err));

		}
		m_InitFlag=true;
	}
	
}

CLinuxNativeRWLock::~CLinuxNativeRWLock()
{	
	Release();
}

void CLinuxNativeRWLock::Release()
{
	if(m_InitFlag)
	{
		int err = pthread_rwlock_destroy(&m_RWLock);
		if (err) {
			fprintf(stderr,
				"%s:%d: CLinuxNativeRWLock::~CLinuxNativeRWLock "
				"failed, error %d: %s\n",
				"", 0, err, strerror(err));
			/* possibly leaking, but most likely not fatal */
		}
		m_InitFlag=false;
	}

}


bool CLinuxNativeRWLock::ReadLock(unsigned long Milliseconds,const char* filename,int line)
{
	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += Milliseconds/1000;
	/* 15 minutes -- that's a CLEAR deadlock. This long of a timeout
	* lets us do useful work between bounces (if we still have
	* non-deadlocked threads), while clearing up an "every netthread
	* deadlocked" situation within "no-need-to-wake-up-IT" time,
	* (assuming it happens very infrequently, either way).
	*
	* Think twice before changing it up or down. 
	*/    
	int err = pthread_rwlock_timedrdlock(&m_RWLock, &timeout);
	if (err) {
		fprintf(stderr, "FATAL: %s:%d: CLinuxNativeRWLock::ReadLock failed, error %d: %s\n",
			filename, line, err, strerror(err));
		return false;
	}
	return true;
}

void CLinuxNativeRWLock::ReadUnlock(const char* filename,int line)
{
	int err = pthread_rwlock_unlock(&m_RWLock);
	if (err) {
		fprintf(stderr, "%s:%d: CLinuxNativeRWLock::ReadUnlock failed, error %d: %s\n",
			filename, line, err, strerror(err));
		/* probably leaking, but not fatal */
	}
}

bool CLinuxNativeRWLock::WriteLock(unsigned long Milliseconds,const char* filename,int line)
{
	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += Milliseconds/1000;
	int err = pthread_rwlock_timedwrlock(&m_RWLock, &timeout);
	if (err) {
		fprintf(stderr, "FATAL: %s:%d: CLinuxNativeRWLock::WriteLock failed, error %d: %s\n",
			filename, line, err, strerror(err));
		return false;
	}
	return true;
}

void CLinuxNativeRWLock::WriteUnlock(const char* filename,int line)
{
	int err = pthread_rwlock_unlock(&m_RWLock);
	if (err) {
		fprintf(stderr, "%s:%d: CLinuxNativeRWLock::WriteUnlock failed, error %d: %s\n",
			filename, line, err, strerror(err));
		/* probably leaking, but not fatal */
	}
}






#endif  //_WIN32



