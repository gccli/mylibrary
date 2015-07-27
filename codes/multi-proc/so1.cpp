#include "comm.h"

static CALLBACK_FUNC fpcallback; //

extern "C" int SO1API(int cmd, void *param)
{
  if (cmd == 0) {
    Mylog("Initialize shared library 'so1'\n");
    fpcallback = (CALLBACK_FUNC)param;
    CommClass::GetInstance()->show();
  }
  else if (cmd == 1) {
      if (fpcallback && fpcallback(0, __FUNCTION__, (void *)SO1API, (void *)fpcallback))
	  ;
  }

  return 0;
}
