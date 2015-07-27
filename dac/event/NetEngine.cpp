#include "NetEngine.h"
#include "NetError.h"
#include "fde.h"
#include "dactime.h"

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/epoll.h>

static int kdpfd;
static int max_poll_time = 1000;
static struct epoll_event *pevents;

int NetModInit()
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
	printf("getrlimit - %s\n", strerror(errno));
	return -1;
    }
    dac_maxfd = rl.rlim_cur;
    if (dac_maxfd < DAC_MAX_FD){
	dac_maxfd = DAC_MAX_FD;

	rl.rlim_cur = dac_maxfd;
        if (rl.rlim_cur > rl.rlim_max)
            rl.rlim_max = rl.rlim_cur;
	if (setrlimit(RLIMIT_NOFILE, &rl)) {
	    printf("getrlimit - %s\n", strerror(errno));
	    return -1;
	}
    }    

    debugs("max fd %d", dac_maxfd);

    fd_table =(fde *) calloc(dac_maxfd, sizeof(fde));

    pevents = (struct epoll_event *) calloc(DAC_MAX_FD, sizeof(struct epoll_event));
    assert(pevents != NULL);
    kdpfd = epoll_create(DAC_MAX_FD);
    assert(kdpfd > 0);

    return 0;
}

void checkTimeouts(void)
{
    int fd;
    fde *F = NULL;
    AsyncCall::Pointer callback;
/*
    for (fd = 0; fd <= dac_maxfd; ++fd) {
        F = &fd_table[fd];

        if (writeTimedOut(fd)) {
            // We have an active write callback and we are timed out
            debugs(5, 5, "checkTimeouts: FD " << fd << " auto write timeout");
	    Comm::SetSelect(fd, COMM_SELECT_WRITE, NULL, NULL, 0);
            COMMIO_FD_WRITECB(fd)->finish(COMM_ERROR, ETIMEDOUT);
        } else if (AlreadyTimedOut(F))
            continue;

        debugs(5, 5, "checkTimeouts: FD " << fd << " Expired");

        if (F->timeoutHandler != NULL) {
            debugs(5, 5, "checkTimeouts: FD " << fd << ": Call timeout handler");
            callback = F->timeoutHandler;
            F->timeoutHandler = NULL;
            ScheduleCallHere(callback);
        } else {
            debugs(5, 5, "checkTimeouts: FD " << fd << ": Forcing comm_close()");
            comm_close(fd);
        }
    }
*/
}


static const char* epolltype_atoi(int x)
{
    switch (x) {

	case EPOLL_CTL_ADD:
	    return "EPOLL_CTL_ADD";

	case EPOLL_CTL_DEL:
	    return "EPOLL_CTL_DEL";

	case EPOLL_CTL_MOD:
	    return "EPOLL_CTL_MOD";

	default:
	    return "UNKNOWN_EPOLLCTL_OP";
    }
}

/**
 * This is a needed exported function which will be called to register
 * and deregister interest in a pending IO state for a given FD.
 */
void SetSelect(int fd, unsigned int type, PF * handler, void *client_data, time_t timeout)
{
    fde *F = &fd_table[fd];
    int epoll_ctl_type = 0;

    struct epoll_event ev;
    assert(fd >= 0);

    memset(&ev, 0, sizeof(ev));
    ev.events = 0;
    ev.data.fd = fd;

    if (!F->flags.open) {
        epoll_ctl(kdpfd, EPOLL_CTL_DEL, fd, &ev);
        return;
    }

    // If read is an interest
    if (type & COMM_SELECT_READ) {
        if (handler) {
            // Hack to keep the events flowing if there is data immediately ready
            if (F->flags.read_pending)
                ev.events |= EPOLLOUT;
            ev.events |= EPOLLIN;
        }

        F->read_handler = handler;
        F->read_data = client_data;
        // Otherwise, use previously stored value
    } else if (F->epoll_state & EPOLLIN) {
        ev.events |= EPOLLIN;
    }

    // If write is an interest
    if (type & COMM_SELECT_WRITE) {
        if (handler)
            ev.events |= EPOLLOUT;
        F->write_handler = handler;
        F->write_data = client_data;
        // Otherwise, use previously stored value
    } else if (F->epoll_state & EPOLLOUT) {
        ev.events |= EPOLLOUT;
    }

    if (ev.events)
        ev.events |= EPOLLHUP | EPOLLERR;

    if (ev.events != F->epoll_state) {
        if (F->epoll_state) // already monitoring something.
            epoll_ctl_type = ev.events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        else
            epoll_ctl_type = EPOLL_CTL_ADD;

        F->epoll_state = ev.events;

        if (epoll_ctl(kdpfd, epoll_ctl_type, fd, &ev) < 0) {
            debugs("epoll_ctl(%s)", epolltype_atoi(epoll_ctl_type));
        }
    }

    if (timeout)
        F->timeout = curtime + timeout;
}

