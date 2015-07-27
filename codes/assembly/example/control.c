#include <stdio.h>
void sum()
{
  // sum(1~100)
  /*
    Unsigned Conditional Jumps
    JA/JNBE (CF or ZF) = 0 Above/not below or equal
    JAE/JNB CF = 0 Above or equal/not below
    JB/JNAE CF = 1 Below/not above or equal
    JBE/JNA (CF or ZF) = 1 Below or equal/not above
    JC CF = 1 Carry
    JE/JZ ZF = 1 Equal/zero
    JNC CF = 0 Not carry
    JNE/JNZ ZF = 0 Not equal/not zero
    JNP/JPO PF = 0 Not parity/parity odd
    JP/JPE PF = 1 Parity/parity even
    JCXZ CX = 0 Register CX is zero
    JECXZ ECX = 0 Register ECX is zero
    Signed Conditional Jumps
    JG/JNLE ((SF xor OF) or ZF) = 0 Greater/not less or equal
    JGE/JNL (SF xor OF) = 0 Greater or equal/not less
    JL/JNGE (SF xor OF) = 1 Less/not greater or equal
    JLE/JNG ((SF xor OF) or ZF) = 1 Less or equal/not greater
    JNO OF = 0 Not overflow
    JNS SF = 0 Not sign (non-negative)
    JO OF = 1 Overflow
    JS SF = 1 Sign (negative)
  **/

  /*
    SUB  DEST <- (DEST - SRC);
    The SUB instruction performs integer subtraction. It evaluates the result for both
    signed and unsigned integer operands and sets the OF and CF flags to indicate an
    overflow in the signed or unsigned result, respectively. The SF flag indicates the sign
    of the signed result.
  **/
  int i=0, sum=0;
  __asm__(".L0:\n"
	  "addl %0, %%ebx; # add i to sum\n"
	  "incl %0;        # i++\n"
	  "cmpl $100, %0;  # compare i 100\n"
	  "jle .L0;\n"
	  : "=a" (i), "=b" (sum)
	  : "a" (i), "b" (sum)
	  );
  printf("%d\n", sum);
}

void sumof(int c)
{
}

int main()
{
  int x=0x1011;
  int y=0x0010;
  printf("x^y=0x%x\n", x^y);

  int z;
  sum();
  
  return z;
}
