#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

#include "stack.h"

void runSimpleTests(){

  struct Stack *stackUnderTest = _multithreadStack[0]; // this is the same as getting the stack corresponding to thread number 0
  assert(stackUnderTest != NULL);
  assert(stackUnderTest->_size == 0);

  _instroPushIdentifier(42);
  assert(stackUnderTest->_size == 1);

  _instroPushIdentifier(42);
  assert(stackUnderTest->_size == 2);

  _instroPopIdentifier();
  assert(stackUnderTest->_size == 1);
}

int main(int argc, char** argv){

// initialization is done by libmonitor

runSimpleTests();

printf("Tests were successfull\n");
return 0;
}
