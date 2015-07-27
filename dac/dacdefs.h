#ifndef DAC_DEFS_H__
#define DAC_DEFS_H__

#define DAC_MAX_FD 0x400
#define FD_DESC_SZ 64
#define USER_IDENT_SZ 64

typedef unsigned char tos_t;


typedef void PF(int, void *);
typedef int READ_HANDLER(int, char *, int);
typedef int WRITE_HANDLER(int, const char *, int);

#define COMM_SELECT_READ   (0x1)
#define COMM_SELECT_WRITE  (0x2)

enum fd_type {
    FD_NONE,
    FD_LOG,
    FD_FILE,
    FD_SOCKET,
    FD_PIPE,
    FD_MSGHDR,
    FD_UNKNOWN
};

enum {
    FD_READ,
    FD_WRITE
};

typedef enum {
    PEER_NONE,
    PEER_SIBLING,
    PEER_PARENT,
    PEER_MULTICAST
} peer_t;

#endif
