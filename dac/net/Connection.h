#ifndef _DAC_CONNECTIONDETAIL_H_
#define _DAC_CONNECTIONDETAIL_H_

#include "dac.h"
#include "ip/Address.h"
#include "RefCount.h"

#include <iosfwd>
#include <ostream>

/* TODO: make these a struct of boolean flags members in the connection instead of a bitmap.
 * we can't do that until all non-comm code uses Commm::Connection objects to create FD
 * currently there is code still using comm_open() and comm_openex() synchronously!!
 */
#define COMM_UNSET              0x00
#define COMM_NONBLOCKING        0x01  // default flag.
#define COMM_NOCLOEXEC          0x02
#define COMM_REUSEADDR          0x04  // shared FD may be both accept()ing and read()ing
#define COMM_DOBIND             0x08  // requires a bind()
#define COMM_TRANSPARENT        0x10  // arrived via TPROXY
#define COMM_INTERCEPTION       0x20  // arrived via NAT

/**
 * Store data about the physical and logical attributes of a connection.
 *
 * Some link state can be infered from the data, however this is not an
 * object for state data. But a semantic equivalent for FD with easily
 * accessible cached properties not requiring repeated complex lookups.
 *
 * Connection properties may be changed until the connection is opened.
 * Properties should be considered read-only outside of the Comm layer
 * code once the connection is open.
 *
 * These objects should not be passed around directly,
 * but a Comm::ConnectionPointer should be passed instead.
 */
class Connection : public RefCountable
{
public:

    Connection();

    /** Clear the connection properties and close any open socket. */
    ~Connection();

    /** Copy an existing connections IP and properties.
     * This excludes the FD. The new copy will be a closed connection.
     */

    /** Close any open socket. */
    void close();

    /** determine whether this object describes an active connection or not. */
    bool isOpen() const { return (fd >= 0); }


private:
    /** These objects may not be exactly duplicated. Use copyDetails() instead. */
    Connection(const Connection &c);

    /** These objects may not be exactly duplicated. Use copyDetails() instead. */
    Connection & operator =(const Connection &c);

public:
    /** Address/Port for the Squid end of a TCP link. */
    Ip::Address local;

    /** Address for the Remote end of a TCP link. */
    Ip::Address remote;

    /** Hierarchy code for this connection link */
//    hier_code peerType;

    /** Socket used by this connection. Negative if not open. */
    int fd;

    /** Quality of Service TOS values currently sent on this connection */
//    tos_t tos;

    /** COMM flags set on this connection */
    int flags;

    char rfc931[USER_IDENT_SZ];
};

#endif

