#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "protocol/imap.h"
#include "protocol/email_addr.h"

int main(int argc, char *argv[])
{
	int port = 143;
	int type = 0;
	if (argc > 2 && strcmp(argv[1], "ssl") == 0) {
		port = 993;
		type = 1;
	}
	
	EmailAcct_t acct;
	InitEmailAcct(&acct, "55555", "test@cclinux.org", "$Security", NULL, 0);

	int  uid = 0;
	int  len = 0;
	char *result = NULL;

	BIO *bio;
	// SSL
	bio = imap_connect("mail.cclinux.org", port, type);
	imap_login(bio, &acct);
	imap_select(bio, &acct, "INBOX", &uid);
	imap_noop(bio, &acct);
	imap_peek(bio, &acct, uid-1, &result, &len);
	imap_logout(bio, &acct);

	printf("\n\n");

	return 0;
}

