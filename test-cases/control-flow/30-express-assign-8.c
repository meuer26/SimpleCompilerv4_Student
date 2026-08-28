int func() 
{
   int x;
   int y = 2;
   int z = 5;
   int a = 0;
   int b = 1;
   int alpha;
   int c;

   do{
      y++;
      x = y + z;
      c = !b;
      z++;
      alpha = !a;

   } while (z < 900);

   return alpha;
}