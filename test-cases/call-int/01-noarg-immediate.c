int myCallee()
{
   return 42;
}

int func(int argA)
{
   return myCallee();
}
