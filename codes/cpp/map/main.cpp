#include "feedback.h"
#include <unistd.h>
#include <assert.h>
#include <sys/syscall.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/sha.h>

#include <vector>
#include <string>

using namespace std;

#define gettid() syscall(__NR_gettid)


void *thread_func0(void *arg)
{
    char buffer[1024];
    int  buflen;
    int  i=0;
    for( ; ;i++) {
	buflen=sprintf(buffer, "THREAD[%05d] AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDDDDDDEEEEEEEEEEEEEEEEEEEE", gettid());
	FBComp_feedback(i%8, buffer, buflen);
	FBComp::Instance().ArrangeInternal(0, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 50);
    }

    return NULL;
}



vector<string> vs;

void *thread_func1(void *param)
{
    for(int i=0; ; i=(i+2)%20000)
    {
        const char *filename = vs[i].c_str();
        int fd;
        if ((fd = open(filename, O_RDONLY)) < 0) {
            printf("open '%s' error - %s", filename, strerror(errno));
            return NULL;
        }

        struct stat st;
        if (fstat(fd, &st) != 0) {
            printf("stat error - %s", strerror(errno));
            return NULL;
        }
        size_t filesz = st.st_size;

        void *addr = NULL;
        if ((addr = mmap(0, filesz, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
            printf("mmap error - %s", strerror(errno));
            return 0;
        }
        unsigned char md[20];
        SHA1((unsigned char *)addr, filesz, md);
        for(int j=0; j<20; ++j) {
	    printf("%.2x",md[j]);
        }printf("\n");

        close(fd);
        munmap(addr, filesz);
    }

    return NULL;
}

void *thread_func2(void *param)
{
    for(int i=1; ; i = (i+2)%20000)
    {
        const char *filename = vs[i].c_str();
        int fd;
        if ((fd = open(filename, O_RDONLY)) < 0) {
            printf("open '%s' error - %s", filename, strerror(errno));
            return NULL;
        }

        struct stat st;
        if (fstat(fd, &st) != 0) {
            printf("stat error - %s", strerror(errno));
            return NULL;
        }
        size_t filesz = st.st_size;

        void *addr = NULL;
        if ((addr = mmap(0, filesz, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
            printf("mmap error - %s", strerror(errno));
            return 0;
        }

        char *p1, *p2, *p3, *p4;
        int i0, i1, i2, i3, i4;
        p1 = (char *)addr;
        int isexe=0;
        if (strncmp(p1, "MZ", 2) == 0)
	    isexe=1;
        i0 = *(int *)p1;

        p2 = p1+10;
        i1 = *(int *)p2;

        p3 = p2+10;
        i2 = *(int *)p3;

        p4 = p3+60;
        i3 = *(int *)p4;

        p4 -= 30;
        i4 = *(int *)p4;

        printf("%s, %d,%d,%d,%d\n", isexe?"yes":"no",i0, i1, i2, i3, i4);
        close(fd);
        munmap(addr, filesz);
    }

    return NULL;
}


int main(int argc, char *argv[])
{

    FILE *fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int num=0;

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("failed to open '%s'\n", argv[1]);
        return -1;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        if (read-1 < 0) continue;
        line[read-1] = 0;// remove '\n'
        vs.push_back(line);
        num++;
        if (num > 20001)
            break;
    }

    for(int i=0; i<20; ++i) {
        pthread_t th0, th1;
        pthread_create(&th0, NULL, thread_func1, NULL);
        pthread_create(&th1, NULL, thread_func2, NULL);
    }
    pthread_t th0, th1;

    FBComp::Instance().Initialize(102400);

    for(int i=0; i<30; ++i) {
	pthread_create(&th0, NULL, thread_func0, NULL);
    }

    char buffer[204800+4];
    int  bufsize = sizeof(buffer);
    while(1) {
	int x = FBComp::Instance().GatherFeedback(buffer, bufsize);
	if (x > 0) {
	    printf("\033[31mGather Feedback %d bytes\033[0m\n", x);
	}
    }

    return 0;
}
