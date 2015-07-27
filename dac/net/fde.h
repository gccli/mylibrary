#ifndef FD_ENTRY_H__
#define FD_ENTRY_H__

#include "daccomm.h"
#include "fd.h"
#include "ip/Address.h"
#include "AsyncCall.h"
#include <openssl/ssl.h>

class fde
{

  public:
    fde() { clear(); };

    /// True if comm_close for this fd has been called
    bool closing() { return flags.close_request; }

    /* NOTE: memset is used on fdes today. 20030715 RBC */
//    static void DumpStats (StoreEntry *);

    char const *remoteAddr() const;
//    void dumpStats (StoreEntry &, int);
    bool readPending(int);
//    void noteUse(PconnPool *);

  public:

    /// global table of FD and their state.
    static fde* Table;

    unsigned int type;
    unsigned short remote_port;

    Ip::Address local_addr;
//    tos_t tosToServer;          /**< The TOS value for packets going towards the server.
//				   See also tosFromServer. */
    mode_t nfmarkToServer;    /**< The netfilter mark for packets going towards the server.
//				   See also nfmarkFromServer. */
    int sock_family;
    char ipaddr[MAX_IPSTRLEN];            /* dotted decimal address of peer */
    char desc[FD_DESC_SZ];

    struct _fde_flags {
        unsigned int open:1;
        unsigned int close_request:1; // file_ or comm_close has been called
        unsigned int write_daemon:1;
        unsigned int socket_eof:1;
        unsigned int nolinger:1;
        unsigned int nonblocking:1;
        unsigned int ipc:1;
        unsigned int called_connect:1;
        unsigned int nodelay:1;
        unsigned int close_on_exec:1;
        unsigned int read_pending:1;
        unsigned int write_pending:1;
        unsigned int transparent:1;
    } flags;

    int64_t bytes_read;
    int64_t bytes_written;

    struct {
        int uses;                   /* ie # req's over persistent conn */
//        PconnPool *pool;
    } pconn;

#if USE_DELAY_POOLS
    ClientInfo * clientInfo;/* pointer to client info used in client write limiter or NULL if not present */
#endif
    unsigned epoll_state;

//    _fde_disk disk;
    PF *read_handler;
    void *read_data;
    PF *write_handler;
    void *write_data;
    AsyncCall::Pointer timeoutHandler;
    time_t timeout;
    time_t writeStart;
    void *lifetime_data;
    AsyncCall::Pointer closeHandler;
    AsyncCall::Pointer halfClosedReader; /// read handler for half-closed fds
//    CommWriteStateData *wstate;         /* State data for comm_write */
    READ_HANDLER *read_method;
    WRITE_HANDLER *write_method;

    SSL *ssl;
    SSL_CTX *dynamicSslContext; ///< cached and then freed when fd is closed


//    tos_t tosFromServer;                /**< Stores the TOS flags of the packets from the remote server.
//                                            See FwdState::dispatch(). Note that this differs to
//                                            tosToServer in that this is the value we *receive* from the,
//                                            connection, whereas tosToServer is the value to set on packets
//                                            *leaving* Squid.  */
    unsigned int nfmarkFromServer;      /**< Stores the Netfilter mark value of the connection from the remote
                                            server. See FwdState::dispatch(). Note that this differs to
                                            nfmarkToServer in that this is the value we *receive* from the,
                                            connection, whereas nfmarkToServer is the value to set on packets
                                            *leaving* Squid.   */

  private:
    /** Clear the fde class back to NULL equivalent. */
    inline void clear() {
        type = 0;
        remote_port = 0;
        local_addr.SetEmpty();
//        tosToServer = '\0';
        nfmarkToServer = 0;
        sock_family = 0;
        memset(ipaddr, '\0', MAX_IPSTRLEN);
        memset(desc,'\0',FD_DESC_SZ);
        memset(&flags,0,sizeof(_fde_flags));
        bytes_read = 0;
        bytes_written = 0;
        pconn.uses = 0;
//        pconn.pool = NULL;
#if USE_DELAY_POOLS
        clientInfo = NULL;
#endif
        epoll_state = 0;
        read_handler = NULL;
        read_data = NULL;
        write_handler = NULL;
        write_data = NULL;
        timeoutHandler = NULL;
        timeout = 0;
        writeStart = 0;
        lifetime_data = NULL;
        closeHandler = NULL;
        halfClosedReader = NULL;
//        wstate = NULL;
        read_method = NULL;
        write_method = NULL;

        ssl = NULL;
        dynamicSslContext = NULL;

//        tosFromServer = '\0';
        nfmarkFromServer = 0;
    }
};

#define fd_table fde::Table


#endif
