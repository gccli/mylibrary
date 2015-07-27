#include "Tcpserver.h"
#include "NetEngine.h"
#include "fde.h"

TCPAcceptAdaptor::TCPAcceptAdaptor(Connection *conn, Subscription *sub)
    :m_conn(conn)
    ,m_sub(sub)
{
}

TCPAcceptAdaptor::~TCPAcceptAdaptor() 
{
}

int TCPAcceptAdaptor::init()
{
    if (listen(m_conn->fd, dac_maxfd >> 2) < 0) {
	dacerror("failed to listen address: %s\n", strerror(errno));
	return 1;
    }

    debugs("*** TCP server started, waiting for client connection...");
    SetSelect(m_conn->fd, COMM_SELECT_READ, do_accept, this, 0);

    return 0;
}

void TCPAcceptAdaptor::notify(comm_err_t flag, Connection *details)
{
    if (flag == COMM_ERR_CLOSING) {
	return;
    }

    AsyncCall::Pointer call = m_sub->callback();
    CommAcceptCbParams &params = GetCommParams<CommAcceptCbParams>(call);
    params.fd = m_conn->fd;
    params.conn = details;
    params.flag = flag;
    params.xerrno = errcode;
    ScheduleCallHere(call);
}

void TCPAcceptAdaptor::do_accept(int fd, void *data)
{
    debugs("New connection on FD %d", fd);

    assert(fd > 0);
    TCPAcceptAdaptor *afd = static_cast<TCPAcceptAdaptor*>(data);
    afd->accept_next();

    SetSelect(fd, COMM_SELECT_READ, TCPAcceptAdaptor::do_accept, afd, 0);
}

void TCPAcceptAdaptor::accept_once()
{
    /* Accept a new connection */
    Connection* newconn = new Connection();
    const comm_err_t flag = Accept(newconn);

    /* Check for errors */
    if (!newconn->isOpen()) {
        if (flag == COMM_NOMESSAGE) {
            /* register interest again */
            debugs("try later ");
            SetSelect(m_conn->fd, COMM_SELECT_READ, do_accept, this, 0);
            return;
        }

        // A non-recoverable error; notify the caller */
        debugs("non-recoverable error.");

        return;
    }

    char buf[64] = {0};

    debugs("Listener: accepted new connection from %s, fd %d", newconn->remote.NtoA(buf,sizeof(buf)), newconn->fd);
    notify(flag, newconn);
}

void TCPAcceptAdaptor::accept_next()
{
    accept_once();
}

comm_err_t TCPAcceptAdaptor::Accept(Connection *details)
{
    int sock;
    struct addrinfo *gai = NULL;
    details->local.InitAddrInfo(gai);

    errcode = 0; // reset local errno copy.
    if ((sock = accept(m_conn->fd, gai->ai_addr, &gai->ai_addrlen)) < 0) {
        errcode = errno; // store last accept errno locally.

        details->local.FreeAddrInfo(gai);

        if (ignore_errno(errno)) {
            return COMM_NOMESSAGE;
        } else if (ENFILE == errno || EMFILE == errno) {
            return COMM_ERROR;
        } else {
            return COMM_ERROR;
        }
    }

    assert(sock >= 0);
    details->fd = sock;
    details->remote = *gai;

    // lookup the local-end details of this new connection
    details->local.InitAddrInfo(gai);
    details->local.SetEmpty();
    getsockname(sock, gai->ai_addr, &gai->ai_addrlen);
    details->local = *gai;
    details->local.FreeAddrInfo(gai);

    /* fdstat update */
    // XXX : these are not all HTTP requests. use a note about type and ip:port details->
    // so we end up with a uniform "(HTTP|FTP-data|HTTPS|...) remote-ip:remote-port"
    fd_open(sock, FD_SOCKET, "HTTP Request");

//    fdd_table[sock].close_file = NULL;
//    fdd_table[sock].close_line = 0;

    fde *F = &fd_table[sock];
    details->remote.NtoA(F->ipaddr,MAX_IPSTRLEN);
    F->remote_port = details->remote.GetPort();
    F->local_addr = details->local;
    F->sock_family = details->local.IsIPv6()?AF_INET6:AF_INET;

    // set socket flags
    fd_set_close_onexec(sock);
    fd_set_nonblocking(sock);

    /* IFF the socket is (tproxy) transparent, pass the flag down to allow spoofing */
    F->flags.transparent = fd_table[m_conn->fd].flags.transparent; // XXX: can we remove this line yet?


    return COMM_OK;
}
