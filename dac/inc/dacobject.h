#ifndef __DAC_OBJECT_H__
#define __DAC_OBJECT_H__


typedef enum {
	DAC_OBJ_NONE   = 0,
	DAC_OBJ_APP    = 1,
	DAC_OBJ_EDGE   = 2,
	DAC_OBJ_CENTRE = 3
} DACAppType;

#define INVALID_OBJECT_ID 0


class DACObject 
{
public:
	DACObject(int type)
		:m_type(type)
	{}
	virtual ~DACObject()
	{
		if (m_addr) { free(m_addr); m_addr = NULL; }
	}

protected:
	virtual void SetId(unsigned int id) { m_id = id; }
	virtual void SetStatus(unsigned int status) { m_status = status; }
	virtual void SetName(const char *name) { strncpy(m_name, name, sizeof(m_name)); }

public:
	unsigned char    m_type;
	unsigned int     m_status;
	unsigned int     m_id;
	char             m_name[32];
	unsigned short   m_addrlen;
	struct sockaddr *m_addr;
};

#endif

