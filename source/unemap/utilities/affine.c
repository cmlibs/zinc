/*******************************************************************************
FILE : affine.c

LAST MODIFIED : 30 January 2000

DESCRIPTION :
Performs an affine transformation on each region of a signal file.  Does not use
X-windows.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 30 January 2000

DESCRIPTION :
==============================================================================*/
{
	char option,file_name[120];
	FILE *signal_file;
	float a,b,c;
	int i,j,return_code;
	Linear_transformation linear_trans;
	struct Device **signal_device;
	struct Position *position;
	struct Region *region;
	struct Region_list_item *region_list_item;
	struct Rig *signal_rig;

	return_code=0;
	/* read the signal file */
	printf("Signal file name ? ");
	scanf("%s",file_name);
	if ((signal_file=fopen(file_name,"rb"))&&read_signal_file(signal_file,
		&signal_rig))
	{
		fclose(signal_file);
		i=signal_rig->number_of_regions;
		region_list_item=signal_rig->region_list;
		while (region_list_item&&(i>0))
		{
			if ((region=region_list_item->region)&&(region->name)&&
				((SOCK==region->type)||(TORSO==region->type)))
			{
				printf("Affine transformation for %s ? (y/n) ",region->name);
				scanf("%s",file_name);
				option=file_name[0];
				if (('Y'==option)||('y'==option))
				{
					printf("translation\n");
					printf("translate_x ? ");
					scanf("%f",&(linear_trans.translate_x));
					printf("translate_y ? ");
					scanf("%f",&(linear_trans.translate_y));
					printf("translate_z ? ");
					scanf("%f",&(linear_trans.translate_z));
					printf("transformation (row-wise)\n");
					printf("txx ? ");
					scanf("%f",&(linear_trans.txx));
					printf("txy ? ");
					scanf("%f",&(linear_trans.txy));
					printf("txz ? ");
					scanf("%f",&(linear_trans.txz));
					printf("tyx ? ");
					scanf("%f",&(linear_trans.tyx));
					printf("tyy ? ");
					scanf("%f",&(linear_trans.tyy));
					printf("tyz ? ");
					scanf("%f",&(linear_trans.tyz));
					printf("tzx ? ");
					scanf("%f",&(linear_trans.tzx));
					printf("tzy ? ");
					scanf("%f",&(linear_trans.tzy));
					printf("tzz ? ");
					scanf("%f",&(linear_trans.tzz));
					j=signal_rig->number_of_devices;
					signal_device=signal_rig->devices;
					while (j>0)
					{
						if ((*signal_device)&&((*signal_device)->description)&&
							(region==(*signal_device)->description->region)&&
							(ELECTRODE==(*signal_device)->description->type))
						{
							position=
								&((*signal_device)->description->properties.electrode.position);
							linear_transformation(&linear_trans,position->x,position->y,
								position->z,&a,&b,&c);
							position->x=a;
							position->y=b;
							position->z=c;
						}
						signal_device++;
						j--;
					}
				}
			}
			region_list_item=region_list_item->next;
			i--;
		}
		printf("New signal file name ? ");
		scanf("%s",file_name);
		if ((signal_file=fopen(file_name,"wb"))&&write_signal_file(signal_file,
			signal_rig))
		{
			printf("New signal file created\n");
		}
		else
		{
			printf("ERROR.  Writing new signal file\n");
		}
	}
	else
	{
		printf("ERROR.  Could not read signal file\n");
	}

	return (return_code);
} /* main */
