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
#ifndef WSA_FEEDBACK_CHUNK_H
#define WSA_FEEDBACK_CHUNK_H

#include <list>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// 
typedef struct __FBFeed
{
	__FBFeed(int id, unsigned int s, unsigned int n)
		:antid(id)
		,start(s)
		,len(n)
	{ }
	int          antid;
	unsigned int start;
	unsigned int len;
} FBFeed;

class FBChunk
{
public:
#pragma pack(1)	
	struct __FBChunkHdr
	{
		unsigned int   ts;      // timestamp, when the chunk create
		unsigned short type;    // chunk type
		unsigned short num;     // Feed Number
		unsigned int   attrlen; // Length of Attributes
		void          *attr;    // Attributes
	} hdr;	
#pragma pack()

public:
	FBChunk(int id);
	~FBChunk();

	int Insert(int antid, const char *fbbuf, int fblen);
	int SetAttribute(int type, const void *transattr, int attrlen);

	unsigned int TotalLength() 
	{
		return (contlen+hdr.attrlen+12+4);// sizeof(int)+sizeof(short)+sizeof(short)+sizeof(int)
	}
	unsigned int Length() const { return contlen; }
	unsigned int Length(int antid);
	const char *Buffer() const { return content; }
	const char *Buffer(int antid);

private:
	FBChunk(const FBChunk &);
	FBChunk & operator = (const FBChunk &);

private:
	int chunkid; // equal to thread id(lwpid)
	std::list<FBFeed> feedlist;
	char *content;
	unsigned int   contlen;
	unsigned int   bufsize;
};

#endif

