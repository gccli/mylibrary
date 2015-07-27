#include "utilsock.h"

#include <time.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "error.h"
#include "imap.h"
#include "globaldefines.h"

inline int BIO_READ(BIO *bio, char *buffer, int buflen, int timeout)
{
	int ret = 0;
	int retry = 0, n;
	int total_retry_count = timeout*1000;

	for (;;)
	{
		n = BIO_read(bio, buffer, buflen);
		if (n >= 0) break;
		if (n < 0)
		{
			if (BIO_should_retry(bio))
			{
				if ((retry++ % 1000) == 2)
					printf("read retry ...\n");
				if (total_retry_count > 0 && retry > total_retry_count) {
					ret = -1;
					errno = ETIMEDOUT;
					break;
				}
				usleep(1000);
				continue;
			}
			ret = -1;
			ERR_print_errors_fp(stderr);
			break;
		}
	}

	if (ret != 0) 
		return -1;

	return n;
}

int BIO_WRITE(BIO *bio, const char *buffer, int buflen)
{
	int ret = 0;
	int len = buflen;
	int offset = 0, retry = 0, n;
	for ( ;len > 0; )
	{
		n = BIO_write(bio, buffer+offset, len);
		if (n <= 0)
		{
			if (BIO_should_retry(bio))
			{
				if ((retry++ % 1000) == 0)
					printf("write retry ...\n");
				usleep(1000);
				continue;
			}
			ret = -1;
			ERR_print_errors_fp(stderr);
			break;
		}
		offset += n;
		len    -= n;
	}

	if (ret != 0)
		return -1;

	return offset;
}

static SSL_CTX *imap_ssl_ctx;
BIO* imap_connect(const char *hostname, int port, int sslType)
{
	int  error;
	BIO *bio = NULL;
	SSL *ssl = NULL;

	char temp[128] = "\0";
	snprintf(temp, sizeof(temp), "%s:%d", hostname, port);

    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

	printf("try to connect imap server %s %s\n", temp, sslType?"with ssl":"");
	
	if (sslType != 0) 
	{
		SSL_library_init();
		if (imap_ssl_ctx == NULL) {
			imap_ssl_ctx = SSL_CTX_new(SSLv23_client_method());
		}

		if ((bio = BIO_new_ssl_connect(imap_ssl_ctx)) == NULL)
			goto err;
		BIO_get_ssl(bio, &ssl);
		if (ssl == NULL)
			goto err;

		BIO_set_conn_hostname(bio, temp);
		BIO_set_nbio(bio, 1);// set nonblocking io

		for(;;) {
			if (BIO_do_handshake(bio) <= 0) {
				if (BIO_should_retry(bio)) {
					usleep(10000);
					continue;
				}
			}
			else break;
		}
	}
	else 
	{
		if ((bio = BIO_new_connect(temp)) == NULL)
			goto err;
		BIO_set_nbio(bio, 1);// set nonblocking io
	}



	char buffer[1024] = "\0";
	BIO_READ(bio, buffer, sizeof(buffer), 3);
	
	printf("imap server response:%s", buffer);
	return bio;

err:
	if ((error = ERR_peek_error()) == 0)
	{
		printf("failure: %d %s\n", errno, strerror(errno));
	}
	else
	{
		ERR_print_errors_fp(stderr);
	}
	if (imap_ssl_ctx) 
	{
		SSL_CTX_free(imap_ssl_ctx);
		imap_ssl_ctx = NULL;
	}

	return NULL;
}

int imap_login(BIO *bio, EmailAcct_t *pAcct)
{
	char buffer[1024]     = "\0";
	char logincmd[256]    = "\0";
	snprintf(logincmd, sizeof(logincmd), "TAG00 LOGIN %s %s\r\n", pAcct->credential, pAcct->password);
	printf("Sending IMAP LOGIN:%s", logincmd);

	int ret = BIO_WRITE(bio, logincmd, strlen(logincmd));
    if(ret <= 0)
    {
        printf("failed to send IMAP LOGIN command");
		return 1;
    }

	BIO_READ(bio, buffer, sizeof(buffer), 3);
	printf("IMAP LOGIN response:%s", buffer);
	if (strstr(buffer, "TAG00 NO") != NULL)
	{
		return E_PROTO_AUTHFAILED;
	}
    if (strstr(buffer, "TAG00 OK") == NULL)
    {
    	return 1;
    }

    return 0;
}

