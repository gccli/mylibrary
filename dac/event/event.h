#ifndef EVENT_H__
#define EVENT_H__

#include "daccomm.h"
#include "AsyncEngine.h"

typedef void EVH(void *);

class ev_entry
{
public:
    ev_entry(char const * name, EVH * func, void *arg, double when, int weight, bool cbdata=true);
    ~ev_entry();
    const char *name;
    EVH *func;
    void *arg;
    double when;

    int weight;
    bool cbdata;

    ev_entry *next;
};

// manages time-based events
class EventScheduler : public AsyncEngine
{
public:
    EventScheduler();
    ~EventScheduler();
    /* cancel a scheduled but not dispatched event */
    void cancel(EVH * func, void * arg);
    /* clean up the used memory in the scheduler */
    void clean();
    /* either EVENT_IDLE or milliseconds remaining until the next event */
    int timeRemaining() const;
    /* cache manager output for the event queue */
//    void dump(StoreEntry *);
    /* find a scheduled event */
    bool find(EVH * func, void * arg);
    /* schedule a callback function to run in when seconds */
    void schedule(const char *name, EVH * func, void *arg, double when, int weight, bool cbdata=true);
    int checkEvents(int timeout);
    static EventScheduler *GetInstance();

  private:
    static EventScheduler _instance;
    ev_entry * tasks;
};

#endif
