
// EIP: program uses the EIP register to control execution.
// ESP: Stack Pointer
// EBP: Stack bottom pointer


void g()
{
  printf("hello world\n");
}


void f()
{
  char buffer[4];
  memcpy(buffer, "AAAABBBBCCCCDDDDEEEEFFFFGGGG", 28);
}


int main(int argc, char *argv[])
{
  f();
  return 0;
}

