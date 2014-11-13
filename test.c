#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#include "driver.h"

void runSimpleTests(){

  struct Stack *stackUnderTest = getStack(0); // this is the same as getting the stack corresponding to thread number 0
  assert(stackUnderTest != NULL);
  assert(stackUnderTest->_initialized == 1);
  assert(stackUnderTest->_size == 0);

  _instroPushIdentifier(42, key);
  assert(stackUnderTest->_size == 1);

  _instroPushIdentifier(42, key);
  assert(stackUnderTest->_size == 2);

  _instroPopIdentifier(key);
  assert(stackUnderTest->_size == 1);
}

int main(int argc, char** argv){

initBuffer();
createStackInstance();

runSimpleTests();

printf("Tests were successfull\n");
return 0;
}
