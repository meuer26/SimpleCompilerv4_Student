int func()
{
   int x = 1;
   int y = 0;

   if (x < 10)
   {
      do
      {
         y++;
      }
      while (y < 5);
   }
   else
   {
      y = 99;
   }

   return y;
}
