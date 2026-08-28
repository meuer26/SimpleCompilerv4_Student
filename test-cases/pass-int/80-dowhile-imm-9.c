int func(int beta) 
{
   int x = 5;
   int y = 70;
   int alpha;
   int gamma = 200;
   beta = 200;

   do
   {
      x++;
      y++;
      alpha = 100;
      beta++;
      x++;
      y++;
      beta++;
      x++;
   }
   while (x < y);
   
   return beta;
}