int imap_logout(BIO *bio, EmailAcct_t *pAcct)
{
	char buffer[1024] = "\0";
	int  buflen;
	buflen = sprintf(buffer, "TAG00 LOGOUT\r\n");
	
	if (BIO_WRITE(bio, buffer, buflen) <= 0) {
		return 1;
	}
	if (BIO_READ(bio, buffer, sizeof(buffer), 3) <= 0) {
		return 1;
	}

	printf("IMAP LOGOUT command response: %s", buffer);
	return 0;
}

int imap_select(BIO *bio, EmailAcct_t *pAcct, const char *mailbox, int *pUIDNext)
{
	char buffer[1024] = "\0";
	int buflen = sprintf(buffer, "TAG01 SELECT %s\r\n", mailbox);

	int ret = BIO_WRITE(bio, buffer, buflen);
    if(ret <= 0)
    {
        printf("failed to send select command");
		return -1;
    }

	memset(buffer, 0, sizeof(buffer));
	if ((ret = BIO_READ(bio, buffer, sizeof(buffer), 3)) <= 0) {
		printf("failed to read IMAP SELECT response");
		return 1;
	}
	printf("IMAP SELECT command response:%s", buffer);
    if (strstr(buffer, "TAG01 OK") == NULL)
    {
    	return 1;
    }

	char *p = strstr(buffer, "UIDNEXT");
	int nextUid = 0;
	sscanf(p, "UIDNEXT %d", &nextUid);
	*pUIDNext = nextUid;
	printf("IMAP SELECT ok, next uid: %d\n", nextUid);

	return 0;
}

int imap_noop(BIO *bio, EmailAcct_t *pAcct)
{
	char buffer[1024] = "\0";
	int buflen = sprintf(buffer, "TAG03 NOOP\r\n");

	int ret = BIO_WRITE(bio, buffer, buflen);
    if(ret <= 0)
    {
        printf("failed to send NOOP command");
		return -1;
    }

	memset(buffer, 0, sizeof(buffer));
	if ((ret = BIO_READ(bio, buffer, sizeof(buffer), 3)) <= 0) {
		printf("failed to read IMAP NOOP response");
		return 1;
	}
	printf("IMAP NOOP command response:%s", buffer);
    if (strstr(buffer, "TAG03 OK") == NULL)
    {
    	return 1;
    }
	return 0;
}

int imap_peek(BIO *bio, EmailAcct_t *pAcct, int UID, char **result, int *length)
{
	int  inum = 0;
	char star[] = {'/','-','\\','|'};

	char buffer[1024] = "\0";
	int ret, buflen;

	const int padsz = 4096;
	int skip = 0, mailsz = 0;
	buflen = sprintf(buffer, "TAG02 UID FETCH %d BODY.PEEK[]\r\n", UID);
	if (BIO_WRITE(bio, buffer, buflen) <= 0) {
		 printf("failed to write IMAP UID FETCH command");
		 return 1;
	}

	// first read	
	if ((ret = BIO_READ(bio, buffer, sizeof(buffer), 15)) <= 0) {
		printf("failed to read IMAP UID FETCH response");
		return 1;
	}

	buflen = ret;
	// get mail length
	char *p = strchr(buffer, '{');
	while (p == NULL)
	{
		ret = BIO_READ(bio, buffer+buflen, sizeof(buffer)-buflen, 15);
		if (ret <= 0) {
			printf("failed to read IMAP UID FETCH command, buflen:%d, reason:%s", buflen, strerror(errno));
			return 1;
		}
		buflen += ret;
		p = strchr(buffer, '{');
	}

	sscanf(p+1, "%d", &mailsz);
	printf("UID %d mail size %d", UID, mailsz);

	char *pBuffer = malloc(mailsz+padsz);
	memcpy(pBuffer, buffer, buflen);

	skip = 0;
	if (strstr(pBuffer, "TAG02 OK") != NULL)
	{
		printf("IMAP UID FETCH ok, response:%s", pBuffer);
		skip = 1;
	}

	p = pBuffer + buflen;
	while(!skip) {
		if ((ret = BIO_READ(bio, p, mailsz+padsz-buflen, 15)) <= 0) {
			printf("failed to read IMAP UID FETCH response, reason:%s", strerror(errno));
			return 1;
		}
		int offset = ret - 128;
		if (strstr(p+offset, "TAG02 OK") != NULL)
		{
			buflen += ret;
			break;
		}
		buflen += ret;
		p      += ret;

		fprintf(stderr, TCKL"%2c%8d bytes read, %d/%d bytes data received ", star[inum++%4], buflen, mailsz-buflen, mailsz);
	}
	fprintf(stderr, "\nmail length:%d saved in /tmp/mail.txt\n"TCKL, buflen);
	MUASaveail(pBuffer, buflen, "/tmp/mail.txt");

	*result = pBuffer;
	*length = buflen;

	return 0;
}


