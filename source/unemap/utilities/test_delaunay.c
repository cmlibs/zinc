/*******************************************************************************
FILE : test_delaunay.c

LAST MODIFIED : 18 April 2004

DESCRIPTION :
Used to test the Delaunay triangularization.

Reads in a file with x y z locations, one to a line, and outputs an exnode file
and an exelem file.

BUILD (for FORTRAN_STANDALONE) :
gcc -g -c -DFORTRAN_STANDALONE test_delaunay.c -o test_delaunay.o
g77 -g delaunay.f test_delaunay.o -lm -o test_delaunay
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#if defined (FORTRAN_STANDALONE)
#include <math.h>

#define sphere_delaunay sphere_delaunay__

void sphere_delaunay(int *,double *,int *,int *,int *,double *,int *);

#define ENTER( function_name )

#define LEAVE

#define ALLOCATE( result , type , number ) \
 ( result = ( 0 < ( number ) ) ? ( type * )malloc( ( number ) * sizeof( type ) ) : ( type * )NULL )

#define DEALLOCATE( ptr ) { if ( ptr ) { free( (char *)( ptr ) ); ( ptr ) = NULL; } }

#define REALLOCATE( final , initial , type , number ) \
 ( final = ( 0 < ( number ) ) ? ( type * )realloc( (char *)( initial ) , ( number ) * sizeof( type ) ) : ( type * )NULL )

#if !defined (PI)
#define PI 3.14159265358979323846
#endif

#define MESSAGE_STRING_SIZE 1000
static char message_string[MESSAGE_STRING_SIZE];

/*
Types
-----
*/
enum Message_type
/*******************************************************************************
LAST MODIFIED : 31 May 1996

DESCRIPTION :
The different message types.
==============================================================================*/
{
	ERROR_MESSAGE,
	INFORMATION_MESSAGE,
	WARNING_MESSAGE
}; /* enum Message_type */

/*
Functions
---------
*/
int display_message(enum Message_type message_type,char *format, ... )
/*******************************************************************************
LAST MODIFIED : 7 September 2000

DESCRIPTION :
A function for displaying a message of the specified <message_type>.  The printf
form of arguments is used.
==============================================================================*/
{
	int return_code;
	va_list ap;

	ENTER(display_message);
	va_start(ap,format);
	return_code=vsprintf(message_string,format,ap);
	if (return_code >= (MESSAGE_STRING_SIZE-1))
	{
		char error_string[100];

		sprintf(error_string,"Overflow of message_string.  "
			"Following is truncated to %d characters:",return_code);
		return_code=printf("ERROR: %s\n",error_string);
	}
	switch (message_type)
	{
		case ERROR_MESSAGE:
		{
			return_code=printf("ERROR: %s\n",message_string);
		} break;
		case INFORMATION_MESSAGE:
		{
			/* make sure we don't interpret % characters by printing the string */
			return_code=printf("%s",message_string);
		} break;
		case WARNING_MESSAGE:
		{
			return_code=printf("WARNING: %s\n",message_string);
		} break;
		default:
		{
			return_code=printf("UNKNOWN: %s\n",message_string);
		} break;
	}
	va_end(ap);
	LEAVE;

	return (return_code);
} /* display_message */
#else /* defined (FORTRAN_STANDALONE) */
#include "general/debug.h"
#include "unemap/delaunay.h"
#endif /* defined (FORTRAN_STANDALONE) */

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 18 April 2004

