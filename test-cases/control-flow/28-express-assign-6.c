int func() 
{
   int x;
   int y = 2;
   int z = 5;
   int a = 0;
   int alpha;

   do{
      y++;
      x = y + z;
      z++;
      alpha = x + z;

   } while (z < 900);

   return alpha;
}