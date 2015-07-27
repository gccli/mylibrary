#include "dac.h"
#include "daccenter.h"
#include "event/event.h"
#include "event/EventLoop.h"
#include "event/NetEngine.h"
#include "net/netcomm.h"

int dac_loglevel = 5;
const char *console_port = "8000";
const char *server_port  = "8001";

void SignalHandler(int sig)
{
}

static int DACCommInit()
{
    DAClogConfig logcfg;
    logcfg.filename = strdup("dac");
    logcfg.level = dac_loglevel;
    logcfg.option |= LogOpt_Stderr;
    if (DAClogInit(&logcfg))
	return -1;

    return 0;
}

int main(int argc, char *argv[])
{
    EventLoop Loop;
    NetEventEngine netengine;
    DACCommInit();
    NetModInit();
    DACcenterInit();

    Loop.RegisterEngine(EventScheduler::GetInstance());
    Loop.RegisterEngine(&netengine);
    Loop.run();

    return 0;
}

