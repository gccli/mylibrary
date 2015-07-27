#ifndef __DAC_TCP_ACCEPT_ADAPTOR_H__
#define __DAC_TCP_ACCEPT_ADAPTOR_H__

#include "dac.h"
#include "net/netcomm.h"

class TCPAcceptAdaptor
{
public:
    TCPAcceptAdaptor(Connection *conn, Subscription* sub);
    virtual ~TCPAcceptAdaptor();
    
    void notify(comm_err_t flag, Connection *details);

    int init();

private:
    static void do_accept(int fd, void *data);

    void accept_once();
    void accept_next();
    comm_err_t Accept(Connection *details);

private:
    int       errcode;
    Connection *m_conn;
    Subscription *m_sub;
};

#endif

