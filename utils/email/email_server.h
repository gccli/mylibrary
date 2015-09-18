#ifndef PROTOCOL_EMAIL_SERVER_H__
#define PROTOCOL_EMAIL_SERVER_H__

enum EmailServerType
{
	EmailServerType_POP3           = 0x01,
	EmailServerType_IMAP           = 0x02,
	EmailServerType_Exchange       = 0x10, // default exchange ews
	EmailServerType_Exchange_RPC   = EmailServerType_Exchange | 0x01,
	EmailServerType_Exchange_EAS   = EmailServerType_Exchange | 0x02
};

enum EmailServerLinkType
{
	EmailServerLinkType_DEFAULT   = 0,
	EmailServerLinkType_SSL       = 1,
	EmailServerLinkType_TLS       = 2
};

typedef struct 
{
	char           *domain;
	char           *realm;

	char           *rcvServ;       // POP3 IMAP
	int             rcvType;       // POP3, IMAP, EWS, EAS, RPC/MAPI
	int             rcvLinkType;   // SSL or TLS
	int             rcvPort;
	char           *sndServ;       // SMTP Server hostname  e.g. smtp.gmail.com
	int             sndPort;       // SMTP Service port  e.g. 25 or 465
	int             sndLinkType;
	int             credentialType;// 
	char           *webURL;        // Exchange Web Service URL
} EmailServ_t;


static inline void ServInit(EmailServ_t *server, const char *domain, const char *realm)
{
	memset(server, 0, sizeof(EmailServ_t));
	server->domain = strdup(domain);
	if (realm)
		server->realm = strdup(realm);
}

static inline void ServSetRcv(EmailServ_t *server, int rcvtype, const char *hostname, int port, int ssltype)
{
	if (server->rcvServ)
		free(server->rcvServ);
	server->rcvServ = strdup(hostname);
	server->rcvPort = port;
	server->rcvLinkType = ssltype;
}

static inline void ServSetSnd(EmailServ_t *server, const char *hostname, int port, int ssltype)
{
	if (server->sndServ)
		free(server->sndServ);
	server->sndServ = strdup(hostname);
	server->rcvPort = port;
	server->rcvLinkType = ssltype;
}


#endif

