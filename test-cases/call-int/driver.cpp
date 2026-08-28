#include <stdio.h>

extern "C"
{
    int funcFinal(int argA);
}

int main()
{
   int argA = 5;
   printf("\nThe Answer = %d\n\n", funcFinal(argA));
}
