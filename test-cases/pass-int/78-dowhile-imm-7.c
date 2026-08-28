int func(int y) 
{
   int x = 5;
   
   do
   {
      x++;
      y++;
      x++;
      y++;
      x++;
   }
   while (x < y);
   
   return y;
}