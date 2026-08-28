int func(int alpha) 
{
   int x = 5;
   int y = 70;
   int beta;
   int gamma = 200;
   
   do
   {
      x++;
      y++;
      alpha++;
      alpha++;
      x++;
      y++;
      x++;
   }
   while (x < y);
   
   return alpha;
}