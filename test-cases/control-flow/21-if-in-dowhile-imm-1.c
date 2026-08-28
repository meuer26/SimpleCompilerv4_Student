int func()
{
   int x = 0;
   int counted = 0;

   do
   {
      if (x < 5)
      {
         counted++;
      }
      x++;
   }
   while (x < 10);

   return counted;
}
