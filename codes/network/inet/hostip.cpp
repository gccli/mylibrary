#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <netdb.h>

struct DomainIpMap
{
  char       domainname[64];
  char       ip[20];
} domainipmap[100];

int gl_domainresolv_update_interval = 600;
unsigned long lastTime = 0;

int getdomainip(const char* popServ, char* popIp)
{
  int i = 0;
  unsigned long curTime = time(NULL);
  if(curTime - lastTime < 0 || curTime - lastTime > gl_domainresolv_update_interval)
    {
      memset(domainipmap, 0, sizeof(domainipmap));
      lastTime = curTime;
    }
  
  for(i =0; i < 100; i++)
    {
      if(strcmp(popServ, domainipmap[i].domainname) == 0)
        {
	  strcpy(popIp, domainipmap[i].ip);
	  return 0;
        }
      else if(strlen(domainipmap[i].domainname) == 0)
        {
	  break;
        }
    }
  struct hostent * host_addr = gethostbyname(popServ); //pop.163.com
  if(host_addr==NULL) 
    { 
      printf("popServ:%s, gethostbyname fail\n", popServ);
      return 1; 
    } 
  
  struct in_addr addr;
  addr.s_addr = *((unsigned long* )host_addr->h_addr);
  inet_ntop (AF_INET, &addr, popIp, INET_ADDRSTRLEN);

  for(i =0; i < 100; i++)
    {
      if(strlen(domainipmap[i].domainname) == 0)
        {
	  strcpy(domainipmap[i].domainname, popServ);
	  strcpy(domainipmap[i].ip, popIp);
	  break;
        }
    }
    
  return 0;
}
/*
struct addrinfo {
	 int	 ai_flags;
	 int	 ai_family;
	 int	 ai_socktype;
	 int	 ai_protocol;
	 size_t  ai_addrlen;
	 struct sockaddr *ai_addr;
	 char	*ai_canonname;
	 struct addrinfo *ai_next;
 };
*/
void gethost(const char *hostname, int addrtype)
{
	struct addrinfo hints, *res = NULL;

	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_INET;

	getaddrinfo(hostname, "domain", &hints, &res);
	
}


int main(int argc, char* argv[])
{
  char* hostname;
  if (argc == 2)
    hostname = argv[1];
  else 
    hostname = "baidu.com";
  char ip[16];
  for (int i=0; i<10; ++i) {
    getdomainip (hostname, ip);
    printf ("%s\n", ip);
  }

  return 0;
}
