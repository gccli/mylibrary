#include "event.h"
#include "dactime.h"
#include "AsyncCall.h"
#include <math.h>

// EventDialer
// This AsyncCall dialer can be configured to check that the event cbdata is
// valid before calling the event handler
////////////////////////////////////////////////////////////////
class EventDialer: public CallDialer
{
public:
    typedef CallDialer Parent;

    EventDialer(EVH *aHandler, void *anArg, bool lockedArg);
    EventDialer(const EventDialer &d);
    virtual ~EventDialer();

    virtual void print(std::ostream &os) const;
    virtual bool canDial(AsyncCall &call);

    void dial(AsyncCall &) { theHandler(theArg); }

private:
    EVH *theHandler;
    void *theArg;
    bool isLockedArg;
};

EventDialer::EventDialer(EVH *aHandler, void *anArg, bool lockedArg):
    theHandler(aHandler), theArg(anArg), isLockedArg(lockedArg)
{
//    if (isLockedArg)
    //(void)cbdataReference(theArg);
}

EventDialer::EventDialer(const EventDialer &d):
    theHandler(d.theHandler), theArg(d.theArg), isLockedArg(d.isLockedArg)
{
//    if (isLockedArg)
    //(void)cbdataReference(theArg);
}

EventDialer::~EventDialer()
{
    //if (isLockedArg)
    //cbdataReferenceDone(theArg);
}

bool EventDialer::canDial(AsyncCall &call)
{
    // TODO: add Parent::canDial() that always returns true
    //if (!Parent::canDial())
    //    return false;

    //if (isLockedArg && !cbdataReferenceValid(theArg))
    //return call.cancel("stale handler data");

    return true;
}

void EventDialer::print(std::ostream &os) const
{
//    os << '(';
//    if (theArg)
//        os << theArg << (isLockedArg ? "*?" : "");
//    os << ')';
}


// ev_entry
////////////////////////////////////////////////////////////////
ev_entry::ev_entry(char const *n, EVH *f, void *a, double w,
		   int wt, bool have_argument) 
  :name(n)
  ,func(f)
  ,arg(have_argument ? a : NULL), when(w)
  ,weight(wt)
  ,cbdata(have_argument)
{
}

ev_entry::~ev_entry()
{
}

// EventScheduler
////////////////////////////////////////////////////////////////
EventScheduler EventScheduler::_instance;
EventScheduler::EventScheduler(): tasks(NULL)
{}
EventScheduler::~EventScheduler()
{
    clean();
}


void EventScheduler::cancel(EVH *func, void *arg)
{
    ev_entry **E;
    ev_entry *event;

    for (E = &tasks; (event = *E) != NULL; E = &(*E)->next) {
        if (event->func != func)
            continue;

        if (arg && event->arg != arg)
            continue;

        *E = event->next;

        delete event;

        if (arg)
            return;

        if (NULL == *E)
            break;
    }
}

int EventScheduler::timeRemaining() const
{
    if (!tasks)
        return EVENT_IDLE;

    if (tasks->when <= current_dtime) // we are on time or late
        return 0; // fire the event ASAP

    const double diff = tasks->when - current_dtime; // microseconds
    // Round UP: If we come back a nanosecond earlier, we will wait again!
    const int timeLeft = static_cast<int>(ceil(1000*diff)); // milliseconds
    // Avoid hot idle: A series of rapid select() calls with zero timeout.
    const int minDelay = 1; // millisecond
    return minDelay>timeLeft?minDelay:timeLeft;
}
static const char *last_event_ran;
int EventScheduler::checkEvents(int timeout)
{
    int result = timeRemaining();
    if (result != 0)
        return result;


    do {
        ev_entry *event = tasks;
        assert(event);

        /*XXX assumes event->name is static memory! */
	AsyncCall::Pointer call = asyncCall(41,5, event->name,
                                            EventDialer(event->func, event->arg, event->cbdata));
        ScheduleCall(__FILE__,__LINE__,call);

        last_event_ran = event->name; // XXX: move this to AsyncCallQueue
        const bool heavy = event->weight ;//&&
	    //(!event->cbdata || cbdataReferenceValid(event->arg));

        tasks = event->next;
        delete event;

        result = timeRemaining();

        // XXX: We may be called again during the same event loop iteration.
        // Is there a point in breaking now?
        if (heavy)
            break; // do not dequeue events following a heavy event
    } while (result == 0);

    return result;
}


void EventScheduler::clean()
{
    while (ev_entry *event = tasks) {
        tasks = event->next;
        delete event;
    }

    tasks = NULL;
}


//void EventScheduler::dump(StoreEntry *sentry)
//{
//}

bool EventScheduler::find(EVH *func, void *arg)
{
    ev_entry *event;

    for (event = tasks; event != NULL; event = event->next) {
        if (event->func == func && event->arg == arg)
            return true;
    }

    return false;
}

EventScheduler *EventScheduler::GetInstance()
{
    return &_instance;
}

void EventScheduler::schedule(const char *name, EVH *func, void *arg, double when, int weight, bool cbdata)
{
    // Use zero timestamp for when=0 events: Many of them are async calls that
    // must fire in the submission order. We cannot use current_dtime for them
    // because it may decrease if system clock is adjusted backwards.
    const double timestamp = when > 0.0 ? current_dtime + when : 0;
    ev_entry *event = new ev_entry(name, func, arg, timestamp, weight, cbdata);

    ev_entry **E;
//    debugs(41, 7, HERE << "schedule: Adding '" << name << "', in " << when << " seconds");
    /* Insert after the last event with the same or earlier time */

    for (E = &tasks; *E; E = &(*E)->next) {
        if ((*E)->when > event->when)
            break;
    }

    event->next = *E;
    *E = event;
}
