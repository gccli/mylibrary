#ifndef __DAC_CFG_H__
#define __DAC_CFG_H__

extern int dac_maxfd;
extern int dac_loglevel;
extern const char *console_port;
extern const char *server_port;
extern const char *queue_name;

class DACConfig
{
public:
    DACConfig();
    virtual ~DACConfig();
private:
    char m_cfgfile[32];
};

#endif

