#ifndef FD_H__
#define FD_H__

void fd_close(int fd);
void fd_open(int fd, unsigned int type, const char *);
void fd_note(int fd, const char *);
void fd_bytes(int fd, int len, unsigned int type);

int fd_set_nonblocking(int fd, int on=1);
int fd_unset_nonblocking(int fd);
void fd_set_close_onexec(int fd);

#endif

