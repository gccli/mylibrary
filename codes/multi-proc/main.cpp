#include "comm.h"
#include <dlfcn.h>
#include <map>
#include <string>
#include <pthread.h>
using namespace std;

static pthread_key_t  th_key;
static pthread_once_t th_once = PTHREAD_ONCE_INIT;

static void verify_destructor(void* ptr) 
{
    Mylog("*** free %p\n", ptr);
    free(ptr);
}
static void verify_once()
{
    Mylog("*** init once \n");
    pthread_key_create(&th_key, verify_destructor);
}

static int so1callback(int cmd, const char *name, void *para1, void *para2)
{
    Mylog("'so1' callback called by %s, caller:%p callee:%p\n", name, para1, para2);

    const char *sample_file_list[] = 
	{
	    "/home/sample/esg/0041558-F-51F73B25F8FFD",
	    "/home/sample/esg/0071084-F-3B6A9AE1C220D",
	    "/home/sample/esg/0099865-M-A47E697A8F6E6",
	    "/home/sample/esg/0007726-M-5A8F7DE64C28D",
	    "/home/sample/esg/0040327-M-E79B7D2EB280A",
	    "/home/sample/esg/0068520-F-CBA2041ACDD37",
	    "/home/sample/esg/0059688-M-9BBEE496DFF3D",
	    "/home/sample/esg/0048136-M-C8167EEE92CCF",
	    "/home/sample/esg/0078653-F-E43389E72AC9D",
	    "/home/sample/esg/0101263-F-E6CD72D6C4ABB",
	    "/home/sample/esg/0101263-F-E6CD72D6C4ABB",
	    "/home/sample/esg/0085835-F-803FBA2747F5C",
	    "/home/sample/esg/0069961-M-4BC41D017FA12",
	    "/home/sample/esg/0029599-F-D35C0696407C7",
	    NULL
	};


    size_t sz = sizeof(sample_file_list)/sizeof(char*);

    void *tsd = NULL;


    for(size_t i=0; i<sz && sample_file_list[i];++i) {

	pthread_once(&th_once, verify_once);
	if ((tsd = pthread_getspecific(th_key)) == NULL) {
	    tsd = calloc(1, 12);
	    pthread_setspecific(th_key, tsd);
	}

	const char *filename = sample_file_list[i];
	printf("%s\n", filename);
	P7ZDecompress(filename, "/tmp/decomp", 0);
    }
    return 0;
}

static int so2callback(int cmd, const char *name, void *para1, void *para2)
{
    Mylog("'so2' callback called by %s, caller:%p callee:%p\n", name, para1, para2);
    return 0;
}

class SoLoader 
{
public:
    SoLoader(){}
    virtual ~SoLoader();

    void *LocateApi(const char *so, const char *name);

private:
    map<string, void *> m_handles;
};

SoLoader::~SoLoader()
{
    map<string, void *>::iterator it = m_handles.begin();
    for(; it != m_handles.end(); ++it) {
	Mylog("Close handle for '%s'\n", it->first.c_str());
	dlclose(it->second);
    }
    m_handles.clear();
}

void *SoLoader::LocateApi(const char *so, const char *name)
{
    void *handle = dlopen(so, RTLD_LAZY);
    if (!handle) {
	Mylog("Can't open so '%s' - %s\n", so, dlerror());
	return NULL;
    }
    Mylog("Open handle %p \n", handle);
    
    void *addr = dlsym(handle, name);
    if (!addr) {
	Mylog("Can't locate '%s' from '%s' - %s\n", name, so, dlerror());
	return NULL;
    }

    m_handles[string(so)] = handle;
    return addr;
}

SoLoader loader;
int main(int argc, char *argv[])
{
    SOAPI_FUNC SO1API=NULL;
    SOAPI_FUNC SO2API=NULL;

    SO1API = (SOAPI_FUNC) loader.LocateApi("libso1.so", "SO1API");
    SO2API = (SOAPI_FUNC) loader.LocateApi("libso2.so", "SO2API");

    P7ZDecompress("/home/sample/esg/0029599-F-D35C0696407C7", "/tmp/decomp", 0);

    if (SO1API == NULL || SO2API == NULL)
	return 1;

    Mylog("SO1API:%p, SO2API:%p, so1callback:%p, so2callback:%p\n"
	   ,SO1API, SO2API, so1callback, so2callback);


//    SO1API(0, (void *)so1callback);
//    SO2API(0, (void *)so2callback);

//    SO1API(1, NULL);
//    SO2API(1, NULL);
    
    pid_t childpid = fork();
    if (childpid == 0) {
	Mylog("Child<%d> created\n", getpid());

	SO1API = (SOAPI_FUNC) loader.LocateApi("libso1.so", "SO1API");
	SO2API = (SOAPI_FUNC) loader.LocateApi("libso2.so", "SO2API");
	SO1API(0, (void *)so1callback);
	for(int i=0; i<1; ++i) {
	    SO1API(1, NULL);
	}
	exit(0);
    }
    sleep(1);
    Mylog("Child<%d> created by Parent<%d>\n", childpid, getpid());
    Mylog("Parent exit\n");
    fflush(stdout);
    return 0;
}
