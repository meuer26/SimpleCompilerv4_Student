int func() 
{
   int x = 5;
   int y = 70;
   
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