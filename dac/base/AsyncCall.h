#ifndef ASYNC_CALL_H__
#define ASYNC_CALL_H__

#include "daccomm.h"
#include "RefCount.h"
#include "InstanceId.h"

class CallDialer;
class AsyncCallQueue;


class AsyncCall: public RefCountable
{
public:
    typedef RefCount <AsyncCall> Pointer;
    friend class AsyncCallQueue;
    AsyncCall(int aDebugSection, int aDebugLevel, const char *aName);
    virtual ~AsyncCall();

    void make();
    bool cancel(const char *reason);
    bool canceled() { return isCanceled != NULL; }    
    virtual CallDialer *getDialer() = 0;
    void print(std::ostream &os);
    void dequeue(AsyncCall::Pointer &head, AsyncCall::Pointer &prev);
    void setNext(AsyncCall::Pointer aNext) {
        m_next = aNext;
    }

    AsyncCall::Pointer &Next() {
        return m_next;
    }

  public:
    const char *const name;
    const int debugSection;
    const int debugLevel;
    const InstanceId<AsyncCall> id;

  protected:
    virtual bool canFire();
    virtual void fire() = 0;
    AsyncCall::Pointer m_next; // used exclusively by AsyncCallQueue

private:
    const char *isCanceled; // set to the cancelation reason by cancel()
    // not implemented to prevent nil calls from being passed around and unknowingly scheduled, for now.
    AsyncCall();
    AsyncCall(const AsyncCall &);
};

class CallDialer
{
public:
    CallDialer() {}
    virtual ~CallDialer() {}

    //virtual bool canDial(AsyncCall &call) = 0;
    //virtual void dial(AsyncCall &call) = 0;

    virtual void print(std::ostream &os) const = 0;
};

/**
 * This template implements an AsyncCall using a specified Dialer class
 */
template <class Dialer>
class AsyncCallT: public AsyncCall
{
public:
  AsyncCallT(int aDebugSection, int aDebugLevel, const char *aName, const Dialer &aDialer)
      : AsyncCall(aDebugSection, aDebugLevel, aName),
	dialer(aDialer) {}

  AsyncCallT(const AsyncCallT<Dialer> &o)
      : AsyncCall(o.debugSection, o.debugLevel, o.name),
	dialer(o.dialer) {}

    ~AsyncCallT() {}

    CallDialer *getDialer() { return &dialer; }

protected:
    virtual bool canFire() {
        return AsyncCall::canFire() && dialer.canDial(*this);
    }
    virtual void fire() { dialer.dial(*this); }
    Dialer dialer;

  private:
    AsyncCallT & operator=(const AsyncCallT &); // not defined. call assignments not permitted.
};

template <class Dialer>
inline AsyncCall *asyncCall(int aDebugSection, int aDebugLevel, const char *aName,
			    const Dialer &aDialer)
{
    return new AsyncCallT<Dialer>(aDebugSection, aDebugLevel, aName, aDialer);
}


class AsyncCallQueue
{
public:
    static AsyncCallQueue &Instance();

    // make this async call when we get a chance
    void schedule(AsyncCall::Pointer &call);

    // fire all scheduled calls; returns true if at least one was fired
    bool fire();

private:
    AsyncCallQueue();
    void fireNext();

    AsyncCall::Pointer m_head;
    AsyncCall::Pointer m_tail;
    static AsyncCallQueue *m_instance;
};

bool ScheduleCall(const char *fileName, int fileLine, AsyncCall::Pointer &call);
#define ScheduleCallHere(call) ScheduleCall(__FILE__,__LINE__,call)

#endif

