#ifndef STACK_QUEUE_H__
#define STACK_QUEUE_H__
#include <stdio.h>
#include <stdlib.h>

#define CAPACITY 10

class Stack {
public:
  Stack()
    :top(0)
  { }

  bool empty() const { return top == 0; }
  bool full() const { return top == CAPACITY; }
  bool push(int x) {
    if (full())
      return false;
    top++;
    storage[top] = x;
    return true;
  }

  bool pop(int x, int &r) {
    if (empty())
      return false;
    top--;
    r = storage[top];
    return true;
  }



  int top;
  int storage[CAPACITY+1];

};

#endif
