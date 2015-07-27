#include "Connection.h"
#include "fde.h"
#include "dactime.h"
#include "netcomm.h"

/*bool
Comm::IsConnOpen(const Comm::ConnectionPointer &conn)
{
    return conn != NULL && conn->isOpen();
    }*/

Connection::Connection() :
        local(),
        remote(),
        fd(-1),
        flags(COMM_NONBLOCKING)
{
    memset(rfc931, 0, sizeof(rfc931));
}

Connection::~Connection()
{
    if (fd >= 0) {
        close();
    }
}
/*
ConnectionPointer
Connection::copyDetails() const
{
    ConnectionPointer c = new Connection;

    c->local = local;
    c->remote = remote;

    c->flags = flags;

    // ensure FD is not open in the new copy.
    c->fd = -1;

    // ensure we have a cbdata reference to peer_ not a straight ptr copy.

    return c;
    }*/

void Connection::close()
{
    if (isOpen()) {
        comm_close(fd);
        fd = -1;
    }
}
