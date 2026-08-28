int myCallee(int x)
{
   return x + 1;
}

int func(int argA)
{
   int y = myCallee(argA);
   return y;
}
