#include <stdio.h>

extern "C" 
{
    int funcFinal();
}

int main()
{
    printf("\nThe Answer = %d\n\n", funcFinal());
}