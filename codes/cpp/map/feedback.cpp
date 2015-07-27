/////////////////////////////////////////////////////////////////////////////
//
//                          PROPRIETARY MATERIALS
//
//   Websense, Inc. (Websense) has prepared this material for use by
//   Websense personnel, licensees, and customers.  The information
//   contained herein is the property of Websense and shall not be
//   reproduced in whole or part without the prior written consent of an
//   authorized representative of Websense, Inc.
//
//                          RESTRICTED RIGHTS LEGEND
//
//   Use, duplication  or disclosure  by the U.S. Government is subject
//   to restrictions as set forth in Technical Data and Computer Software
//   at 48 CFR 252.227-7015 and 48 CFR 227.27.  All other Government use,
//   duplication or disclosure shall be governed exclusively by the terms
//   of the Websense Subscription Agreement. The manufacturer is Websense,
//   Inc.
//
//
//                          Copyright (c) 1997 - 2007
//                             All Rights Reserved
//                               Websense, Inc.
//                          10240 Sorrento Valley Rd
//                            San Diego, CA 92121
//                              (858) 320-8000
//
/////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <assert.h>
#include <sys/syscall.h>

#include <string.h>
#include "feedback.h"
#include "chunk.h"
#include "readwritelock.h"

#include <string>
using namespace std;


#define VSRWLOCK_T CLinuxNativeRWLock

#define VSRwLockInit(n) (n.Init())
#define VSRwLockDestroy(n) (n.Release())

#define VSReadLock(n) (n.ReadLock())
#define VSWriteLock(n) (n.WriteLock())
#define VSReadUnLock(n)  (n.ReadUnlock())
#define VSWriteUnLock(n) (n.WriteUnlock())

static VSRWLOCK_T gFeedback_RWlock; 

#define gettid() syscall(__NR_gettid)
#define MIN_BUFFER_SIZE  10240
#define MAX_BUFFER_SIZE  204800



int gFeedbackEnable=1;
int gFeedbackBufsize=MAX_BUFFER_SIZE;

FBComp *FBComp::inst = NULL;
int FBComp::Destroy()
{
	if (!gFeedbackEnable) return 0;

    Cleanup();

    return 0;
}
FBComp::FBComp()
	:m_bufsize(0)
 	,m_max_bufsize(MAX_BUFFER_SIZE)
{
	snprintf(hdr.fbver, MAX_FB_VERSION_LEN, "%d.%d", 1,3);
	strncpy(hdr.libver, "7.8", MAX_LIB_VERSION_LEN);
}

FBComp::~FBComp()
{
}

FBComp& FBComp::Instance()
{
	if (inst == NULL)
	{
		inst = new FBComp;
	}

	return *inst;
}

