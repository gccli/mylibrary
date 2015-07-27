#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef void *(*P_MALLOC_HOOK)(size_t, const void *);
class MemoryManger {
public:
    MemoryManger() {}
    ~MemoryManger() {}

    int Initialize()
    {
	int size;
	m_old_malloc_hook = __malloc_hook;
	__malloc_hook = malloc_hook;

	// adjusts parameters for memory allocation
	mallopt(M_CHECK_ACTION,3);
	size = 1024*16;
	printf("Set M_MMAP_THRESHOLD %d\n", size);
	mallopt(M_MMAP_THRESHOLD, size);
	return 0;
    }

private:
    static P_MALLOC_HOOK m_old_malloc_hook;
    static void *malloc_hook(size_t size, const void *caller);
};

P_MALLOC_HOOK MemoryManger::m_old_malloc_hook;
void *MemoryManger::malloc_hook(size_t size, const void *caller)
{
    void *result;

    __malloc_hook = m_old_malloc_hook; // Restore all old hooks
    result = malloc(size);
    m_old_malloc_hook = __malloc_hook; // Save underlying hooks
    printf("malloc(%zu) returns %p\n", size, result);
    __malloc_hook = malloc_hook;       // Restore our own hooks

    return result;
}

int i;
void bye(void)
{
    printf("%d\n", i);
}

int main(int argc, char *argv[])
{
    int size, total, loop;
    char temp[1024];
    if (argc < 2) {
	printf("Usage: %s size loop\n", argv[0]);
	return 0;
    }
    atexit(bye);
    size = atoi(argv[1]);
    loop = atoi(argv[2]);

    MemoryManger mm;
    mm.Initialize();
    strdupa(argv[1]);
    total=size*loop;

    printf ("Total sizeof memory allocated is %d\n", total);

    for(i=1; i<=loop; ++i) {
	int allocsz = size*i+random()%1024;
	char *p = (char *)calloc(allocsz, 1);
	if (i==1) {
	    printf("--------------------------------\n");
	}
	free(p);
	strcpy(temp, p);
    }

    return 0;
}

