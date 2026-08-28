int func(int y) 
{
   int x = 5;
   y = 70;
   
   do
   {
      x++;
      y++;
      x++;
      y++;
      x++;
   }
   while (x < 127);
   
   return x;
}