#include <stdio.h>
#include <stdlib.h>
#include <math.h>
main()
{
  FILE *fp;
  int i, x, y;
  double a, radius;

  radius = 150.0;
  fp = fopen("circle.dat", "w");
  for (i = 0; i < 16; i++) {
    a = (2.0*3.141592654/16.0)*((double) i);
    x = 320.0 + radius*sin(a);
    y = 240.0 - radius*cos(a);
    fprintf(fp, "%02d %03d %03d\n", i, x, y);
  }
  fclose(fp);
}
