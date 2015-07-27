#include "mycdev.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

void mycdev_write(int fd, char *message)
{
    int ret;
    ret = ioctl(fd, MYCDEV_WRITE, message);

    if (ret < 0) {
	printf("mycdev_write failed:%d\n", ret);
	exit(-1);
    }
}

void mycdev_read(int fd)
{
    int ret;
    char message[100];

    /* 
     * Warning - this is dangerous because we don't tell
     * the kernel how far it's allowed to write, so it
     * might overflow the buffer. In a real production
     * program, we would have used two ioctls - one to tell
     * the kernel the buffer length and another to give
     * it the buffer to fill
     */
    ret = ioctl(fd, MYCDEV_READ, message);

    if (ret < 0) {
	printf("mycdev_read failed:%d\n", ret);
	exit(-1);
    }

    printf("get_msg message:%s\n", message);
}

int mycdev_nthbyte(int fd, int i)
{
    int c = ioctl(fd, MYCDEV_NTHBYTE, i);
    if (c < 0) {
	printf("nthbyte failed at the %d'th byte:\n", i);
	exit(-1);
    }
    printf("get_nth_byte message: %d\n", c);
    return c;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
	printf("usage: %s devicepath\n", argv[0]);
	return 1;
    }

    int fd;
    fd = open(argv[1], 0);
    if (fd < 0) {
	printf("Can't open device file: %s\n", argv[1]);
	return 0;
    }

    mycdev_write(fd, "hello");
    mycdev_nthbyte(fd, 5);
    mycdev_read(fd);

    close(fd);

    return 0;
}