void ResetSelect(int fd)
{
    fde *F = &fd_table[fd];
    F->epoll_state = 0;
    SetSelect(fd, 0, NULL, NULL, 0);
}


int ignore_errno(int ierrno)
{
    switch (ierrno) {
	case EINPROGRESS:
	case EWOULDBLOCK:
	case EALREADY:
	case EINTR:
	case ERESTART:
	    return 1;
	default:
	    return 0;
    }
}

comm_err_t DoSelect(int msec)
{
    int num, i,fd;
    fde *F;
    PF *hdl;

    struct epoll_event *cevents;

    if (msec > max_poll_time)
        msec = max_poll_time;

    for (;;) {
        num = epoll_wait(kdpfd, pevents, DAC_MAX_FD, msec);
        if (num >= 0)
            break;

        if (ignore_errno(errno))
            break;

        GetCurrentTime();

        return COMM_ERROR;
    }
    GetCurrentTime();

    if (num == 0)
        return COMM_TIMEOUT;/* No error.. */

    for (i = 0, cevents = pevents; i < num; ++i, ++cevents) {
        fd = cevents->data.fd;
        F = &fd_table[fd];
//        debugs(5, DEBUG_EPOLL ? 0 : 8, HERE << "got FD " << fd << " events=" <<
//               std::hex << cevents->events << " monitoring=" << F->epoll_state <<
//               " F->read_handler=" << F->read_handler << " F->write_handler=" << F->write_handler);
	debugs("got FD %d", fd);

        // TODO: add EPOLLPRI??
        if (cevents->events & (EPOLLIN|EPOLLHUP|EPOLLERR) || F->flags.read_pending) {
            if ((hdl = F->read_handler) != NULL) {
                debugs("Calling read handler on FD %d", fd);
                F->flags.read_pending = 0;
                F->read_handler = NULL;
                hdl(fd, F->read_data);
            } else {
                debugs("no read handler for FD %d", fd);
                SetSelect(fd, COMM_SELECT_READ, NULL, NULL, 0);
            }
        }

        if (cevents->events & (EPOLLOUT|EPOLLHUP|EPOLLERR)) {
            if ((hdl = F->write_handler) != NULL) {
                debugs("Calling write handler on FD %d", fd);
                F->write_handler = NULL;
                hdl(fd, F->write_data);
            } else {
                debugs("no write handler for FD %d", fd);
                // remove interest since no handler exist for this event.
                SetSelect(fd, COMM_SELECT_WRITE, NULL, NULL, 0);
            }
        }
    }

    return COMM_OK;
}


// NetEventEngine
////////////////////////////////////////////////////////////////
int NetEventEngine::checkEvents(int timeout)
{
    static time_t last_timeout = 0;

    /* No, this shouldn't be here. But it shouldn't be in each comm handler. -adrian */
    if (curtime > last_timeout) {
        last_timeout = curtime;
        checkTimeouts();
    }

    switch (DoSelect(timeout)) {
	case COMM_OK:
	case COMM_TIMEOUT:
	    return 0;

	case COMM_IDLE:
	case COMM_SHUTDOWN:
	    return EVENT_IDLE;

	case COMM_ERROR:
	    return EVENT_ERROR;

	default:
	    assert(false);
	    return EVENT_ERROR;
    };
}
