#include <sys/syscall.h>

#include "sslcommon.h"

#define SSL_FAILURE 0
#define SSL_SUCCESS 1

//////////////////////////////////////////////////
// OpenSSL thread-safe
static pthread_mutex_t *ssl_mutex = NULL;
static void thread_lock(int mode, int n, const char * file, int line)
{
	if (mode & CRYPTO_LOCK)
		pthread_mutex_lock(&ssl_mutex[n]);
	else
		pthread_mutex_unlock(&ssl_mutex[n]);
}

int THREAD_setup(void)
{
	int i, mutex_count = CRYPTO_num_locks();
	if ((ssl_mutex = calloc(mutex_count, sizeof(pthread_mutex_t))) == NULL)
		return SSL_FAILURE;

	for (i=0 ;i<mutex_count; ++i)
		pthread_mutex_init(&ssl_mutex[i], NULL);

	CRYPTO_set_id_callback(thread_id);
	CRYPTO_set_locking_callback(thread_lock);

	return SSL_SUCCESS;
}

int THREAD_cleanup(void)
{
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
	if (ssl_mutex == NULL)
		return SSL_FAILURE;
	int i;
	for (i = 0; i < CRYPTO_num_locks(); i++)
		pthread_mutex_destroy(&ssl_mutex[i]);
	free(ssl_mutex);
	ssl_mutex = NULL;

	return SSL_SUCCESS;
}
//////////////////////////////////////////////////



// Common Functions
void SSLinit()
{
	if (!THREAD_setup() || ! SSL_library_init())
	{
		fprintf(stderr, "OpenSSL initialization failed!\n");
		exit(-1);
	}
	SSL_load_error_strings();
}

void SSLseeding(int size, const char *filename)
{
	if (access(filename, F_OK) != 0)
	{
		RAND_load_file("/dev/random", size);
		RAND_write_file(filename);
	}

	int nb = RAND_load_file(filename, -1);
	printf("Seeded the PRNG with %d byte(s) of data from %s\n", nb, filename);
}

int SSLpasswd_cb(char *buf, int size, int rwflag, void *password)
{
	strncpy(buf, (char *)password, size);
	buf[size] = 0;

	return strlen(buf);
}


SSL_CTX *SSLnew_ctx(const char *certificate)
{
	SSL_CTX *ctx = SSL_CTX_new(TLSv1_method());
	SSL_CTX_set_default_passwd_cb(ctx, SSLpasswd_cb);
	SSL_CTX_set_default_passwd_cb_userdata(ctx, "lijing");

	if (SSL_CTX_use_certificate_chain_file(ctx, certificate) <= 0)
	{
		SSL_LOGERR("use certificate");
		return NULL;
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, certificate, SSL_FILETYPE_PEM) <= 0)
	{
		SSL_LOGERR("use private key");
		return NULL;
	}
	if (SSL_CTX_check_private_key(ctx) <= 0) 
	{
		SSL_LOGERR("check private key");
		return NULL;
	}

	return ctx;	
}


