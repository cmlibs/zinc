/*******************************************************************************
FILE : biosense2vrml.c

LAST MODIFIED : 24 February 2000

DESCRIPTION :
Converts a a Biosense location and activation time file to a vrml file.

Created by cutting and pasting from graphics/rendervrml.c and
graphics/spectrum_settings.c (didn't want the generality/complexity of cmgui).
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
	/*???DB.  Contains definition of __BYTE_ORDER for Linux */
#include "general/debug.h"
#include "general/myio.h"
#include "user_interface/message.h"

int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 24 February 2000

DESCRIPTION :
==============================================================================*/
{
	char flag,*status_array,*status_array_temp;
	FILE *biosense_file,*vrml_file;
	float blue,centre_x,centre_y,centre_z,green,max_x,max_y,max_z,min_x,min_y,
		min_z,red,sphere_radius,value,view_radius,x,*x_array,*x_array_temp,y,
		*y_array,*y_array_temp,z,*z_array,*z_array_temp;
	int i,id,*id_array,*id_array_temp,max_time,min_time,number_of_positions,
		return_code,time,*time_array,*time_array_temp;

	return_code=0;
	/* check arguments */
	if (3==argc)
	{
		if (biosense_file=fopen(argv[1],"r"))
		{
			/* count the number of position and determine the minimum and maximum for
				activation time, x, y, and z */
			return_code=1;
			number_of_positions=0;
			status_array=(char *)NULL;
			x_array=(float *)NULL;
			y_array=(float *)NULL;
			z_array=(float *)NULL;
			time_array=(int *)NULL;
			id_array=(int *)NULL;
			max_time=0;
			min_time=1;
			max_x=0;
			min_x=1;
			max_y=0;
			min_y=1;
			max_z=0;
			min_z=1;
			while (return_code&&(4==fscanf(biosense_file," %d %g %g %g",&id,&x,&y,
				&z)))
			{
				number_of_positions++;
				if (
					REALLOCATE(status_array_temp,status_array,char,number_of_positions)&&
					REALLOCATE(x_array_temp,x_array,float,number_of_positions)&&
					REALLOCATE(y_array_temp,y_array,float,number_of_positions)&&
					REALLOCATE(z_array_temp,z_array,float,number_of_positions)&&
					REALLOCATE(id_array_temp,id_array,int,number_of_positions)&&
					REALLOCATE(time_array_temp,time_array,int,number_of_positions))
				{
					status_array=status_array_temp;
					x_array=x_array_temp;
					y_array=y_array_temp;
					z_array=z_array_temp;
					time_array=time_array_temp;
					id_array=id_array_temp;
					x_array[number_of_positions-1]=x;
					if (max_x<min_x)
					{
						min_x=x;
						max_x=x;
					}
					else
					{
						if (x<min_x)
						{
							min_x=x;
						}
						else
						{
							if (max_x<x)
							{
								max_x=x;
							}
						}
					}
					y_array[number_of_positions-1]=y;
					if (max_y<min_y)
					{
						min_y=y;
						max_y=y;
					}
					else
					{
						if (y<min_y)
						{
							min_y=y;
						}
						else
						{
							if (max_y<y)
							{
								max_y=y;
							}
						}
					}
					z_array[number_of_positions-1]=z;
					if (max_z<min_z)
					{
						min_z=z;
						max_z=z;
					}
					else
					{
						if (z<min_z)
						{
							min_z=z;
						}
						else
						{
							if (max_z<z)
							{
								max_z=z;
							}
						}
					}
					id_array[number_of_positions-1]=id;
					if (1==fscanf(biosense_file," %d",&time))
					{
						time_array[number_of_positions-1]=time;
						if (max_time<min_time)
						{
							min_time=time;
							max_time=time;
						}
						else
						{
							if (time<min_time)
							{
								min_time=time;
							}
							else
							{
								if (max_time<time)
								{
									max_time=time;
								}
							}
						}
						status_array[number_of_positions-1]=1;
					}
					else
					{
						status_array[number_of_positions-1]=0;
					}
					if (1!=fscanf(biosense_file," %c",&flag))
					{
						printf("Could not read flag %d\n",number_of_positions);
						return_code=0;
					}
				}
				else
				{
					printf("Could not reallocate storage information %d\n",
						number_of_positions);
					return_code=0;
				}
			}
			if (return_code)
			{
				if (feof(biosense_file))
				{
					if (number_of_positions<=0)
					{
						printf("No data\n");
						return_code=0;
					}
				}
				else
				{
					printf("Error reading location %d\n",number_of_positions);
					return_code=0;
				}
			}
			if (return_code)
			{
				/*???debug */
				printf("number_of_positions=%d\n",number_of_positions);
				printf("  min_x=%g, max_x=%g\n",min_x,max_x);
				printf("  min_y=%g, max_y=%g\n",min_y,max_y);
				printf("  min_z=%g, max_z=%g\n",min_z,max_z);
				printf("  min_time=%d, max_time=%d\n",min_time,max_time);
				/* open file and add header */
				if (vrml_file=fopen(argv[2],"w"))
				{
					/* 1. Write the VRML header */
					/*???RC Add copyright message, source/how to contact UniServices??
						Treat this as Advertising... */
					fprintf(vrml_file,"#VRML V2.0 utf8\n# CMGUI VRML Generator\n");
					/* 2. Write scene graph */
					/* transform.... */
					/* draw objects */
					fprintf(vrml_file,"Group\n");
					fprintf(vrml_file,"{\n");
					fprintf(vrml_file,"  children\n");
					fprintf(vrml_file,"  [\n");
					/* get centre and radius of smallest sphere enclosing visible scene */
					centre_x=0.5*(max_x+min_x);
					centre_y=0.5*(max_y+min_y);
					centre_z=0.5*(max_z+min_z);
					/*???RC enlargement factor should be read in from defaults file */
					/* can only proceed if positive radius */
					if (0.0<(view_radius=/* factor */1.0*sqrt((max_x-min_x)*(max_x-min_x)+
						(max_y-min_y)*(max_y-min_y)+(max_z-min_z)*(max_z-min_z))))
					{
						fprintf(vrml_file,"    Viewpoint\n");
						fprintf(vrml_file,"    {\n");
						fprintf(vrml_file,"      description \"default\"\n");
						fprintf(vrml_file,"      position %f %f %f\n",centre_x,centre_y,
							centre_z+view_radius);
						fprintf(vrml_file,"    } #Viewpoint\n");
						fprintf(vrml_file,"    NavigationInfo\n");
						fprintf(vrml_file,"    {\n");
						fprintf(vrml_file,"      type [\"EXAMINE\",\"ANY\"]\n");
						fprintf(vrml_file,"    } #NavigationInfo\n");
						sphere_radius=1;
						for (i=0;i<number_of_positions;i++)
						{
							if (status_array[i]&&(min_time<max_time))
							{
								value=((float)(time_array[i]-min_time))/
									(float)(max_time-min_time);
								if (value<1.0/3.0)
								{
									red=1.0;
									blue=0.0;
									if (value<1.0/6.0)
									{
										green=value*4.5;
									}
									else
									{
										green=0.75+(value-1.0/6.0)*1.5;
									}
								}
								else
								{
									if (value<2.0/3.0)
									{
										red=(2.0/3.0-value)*3.0;
										green=1.0;
										blue=(value-1.0/3.0)*3.0;
									}
									else
									{
										red=0.0;
										blue=1.0;
										if (value<5.0/6.0)
										{
											green=1.0-(value-2.0/3.0)*1.5;
										}
										else
										{
											green=0.75-(value-5.0/6.0)*4.5;
										}
									}
								}
							}
							else
							{
								red=1;
								green=1;
								blue=1;
							}
							/*???debug */
							printf("%d.  %g %g %g\n",i+1,red,green,blue);
							fprintf(vrml_file,"    Transform\n");
							fprintf(vrml_file,"    {\n");
							fprintf(vrml_file,"      translation %f %f %f\n",x_array[i],
								y_array[i],z_array[i]);
							fprintf(vrml_file,"      scale 1.0 1.0 1.0\n");
							fprintf(vrml_file,"      children\n");
							fprintf(vrml_file,"      [\n");
							fprintf(vrml_file,"        Shape\n");
							fprintf(vrml_file,"        {\n");
							fprintf(vrml_file,"          appearance\n");
							fprintf(vrml_file,"          Appearance\n");
							fprintf(vrml_file,"          {\n");
							fprintf(vrml_file,"            material\n");
							fprintf(vrml_file,"            Material\n");
							fprintf(vrml_file,"            {\n");
							fprintf(vrml_file,"              diffuseColor %f %f %f\n",red,
								green,blue);
							fprintf(vrml_file,"              ambientIntensity 1.0\n");
							fprintf(vrml_file,"              emissiveColor 0.0 0.0 0.0\n");
							fprintf(vrml_file,"              specularColor 0.0 0.0 0.0\n");
							fprintf(vrml_file,"              transparency 0.0\n");
							fprintf(vrml_file,"              shininess 0.0\n");
							fprintf(vrml_file,"            } #Material\n");
							fprintf(vrml_file,"          } #Appearance\n");
							fprintf(vrml_file,"          geometry\n");
							fprintf(vrml_file,"          Sphere\n");
							fprintf(vrml_file,"          {\n");
							fprintf(vrml_file,"            radius %f\n",sphere_radius);
							fprintf(vrml_file,"          } #Sphere\n");
							fprintf(vrml_file,"        } #Shape\n");
							fprintf(vrml_file,"      ]\n");
							fprintf(vrml_file,"    } #Transform\n");
							fprintf(vrml_file,"    Transform\n");
							fprintf(vrml_file,"    {\n");
							fprintf(vrml_file,"      translation %f %f %f\n",
								x_array[i]+1.1*sphere_radius,y_array[i],z_array[i]);
							fprintf(vrml_file,"      children\n");
							fprintf(vrml_file,"      [\n");
							fprintf(vrml_file,"        Billboard\n");
							fprintf(vrml_file,"        {\n");
							fprintf(vrml_file,"          axisOfRotation 0 0 0\n");
							fprintf(vrml_file,"          children\n");
							fprintf(vrml_file,"          [\n");
							fprintf(vrml_file,"            Shape\n");
							fprintf(vrml_file,"            {\n");
							fprintf(vrml_file,"              appearance\n");
							fprintf(vrml_file,"              Appearance\n");
							fprintf(vrml_file,"              {\n");
							fprintf(vrml_file,"                material\n");
							fprintf(vrml_file,"                Material\n");
							fprintf(vrml_file,"                {\n");
							fprintf(vrml_file,"                  diffuseColor %f %f %f\n",
								red,green,blue);
							fprintf(vrml_file,"                  ambientIntensity 1.0\n");
							fprintf(vrml_file,
								"                  emissiveColor 0.0 0.0 0.0\n");
							fprintf(vrml_file,
								"                  specularColor 0.0 0.0 0.0\n");
							fprintf(vrml_file,"                  transparency 0.0\n");
							fprintf(vrml_file,"                  shininess 0.0\n");
							fprintf(vrml_file,"                } #Material\n");
							fprintf(vrml_file,"              } #Appearance\n");
							fprintf(vrml_file,"              geometry\n");
							fprintf(vrml_file,"              Text\n");
							fprintf(vrml_file,"              {\n");
							fprintf(vrml_file,"                string\n");
							fprintf(vrml_file,"                [\n");
							fprintf(vrml_file,"                  \"%d\"\n",i+1);
							fprintf(vrml_file,"                ]\n");
							fprintf(vrml_file,"                fontstyle\n");
							fprintf(vrml_file,"                Fontstyle\n");
							fprintf(vrml_file,"                {\n");
							fprintf(vrml_file,"                  size %f\n",2*sphere_radius);
							fprintf(vrml_file,"                } #Fontstyle\n");
							fprintf(vrml_file,"              } #Text\n");
							fprintf(vrml_file,"            } #Shape\n");
							fprintf(vrml_file,"          ]\n");
							fprintf(vrml_file,"        } #Billboard\n");
							fprintf(vrml_file,"      ]\n");
							fprintf(vrml_file,"    } #Transform\n");
						}
						return_code=1;
					}
					else
					{
						printf("Negative radius\n");
						return_code=0;
					}
					fprintf(vrml_file,"  ]\n");
					fprintf(vrml_file,"} #Group\n");
					/* set lights... */
					fclose(vrml_file);
				}
				else
				{
					printf("Could not open vrml file: %s\n",argv[2]);
					return_code=0;
				}
			}
			fclose(biosense_file);
		}
		else
		{
			printf("Could not open Biosense file: %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf("usage: biosense2vrml biosense_file vrml_file\n");
		printf("  biosense_file is the name of the Biosense file (provided)\n");
		printf("  vrml_file is the name of the vrml file (created)\n");
		return_code=0;
	}

	return (return_code);
} /* main */
