#ifndef PROTOCOL_IMAP_H__
#define PROTOCOL_IMAP_H__

#include "email_addr.h"
#include "email_server.h"

BIO* imap_connect(const char *hostname, int port, int sslType);
int  imap_login(BIO *bio, EmailAcct_t *pAcct);
int  imap_select(BIO *bio, EmailAcct_t *pAcct, const char *mailbox, int *pUIDNext);
int  imap_noop(BIO *bio, EmailAcct_t *pAcct);
int  imap_peek(BIO *bio, EmailAcct_t *pAcct, int UID, char **result, int *length);
int  imap_logout(BIO *bio, EmailAcct_t *pAcct);



static inline void MUASaveail(void *buffer, int buflen, const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (fp)
	{
		fwrite(buffer, buflen, 1, fp);
		fclose(fp);
	}
}

#endif

