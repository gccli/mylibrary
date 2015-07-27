#include <unistd.h>
#include <stdio.h>
// Load Effective Address
// LEAL Store effective address for m in register r32.
void func1(int x)
{
  int y;
  __asm__ ("leal (%1,%1,4), %0\n\t"
	   : "=r" (y)
	   : "r"  (x)
	   );
  printf("x*5=%d\n", y);
}

// "0" here specifies the same constraint as the 0th output variable
void func2(int x)
{
  int y=0;
  __asm__ ("leal (%0,%0,4), %0\n\t"
	   : "=r" (y)
	   : "0" (x)
	   );
  printf("x*5=%d\n", y);
}

// c is refered to ecx, so not list ecx in clobber list
// read-write operands
void func3(int *x)
{
  __asm__ ("leal (%%ecx,%%ecx,4), %%ecx\n\t"
	   : "=c" (*x)
	   : "c" (*x)
	   );
  printf("x*5=%d\n", *x);
}

// the register %eax is used as both the input and the output variable.
// "0" here specifies the same constraint as the 0th output variable
void func4(int *x)
{
  __asm__ ("incl %0" :"=a"(*x):"0"(*x));

  printf("incl x = %d\n", *x);
}

static inline pid_t gettid()
{
  pid_t tid;
  __asm__("movl $224,%%eax;\n"  /* SYS_gettid is 224 */
	  "int  $0x80;\n"       /* enter kernel mode */
	  "movl %%eax, %0"
	  : "=c" (tid)
	  );

  return tid;
}

// The source address is stored in esi, destination in edi, and then starts the copy, when we reach at 0, copying is complete. 
// Constraints "&S", "&D", "&a" say that the registers esi, edi and eax are early clobber registers, ie, their contents will change before the completion of the function. 
// LODSB (byte loaded into register AL), LODSW (word loaded into AX), or LODSD(doubleword loaded into EAX)

static inline char *strcpy(char *dest, const char *src)
{
  int d0, d1, d2;
  __asm__ __volatile__("1:\tlodsb\n\t"
                       "stosb\n\t"
                       "testb %%al,%%al\n\t"
                       "jne 1b"
		       : "=&S" (d0), "=&D" (d1), "=&a" (d2)
		       : "0" (src), "1" (dest) 
		       : "memory");

  printf("src 0x%x, dst 0x%x, 0x%x\n", d0, d1, d2);
  printf("len = %d\n", d1-(int )dest);
  return dest;
}

static inline int mystrlen(char *src)
{
  int d0,d1;
  __asm__ __volatile__("1:\tlodsb\n\t"
                       "scasb\n\t"
                       "testb %%al,%%al\n\t"
                       "jne 1b"
		       : "=&S" (d0), "=&a" (d1)
		       : "0" (src)
		       : "memory");

  return d0-(int )src;
}

static inline int mystrlen1(char *src)
{
  int d0,d1;
  __asm__ __volatile__("cld\n\t"
		       "1:\tlodsb\n\t"
                       "testb %%al,%%al\n\t"
                       "jne 1b"
		       : "=&S" (d0), "=&a" (d1)
		       : "0" (src)
		       : "memory");

  return d0-(int )src;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
      printf("usage: %s arg\n", argv[0]);
      return 1;
    }

  char buffer[12];

  int var = 12;
  func1(var);
  func2(var);
  func3(&var);
  func4(&var);
  printf("incl x = %d\n", var);

  printf("gettid() %d\n", gettid());
  printf("getpid() %d\n", getpid());

  printf("src %p, dst %p\n", &argv[1], buffer);
  strcpy(buffer, argv[1]);
  printf("%s\n", buffer);
  //  printf("%d\n", mystrlen(buffer));

  printf("%d\n", mystrlen1(buffer));

  return 0;
}
