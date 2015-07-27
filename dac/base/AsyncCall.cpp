#include "AsyncCall.h"

// AsyncCall
////////////////////////////////////////////////////////////////
InstanceIdDefinitions(AsyncCall, "call");

AsyncCall::AsyncCall(int aDebugSection, int aDebugLevel, const char *aName)
    : name(aName), debugSection(aDebugSection)
    , debugLevel(aDebugLevel), m_next(0), isCanceled(NULL)
{
    
}

AsyncCall::~AsyncCall()
{
    assert(!m_next); // AsyncCallQueue must clean
}

void AsyncCall::make()
{
    debugs("make call %s [%d]", name, id.value);
    if (canFire()) {
        fire();
        return;
    }

    if (!isCanceled) // we did not cancel() when returning false from canFire()
        isCanceled = "unknown reason";

//    debugs(debugSection, debugLevel, HERE << "will not call " << name << " [" << id << ']' << " because of " << isCanceled);
}

bool AsyncCall::cancel(const char *reason)
{
//    debugs(debugSection, debugLevel, HERE << "will not call " << name <<
//           " [" << id << "] " << (isCanceled ? "also " : "") <<
//           "because " << reason);

    isCanceled = reason;
    return false;
}

bool AsyncCall::canFire()
{
    return !isCanceled;
}

void AsyncCall::print(std::ostream &os)
{
    os << name;
    if (const CallDialer *dialer = getDialer())
        dialer->print(os);
    else
        os << "(?" << this << "?)";
}

void AsyncCall::dequeue(AsyncCall::Pointer &head, AsyncCall::Pointer &prev)
{
    if (prev != NULL)
        prev->setNext(Next());
    else
        head = Next();
    setNext(NULL);
}

// AsyncCallQueue
////////////////////////////////////////////////////////////////
AsyncCallQueue *AsyncCallQueue::m_instance = 0;
AsyncCallQueue::AsyncCallQueue(): m_head(NULL), m_tail(NULL)
{
}

void AsyncCallQueue::schedule(AsyncCall::Pointer &call)
{
    assert(call != NULL);
    assert(!call->m_next);
    if (m_head != NULL) { // append
        assert(!m_tail->m_next);
        m_tail->m_next = call;
        m_tail = call;
    } else { // create queue from cratch
        m_head = m_tail = call;
    }
}

// Fire all scheduled calls; returns true if at least one call was fired.
// The calls may be added while the current call is in progress.
bool
AsyncCallQueue::fire()
{
    const bool made = m_head != NULL;
    while (m_head != NULL)
        fireNext();
    return made;
}

void
AsyncCallQueue::fireNext()
{
    AsyncCall::Pointer call = m_head;
    m_head = call->m_next;
    call->m_next = NULL;
    if (m_tail == call)
        m_tail = NULL;

//    debugs(call->debugSection, call->debugLevel, "entering " << *call);
    call->make();
//    debugs(call->debugSection, call->debugLevel, "leaving " << *call);
}

AsyncCallQueue &AsyncCallQueue::Instance()
{
    if (!m_instance)
        m_instance = new AsyncCallQueue();
   
    return *m_instance;
}

// Global functions
////////////////////////////////////////////////////////////////
bool ScheduleCall(const char *fileName, int fileLine, AsyncCall::Pointer &call)
{
//    debugs(call->debugSection, call->debugLevel, fileName << "(" << fileLine <<
//           ") will call " << *call << " [" << call->id << ']' );
    AsyncCallQueue::Instance().schedule(call);
    return true;
}
