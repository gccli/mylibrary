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
#ifndef WSA_FEEDBACK_H
#define WSA_FEEDBACK_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <list>
#include <map>

#include <pthread.h>

#define FB_ERR_LOG printf
#define FB_DBG_LOG printf
#define FB_WARN_LOG printf


#define MAX_FB_VERSION_LEN  6
#define MAX_LIB_VERSION_LEN 8
#define MAX_ANT_NAME_LEN    4
#define MAX_ANT_VERSION_LEN 4
#define MAX_DB_VERSION_LEN  8


typedef enum 
{
	ATTR_TYPE_URL          = 0x01,
	ATTR_TYPE_REFURL       = 0x02,
	ATTR_TYPE_UA           = 0x03,
	ATTR_TYPE_CUSTOMER_ID  = 0x04,
	ATTR_TYPE_USER_ID      = 0x05,
	ATTR_TYPE_PRODUCT_NAME = 0x06,
	ATTR_TYPE_PRODUCT_VER  = 0x07,
	ATTR_TYPE_FILENAME     = 0x08,
	ATTR_TYPE_URL_HASH     = 0x09,
	ATTR_TYPE_SCAN_RESTATTR= 0x0A,
	ATTR_TYPE_SCAN_REST    = 0x0B,
	ATTR_TYPE_TRAFFIC_DIR  = 0x0C
} FB_FeedType;

typedef enum
{
	FB_SRCID_SCANFLOW = 0x80,
} FB_SourceID;

typedef enum
{
	FB_ChunkType_Feedback = 1,
	FB_ChunkType_Statistics = 2,
} FB_ChunkType;


typedef struct _FB_ChunkAttr
{
	void *data;
	int   len;
	_FB_ChunkAttr()
		:data(NULL)
		,len(0)
	{}
	~_FB_ChunkAttr()
	{
		if (len > 0 && data)
			free(data);
		len = 0;
	}
	
} FB_ChunkAttr;



/*
// feed
<--- 2 byte ---><--------- 4 byte -------------><------ n type ------
|-------+-------+-------+-------+-------+-------+-------+-------+----
|   source id   |            length             |     playload
|-------+-------+-------+-------+-------+-------+-------+-------+----



// chunk header
<--------- 4 byte -------------><--------- 4 byte ------------->
|-------+-------+-------+-------+-------+-------+-------+-------+
|           Timestamp           |      type     |  feed number  |
|-------+-------+-------+-------+-------+-------+---- ~ ---------
|        attribute length       |           attribyte
|-------+-------+-------+-------+-------+-------+---- ~ ---------

*/

// global variables
extern int gFeedbackEnable;
extern int gFeedbackBufsize;

// feedback component

class FBChunk;
class FBComp
{
struct __FBBufferHdr
{
	char  fbver[MAX_FB_VERSION_LEN];
	char  libver[MAX_LIB_VERSION_LEN];
	// verpairnum 2 bytes
	// verpairnum * [engine_name 4 bytes, engine_version 4 bytes, db_version 8 bytes]
	// ts 4 bytes
} hdr;

public:
	FBComp();
	~FBComp();

	int Initialize(int flags);
	int Destroy();
	static FBComp& Instance();

	int Feedback(int srcid, const char *fbbuf, int len);

//	int Arrange(CCA_TRANS *pStream, CCA_SCAN_RESULT *pResult, int type);
//	int Arrange(CCA_SCAN_OBJECT *pScanObj, CCA_SCAN_RESULT *pResult, int type);

	int GatherFeedback(char *buffer, size_t buflen);

	void Cleanup();

public:
	static void AppendAttribute(FB_ChunkAttr *attr, uint8_t type, const char *buf, uint32_t len);
	int ArrangeInternal(int type, const void *info, int len);

private:


	unsigned int GetTotalSize(int antid);
	void CleanupList(std::list<FBChunk *> &rlist);
	void CleanupPeedingList();
	
private:
	static FBComp *inst;
	std::list<FBChunk *> chunk_list;
	std::map<int, FBChunk *> peeding_list;

	unsigned int m_bufsize;
 	unsigned int m_max_bufsize;
};

#ifdef __cplusplus
extern "C" {
#endif

int FBComp_feedback(int srcid, const char *fbbuf, int len);


#ifdef __cplusplus
}
#endif

#endif

