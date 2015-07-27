
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
#ifndef __RW_MUTEX__H__
#define __RW_MUTEX__H__




typedef enum _RW_LOCK_PRIORITY
{
	READ_PRIORITY,
	WRITE_PRIORITY,
	NORMAL_PRIORITY,
}RW_LOCK_PRIORITY;

class CReadWriteLock
{
private:
	
	CReadWriteLock &operator = (const CReadWriteLock& dest){return *this;};
	CReadWriteLock(const CReadWriteLock& dest){};
protected:
	volatile int m_ReadNum;
	volatile int m_WriteNum;
	volatile int m_LockType;
	volatile int m_QueueReadNum;
	volatile int m_QueueWriteNum;	
	int m_Priority;
	bool m_InitFlag;

	enum
	{
		LOCK_NONE = 0,
		READ_LOCK = 1,
		WRITE_LOCK = 2,
	};
public:
	CReadWriteLock()
	{
		m_ReadNum=0;
		m_WriteNum=0;
		m_LockType=LOCK_NONE;
		m_QueueReadNum=0;
		m_QueueWriteNum=0;
		m_Priority=NORMAL_PRIORITY;
		m_InitFlag=false;
		Init();
	};
	virtual ~CReadWriteLock(){Release();};
	
	virtual void Init(){};
	virtual void Release(){};
	virtual bool ReadLock(unsigned long timeout=0-1,const char* filename=__FILE__,int line=__LINE__)=0;
	virtual void ReadUnlock(const char* filename=__FILE__,int line=__LINE__)=0;
	virtual bool WriteLock(unsigned long timeout=0-1,const char* filename=__FILE__,int line=__LINE__)=0;
	virtual void WriteUnlock(const char* filename=__FILE__,int line=__LINE__)=0;

	virtual void SetPriority(RW_LOCK_PRIORITY Priority)=0;
};

#ifdef _WIN32

#include <windows.h>


class CWin32RWLock : public CReadWriteLock
{
private:
	HANDLE m_Mutex;
	HANDLE m_ReadEvent;
	HANDLE m_WriteEvent;

public:
	CWin32RWLock();
	~CWin32RWLock();
	void Init();
	void Release();


	bool ReadLock(unsigned long waitTime = INFINITE,const char* filename=__FILE__,int line=__LINE__);

	void ReadUnlock(const char* filename=__FILE__,int line=__LINE__);

	bool WriteLock(unsigned long waitTime = INFINITE,const char* filename=__FILE__,int line=__LINE__);

	void WriteUnlock(const char* filename=__FILE__,int line=__LINE__);
	
	void SetPriority(RW_LOCK_PRIORITY Priority){};
};

#else

//#undef _XOPEN_SOURCE
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>

class CLinuxRWLock :public CReadWriteLock
{

private :


	pthread_mutex_t m_Mutex;
	pthread_cond_t m_ReadEvent;
	pthread_cond_t m_WriteEvent;


public:
	CLinuxRWLock();
	~CLinuxRWLock();
	void Init();
	void Release();
	bool ReadLock(unsigned long timeout=0-1,const char* filename=__FILE__,int line=__LINE__);
	void ReadUnlock(const char* filename=__FILE__,int line=__LINE__);
	bool WriteLock(unsigned long timeout=0-1,const char* filename=__FILE__,int line=__LINE__);
	void WriteUnlock(const char* filename=__FILE__,int line=__LINE__);
	void SetPriority(RW_LOCK_PRIORITY Priority){m_Priority=Priority;}
};


class CLinuxNativeRWLock :public CReadWriteLock
{

private :

	pthread_rwlock_t m_RWLock;

	static void die_now(void);
public:
	CLinuxNativeRWLock();
	~CLinuxNativeRWLock();
	void Init();
	void Release();
	bool ReadLock(unsigned long timeout=15*60*1000,const char* filename=__FILE__,int line=__LINE__);
	void ReadUnlock(const char* filename=__FILE__,int line=__LINE__);
	bool WriteLock(unsigned long timeout=15*60*1000,const char* filename=__FILE__,int line=__LINE__);
	void WriteUnlock(const char* filename=__FILE__,int line=__LINE__);
	void SetPriority(RW_LOCK_PRIORITY Priority){m_Priority=Priority;}
};



#endif //#ifdef _WIN32
#endif // __RW_MUTEX__H__ 


