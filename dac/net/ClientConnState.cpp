#include "ClientConnState.h"

#define CLIENT_BUFSIZE  4096

CliConnStat::CliConnStat(Connection *conn)
    :AsyncJob("ClientState")
    ,reader(NULL)
    ,clientConnection(conn)
{
    size = CLIENT_BUFSIZE;
    buff = (char *)calloc(1, size+64);
    used = 0;
}

CliConnStat::~CliConnStat()
{
}

void CliConnStat::readdata()
{
    if (reader != NULL)
	return ;

    debugs("reading request...");
    
    typedef CommCbMemFunT<CliConnStat, CommIoCbParams> Dialer;
    reader = JobCallback(33, 5, Dialer, this, CliConnStat::readrequest);
    comm_read(clientConnection, buff+used, size-used, reader);    
}

void CliConnStat::readrequest(const CommIoCbParams &io)
{
    debugs("request size %d", io.size);
    debug5("reader %p", reader);
    reader = NULL;

    /* Bail out quickly on COMM_ERR_CLOSING - close handlers will tidy up */

    if (io.flag == COMM_ERR_CLOSING) {
        debugs("closing Bailout.");
        return;
    }

    //assert(Comm::IsConnOpen(clientConnection));
    //assert(io.conn->fd == clientConnection->fd);

    /*
     * Don't reset the timeout value here.  The timeout value will be
     * set to Config.Timeout.request by httpAccept() and
     * clientWriteComplete(), and should apply to the request as a
     * whole, not individual read() calls.  Plus, it breaks our
     * lame half-close detection
     */
/*    if (connReadWasError(io.flag, io.size, io.xerrno)) {
        notifyAllContexts(io.xerrno);
        io.conn->close();
        return;
    }
*/
    if (io.flag == COMM_OK) {
        if (io.size > 0) {
//            kb_incr(&(statCounter.client_http.kbytes_in), io.size);

            // may comm_close or setReplyToError
	    //if (!handleReadData(io.buf, io.size))
	    //  return;
	    used += io.size;
        } else if (io.size == 0) {
            debugs("closed?");
/*
            if (connFinishedWithConn(io.size)) {
                clientConnection->close();
                return;
            }
*/
            /* It might be half-closed, we can't tell */
            fd_table[io.conn->fd].flags.socket_eof = 1;

	    //          commMarkHalfClosed(io.conn->fd);

            fd_note(io.conn->fd, "half-closed");

            /* There is one more close check at the end, to detect aborted
             * (partial) requests. At this point we can't tell if the request
             * is partial.
             */
            /* Continue to process previously read data */
        }
    }

    /* Process next request */
    readdata();
}
