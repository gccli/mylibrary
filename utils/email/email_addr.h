#ifndef PROTOCOL_EMAIL_H__
#define PROTOCOL_EMAIL_H__

enum CredentialType
{
	CREDENTIAL_USERNAME            = 0, // lijing
	CREDENTIAL_EMAIL               = 1, // lijing@cclinux.org
	CREDENTIAL_USERNAME_WITH_REALM = 2, // cclinux\lijing
	CREDENTIAL_EMAIL_WITH_REALM    = 3  // cclinux\lijing@cclinux.org
};

typedef struct {
	char             phone[16];
	char             email[80];
	char             password[32];
	char             credential[128]; // login username
	int              credential_type;
	void             *private;	
} EmailAcct_t;

static inline void InitEmailAcct(EmailAcct_t *pEmailAcct, 
	const char* phone,
	const char* email,
	const char* password,
	const char* realm,
	int flags)
{
	int len = 0;
	memset (pEmailAcct, 0, sizeof(EmailAcct_t));
	strncpy(pEmailAcct->phone, phone, sizeof(pEmailAcct->phone));
	strncpy(pEmailAcct->email, email, sizeof(pEmailAcct->email));
	strncpy(pEmailAcct->password, password, sizeof(pEmailAcct->password));
	pEmailAcct->credential_type = flags & 0x0f;

	char username[80] = "\0";
	char *p = strpbrk(pEmailAcct->email, "@");
	len = p - pEmailAcct->email;
	memcpy(username, pEmailAcct->email, len);
	p++;

	switch (pEmailAcct->credential_type)
	{
	case CREDENTIAL_USERNAME:
		sprintf(pEmailAcct->credential, "%s", username);
		break;
	case CREDENTIAL_EMAIL:
		sprintf(pEmailAcct->credential, "%s", pEmailAcct->email);
		break;
	case CREDENTIAL_USERNAME_WITH_REALM:
		sprintf(pEmailAcct->credential, "%s\\%s", realm?realm:p, username);
		break;
	case CREDENTIAL_EMAIL_WITH_REALM:
		sprintf(pEmailAcct->credential, "%s\\%s",  realm?realm:p, pEmailAcct->email);
		break;
	}

	if (pEmailAcct->credential[0] == 0)
		strcpy(pEmailAcct->credential, pEmailAcct->email);
}

#endif

