/*******************************************************************************
FILE : test_delauney.c

LAST MODIFIED : 7 October 2000

DESCRIPTION :
Used to test the Delauney triangularization.

Reads in a file with x y z locations, one to a line, and outputs an exnode file
and an exelem file.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "general/debug.h"
#include "unemap/delauney.h"

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
==============================================================================*/
{
	char out_file_name[81];
	FILE *locations_file,*out_file;
	float *locations,*locations_temp,x,y,z;
	int i,id,*id_array,*id_array_temp,number_of_locations,number_of_triangles,
		return_code,*triangles;

	return_code=0;
	/* check arguments */
	if (3==argc)
	{
		if (locations_file=fopen(argv[1],"r"))
		{
			/* count the number of locations */
			return_code=1;
			number_of_locations=0;
			locations=(float *)NULL;
			id_array=(int *)NULL;
			while (return_code&&(4==fscanf(locations_file," %d %g %g %g",&id,&x,&y,
				&z)))
			{
				number_of_locations++;
				if (REALLOCATE(locations_temp,locations,float,3*number_of_locations)&&
					REALLOCATE(id_array_temp,id_array,int,number_of_locations))
				{
					locations=locations_temp;
					id_array=id_array_temp;
					locations[3*number_of_locations-3]=x;
					locations[3*number_of_locations-2]=y;
					locations[3*number_of_locations-1]=z;
					id_array[number_of_locations-1]=id;
				}
				else
				{
					printf("Could not reallocate storage information %d\n",
						number_of_locations);
					return_code=0;
				}
			}
			if (return_code)
			{
				if (feof(locations_file))
				{
					if (number_of_locations<=0)
					{
						printf("No data\n");
						return_code=0;
					}
				}
				else
				{
					printf("Error reading location %d\n",number_of_locations);
					return_code=0;
				}
			}
			if (return_code)
			{
				/*???debug */
				printf("number_of_locations=%d\n",number_of_locations);
				if (cylinder_delauney(number_of_locations,locations,
					&number_of_triangles,&triangles))
				{
					/*???debug */
					printf("number_of_triangles=%d\n",number_of_triangles);
					/* write exnode file */
					strcpy(out_file_name,argv[2]);
					strcat(out_file_name,".exnode");
					if (out_file=fopen(out_file_name,"w"))
					{
						fprintf(out_file,"Group name : %s\n",argv[2]);
						fprintf(out_file,"#Fields=1\n");
						fprintf(out_file,
					"1) coordinates, coordinate, rectangular cartesian, #Components=3\n");
						fprintf(out_file,"  x.  Value index= 1, #Derivatives= 0\n");
						fprintf(out_file,"  y.  Value index= 2, #Derivatives= 0\n");
						fprintf(out_file,"  z.  Value index= 3, #Derivatives= 0\n");
						locations_temp=locations;
						id_array_temp=id_array;
						for (i=number_of_locations;i>0;i--)
						{
							fprintf(out_file,"Node: %d\n",*id_array_temp);
							id_array_temp++;
							fprintf(out_file,"  %g\n",*locations_temp);
							locations_temp++;
							fprintf(out_file,"  %g\n",*locations_temp);
							locations_temp++;
							fprintf(out_file,"  %g\n",*locations_temp);
							locations_temp++;
						}
						fclose(out_file);
					}
					else
					{
						printf("Could not open exnode file: %s\n",out_file_name);
						return_code=0;
					}
					/* write exelem file */
					strcpy(out_file_name,argv[2]);
					strcat(out_file_name,".exelem");
					if (out_file=fopen(out_file_name,"w"))
					{
						fprintf(out_file,"Group name : %s\n",argv[2]);
						fprintf(out_file,"Shape.  Dimension=1\n");
						for (i=1;i<=3*number_of_triangles;i++)
						{
							fprintf(out_file,"Element: 0 0 %d\n",i);
						}
						fprintf(out_file,"Shape.  Dimension=2, simplex(2)*simplex\n");
						fprintf(out_file,"#Scale factor sets=1\n");
						fprintf(out_file,"  l.simplex(2)*l.simplex, #Scale factors=3\n");
						fprintf(out_file,"#Nodes= 3\n");
						fprintf(out_file,"#Fields=1\n");
						fprintf(out_file,
					"1) coordinates, coordinate, rectangular cartesian, #Components=3\n");
						fprintf(out_file,
						"  x.  l.simplex(2)*l.simplex, no modify, standard node based.\n");
						fprintf(out_file,"    #Nodes= 3\n");
						fprintf(out_file,"      1.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   1\n");
						fprintf(out_file,"      2.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   2\n");
						fprintf(out_file,"      3.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   3\n");
						fprintf(out_file,
						"  y.  l.simplex(2)*l.simplex, no modify, standard node based.\n");
						fprintf(out_file,"    #Nodes= 3\n");
						fprintf(out_file,"      1.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   1\n");
						fprintf(out_file,"      2.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   2\n");
						fprintf(out_file,"      3.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   3\n");
						fprintf(out_file,
						"  z.  l.simplex(2)*l.simplex, no modify, standard node based.\n");
						fprintf(out_file,"    #Nodes= 3\n");
						fprintf(out_file,"      1.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   1\n");
						fprintf(out_file,"      2.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   2\n");
						fprintf(out_file,"      3.  #Values=1\n");
						fprintf(out_file,"        Value indices:     1\n");
						fprintf(out_file,"        Scale factor indices:   3\n");
						for (i=0;i<number_of_triangles;i++)
						{
							fprintf(out_file,"Element: %d 0 0\n",i+1);
							fprintf(out_file,"  Faces:\n");
							fprintf(out_file,"    0 0 %d\n",3*i+1);
							fprintf(out_file,"    0 0 %d\n",3*i+2);
							fprintf(out_file,"    0 0 %d\n",3*i+3);
							fprintf(out_file,"  Nodes:\n");
							fprintf(out_file,"    %d %d %d\n",id_array[triangles[3*i]],
								id_array[triangles[3*i+1]],id_array[triangles[3*i+2]]);
							fprintf(out_file,"  Scale factors:\n");
							fprintf(out_file,"    1. 1. 1.\n");
						}
						fclose(out_file);
					}
					else
					{
						printf("Could not open exelem file: %s\n",out_file_name);
						return_code=0;
					}
				}
			}
			fclose(locations_file);
		}
		else
		{
			printf("Could not open locations file: %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf("usage: test_delauney locations_file group_name\n");
		printf(
			"  locations_file is the name of the file vertex locations (provided)\n");
		printf(
			"  group_name is the name of the exnode and exelem files (created)\n");
		return_code=0;
	}

	return (return_code);
} /* main */
