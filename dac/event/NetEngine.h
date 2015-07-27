#ifndef NET_IO_ENGINE_H__
#define NET_IO_ENGINE_H__

#include "daccomm.h"
#include "AsyncEngine.h"
// Global functions
////////////////////////////////////////////////////////////////


// NetEventEngine
////////////////////////////////////////////////////////////////

class NetEventEngine : public AsyncEngine
{
public:
    virtual int checkEvents(int timeout);
};

#endif
