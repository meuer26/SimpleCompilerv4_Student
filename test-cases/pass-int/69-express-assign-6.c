int func(int alpha) 
{
   int x;
   int y = 2;
   int z = 5;
   int a = 0;

   do{
      y++;
      x = y + z;
      z++;
      alpha = x + z;

   } while (z < 900);

   return alpha;
}