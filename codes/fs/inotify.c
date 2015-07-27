#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/shm.h>

#include <pthread.h>
#include <semaphore.h>

#include <sys/inotify.h>

struct filelist
{
	int wd;
	char *filename;
};

char *getfilename(struct filelist *filelst, int wd)
{
	struct filelist *p = filelst;
	while (p)
	{
		if (p->wd == wd)
			return p->filename;
		p++;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("usage: %s <watched filelist>\n", argv[0]);
		return 0;
	}

	int fd = inotify_init();
	if (fd < 0)
	{
		perror("inotify_init");
		return 1;
	}

	uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY;
	struct filelist *filelist = NULL;
	int i,j;
	for (i=1,j=0; i<argc; ++i)
	{
		filelist = realloc(filelist, (2+j)*sizeof(struct filelist));
		filelist[j+1].wd = filelist[j].wd = -1;
		if ((filelist[j].wd = inotify_add_watch(fd, argv[i], mask)) < 0)
		{
			perror("inotify_add_watch");
			continue;
		}
		filelist[j].filename = strdup(argv[i]);
		printf("file:'%s' monitored, wd:%d\n", filelist[j].filename, filelist[j].wd);
		j++;		
	}

	char buffer[16*1024] = {0};

	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	while(1)
	{
		FD_CLR(fd, &rfds);
		FD_SET(fd, &rfds);
		int ret = select (fd + 1, &rfds, NULL, NULL, &tv);
		if (ret <= 0)
			continue ;

		int len = read(fd, buffer, sizeof(buffer));
		int i = 0;
		while (i < len)
		{
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			printf("a event comes, wd:%d mask:0x%x len:%d, %s:'%s' ",
				event->wd, event->mask, event->len,
				(event->mask&IN_ISDIR)?"directory":"file",
				(event->len > 0)?event->name:getfilename(filelist, event->wd));

	
			if (event->mask & IN_CREATE)
				printf("created.\n");
			if (event->mask & IN_DELETE)
				printf("deleted.\n");
			if (event->mask & IN_MODIFY)
				printf("modified.\n");

			i += sizeof(struct inotify_event) + event->len;
		}
	}

	return 0;
}

