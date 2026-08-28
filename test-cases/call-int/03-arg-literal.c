int myCallee(int x)
{
   return x + 1;
}

int func(int argA)
{
   return myCallee(100);
}
