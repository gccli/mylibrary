#ifndef CLIENT_CONN_STATE_H__
#define CLIENT_CONN_STATE_H__

#include "netcomm.h"

class CliConnStat: public AsyncJob 
{
public:
    CliConnStat(Connection *conn);
    ~CliConnStat();

    void readdata();
    void readrequest(const CommIoCbParams &io);
    void *toCbdata() { return this; }

private:
    AsyncCall::Pointer reader;
    Connection *clientConnection;

    char *buff;
    long  size;
    long  used;
};

#endif

