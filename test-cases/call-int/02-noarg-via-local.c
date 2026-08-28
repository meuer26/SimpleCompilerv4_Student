int myCallee()
{
   return 100;
}

int func(int argA)
{
   int y = myCallee();
   return y;
}
