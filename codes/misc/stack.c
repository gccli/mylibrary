#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int glx;
void *glp;
static int gly;
static void *glq;
const int glz = 100; 
const char *glr = "hello, world";
char glarray[128];

void basic_section()
{
  printf(".text    The machine code of the compiled program\n");
  printf(".rodata  Read-only data such as the format strings in printf statements, and jump tables for switch statements\n");
  printf(".data    Initialized global C variables. Local C variables are maintained at run time on the stack, and do not appear in either the .data or .bss sections.\n");
  printf(".bss     Uninitialized global C variables. This section occupies no actual space in the object file; \n");
  printf("\n");
}


void stack()
{
  printf("<-  Process Private Address Space  ->\n");

  printf("0xffffffff |------------------------|\n");
  printf("           | kernel virtual memory  |\n");
  printf("0xc0000000 |------------------------|\n");
  printf("           |      user stack        |\n");
  printf("           | (created at runtime)   |\n");
  printf("           |------------------------| <- %%esp (stack pointer)\n");
  printf("           |                        |  grown down\n");
  printf("           |                        |\n");
  printf("           |------------------------|\n");
  printf("           |memory mapped region for|\n");
  printf("           |   shared libraries     |\n");
  printf("0x40000000 |------------------------|\n");
  printf("           |                        |\n");
  printf("           |                        |  grow up\n");
  printf("           |------------------------| <- brk\n");
  printf("           |     run-time heap      |\n");
  printf("           |    (malloc, calloc)    |\n");
  printf("           |------------------------|\n");
  printf("           |   read/write segment   |\n");
  printf("           |      (.data, .bss)     |\n");
  printf("           |------------------------|\n");
  printf("           |   read-only segment    |\n");
  printf("           |(.init, .text, .rodata) |\n");
  printf("0x08048000 |------------------------|\n");
  printf("           |         unused         |\n");
  printf("0x00000000 |------------------------|\n");
  printf("\n");

}

void stack_growdown(int i)
{
  char buffer[256];
  printf("address on stack 0x%08x\n", buffer);
  if (i==0)
    return ;
  i--;
  stack_growdown(i);
}

void heap_growup(int i)
{
  while (i--) {
    char *buffer = calloc(1,256);
    printf("address on heap 0x%08x\n", buffer);
  }
}

int main(int argc, char *argv[])
{
  printf("BRK -> 0x%08x\n\n", sbrk(0));

  stack();
  basic_section();

  int x;
  char *p = malloc(256);
  static int y;
  static char *q;
  const int z = 100;
  const char *r = "hello, world";
  char array[256];

  glp = calloc(256, 1);  

  printf("\nLocal variables\n");
  printf("x:0x%08x p:0x%08x array:0x%08x  (run-time stack)\n", &x, &p, array);
  printf("y:0x%08x q:0x%08x  (local static)\n", &y, &q);
  printf("z:0x%08x r:0x%08x  (local const)\n", &z, r);
  printf("run-time heap:0x%08x\n", p);

  printf("\nGlobal Variables\n");
  printf("x:0x%08x p:0x%08x array:0x%08x (.bss)\n", &glx, &glp, glarray);
  printf("y:0x%08x q:0x%08x  (.data)\n", &gly, &glq);
  printf("z:0x%08x r:0x%08x  (.rodata)\n", &glz, glr);
  printf("run-time heap:0x%08x\n", glp);

  return 0;
}
