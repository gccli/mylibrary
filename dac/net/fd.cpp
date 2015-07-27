#include "netcomm.h"
#include "NetEngine.h"
#include "NetError.h"
#include <fcntl.h>

void fd_close(int fd)
{
    fde *F = &fd_table[fd];

    assert(fd >= 0);
    assert(F->flags.open);

    if (F->type == FD_FILE) {
        assert(F->read_handler == NULL);
        assert(F->write_handler == NULL);
    }

    SetSelect(fd, COMM_SELECT_READ, NULL, NULL, 0);
    SetSelect(fd, COMM_SELECT_WRITE, NULL, NULL, 0);
    F->flags.open = false;
    *F = fde();
}

int socket_read_method(int fd, char *buf, int len)
{
    int i;
    i = recv(fd, (void *) buf, len, 0);
    return i;
}

int socket_write_method(int fd, const char *buf, int len)
{
    int i;
    i = send(fd, (const void *) buf, len, 0);
    return i;
}

int default_read_method(int fd, char *buf, int len)
{
    int i;
    i = read(fd, buf, len);
    return i;
}

int default_write_method(int fd, const char *buf, int len)
{
    int i;
    i = write(fd, buf, len);
    return i;
}

int msghdr_read_method(int fd, char *buf, int len)
{
    int i = recvmsg(fd, reinterpret_cast<msghdr*>(buf), MSG_DONTWAIT);

    return i;
}

int msghdr_write_method(int fd, const char *buf, int len)
{

    const int i = sendmsg(fd, reinterpret_cast<const msghdr*>(buf), MSG_NOSIGNAL);

    return i > 0 ? len : i; // len is imprecise but the caller expects a match
}

void fd_open(int fd, unsigned int type, const char *desc)
{
    fde *F;
    assert(fd >= 0);
    F = &fd_table[fd];

    if (F->flags.open) {
        debugs("Closing open FD %d", fd);
        fd_close(fd);
    }

    assert(!F->flags.open);
    debugs("open FD %d", fd);
    F->type = type;
    F->flags.open = true;
    F->epoll_state = 0;

    switch (type) {

    case FD_MSGHDR:
        F->read_method = &msghdr_read_method;
        F->write_method = &msghdr_write_method;
	break;

    default:
	F->read_method = &default_read_method;
	F->write_method = &default_write_method;
	break;
    }

    if (desc)
        strncpy(F->desc, desc, FD_DESC_SZ);
}

void fd_note(int fd, const char *s)
{
    fde *F = &fd_table[fd];
    strncpy(F->desc, s, FD_DESC_SZ);
}

void fd_bytes(int fd, int len, unsigned int type)
{
    fde *F = &fd_table[fd];

    if (len < 0)
	return;

    assert(type == FD_READ || type == FD_WRITE);

    if (type == FD_READ)
	F->bytes_read += len;
    else
	F->bytes_written += len;
}


int fd_set_nonblocking(int fd, int on)
{
    int flags = 0;
    if ((flags = fcntl (fd, F_GETFL, 0)) < 0) {
	fprintf (stderr, "fcntl(F_GETFL) error: %d (%s)\n", errno, strerror(errno));
	return 1;
    }

    if (on)
	flags |= O_NONBLOCK;
    else 
	flags &= ~O_NONBLOCK;

    if ((flags = fcntl (fd, F_SETFL, flags)) < 0) {
	fprintf (stderr, "fcntl(F_SETFL) error: %d (%s)\n", errno, strerror(errno));
	return COMM_ERROR;
    }

    fd_table[fd].flags.nonblocking = true;

    return 0;
}

int fd_unset_nonblocking(int fd)
{
    return fd_set_nonblocking(fd, 0);
}

void fd_set_close_onexec(int fd)
{
    int flags;
    int dummy = 0;

    if ((flags = fcntl(fd, F_GETFD, dummy)) < 0) {
	debugs("failed");
        return;
    }

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
	debugs("failed");

    fd_table[fd].flags.close_on_exec = true;
}
