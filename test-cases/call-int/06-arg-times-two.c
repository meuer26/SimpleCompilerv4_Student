int myCallee(int x)
{
   return x * 2;
}

int func(int argA)
{
   int y = myCallee(argA);
   return y;
}
