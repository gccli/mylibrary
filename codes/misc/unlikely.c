#include <unistd.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define likely(x) __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

int main(int argc, char *argv[])
{
    if (unlikely(argc < 2)) {
	printf("usage: %s filename\n", argv[0]);
	return 0;
    }

    int fd = open(argv[1], O_RDWR|O_CREAT);
    if (unlikely(fd < 0)) {
	perror("open");
	return 1;
    }

    int len = write(fd, "hello", 5);
    if (likely(len > 0)) {
	printf("%d types write to file\n", len);
    }

    if (unlikely(unlink(argv[1])< 0)) {
	printf("failed to remove file '%s'\n", argv[1]);
    } 

    return 0;
}
