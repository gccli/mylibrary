#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>

#define MAXEVENTS 1024
// 
// http://blog.chinaunix.net/uid-24517549-id-4051156.html
static int make_socket_non_blocking (int sfd)
{
  int flags, s;
  if ((flags = fcntl (sfd, F_GETFL, 0)) == -1) {
      perror ("fcntl");
      return -1;
  }

  flags |= O_NONBLOCK;
  if ((fcntl (sfd, F_SETFL, flags)) == -1) {
      perror ("fcntl");
      return -1;
  }

  return 0;
}

static int create_and_bind (char *port)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */

  s = getaddrinfo (NULL, port, &hints, &result);
  if (s != 0) {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
      return -1;
    }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      socklen_t on = 1;
      setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(socklen_t));

      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0) { /* We managed to bind successfully! */
          break;
      }

      close (sfd);
  }

  if (rp == NULL) {
      fprintf (stderr, "Could not bind\n");
      return -1;
  }

  freeaddrinfo (result);

  return sfd;
}

static void sigfunc(int signo)
{
}

int main (int argc, char *argv[])
{
  int sfd, s;
  int efd;

  size_t client_accept=0, client_quit=0, client_err=0;

  struct epoll_event event;
  struct epoll_event *events;

  if (argc != 2) {
      fprintf (stderr, "Usage: %s [port]\n", argv[0]);
      exit (EXIT_FAILURE);
  }
  signal(SIGUSR1, sigfunc);
  signal(SIGUSR2, sigfunc);

  sfd = create_and_bind (argv[1]);
  if (sfd == -1)
      abort ();

  s = make_socket_non_blocking (sfd);
  if (s == -1)
      abort ();

  s = listen (sfd, SOMAXCONN);
  if (s == -1) {
      perror ("listen");
      abort ();
  }

  efd = epoll_create1 (0);
  if (efd == -1) {
      perror ("epoll_create");
      abort ();
  }

  event.data.fd = sfd;
  event.events = EPOLLIN | EPOLLET;
  s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
  if (s == -1) {
      perror ("epoll_ctl");
      abort ();
  }

  /* Buffer where events are returned */
  events = (struct epoll_event *)calloc (MAXEVENTS, sizeof(event));
  /* The event loop */
  while (1) {
      int n, i;

      n = epoll_wait (efd, events, MAXEVENTS, -1);
      for (i = 0; i < n; i++) {
	  if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
	      /* An error has occured on this fd, or the socket is not
		 ready for reading (why were we notified then?) */
	      fprintf(stderr, "epoll error: %s %x\n", 
		      (events[i].events & EPOLLERR)?"EPOLLERR":((events[i].events & EPOLLHUP)?"EPOLLHUP":"Other"), events[i].events);
	      s = epoll_ctl(efd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
	      close (events[i].data.fd);
	      client_err++;
	      continue;
	  }  else if (sfd == events[i].data.fd) {
              /* We have a notification on the listening socket, which
                 means one or more incoming connections. */
              while (1) {
                  struct sockaddr in_addr;
                  socklen_t in_len;
                  int infd;
                  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                  in_len = sizeof in_addr;
                  infd = accept (sfd, &in_addr, &in_len);
                  if (infd == -1) {
                      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                          /* We have processed all incoming connections. */
                          break;
		      } else {
			  perror("accept");abort();
                          break;
		      }
		  }

                  s = getnameinfo (&in_addr, in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);
                  //if (s == 0) {
                  //    printf("Accepted connection on descriptor %d (host=%s, port=%s)\n", infd, hbuf, sbuf);
		  //}

                  /* Make the incoming socket non-blocking and add it to the
                     list of fds to monitor. */
                  s = make_socket_non_blocking(infd);
                  if (s == -1) abort ();

                  event.data.fd = infd;
                  event.events = EPOLLIN | EPOLLET;
                  s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                  if (s == -1) {
                      perror("epoll_ctl"); abort ();
		  }
		  client_accept++;
                }
              continue;
	  }  else {
              /* We have data on the fd waiting to be read. Read and
                 display it. We must read whatever data is available
                 completely, as we are running in edge-triggered mode
                 and won't get a notification again for the same
                 data. */
              int done = 0;
              while (1) {
                  ssize_t count;
                  char buf[256];

                  count = read(events[i].data.fd, buf, sizeof buf-1);
                  if (count == -1) {
                      /* If errno == EAGAIN, that means we have read all
                         data. So go back to the main loop. */
                      if (errno != EAGAIN) {
			  client_err++;
                          perror("read"); abort ();
                          done = 1;
		      }
                      break;
		  } else if (count == 0) {
                      /* End of file. The remote has closed the connection. */
                      done = 1; client_quit++;
                      break;
		  }
		  buf[count] = 0;
		  printf("client(%zu,%zu,%zu,%d) length:%ld\n", client_accept, client_quit, client_err, events[i].data.fd, count);
	      }

              if (done) {
                  s = epoll_ctl (efd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
		  // printf ("Closed connection on descriptor %d %d\n", events[i].data.fd, s);
                  /* Closing the descriptor will make epoll remove it
                     from the set of descriptors which are monitored. */
                  close (events[i].data.fd);
	      } else {
		  if (write(events[i].data.fd, "OK\r\n", 4) < 0) {
		      perror("write");
		  }
	      }
            }
        }
    }

  free (events);
  close (sfd);

  return EXIT_SUCCESS;
}