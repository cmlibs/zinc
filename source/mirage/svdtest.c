
#include <stdio.h>
#include <stdlib.h>
#include "general/debug.h"
#include "user_interface/message.h"
#include "mirage/svd_cmgui.h"

void main(int argc,char **argv)
{
  int i,j;
  struct SVD_Object *obj = NULL;
  double *max,*min,*ex1,*ex2,*ptr;
  
  if (argc != 2)
  {
    printf("Usage: svdtest <basisfile>\n");
    exit(1);
  }

  if (!(obj = SVD_read_basis(argv[1],&obj)))
  {
    display_message(ERROR_MESSAGE,"svdtest: unable to allocate memory for obj");
    exit(1);
  }

  fprintf(stderr,"number of modes = %d\n",SVD_number_of_modes(obj));
  fprintf(stderr,"number of nodes = %d\n",SVD_number_of_nodes(obj));

  max = malloc((obj->n_modes)*sizeof(double));
  min = malloc((obj->n_modes)*sizeof(double));

  SVD_get_weights(max,min,obj);

#ifndef DUM
  for (i=0;i<(obj->n_modes);i++)
  {
    fprintf(stderr,"%d: %lf %lf %lf\n",i,obj->w[i],max[i],min[i]);
  }
#else
  for (i=0;i<(obj->n_nodes);i++)
  {
    fprintf(stderr,"%d: %d %lf\n",i,obj->index[i],obj->u[i]);
  }  
#endif
  ex1 = SVD_reconstruct(max,obj);
  ex2 = SVD_reconstruct(min,obj);

  printf("Group name: basis_nodes\n#Fields=1\n1) coordinates, coordinate, "
    "rectangular cartesian #Components=3\n x. Value index=1, #Derivatives=0\n"
    " y. Value index=2, #Derivatives=0\n z. Value index=3, #Derivatives=0\n");
  for (i=0;i<obj->n_nodes;i++)
  {
    printf("Node: %d\n",obj->index[i]);
    for (j=0;j<3;j++)
    {
      printf("%lf ",ex1[i*3+j]);
    }
    printf("\n");  
  }

  ptr = SVD_project(ex2,obj->index,obj);

  for (i=0;i<(obj->n_modes);i++)
  {
    fprintf(stderr,"%d: %lf\n",i,ptr[i]);
  }

  SVD_write_basis("aaaa",obj);

  destroy_SVD_Object(&obj);
  
  exit(0);
}
