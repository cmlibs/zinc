/*******************************************************************************
FILE : configuration.c

LAST MODIFIED : 4 September 1995

DESCRIPTION :
Functions for creating and modifying rig configurations.
==============================================================================*/
#include <stddef.h>
#include "myio.h"
#include <math.h>
#include <string.h>
#define CONFIGURATION
#include "configuration.h"
#include "rig.h"
#include "debug.h"
#include "mymemory.h"
#include "geometry.h"

struct Rig *create_standard_Rig(char *name,enum Rig_type rig_type,
	enum Monitoring_status monitoring,enum Experiment_status experiment,
	int number_of_rows,int number_of_columns,int number_of_regions,
	int number_of_auxiliary_inputs,float sock_focus)
/*******************************************************************************
LAST MODIFIED : 16 November 1992

DESCRIPTION :
This function is a specialized version of create_Rig (in rig.c).  It creates a
rig with <number_of_regions> regions with identical electrode layouts.  In each
region the electrodes are equally spaced in <number_of_rows> rows and
<number_of_columns> columns.  There are <number_of_auxiliary_inputs> auxiliary
inputs.
???Move to rig.c ?
==============================================================================*/
{
	struct Rig *rig;
	struct Device **device,**devices;
	struct Region *region;
	struct Region_list_item **region_item,*region_list;
	struct Device_description *description;
	struct Channel *channel;
	int column_number,device_number,number_of_devices,region_number,row_number;
	char *device_name,no_error,*region_name;
	float x,x_max,x_min,x_step,y,y_max,y_min,y_step;

	ENTER(create_standard_Rig);
	/* check the arguments */
	if ((number_of_rows>0)&&(number_of_columns>0)&&(number_of_regions>0)&&
		(number_of_auxiliary_inputs>=0)&&((rig_type!=SOCK)||(sock_focus>0)))
	{
		/* create the list of devices */
		number_of_devices=number_of_regions*number_of_rows*number_of_columns+
			number_of_auxiliary_inputs;
		if ((MYMALLOC(devices,struct Device *,number_of_devices))&&
			(MYMALLOC(device_name,char,2+(int)log10((double)number_of_devices)))&&
			(MYMALLOC(region_name,char,9+(int)log10((double)number_of_regions))))
		{
			/* create the regions */
			strcpy(region_name,"region ");
			device=devices;
			device_number=0;
			region_number=0;
			region_list=(struct Region_list_item *)NULL;
			region_item= &region_list;
			no_error=1;
			switch (rig_type)
			{
				case PATCH:
				{
					x_min=0;
					x_max=(float)(number_of_columns-1);
					x_step=1;
					y_min=0;
					y_max=(float)(number_of_rows-1);
					y_step=1;
				} break;
				case SOCK:
				{
					x_min=0;
					x_max=8*atan(1);
					x_step=x_max/(float)number_of_columns;
					x_max -= x_step/2;
					y_min=0;
					y_max=2.15;
						/*???general ?*/
					y_step=y_max/(float)number_of_rows;
				} break;
			}
			while ((region_number<number_of_regions)&&no_error)
			{
				/* create the region */
				sprintf(region_name+7,"%d",region_number+1);
				if ((region=create_Region(region_name,region_number,number_of_rows*
					number_of_columns))&&(*region_item=create_Region_list_item(region,
					(struct Region_list_item *)NULL)))
				{
					region_item= &((*region_item)->next);
					/* create the electrodes for the region */
					y=y_max;
					row_number=0;
					while ((row_number<number_of_rows)&&no_error)
					{
						x=x_max;
						column_number=0;
						while ((column_number<number_of_columns)&&no_error)
						{
							sprintf(device_name,"%d",device_number+1);
							if ((description=create_Device_description(device_name,ELECTRODE,
								region))&&(channel=create_Channel(device_number+1,0,0))&&
								(*device=create_Device(device_number,description,channel,
								(struct Signal *)NULL)))
							{
								/* assign position */
								switch (rig_type)
								{
									case PATCH:
									{
										description->properties.electrode.position.patch.x=x;
										description->properties.electrode.position.patch.y=y;
									} break;
									case SOCK:
									{
										prolate_spheroidal_to_cartesian(1,y,x,sock_focus,
											&(description->properties.electrode.position.sock.x),
											&(description->properties.electrode.position.sock.y),
											&(description->properties.electrode.position.sock.z));
									} break;
								}
								device_number++;
								device++;
							}
							else
							{
								print_message(1,
									"create_standard_Rig.  Could not allocate memory for device");
								no_error=0;
								destroy_Channel(&channel);
								destroy_Device_description(&description);
							}
							column_number++;
							x -= x_step;
						}
						row_number++;
						y -= y_step;
					}
					region_number++;
				}
				else
				{
					print_message(1,
						"create_standard_Rig.  Could not allocate memory for region");
					destroy_Region(&region);
					no_error=0;
				}
			}
			if (no_error)
			{
				/* create the auxiliary devices */
				while (device_number<number_of_devices)
				{
					sprintf(device_name,"%d",device_number+1);
					if ((description=create_Device_description(device_name,AUXILIARY,
						region))&&(channel=create_Channel(device_number+1,0,0))&&
						(*device=create_Device(device_number,description,channel,
						(struct Signal *)NULL)))
					{
						device_number++;
						device++;
					}
					else
					{
						print_message(1,
							"create_standard_Rig.  Could not allocate memory for device");
						no_error=0;
						destroy_Channel(&channel);
						destroy_Device_description(&description);
					}
				}
				if (no_error)
				{
					/* create the rig */
					rig=create_Rig(name,rig_type,monitoring,experiment,
						number_of_devices,devices,(struct Page_list_item *)NULL,
						number_of_regions,region_list,(struct Region *)NULL);
					switch (rig_type)
					{
						case SOCK:
						{
							rig->properties.sock.focus=sock_focus;
						} break;
					}
				}
				else
				{
					rig=(struct Rig *)NULL;
				}
			}
			if (!rig)
			{
				/* free devices */
				while (device_number>0)
				{
					device--;
					destroy_Device(device);
					device_number--;
				}
				MYFREE(devices);
				/* free regions */
				while (region_number>0)
				{
					destroy_Region(&(region_list->region));
					region_item= &(region_list);
					region_list=region_list->next;
					destroy_Region_list_item(region_item);
					region_number--;
				}
			}
			MYFREE(device_name);
			MYFREE(region_name);
		}
		else
		{
			print_message(1,
				"create_standard_Rig.  Could not allocate memory for device list");
			MYFREE(device_name);
			MYFREE(devices);
			rig=(struct Rig *)NULL;
		}
	}
	else
	{
		print_message(1,"create_standard_Rig.  Invalid arguments");
		rig=(struct Rig *)NULL;
	}
	LEAVE;

	return (rig);
} /* create_Rig */

/*??? temp */
main()
{
	struct Rig *rig;

	rig=
		create_standard_Rig("sock",SOCK,MONITORING_OFF,EXPERIMENT_OFF,6,19,1,0,30);
	write_configuration_file("new.cnfg",(void *)&rig);
}
