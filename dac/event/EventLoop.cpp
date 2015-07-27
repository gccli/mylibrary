#include "EventLoop.h"
#include "AsyncEngine.h"
#include "AsyncCall.h"

EventLoop::EventLoop()
{
}

void EventLoop::RegisterEngine(AsyncEngine *engine)
{
    engines.push_back(engine);
}

void EventLoop::run()
{
    while(true) {

	bool activity = false;
	primary = engines.back();

	do {
	    // generate calls and events
	    typedef std::vector<AsyncEngine *>::iterator EVI;
	    for (EVI i = engines.begin(); i != engines.end(); ++i) {
		if (*i != primary)
		    CheckEngine(*i, false);
	    }

	    // dispatch calls accumulated so far
	    activity = DispatchCalls();
	} while (activity);
 
	CheckEngine(primary, true);
	timeservice.tick();
	activity = DispatchCalls();
    }
}

void EventLoop::stop()
{
}


void EventLoop::CheckEngine(AsyncEngine * engine, bool const primary)
{
    int loop_delay = 1000;
    int requested_delay;

    if (!primary)
        requested_delay = engine->checkEvents(0);
    else
        requested_delay = engine->checkEvents(loop_delay);
    
    if (requested_delay < 0)
        switch (requested_delay) {
	    case AsyncEngine::EVENT_IDLE:
		debug5("Engine is idle\n");
		break;

	    case AsyncEngine::EVENT_ERROR:
		dacerror("error\n");
		break;
	    default:
		assert(0);
        }
    else {
        if (requested_delay < loop_delay)
            loop_delay = requested_delay;
    }
}

bool EventLoop::DispatchCalls()
{
    bool dispatched = AsyncCallQueue::Instance().fire();
    return dispatched;
}

