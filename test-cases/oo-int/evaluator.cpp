#include <stdio.h>

extern "C"
{
    int func(int argA);
    int funcFinal(int argA);
}

int main()
{
    int argA = 5;

    printf("\nThe answer should be: %d\n", (unsigned int)func(argA));
    printf("The given answer was: %d\n", (unsigned int)funcFinal(argA));

    unsigned int baseline = func(argA);
    unsigned int test = funcFinal(argA);

    if (baseline == test)
    {
        printf("\nTEST PASSED!\n\n");
        return 0;
    }
    else
    {
        printf("\n***** TEST FAILED! *****\n\n");
        return -1;
    }
}