int FBComp::Initialize(int flags)
{
	if (!gFeedbackEnable) return 0;

	int bufsz = gFeedbackBufsize;
	bufsz = (bufsz < MIN_BUFFER_SIZE) ? MIN_BUFFER_SIZE : bufsz;
	bufsz = (bufsz > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : bufsz;
	m_max_bufsize = bufsz;

	VSRwLockInit(gFeedback_RWlock);

	FB_DBG_LOG("feedback component initialized, max buffer size is %u\n", m_max_bufsize);

    return 0;
}

void FBComp::Cleanup()
{
	if (!gFeedbackEnable) return ;

    if(!VSWriteLock(gFeedback_RWlock)) {
		FB_ERR_LOG("failed to require r/w lock");
		return ;
	}

	CleanupList(chunk_list);
	m_bufsize = 0;
	CleanupPeedingList();

	VSWriteUnLock(gFeedback_RWlock);
	VSRwLockDestroy(gFeedback_RWlock);

}

void FBComp::CleanupList(std::list<FBChunk *> &rlist)
{
	std::list<FBChunk *>::iterator j = rlist.begin();
	for (; j != rlist.end(); ++j)
		delete *j; // 
	rlist.clear();	
}

void FBComp::CleanupPeedingList()
{	
	std::map<int, FBChunk *>::iterator i = peeding_list.begin();
	for(; i != peeding_list.end(); ++i)
		delete i->second;
	peeding_list.clear();
}

unsigned int FBComp::GetTotalSize(int srcid)
{
	unsigned int sz = 0;
	std::list<FBChunk *>::iterator it = chunk_list.begin();
	for (; it != chunk_list.end(); ++it)
		sz += (*it)->Length(srcid);

	return sz;
}

int FBComp::Feedback(int srcid, const char *fbbuf, int fblen)
{
	if (!gFeedbackEnable) return 0;

	if (fbbuf == NULL || fblen <= 0) {
		FB_WARN_LOG("invalid parameters, id:%d buffer:%p length:%d", srcid, fbbuf, fblen);
		return -1;
	}
	if ((unsigned int )fblen > m_max_bufsize) {
		FB_WARN_LOG("id:%d feedback data too large", srcid);
		return -1;
	}

	int ret = -1;
	int tid = (int)gettid();

	FB_DBG_LOG("generate feedback from source %d, buffer %p length %d\n", srcid, fbbuf, fblen);
    if(!VSReadLock(gFeedback_RWlock)) {
		FB_ERR_LOG("Failed to require r/w (read) lock");
		return -1;
    }

	std::map<int, FBChunk *>::iterator it = peeding_list.find(tid);
	if (it == peeding_list.end()) {
		VSReadUnLock(gFeedback_RWlock); // read lock
		FBChunk *pChunk = new FBChunk(tid);
		ret = pChunk->Insert(srcid, fbbuf, fblen);
		if (ret == 0)
		{
			if(!VSWriteLock(gFeedback_RWlock)) {
				FB_ERR_LOG("failed to require r/w (write) lock");
				delete pChunk;
				return -1;
			}
			peeding_list[tid] = pChunk;
			VSWriteUnLock(gFeedback_RWlock);
		}
		else {
			delete pChunk;
		}
	}
	else {
		ret = it->second->Insert(srcid, fbbuf, fblen);
		VSReadUnLock(gFeedback_RWlock); // read lock
	}

    return ret;
}

int FBComp::ArrangeInternal(int type, const void *info, int len)
{
	pid_t tid = gettid();
	FBChunk *pChunk;
	unsigned int chunklen;

    if(!VSReadLock(gFeedback_RWlock)) {
		FB_ERR_LOG("Failed to require r/w (read) lock\n");
		return -1;
    }

	std::map<int, FBChunk *>::iterator it = peeding_list.find(tid);
	if (it == peeding_list.end()) {
		VSReadUnLock(gFeedback_RWlock); // read lock
		FB_WARN_LOG("thread[%d] has no chunk exist\n", tid);
		return -1;
	}

	if ((pChunk = it->second) == NULL) {
		VSReadUnLock(gFeedback_RWlock); // read lock
		FB_ERR_LOG("thread[%d] null chunk\n", tid);
		return -1;
	}

	VSReadUnLock(gFeedback_RWlock); // read lock
	
	pChunk->SetAttribute(type, info, len);
	chunklen = pChunk->TotalLength();

	if(!VSWriteLock(gFeedback_RWlock)) {
		FB_ERR_LOG("failed to require r/w (write) lock\n");
		delete pChunk;
		return -1;
	}

	int oldbufsz = m_bufsize;
	if (oldbufsz + chunklen > m_max_bufsize)
	{
		FB_ERR_LOG("buffer not enough, %d/%d, drop this chunk(len:%d), chunk:%d pending:%d\n", 
			m_bufsize, m_max_bufsize, chunklen, chunk_list.size(), peeding_list.size());
		peeding_list.erase(it);
		delete pChunk;
		VSWriteUnLock(gFeedback_RWlock);
		return -1;
	}

	m_bufsize += chunklen;

	peeding_list.erase(it);
	chunk_list.push_back(pChunk);
	FB_DBG_LOG("thread[%d] arrange chunk, length:%d, number:%d, peeding:%d, bufsize %d -> %d\n",
		tid, chunklen, chunk_list.size(), peeding_list.size(), oldbufsz, m_bufsize);

	VSWriteUnLock(gFeedback_RWlock);

    return 0;
}

int FBComp::GatherFeedback(char *buffer, size_t buflen)
{
	if (!gFeedbackEnable) return 0;
	if (!buffer || buflen == 0) {
		FB_ERR_LOG("invalid parameters");
		return -1;
	}
	
	list<FBChunk *> mylist;

	unsigned int offset=0, hdrsize=0, curbufsz=0;

	uint16_t version_num = 0;
	uint16_t verpair_len = MAX_ANT_NAME_LEN+MAX_ANT_VERSION_LEN+MAX_DB_VERSION_LEN;	
	char temp[128] = {0};
	string version_buf;


	hdrsize = sizeof(short)+MAX_LIB_VERSION_LEN+sizeof(short);
	hdrsize = verpair_len*version_num+sizeof(int);

	if (buflen < hdrsize+4) // header length + sizeof(int)
	{
		FB_ERR_LOG("buffer is too small to fill feedback header");
		return -1;
	}

	if(!VSWriteLock(gFeedback_RWlock)) {
		FB_ERR_LOG("failed to require r/w (write) lock");
		return -1;
	}

	if (m_bufsize == 0)
	{
		FB_WARN_LOG("no feedback data ready\n");
		VSWriteUnLock(gFeedback_RWlock);
		return 0;
	}

	FB_DBG_LOG("cleanup chunk list, current chunk:%d, peeding:%d, bufsize:%d\n",
		chunk_list.size(), peeding_list.size(), m_bufsize);
	curbufsz = m_bufsize; m_bufsize = 0;
	mylist = chunk_list; chunk_list.clear();
	VSWriteUnLock(gFeedback_RWlock);

	if (curbufsz > buflen)
	{
		FB_WARN_LOG("no enough buffer, feedback data will not complete");
	}

	unsigned int ts = (unsigned int)time(NULL); // danger

	// packet header
	memcpy(buffer, &hdr.fbver, MAX_FB_VERSION_LEN);
	offset += MAX_FB_VERSION_LEN;
	memcpy(buffer+offset, &hdr.libver, MAX_LIB_VERSION_LEN);
	offset += MAX_LIB_VERSION_LEN;
	memcpy(buffer+offset, &version_num, MAX_LIB_VERSION_LEN);
	offset += sizeof(short);
	if (version_num > 0) {
		memcpy(buffer+offset, version_buf.c_str(), version_buf.size());
		offset += version_buf.size();
	}
	memcpy(buffer+offset, &ts, sizeof(int));
	offset += sizeof(int);

	// packet content length
	memcpy(buffer+offset, &curbufsz, sizeof(int));
	offset += sizeof(int);

	// packet content
	hdrsize = sizeof(FBChunk::__FBChunkHdr)-sizeof(void *);
	FBChunk *p = NULL;
	std::list<FBChunk *>::iterator it = mylist.begin();
	for(; it != mylist.end(); ++it)
	{
		p = *it;
		unsigned int contlen = p->Length();
		unsigned int total = p->TotalLength();
		if (offset+total > buflen)
			goto _END;

		// chunk header
		memcpy(buffer+offset, &p->hdr, hdrsize);
		offset += hdrsize;
		if (p->hdr.attrlen > 0 && p->hdr.attr) {
			memcpy(buffer+offset, p->hdr.attr, p->hdr.attrlen);
			offset += p->hdr.attrlen;
		}

		// chunk length
		memcpy(buffer+offset, &contlen, 4);
		offset += 4;

		// chunk content
		if (contlen > 0)
		{
			memcpy(buffer+offset, p->Buffer(), contlen);
			offset += contlen;
		}
	}

_END:

	// free memory
	CleanupList(mylist);

	return offset;	
}


void FBComp::AppendAttribute(FB_ChunkAttr *attr, uint8_t type, const char *buf, uint32_t len)
{
	attr->data = realloc(attr->data, attr->len + len + sizeof(char)+sizeof(short));
	if (attr->data == NULL)
	{
		FB_ERR_LOG("memory allocate failure");
		return ;
	}
	unsigned char *p = (unsigned char *) attr->data;
	p += attr->len;
	*p = type;
	p++;
	memcpy(p, &len, sizeof(short));
	p += sizeof(short);
	memcpy(p, buf, len);

	attr->len += (sizeof(char)+sizeof(short)+len);
}


//////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

int FBComp_feedback(int srcid, const char *fbbuf, int len)
{
    FBComp::Instance().Feedback(srcid,fbbuf,len);
    return 0;
}

#ifdef __cplusplus
} // extern "C"
#endif
