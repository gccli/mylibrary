#include <iostream>
using namespace std;

char *MemoryPool;

void OutOfMemory()
{
    printf("Unable to satisfy request for memory\n");
    sleep(2);
    delete [] MemoryPool;

    set_new_handler(NULL);
}

int main(int argc, char *argv[])
{
    MemoryPool = new char[512*1024*1024];

    set_new_handler(OutOfMemory);

    for (int i=1; ; ++i)
    {
	char *p = new (std::nothrow) char[1024*1024];
	if (i > 3000) {
	    printf("%d megabyte memory allocated\n", i);
	}
	else if ((i%100) == 0)
	{
	    printf("%d megabyte memory allocated\n", i);
	}
	if (p == NULL) {
	    printf("failed allocate %d'th megabyte memory\n", i);
	    break;
	}
  }

  return 0;
}