DESCRIPTION :
==============================================================================*/
{
	char out_file_name[81],geometry_type[9];
	FILE *locations_file,*out_file;
	int i,id,*id_array,*id_array_temp,number_of_locations,number_of_triangles,
		return_code,*triangles;
#if defined (FORTRAN_STANDALONE)
	int maximum_number_of_triangles;
	double *locations,*locations_temp,*rworking,x,y,z;
#else /* defined (FORTRAN_STANDALONE) */
	float *locations,*locations_temp,x,y,z;
#endif /* defined (FORTRAN_STANDALONE) */

	return_code=0;
	/* check arguments */
	if (4==argc)
	{
		i=0;
		while ((i<8)&&argv[1][i])
		{
			if (isupper(argv[1][i]))
			{
				geometry_type[i]=tolower(argv[1][i]);
			}
			else
			{
				geometry_type[i]=argv[1][i];
			}
			i++;
		}
		geometry_type[i]='\0';
#if defined (FORTRAN_STANDALONE)
		if (!strcmp(geometry_type,"sphere"))
#else /* defined (FORTRAN_STANDALONE) */
		if ((!strcmp(geometry_type,"cylinder"))||(!strcmp(geometry_type,"sphere"))||
			(!strcmp(geometry_type,"plane")))
#endif /* defined (FORTRAN_STANDALONE) */
		{
			if (locations_file=fopen(argv[2],"r"))
			{
				/* count the number of locations */
				return_code=1;
				number_of_locations=0;
#if defined (FORTRAN_STANDALONE)
				locations=(double *)NULL;
#else /* defined (FORTRAN_STANDALONE) */
				locations=(float *)NULL;
#endif /* defined (FORTRAN_STANDALONE) */
				id_array=(int *)NULL;
				if (strcmp(geometry_type,"plane"))
				{
					while (return_code&&
#if defined (FORTRAN_STANDALONE)
						(4==fscanf(locations_file," %d %lf %lf %lf",&id,&x,&y,&z))
#else /* defined (FORTRAN_STANDALONE) */
						(4==fscanf(locations_file," %d %f %f %f",&id,&x,&y,&z))
#endif /* defined (FORTRAN_STANDALONE) */
						)
					{
						number_of_locations++;
						if (
#if defined (FORTRAN_STANDALONE)
							REALLOCATE(locations_temp,locations,double,
							3*number_of_locations)&&
#else /* defined (FORTRAN_STANDALONE) */
							REALLOCATE(locations_temp,locations,float,3*number_of_locations)&&
#endif /* defined (FORTRAN_STANDALONE) */
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
				}
				else
				{
					while (return_code&&
#if defined (FORTRAN_STANDALONE)
						(3==fscanf(locations_file," %d %lf %lf",&id,&x,&y))
#else /* defined (FORTRAN_STANDALONE) */
						(3==fscanf(locations_file," %d %f %f",&id,&x,&y))
#endif /* defined (FORTRAN_STANDALONE) */
						)
					{
						number_of_locations++;
						if (
#if defined (FORTRAN_STANDALONE)
							REALLOCATE(locations_temp,locations,double,
							2*number_of_locations)&&
#else /* defined (FORTRAN_STANDALONE) */
							REALLOCATE(locations_temp,locations,float,2*number_of_locations)&&
#endif /* defined (FORTRAN_STANDALONE) */
							REALLOCATE(id_array_temp,id_array,int,number_of_locations))
						{
							locations=locations_temp;
							id_array=id_array_temp;
							locations[2*number_of_locations-2]=x;
							locations[2*number_of_locations-1]=y;
							id_array[number_of_locations-1]=id;
						}
						else
						{
							printf("Could not reallocate storage information %d\n",
								number_of_locations);
							return_code=0;
						}
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
#if defined (FORTRAN_STANDALONE)
					maximum_number_of_triangles=4*number_of_locations;
					ALLOCATE(rworking,double,4*maximum_number_of_triangles);
					ALLOCATE(triangles,int,3*maximum_number_of_triangles);
					if (rworking&&triangles)
					{
						sphere_delaunay(&number_of_locations,locations,
							&maximum_number_of_triangles,triangles,&number_of_triangles,
							rworking,&return_code);
						/*???DB.  Different return code convention */
						if (0==return_code)
						{
							return_code=1;
							for (i=0;i<3*number_of_triangles;i++)
							{
								triangles[i]--;
							}
						}
						else
						{
							return_code=0;
							DEALLOCATE(triangles);
						}
					}
					else
					{
						printf("Could not allocate working arrays.  %p %p\n",rworking,
							triangles);
						return_code=0;
						DEALLOCATE(triangles);
					}
					DEALLOCATE(rworking);
#else /* defined (FORTRAN_STANDALONE) */
					if (!strcmp(geometry_type,"cylinder"))
					{
						return_code=cylinder_delaunay(number_of_locations,locations,
							&number_of_triangles,&triangles);
					}
					else if (!strcmp(geometry_type,"sphere"))
					{
						return_code=sphere_delaunay(number_of_locations,locations,
							&number_of_triangles,&triangles);
					}
					else
					{
						return_code=plane_delaunay(number_of_locations,locations,
							&number_of_triangles,&triangles);
					}
#endif /* defined (FORTRAN_STANDALONE) */
					if (return_code)
					{
						/*???debug */
						printf("number_of_triangles=%d\n",number_of_triangles);
						/* write exnode file */
						strcpy(out_file_name,argv[3]);
						strcat(out_file_name,".exnode");
						if (out_file=fopen(out_file_name,"w"))
						{
							fprintf(out_file,"Group name : %s\n",argv[3]);
							fprintf(out_file,"#Fields=1\n");
							if (strcmp(geometry_type,"plane"))
							{
								fprintf(out_file,
					"1) coordinates, coordinate, rectangular cartesian, #Components=3\n");
							}
							else
							{
								fprintf(out_file,
					"1) coordinates, coordinate, rectangular cartesian, #Components=2\n");
							}
							fprintf(out_file,"  x.  Value index= 1, #Derivatives= 0\n");
							fprintf(out_file,"  y.  Value index= 2, #Derivatives= 0\n");
							if (strcmp(geometry_type,"plane"))
							{
								fprintf(out_file,"  z.  Value index= 3, #Derivatives= 0\n");
							}
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
								if (strcmp(geometry_type,"plane"))
								{
									fprintf(out_file,"  %g\n",*locations_temp);
									locations_temp++;
								}
							}
							fclose(out_file);
						}
						else
						{
							printf("Could not open exnode file: %s\n",out_file_name);
							return_code=0;
						}
						/* write exelem file */
						strcpy(out_file_name,argv[3]);
						strcat(out_file_name,".exelem");
						if (out_file=fopen(out_file_name,"w"))
						{
							fprintf(out_file,"Group name : %s\n",argv[3]);
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
							if (strcmp(geometry_type,"plane"))
							{
								fprintf(out_file,
					"1) coordinates, coordinate, rectangular cartesian, #Components=3\n");
							}
							else
							{
								fprintf(out_file,
					"1) coordinates, coordinate, rectangular cartesian, #Components=2\n");
							}
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
							if (strcmp(geometry_type,"plane"))
							{
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
							}
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
				printf("Could not open locations file: %s\n",argv[3]);
				return_code=0;
			}
		}
		else
		{
			printf("Invalid geometry type: %s\n",geometry_type);
			return_code=0;
		}
	}
	else
	{
		printf("usage: test_delaunay CYLINDER|PLANE|SPHERE locations_file group_name\n");
		printf(
			"  CYLINDER|PLANE|SPHERE keyword for type of geometry (case insensitive)\n");
		printf(
			"  locations_file is the name of the file vertex locations (provided)\n");
		printf(
			"  group_name is the name of the exnode and exelem files (created)\n");
		return_code=0;
	}

	return (return_code);
} /* main */
