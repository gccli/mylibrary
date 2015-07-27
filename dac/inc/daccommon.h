#ifndef __DAC_COMMON_H__
#define __DAC_COMMON_H__

#include <stdint.h>

typedef enum
{
	ERR_SUCCESS = 0,
	ERR_FAILURE = 1,
	ERR_INVALID = 2,
	ERR_PARAM_ERROR = 3,
	ERR_CONN_FAILED = 4,
} DACErrCode;


typedef enum 
{
	DACMSG_TYPE_ONLINE    = 0,
	DACMSG_TYPE_ONLINE_RSP    = 1,
	DACMSG_TYPE_HEARTBEAT = 2,
	DACMSG_TYPE_HEARTBEAT_RSP = 3,
	DACMSG_TYPE_STATUS_REPORT = 4,	
	DACMSG_TYPE_STATUS_REPORT_RSP = 5,	
} DACMsgType;


typedef enum 
{
	DACMSG_C_OK        = 0,
	DACMSG_C_FAILED    = 1,	
	DACMSG_C_UP       = 2,	
	DACMSG_C_DOWN      = 3,	
	DACMSG_C_TIMESTAMP = 4,	
	DACMSG_C_OVERLOAD  = 5,	
	DACMSG_C_IDLE   = 6,	
	DACMSG_C_END    = 7,	
} DACMsgCode;


#pragma pack(1)
typedef struct {
	unsigned char  type;
	unsigned char  code;
	unsigned int   id;
	unsigned short len;
	unsigned char content[0];
} DACMsgHdr;

// information element
struct DacIEBype
{
	uint8_t type;
	uint8_t value;
};

struct DacIEU16
{
	uint8_t type;
	union {
		uint16_t v16;
		uint8_t  v8[2];
	} u;
};

struct DacIEU32
{
	uint8_t type;
	union {
		uint32_t v32;
		uint16_t v16[2];
		uint8_t  v8[4];
	} u;
};

struct DacIETlv
{
	unsigned char  type;
	unsigned short len;
	unsigned char  value[0];
};

#pragma pack(0)





typedef enum
{
	IE_APP_ID     = 0,
	IE_APP_NAME   = 1,
	IE_APP_IP     = 2,
	IE_APP_PORT   = 3,
	IE_APP_STATUS = 4
} DACIE;



static inline int IEBuildBype(uint8_t *message, int type, uint8_t value)
{
	message[0] = type;
	message[1] = value;
	return sizeof(DacIEBype);
}

static inline int IEBuildU16(uint8_t *message, int type, uint8_t value[2])
{
	DacIEU16 *p = (DacIEU16 *) message;
	p->type = type;
	memcpy(p->u.v8, value, sizeof(p->u.v8));
	return sizeof(DacIEU16);
}

static inline int IEBuildU32(uint8_t *message, int type, uint8_t value[4])
{
	DacIEU32 *p = (DacIEU32 *) message;
	p->type = type;
	memcpy(p->u.v8, value, sizeof(p->u.v8));
	return sizeof(DacIEU32);
}

static inline int IEBuildTlv(uint8_t *message, int type, int len, uint8_t *value)
{
	DacIETlv *p = (DacIETlv *) message;
	p->type = type;
	p->len = len;
	memcpy(p->value, value, len);
	return sizeof(DacIETlv)+len;
}


#endif

