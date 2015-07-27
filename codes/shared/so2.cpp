#include "comm.h"

static CALLBACK_FUNC fpcallback;

int SO2API(int cmd, void *param)
{
  if (cmd == 0) {
    printf("Initialize shared library 'so2'\n");
    fpcallback = (CALLBACK_FUNC)param;
    CommClass::geti()->show();
  }
  else if (cmd == 1) {
      if (fpcallback && fpcallback(0, (void *)SO2API, (void *)fpcallback))
      ;
  }

  return 0;
}
