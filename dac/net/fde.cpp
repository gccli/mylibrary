#include "fde.h"


fde *fde::Table = NULL;


char const *fde::remoteAddr() const
{
    static char buf[MAX_IPSTRLEN];

    if (*ipaddr)
        snprintf(buf, MAX_IPSTRLEN, "%s:%d", ipaddr, (int)remote_port);
    else
        local_addr.ToURL(buf,MAX_IPSTRLEN);

    return buf;
}
