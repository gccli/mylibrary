#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>

/*
 * This program demonstrate the detail of heap area (malloc) and memore mapping area (mmap)
 * The following results can be prove:
 * 1. Invalid read from unmapped area will cause Segmentation fault
 * 2. The area of malloc big memory is mapping area
 * 3. Invalid read freed area will not cause Segmentation fault when allocate small memory
 */

#define MAX_NUM_OF_MAPPING 1024
#define B_GREEN "\033[32m"
#define B_RED   "\033[31m"
#define B_END   "\033[0m"

int main(int argc, char *argv[])
{
    int malloc_block_size   = 1024*1024;
    int malloc_block_count  = 100;
    int mapping_block_size  = 1024*1024;
    int mapping_block_count = 100;
    
    int c,i;
    while (1) {
	int option_index = 0;
	static struct option long_options[] = {
	    {"malloc-block-count",  1, 0, 0},
	    {"malloc-block-size",   1, 0, 0},
	    {"mapping-block-count", 1, 0, 0},
	    {"mapping-block-size",  1, 0, 0},
	    {0, 0, 0, 0}
	};

	c = getopt_long(argc, argv, "h", long_options, &option_index);
	if (c == -1)
	    break;

	switch (c) {
	    case 0:
		if (strcmp(long_options[option_index].name, "malloc-block-size") == 0) {
		    malloc_block_size = atoi(optarg)*1024;
		} else if (strcmp(long_options[option_index].name, "malloc-block-count") == 0) {
		    malloc_block_count = atoi(optarg);
		} else if (strcmp(long_options[option_index].name, "mapping-block-size") == 0) {
		    mapping_block_size = atoi(optarg)*1024;
		} else if (strcmp(long_options[option_index].name, "mapping-block-count") == 0) {
		    mapping_block_count = atoi(optarg);
		}
		break;
	    default:
		printf("Usage: %s [ --malloc-block-size=N_kbytes] [--mapping-block-size=N_kbytes]\n", argv[0]);
		printf("       %*s [ --malloc-block-count=N] [--mapping-block-count=N_kbytes]\n", strlen(argv[0]), "");
		exit(0);
	}
    }    

    printf("Create %d heap block, block size:"B_GREEN"%d"B_END"\n", malloc_block_count, malloc_block_size);
    char *old, *heap = NULL; // malloc area (heap)
    for (i=0; i < malloc_block_count; ++i) {
	old = heap;
	heap = (char*)malloc(malloc_block_size);
	if (heap == NULL) {
	    printf("Can not allocate memory, i=%d\n", i);
	    return 1;
	}
	printf("%p  ", heap);
	if (((i+1) % 8) == 0)
	    printf("\n");
    } 

    printf("\n================================================================\n");
    printf("Create %d memory mapping block, block size:"B_GREEN"%d"B_END"\n", mapping_block_count, mapping_block_size);
    char *addr[MAX_NUM_OF_MAPPING] = {NULL};

    for(i=0; i<mapping_block_count; ++i) {
	if ((addr[i] = (char*)mmap(NULL, mapping_block_size, PROT_READ, MAP_PRIVATE|MAP_ANON, -1, 0)) == MAP_FAILED) {
	    printf("failed to create memory map: %s\n", strerror(errno));
	    return 1;
	} 
	printf("%p  ", addr[i]);
	if (((i+1) % 8) == 0)
	    printf("\n");
    }
    printf("\n================================================================\n");
    printf("++++Page size %zu\n", sysconf(_SC_PAGESIZE));
    printf("++++Total heap memory allocated: %d MB\n", malloc_block_count*(malloc_block_size/1024)/1024);
    printf("++++Total memory mapping allocated: %d MB\n", mapping_block_count*mapping_block_size/(1024*1024));

    setbuf(stdout, NULL);
    char *phi, *plo;// for high address, low address
    for(int j=1; j<mapping_block_count; ++j) {
	int sz;
	if (addr[j-1] > addr[j]) {
	    sz = addr[j-1] - addr[j];
	    phi = addr[j-1];plo = addr[j];
	    printf("++++Memory mapping grow down: area(%d ~ %d) = %d\n", j-1, j, sz);

	} else {
	    sz = addr[j] - addr[j-1];
	    phi = addr[j];plo = addr[j-1];
	    printf("++++Memory mapping grow up: area(%d ~ %d) = %d\n", j-1, j, sz);
	}
	if (sz == mapping_block_size) 
	    break;
    }
    if (old > heap) {
	printf("++++Heap grow down: old(%p ~ %p) new(%p ~ %p), has a gap %zu bytes\n",
	       old, old+malloc_block_size, heap, heap+malloc_block_size, old-heap-malloc_block_size);

    } else {
	printf("++++Heap grow up: old(%p ~ %p) new(%p ~ %p), has a gap %zu bytes\n",
	       old, old+malloc_block_size, heap, heap+malloc_block_size, heap-old-malloc_block_size);
    }
    printf("\n================================================================\n");

    int x = 0;
    printf("Try to access unallocated memory(%p):", old+malloc_block_size);
    *(int *) (old+malloc_block_size) = 100; // invalid write
    x = *(int *) (old+malloc_block_size);   // invalid read
    printf(B_GREEN"pass:%d\n"B_END, x);

    printf("Try to access unallocated memory(%p):", heap+malloc_block_size);
    *(int *) (heap+malloc_block_size) = 200; // invalid write
    x = *(int *) (heap+malloc_block_size);   // invalid read
    printf(B_GREEN"pass:%d\n"B_END, x);

    printf("Try to access freed memory(%p):", heap);
    free(heap);
    *(int *) (heap) = 300; // invalid write
    x = *(int *) heap;     // invalid read
    printf(B_GREEN"pass:%d\n"B_END, x);

    printf("Access mapped memory(%p) at offset 100:", plo);
    x = *(int *) (plo+100);
    printf(B_GREEN"pass:%d\n"B_END, x);

    if ((addr[i] = (char*)mmap(NULL, mapping_block_size+1024, PROT_READ, MAP_PRIVATE|MAP_ANON, -1, 0)) == MAP_FAILED) {
	printf("failed to create memory map: %s\n", strerror(errno));
	return 1;
    } 
    printf("Access mapped memory(%p), length=0x%x at offset 0x%x:", addr[i], mapping_block_size+1024, mapping_block_size+2048);
    x = *(int *) (addr[i]+mapping_block_size+2048);
    printf(B_GREEN"pass:%d\n"B_END, x);

    munmap(phi, mapping_block_size);
    // thsi will access phi, memory mapping grow down, cause coredump
    x = *(int *) (plo+mapping_block_size);
    printf("Access unmapped memory. munmap(%p~%p), dereference(%p), x = 0x%x\n", 
	   phi, phi+mapping_block_size, plo+mapping_block_size, x);

    return 0;
}
