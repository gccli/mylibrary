#include "libapp.h"
#include <map>
#include <string>

using namespace std;
map<int, string> gAppName;


char       dachost[64] = "::1";
int        dacport = 8001;

const char *DACAppName(int port)
{
	map<int, string>::iterator it = gAppName.find(port);

	if(it != gAppName.end())
		return NULL;

	return it->second.c_str();
}

int DACInit(int port, int flags)
{
	CAppObject *gAppobj = new CAppObject;
	if (gAppobj->init(port, dachost, dacport) != 0)
		return 1;

	return 0;
}


