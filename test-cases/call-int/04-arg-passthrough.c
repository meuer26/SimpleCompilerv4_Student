int myCallee(int x)
{
   return x;
}

int func(int argA)
{
   return myCallee(argA);
}
