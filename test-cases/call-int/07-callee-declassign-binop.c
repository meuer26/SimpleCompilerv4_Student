int myCallee(int x)
{
   int t = x + 5;
   return t;
}

int func(int argA)
{
   int y = myCallee(argA);
   return y;
}
