#include "conditional.h"
char szTemp[256] = "/tmp";

// conditional breakpoint                                                                                                                                                                                         
void test_net(const char *ipaddr)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in raddr;
    memset(&raddr, 0, sizeof(raddr));
    raddr.sin_family = AF_INET;
// OLLYDBG: SHORT [eax+2] == 0x5000
    raddr.sin_port = htons(80);

// DWORD [eax+4] == 0x0c01a8c0
    raddr.sin_addr.s_addr = inet_addr(ipaddr);
    if (connect(sock, (struct sockaddr *) &raddr, sizeof(raddr)) < 0) {
        printf("connect fialed\n");
        return ;
    }

    const char *command = "GET / HTTP/1.1\r\n\r\n";
    int len = send(sock, command, strlen(command), 0);
    printf("%d bytes sent\n", len);
    char buffer[1024] = {0};
    len = recv(sock, buffer, sizeof(buffer), 0);
    printf("%d bytes received \n--------\n%s\n", len, buffer);
}

void test_file()
{
    char  szFile[512] = {0};
#ifdef _WIN32
    sprintf(szFile, "%s\\tempfile", szTemp);
#else
    sprintf(szFile, "%s/tempfile", szTemp);
#endif
    // GDB: b open if strncmp((char *)$rdi,"/tmp",4)==0                                                                                                                                                           
    // OLLYDBG: UNICODE [EBP+8] == "D:\Temp"

    FILE *fp = fopen(szFile, "w");
    if (!fp) {
        printf("failed to create file\n");
        return ;
    }

    // GDB: b write if strncmp((char*)$rsi,"hello",5)==0                                                                                                                                                          
    fwrite("hello", 1, 6, fp);
    fclose(fp);
    printf("create file success\n");
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    DWORD dwRetVal = GetTempPath(MAX_PATH, szTemp);
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
#endif
    test_net(argv[1]);
    test_file();

    return 0;
}

