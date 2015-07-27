#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

#define gettid() syscall(__NR_gettid)

int ipc_msg_id   = 0;
int ipc_msg_type = 1;
struct mymsg_s {
  int   mtype;
  char  ptr[128];
};

void sendmsg(int n)
{
  struct mymsg_s mymsg;
  mymsg.mtype = ipc_msg_type;
  size_t msgsz;
  int i, sz;
  for (; i<5; ++i) {
    sz = (rand()%126)+1;
    memset(mymsg.ptr, 'A'+n+i, sz);
    mymsg.ptr[sz] = 0;
    msgsz = 4 + sz;
    int rc = msgsnd(ipc_msg_id, &mymsg, msgsz, IPC_NOWAIT); 
    if (rc != 0) {
      perror("msgsnd");
      break;
    }
  }
}

void showmsg()
{
  struct msqid_ds ds;
  memset(&ds, 0, sizeof(ds));
  msgctl(ipc_msg_id, IPC_STAT, &ds);
  printf("Current number of bytes in queue         :%ld\n", ds.__msg_cbytes);
  printf("Current number of messages in queue      :%ld\n", ds.msg_qnum);
  printf("Maximum number of bytes allowed in queue :%ld\n", ds.msg_qbytes);
  printf("PID of last msgsnd()                     :%d\n", ds.msg_lspid);
}


void *threadRcv(void *param )
{
  printf("Receive Thread[%ld] running...\n", gettid());
  int flag = IPC_NOWAIT | MSG_NOERROR;
  while (1) {
    struct mymsg_s mymsg;
    int rc = msgrcv(ipc_msg_id, &mymsg, sizeof(mymsg), ipc_msg_type, flag);
    if (rc < 0) {
      if (errno == EINTR || errno == ENOMSG) {
	usleep(1000);
	continue;
      }
      perror("msgrcv");
      break;
    }
    showmsg();
    printf("Receive %3d Bytes: %s\n", rc, mymsg.ptr);
  }
  return NULL;
}

void sig_fun(int signo)
{
  printf("a signal %d catched\n", signo);
}

int main(int argc, char* argv[])
{
  signal(SIGINT, sig_fun);

  int msgflg = IPC_CREAT | S_IRUSR | S_IWUSR;
  ipc_msg_id = msgget(IPC_PRIVATE, msgflg);
  if (ipc_msg_id < 0) {
    perror("msgget");
    return 1;
  }
  pthread_t th;
  pthread_create(&th, 0, threadRcv, 0);
 
  pid_t pid = fork();
  if(pid == 0) {
    sendmsg(32);
    exit(0);
  }
  sendmsg(0);
  int status;
  waitpid(pid, &status, 0);

  sleep(2);
  if (msgctl(ipc_msg_id, IPC_RMID, NULL) != 0)
    perror("msgctl");

  pthread_join(th, 0);
  return 0;
}
