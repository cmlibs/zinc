#include <stdio.h>
#define N_POINTS_MAX 600

main()
{
  FILE *fp;
  int k;

  fp = fopen("weights.h", "w");
  
  for (k = 0; k < N_POINTS_MAX; k++)
    fprintf(fp, "%03d  1  1\n", k);
  fclose(fp);
  
}
