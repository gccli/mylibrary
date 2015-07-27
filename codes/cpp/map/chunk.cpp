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

#include "chunk.h"
#include "feedback.h"

using namespace std;

FBChunk::FBChunk(int id)
	:chunkid(id)
	,content(NULL)
	,contlen(0)
	,bufsize(0)
{
	memset(&hdr, 0, sizeof(hdr));
	hdr.ts = time(NULL);
}

FBChunk::~FBChunk()
{
	if (bufsize > 0 && content) {
		free(content);
		content = NULL;
		bufsize = 0;
		contlen = 0;
	}
	if (hdr.attrlen > 0 && hdr.attr)
	{
		free(hdr.attr);
	}
	feedlist.clear();
}


int FBChunk::Insert(int antid, const char *fbbuf, int fblen)
{
	if (fbbuf == NULL || fblen <=0)
	{
		FB_ERR_LOG("failed to insert feed, invalid parameters");
		return -1;		
	}
	
	if (fblen + contlen > bufsize)
	{
		char *pNew = NULL;
		unsigned int newlen = bufsize + 2*fblen + 64;
		if ((pNew = (char *)realloc(content, newlen)) == NULL)
		{
			FB_ERR_LOG("failed to realloc memory, current feed num:%d", feedlist.size());
			return -2;
		}

		bufsize = newlen;
		content = pNew;
	}

	feedlist.push_back(FBFeed(antid, contlen, fblen));

	memcpy(content+contlen, fbbuf, fblen);
	contlen += fblen;

	return 0;
}

int FBChunk::SetAttribute(int type, const void *transattr, int attrlen)
{
	hdr.type = type;
	hdr.num = feedlist.size();
	if (transattr && attrlen > 0)
	{
		hdr.attr = malloc(attrlen);
		memcpy(hdr.attr, transattr, attrlen);

		hdr.attrlen = attrlen;
	}

	return 0;
}

unsigned int FBChunk::Length(int antid)
{
	unsigned int len = 0;

	list<FBFeed>::const_iterator it = feedlist.begin();
	for (; it != feedlist.end(); ++it)
	{
		if (it->antid == antid)
		{
			len = it->len;
			break;
		}
	}

	return len;	
}

const char *FBChunk::Buffer(int antid)
{
	list<FBFeed>::const_iterator it = feedlist.begin();
	for (; it != feedlist.end(); ++it)
	{
		if (it->antid == antid)
			return (content+it->start);
	}

	return NULL;
}


