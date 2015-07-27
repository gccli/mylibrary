#ifndef SQUID_EVENTLOOP_H
#define SQUID_EVENTLOOP_H

#include "daccomm.h"   
#include "dactime.h"   
#include <vector>

class AsyncEngine;
class TimeService;
class EventLoop
{
public:
    EventLoop();

    void RegisterEngine(AsyncEngine *engine);

    void run();
    void stop();

private:
    void CheckEngine(AsyncEngine * engine, bool const primary);
    bool DispatchCalls();

private:
    std::vector<AsyncEngine *> engines;
    TimeService timeservice;
    AsyncEngine *primary;
};

#endif
