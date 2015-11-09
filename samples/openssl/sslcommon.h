#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>

#include <pthread.h>


#define SSL_FAILURE 0
#define SSL_SUCCESS 1

int  THREAD_setup(void);
int  THREAD_cleanup(void);

#define SSL_LOGERR(str) do {                          \
	char tmp_errstr[256];                             \
    unsigned long tmp_errno;                          \
    while((tmp_errno = ERR_get_error())) {            \
		printf("%s %s\n", str,                        \
			ERR_error_string(tmp_errno, tmp_errstr)); \
	}                                                 \
} while(0)


static inline unsigned long thread_id(void)
{
	return syscall(__NR_gettid);
}

void SSLinit();
void SSLseeding(int size, const char *filename);
int  SSLpasswd_cb(char *buf, int size, int rwflag, void *password);
SSL_CTX *SSLnew_ctx(const char *certificate);
