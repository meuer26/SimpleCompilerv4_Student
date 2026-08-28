int func() 
{
   int x = 5;
   int y = 70;
   int beta;
   int alpha;
   int gamma = 200;

   beta = 10;

   do
   {
      x++;
      y++;
      alpha = 100;
      x++;
      y++;
      beta++;
      x++;
   }
   while (x < y);
   
   return beta;
}