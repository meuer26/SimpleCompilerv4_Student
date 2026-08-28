#include <stdio.h>

extern "C" 
{
    int func();
    int funcFinal();
}

int main()
{
    
    printf("\nThe answer should be: %d\n", (unsigned int)func());
    printf("The given answer was: %d\n", (unsigned int)funcFinal());

    unsigned int baseline = func();
    unsigned int test = funcFinal();
    
    if (baseline == test)
    {
        printf("\nTEST PASSED!\n\n");

        return 0; //passed
    }
    else
    {
        printf("\n***** TEST FAILED! *****\n\n");

        return 1; //failed
    }

    return 1;
}