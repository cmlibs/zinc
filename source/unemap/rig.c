/*******************************************************************************
FILE : rig.c

LAST MODIFIED : 11 June 2002

DESCRIPTION :
Contains function definitions for measurement rigs.
???DB.  What about writing devices with multiple signals ?
==============================================================================*/
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
/*ieeefp.h doesn't exist for Linux. Needed for finite() for Irix*/
/*finite() in math.h in Linux */
#if defined (NOT_ANSI)
#include <ieeefp.h>
#endif /* defined (NOT_ANSI) */
#if defined (WIN32)
#include <float.h>
#endif /* defined (WIN32) */
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "unemap/interpolate.h"
#include "unemap/rig.h"
#include "unemap/unemap_package.h"
#include "user_interface/message.h"

/*
Module functions
----------------
*/
static int string_to_region_type(char *string,enum Region_type rig_type,
	enum Region_type *region_type)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(string_to_region_type);
	/* check arguments */
	if (string&&region_type)
	{
		if (MIXED==rig_type)
		{
			if ((('s'==string[0])||('S'==string[0]))&&
				(('o'==string[1])||('O'==string[1]))&&
				(('c'==string[2])||('C'==string[2]))&&
				(('k'==string[3])||('K'==string[3])))
			{
				*region_type=SOCK;
				return_code=1;
			}
			else
			{
				if ((('p'==string[0])||('P'==string[0]))&&
					(('a'==string[1])||('A'==string[1]))&&
					(('t'==string[2])||('T'==string[2]))&&
					(('c'==string[3])||('C'==string[3]))&&
					(('h'==string[4])||('H'==string[4])))
				{
					*region_type=PATCH;
					return_code=1;
				}
				else
				{
					if ((('t'==string[0])||('T'==string[0]))&&
						(('o'==string[1])||('O'==string[1]))&&
						(('r'==string[2])||('R'==string[2]))&&
						(('s'==string[3])||('S'==string[3]))&&
						(('o'==string[4])||('O'==string[4])))
					{
						*region_type=TORSO;
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
		}
		else
		{
			if ((('r'==string[0])||('R'==string[0]))&&
				(('e'==string[1])||('E'==string[1]))&&
				(('g'==string[2])||('G'==string[2]))&&
				(('i'==string[3])||('I'==string[3]))&&
				(('o'==string[4])||('O'==string[4]))&&
				(('n'==string[5])||('N'==string[5])))
			{
				*region_type=rig_type;
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"string_to_region_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* string_to_region_type */

/*
Global functions
----------------
*/
struct Device_description *create_Device_description(
	char *name,enum Device_type type,struct Region *region)
/*******************************************************************************
LAST MODIFIED : 27 July 1999

DESCRIPTION :
This function allocates memory for a device description and initializes the
fields to the specified values.  It does not initialize the fields which depend
on the <type>.  It returns a pointer to the created device description if
successful and NULL if unsuccessful.
==============================================================================*/
{
	struct Device_description *description;

	ENTER(create_Device_description);
	if (ALLOCATE(description,struct Device_description,1))
	{
		if (name)
		{
			if (ALLOCATE(description->name,char,strlen(name)+1))
			{
				strcpy(description->name,name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Device_description.  Could not allocate memory for name");
			}
		}
		else
		{
			description->name=(char *)NULL;
		}
		description->type=type;
		description->region=region;
		switch (type)
		{
			case AUXILIARY:
			{
				(description->properties).auxiliary.number_of_electrodes=0;
				(description->properties).auxiliary.electrodes=(struct Device **)NULL;
				(description->properties).auxiliary.electrode_coefficients=
					(float *)NULL;
			} break;
			case ELECTRODE:
			{
				(description->properties).electrode.position.x=(float)0;
				(description->properties).electrode.position.y=(float)0;
				(description->properties).electrode.position.z=(float)0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Device_description,  Could not allocate memory for description");
	}
	LEAVE;

	return (description);
} /* create_Device_description */

int destroy_Device_description(struct Device_description **description)
/*******************************************************************************
LAST MODIFIED : 28 July 1999

DESCRIPTION :
This function frees the memory associated with the fields of <**description>,
frees the memory for <**description> and changes <*description> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Device_description);
	return_code=1;
	if (*description)
	{
		switch ((*description)->type)
		{
			case AUXILIARY:
			{
				DEALLOCATE(((*description)->properties).auxiliary.electrodes);
				DEALLOCATE(((*description)->properties).auxiliary.
					electrode_coefficients);
			} break;
		}
		DEALLOCATE((*description)->name);
		DEALLOCATE(*description);
		*description=(struct Device_description *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Device_description */

char *get_Device_description_name(struct Device_description *description)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Returns the name used by the Device_description <description>.
==============================================================================*/
{
	char *name;

	ENTER(get_Device_description_name);
	name=(char *)NULL;
	if(description)
	{
		name=description->name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Device_description_name. Invalid argument");
	}
	LEAVE;
	return(name);
}/* get_Device_description_name*/

struct Channel *create_Channel(int number,float offset,float gain)
/*******************************************************************************
LAST MODIFIED : 29 July 2000

DESCRIPTION :
This function allocates memory for a channel and initializes the fields to the
specified values.  It returns a pointer to the created channel if successful and
NULL if unsuccessful.
==============================================================================*/
{
	struct Channel *channel;

	ENTER(create_Channel);
	if (ALLOCATE(channel,struct Channel,1))
	{
		channel->number=number;
		channel->offset=offset;
		channel->gain=gain;
		channel->gain_correction=(float)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Channel.  Could not allocate memory for channel");
	}
	LEAVE;

	return (channel);
} /* create_Channel */

int destroy_Channel(struct Channel **channel)
/*******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**channel>, frees
the memory for <**channel> and changes <*channel> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Channel);
	return_code=1;
	if (*channel)
	{
		DEALLOCATE(*channel);
		*channel=(struct Channel *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Channel */

struct Device *create_Device(int number,struct Device_description *description,
	struct Channel *channel,struct Signal *signal)
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
This function allocates memory for a device and initializes the fields to
specified values.  It returns a pointer to the created measurement device if
successful and NULL if unsuccessful.
==============================================================================*/
{
	struct Device *device;

	ENTER(create_Device);
	if (ALLOCATE(device,struct Device,1))
	{
		device->number=number;
		device->description=description;
		device->channel=channel;
		device->signal=signal;
		/* the important thing is that the maximum is less than the minimum */
		device->signal_display_maximum=(float)0;
		device->signal_display_minimum=(float)1;
		device->highlight=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Device.  Could not allocate memory for device");
	}
	LEAVE;

	return (device);
} /* create_Device */

int destroy_Device(struct Device **device)
/*******************************************************************************
LAST MODIFIED : 3 May 1996

DESCRIPTION :
This function frees the memory associated with the fields of <**device>, frees
the memory for <**device> and changes <*device> to NULL.
==============================================================================*/
{
	int return_code;
	struct Signal *signal,*signal_next;

	ENTER(destroy_Device);
	return_code=1;
	if (*device)
	{
		destroy_Device_description(&((*device)->description));
		destroy_Channel(&((*device)->channel));
		signal=(*device)->signal;
		while (signal)
		{
			signal_next=signal->next;
			destroy_Signal(&signal);
			signal=signal_next;
		}
		DEALLOCATE(*device);
		*device=(struct Device *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Device */

struct Signal *get_Device_signal(struct Device *device)
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Returns the signal used by the <device>.
==============================================================================*/
{
	struct Signal *signal;

	ENTER(get_Device_signal);
	signal=(struct Signal *)NULL;
	if(device)
	{
		signal=device->signal;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Device_signal. Invalid argument");
	}
	LEAVE;
	return(signal);
}/* get_Device_signal */

struct Device_description *get_Device_description(struct Device *device)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Returns the Device_description used by the <device>.
==============================================================================*/
{
	
	struct Device_description *description;

	ENTER(get_Device_description);
	description=(struct Device_description  *)NULL;
	if(device)
	{
		description=device->description;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Device_description. Invalid argument");
	}
	LEAVE;
	return(description);
}/* get_Device_description */

struct Signal_buffer *get_Device_signal_buffer(struct Device *device)
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Returns the signal buffer used by the <device>.
==============================================================================*/
{
	struct Device **electrodes;
	struct Signal_buffer *buffer;

	ENTER(get_Device_signal_buffer);
	buffer=(struct Signal_buffer *)NULL;
	if (device&&(device->description))
	{
		if ((AUXILIARY==device->description->type)&&(0<(device->description->
			properties).auxiliary.number_of_electrodes))
		{
			if ((electrodes=(device->description->properties).auxiliary.electrodes)&&
				(electrodes[0])&&(electrodes[0]->signal))
			{
				buffer=electrodes[0]->signal->buffer;
			}
		}
		else
		{
			if (device->signal)
			{
				buffer=device->signal->buffer;
			}
		}
	}
	LEAVE;

	return (buffer);
} /* get_Device_signal_buffer */

struct Device_list_item *create_Device_list_item(struct Device *device,
	struct Device_list_item *previous,struct Device_list_item *next)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
This function allocates memory for a device list item and initializes all the
fields to specified values.  It returns a pointer to the created device list
item if successful and NULL if unsuccessful.
==============================================================================*/
{
	struct Device_list_item *list_item;

	ENTER(create_Device_list_item);
	if (ALLOCATE(list_item,struct Device_list_item,1))
	{
		list_item->device=device;
		list_item->previous=previous;
		list_item->next=next;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Device_list_item.  Could not allocate memory for list item");
	}
	LEAVE;

	return (list_item);
} /* create_Device_list_item */

int destroy_Device_list(struct Device_list_item **list,int destroy_devices)
/*******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
This function recursively frees the memory for the items in the <list>.  If
<destroy_devices> is non-zero then the devices in the list are destroyed.
<**list> is set to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Device_list);
	return_code=1;
	if (*list)
	{
		if (destroy_devices)
		{
			destroy_Device(&((*list)->device));
		}
		destroy_Device_list(&((*list)->next),destroy_devices);
		DEALLOCATE(*list);
		*list=(struct Device_list_item *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Device_list */

int number_in_Device_list(struct Device_list_item *list)
/*******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
Counts the number of items in a device list.
==============================================================================*/
{
	int count;
	struct Device_list_item *list_item;

	ENTER(number_in_Device_list);
	count=0;
	list_item=list;
	while (list_item)
	{
		count++;
		list_item=list_item->next;
	}
	LEAVE;

	return (count);
} /* number_in_Device_list */

int sort_devices_by_event_time(void *first,void *second)
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
Returns whether the <first> device has an earlier (< 0), the same (0) or a
later (> 0) first event time than the <second> device.
==============================================================================*/
{
	int return_code;
	struct Event *first_event,*second_event;

	ENTER(sort_devices_by_event_time);
	if (second_event=(*((struct Device **)second))->signal->first_event)
	{
		if (first_event=(*((struct Device **)first))->signal->first_event)
		{
			if (first_event->time<second_event->time)
			{
				return_code= -1;
			}
			else
			{
				if (first_event->time>second_event->time)
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		if (first_event=(*((struct Device **)first))->signal->first_event)
		{
			return_code= -1;
		}
		else
		{
			return_code=sort_devices_by_number(first,second);
		}
	}
	LEAVE;

	return (return_code);
} /* sort_devices_by_event_time */

int sort_devices_by_number(void *first,void *second)
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
Returns whether the <first> device has a smaller (< 0), the same (0) or a
larger (> 0) number than the <second> device.
==============================================================================*/
{
	int first_number,return_code,second_number;

	ENTER(sort_devices_by_number);
	first_number=(*((struct Device **)first))->number;
	second_number=(*((struct Device **)second))->number;
	if (first_number<second_number)
	{
		return_code= -1;
	}
	else
	{
		if (first_number>second_number)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* sort_devices_by_number */

struct Page *create_Page(char *name,struct Device_list_item *device_list)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
This function allocates memory for a page and initializes all the fields to the
specified values.  It returns a pointer to the created page if successful and
NULL if unsuccessful.
==============================================================================*/
{
	struct Page *page;

	ENTER(create_Page);
	if (ALLOCATE(page,struct Page,1))
	{
		if (name)
		{
			if (ALLOCATE(page->name,char,strlen(name)+1))
			{
				strcpy(page->name,name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Page.  Could not allocate memory for name");
			}
		}
		else
		{
			page->name=(char *)NULL;
		}
		page->device_list=device_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Page.  Could not allocate memeory for page");
	}
	LEAVE;

	return (page);
} /* create_Page */

int destroy_Page(struct Page **page)
/*******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**page>, frees the
memory for <**page> and changes <*page> to NULL.  It does not destroy the
devices in the device list.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Page);
	return_code=1;
	if (*page)
	{
		DEALLOCATE((*page)->name);
		/* destroy the device list without destroying the devices */
		destroy_Device_list(&((*page)->device_list),0);
		DEALLOCATE(*page);
		*page=(struct Page *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Page */

struct Page_list_item *create_Page_list_item(struct Page *page,
	struct Page_list_item *next)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
This function allocates memory for a page list item and initializes all the
fields to specified values.  It returns a pointer to the created page list item
if successful and NULL if unsuccessful.
==============================================================================*/
{
	struct Page_list_item *list_item;

	ENTER(create_Page_list_item);
	if (ALLOCATE(list_item,struct Page_list_item,1))
	{
		list_item->page=page;
		list_item->next=next;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Page_list_item.  Could not allocate memory for list item");
	}
	LEAVE;

	return (list_item);
} /* create_Page_list_item */

int destroy_Page_list(struct Page_list_item **list)
/*******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
This function recursively frees the memory for the items in the <list>.  It
destroys the pages in the list.  <**list> is set to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Page_list);
	return_code=1;
	if (*list)
	{
		destroy_Page(&((*list)->page));
		destroy_Page_list(&((*list)->next));
		DEALLOCATE(*list);
		*list=(struct Page_list_item *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Page_list */

int number_in_Page_list(struct Page_list_item *list)
/*******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
Counts the number of items in a page list.
==============================================================================*/
{
	int count;
	struct Page_list_item *list_item;

	ENTER(number_in_Page_list);
	count=0;
	list_item=list;
	while (list_item)
	{
		count++;
		list_item=list_item->next;
	}
	LEAVE;

	return (count);
} /* number_in_Page_list */

struct Region *create_Region(char *name,enum Region_type type,int number,
	int number_of_devices
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D)*/														 
)
/*******************************************************************************
LAST MODIFIED : 19 January 2001

DESCRIPTION :
This function allocates memory for a region and initializes all the fields to
the specified values.  It returns a pointer to the created region if successful
and NULL if unsuccessful.
==============================================================================*/
{
	struct Region *region;

	ENTER(create_Region);
	if (ALLOCATE(region,struct Region,1))
	{
		if (name)
		{
			if (ALLOCATE(region->name,char,strlen(name)+1))
			{
				strcpy(region->name,name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Region.  Could not allocate memory for name");
			}
		}
		else
		{
			region->name=(char *)NULL;
		}
		region->type=type;
		switch (type)
		{
			case SOCK:
			{
				(region->properties).sock.focus=(float)0;
				(region->properties).sock.linear_transformation=
					(Linear_transformation *)NULL;
			} break;
		}
		region->number=number;
		region->number_of_devices=number_of_devices;
#if defined (UNEMAP_USE_3D)
    if (unemap_package)
    {
      region->unemap_package=ACCESS(Unemap_package)(unemap_package);
    }
    else
    {
      region->unemap_package = (struct Unemap_package *)NULL;
    }
		region->rig_node_group=(struct GROUP(FE_node) *)NULL;
		region->unrejected_node_group=(struct GROUP(FE_node) *)NULL;
		region->map_3d_package=(struct Map_3d_package *)NULL;
		region->electrode_position_field=(struct FE_field *)NULL;
		region->map_electrode_position_field=(struct FE_field *)NULL;
#endif /* defined (UNEMAP_USE_3D) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Region.  Could not allocate memeory for region");
	}
	LEAVE;

	return (region);
} /* create_Region */

int destroy_Region(struct Region **region)
/*******************************************************************************
LAST MODIFIED : 17 July 2000 

DESCRIPTION :
This function frees the memory associated with the fields of <**region>, frees
the memory for <**region> and changes <*region> to NULL.  It does not destroy
the devices in the device list.
==============================================================================*/
{
	int return_code;
#if defined (UNEMAP_USE_3D)
	struct FE_field *temp_field=(struct FE_field *)NULL;
	struct MANAGER(Computed_field) *computed_field_manager=
		(struct MANAGER(Computed_field) *)NULL;	
	struct MANAGER(FE_field) *fe_field_manager=
		(struct MANAGER(FE_field) *)NULL;	
	struct Unemap_package *unemap_package=(struct Unemap_package *)NULL;
	struct Region *the_region=(struct Region *)NULL;		
#endif /* defined (UNEMAP_USE_3D)*/
	ENTER(destroy_Region);
	return_code=1;
	if (*region)
	{
#if defined (UNEMAP_USE_3D)
		the_region=*region;
		unemap_package=the_region->unemap_package;
		if(the_region&&unemap_package)
		{			
			DEACCESS(Map_3d_package)(&((*region)->map_3d_package)); 
			if(the_region->rig_node_group)
			{
				free_unemap_package_rig_node_group(unemap_package,&(the_region->rig_node_group));
			}
			/* following will deaccess the rig_node_group */	
			if(the_region->unrejected_node_group)
			{
				free_unemap_package_rig_node_group(unemap_package,
					&(the_region->unrejected_node_group));
			}			
			computed_field_manager=get_unemap_package_Computed_field_manager(unemap_package);
			fe_field_manager=get_unemap_package_FE_field_manager(unemap_package);
			if(the_region->electrode_position_field)
			{
				temp_field=the_region->electrode_position_field;
				DEACCESS(FE_field)(&temp_field);
				destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
					the_region->electrode_position_field);
				the_region->electrode_position_field=(struct FE_field *)NULL;
			}	
			if(the_region->map_electrode_position_field)
			{
				temp_field=the_region->map_electrode_position_field;
				DEACCESS(FE_field)(&temp_field);
				destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
					the_region->map_electrode_position_field);
				the_region->map_electrode_position_field=(struct FE_field *)NULL;
			}			
		}	
		DEACCESS(FE_field)(&((*region)->electrode_position_field));
		DEACCESS(FE_field)(&((*region)->map_electrode_position_field));
		DEACCESS(Unemap_package)(&((*region)->unemap_package));	
#endif /* defined (UNEMAP_USE_3D) */
		DEALLOCATE((*region)->name);
		DEALLOCATE(*region);
		*region=(struct Region *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Region */

#if defined (UNEMAP_USE_3D)
struct Unemap_package *get_Region_unemap_package(struct Region *region)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  unemap_package of <region> 
==============================================================================*/
{
	struct Unemap_package *unemap_package=(struct Unemap_package *)NULL;

	ENTER(get_Region_unemap_package);
	if(region)
	{
		unemap_package=region->unemap_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Region_unemap_package."
			" invalid arguments");	
	}
	LEAVE;
	return (unemap_package);
}/* get_Region_unemap_package*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct GROUP(FE_node) *get_Region_rig_node_group(struct Region *region)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  rig_node_group of <region> 
==============================================================================*/
{
	struct GROUP(FE_node) *rig_node_group=(struct GROUP(FE_node) *)NULL;

	ENTER(get_Region_rig_node_group);
	if(region)
	{
		rig_node_group=region->rig_node_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Region_rig_node_group."
			" invalid arguments");	
	}
	LEAVE;
	return (rig_node_group);
}/* get_Region_rig_node_group*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_rig_node_group(struct Region *region,
	struct GROUP(FE_node) *rig_node_group)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets (and accesses) rig_node_group of <region> to <rig_node_group>
==============================================================================*/
{
	int return_code;
	ENTER(set_Region_rig_node_group);
	if(region&&rig_node_group)
	{
		return_code =1;	
		REACCESS(GROUP(FE_node))(&(region->rig_node_group),rig_node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Region_rig_node_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Region_rig_node_group*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct GROUP(FE_node) *get_Region_unrejected_node_group(struct Region *region)
/*******************************************************************************
LAST MODIFIED : 19 January 2001

DESCRIPTION :
Gets  unrejected_node_group of <region> 
==============================================================================*/
{
	struct GROUP(FE_node) *unrejected_node_group=(struct GROUP(FE_node) *)NULL;

	ENTER(get_Region_unrejected_node_group);
	if(region)
	{
		unrejected_node_group=region->unrejected_node_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Region_unrejected_node_group."
			" invalid arguments");	
	}
	LEAVE;
	return (unrejected_node_group);
}/* get_Region_unrejected_node_group*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_unrejected_node_group(struct Region *region,
	struct GROUP(FE_node) *unrejected_node_group)
/*******************************************************************************
LAST MODIFIED : 19 January 2001

DESCRIPTION :
Sets (and accesses) unrejected_node_group of <region> to <unrejected_node_group>
==============================================================================*/
{
	int return_code;
	ENTER(set_Region_unrejected_node_group);
	if(region&&unrejected_node_group)
	{
		return_code =1;	
		REACCESS(GROUP(FE_node))(&(region->unrejected_node_group),unrejected_node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Region_unrejected_node_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Region_unrejected_node_group*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Map_3d_package *get_Region_map_3d_package(struct Region *region)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  Map_3d_package  of <region> 
==============================================================================*/
{
	struct Map_3d_package *map_3d_package=(struct Map_3d_package *)NULL;

	ENTER(get_Region_map_3d_package);
	if(region)
	{
		map_3d_package=region->map_3d_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Region_map_3d_package."
			" invalid arguments");
	}
	LEAVE;
	return (map_3d_package);
}/* get_Region_map_3d_package*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_map_3d_package(struct Region *region,struct Map_3d_package *map_3d_package)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets (and accesses) set_Region_map_3d_package of <region> to <map_3d_package>
==============================================================================*/
{
	int return_code;
	ENTER(set_Region_map_3d_package);
	/* map_3d_package can be NULL*/
	if(region)
	{
		return_code =1;	
		REACCESS(Map_3d_package)(&(region->map_3d_package),map_3d_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Region_map_3d_package."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Region_map_3d_package*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_Region_electrode_position_field(struct Region *region)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Gets  electrode_position_field  of <region> 
==============================================================================*/
{
	struct FE_field *electrode_position_field=(struct FE_field *)NULL;

	ENTER(get_Region_electrode_position_field);
	if(region)
	{
		electrode_position_field=region->electrode_position_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Region_electrode_position_field."
			" invalid arguments");
	}
	LEAVE;
	return (electrode_position_field);
}/* get_Region_electrode_position_field*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_electrode_position_field(struct Region *region,
	struct FE_field *electrode_position_field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Sets (and accesses) electrode_position_field of <region> to <electrode_position_field>
==============================================================================*/
{
	int return_code;
	ENTER(set_Region_electrode_position_field);
	if(region&&electrode_position_field)
	{
		return_code =1;	
		REACCESS(FE_field)(&(region->electrode_position_field),electrode_position_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Region_electrode_position_field."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Region_electrode_position_field*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_Region_map_electrode_position_field(struct Region *region)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Gets  map_electrode_position_field  of <region> 
==============================================================================*/
{
	struct FE_field *map_electrode_position_field=(struct FE_field *)NULL;

	ENTER(get_Region_map_electrode_position_field);
	if(region)
	{
		map_electrode_position_field=region->map_electrode_position_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Region_map_electrode_position_field."
			" invalid arguments");
	}
	LEAVE;
	return (map_electrode_position_field);
}/* set_Region_map_electrode_position_field*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_map_electrode_position_field(struct Region *region,
	struct FE_field *map_electrode_position_field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Sets (and accesses) map_electrode_position_field of <region> to 
<map_electrode_position_field>
==============================================================================*/
{
	int return_code;
	ENTER(set_Region_map_electrode_position_field);
	if(region&&map_electrode_position_field)
	{
		return_code =1;	
		REACCESS(FE_field)(&(region->map_electrode_position_field),map_electrode_position_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Region_map_electrode_position_field."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Region_map_electrode_position_field*/
#endif /* defined (UNEMAP_USE_3D)*/

struct Region_list_item *create_Region_list_item(struct Region *region,
	struct Region_list_item *next)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
This function allocates memory for a region list item and initializes all the
fields to specified values.  It returns a pointer to the created region list
item if successful and NULL if unsuccessful.
==============================================================================*/
{
	struct Region_list_item *list_item;

	ENTER(create_Region_list_item);
	if (ALLOCATE(list_item,struct Region_list_item,1))
	{
		list_item->region=region;
		list_item->next=next;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Region_list_item.  Could not allocate memory for list item");
	}
	LEAVE;

	return (list_item);
} /* create_Region_list_item */

int destroy_Region_list(struct Region_list_item **list)
/*******************************************************************************
LAST MODIFIED : 2 August 1992

DESCRIPTION :
This function recursively frees the memory for the items in the <list>.  It
destroys the regions in the list.  <**list> is set to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Region_list);
	return_code=1;
	if (*list)
	{
		destroy_Region(&((*list)->region));
		destroy_Region_list(&((*list)->next));
		DEALLOCATE(*list);
		*list=(struct Region_list_item *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Region_list */

struct Region *get_Region_list_item_region(
	struct Region_list_item *region_list_item)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  Region  of <region_list_item> 
==============================================================================*/
{
	struct Region *region=(struct Region *)NULL;

	ENTER(get_Region_list_item_region);
	if(region_list_item)
	{
		region=region_list_item->region;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Region_list_item_region."
			" invalid arguments");
	}
	LEAVE;
	return (region);
}/* set_Region_list_item_region*/

int set_Region_list_item_region(struct Region_list_item *region_list_item,
	struct Region *region)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets region of <region_list_item> 
==============================================================================*/
{
	int return_code;
	ENTER(set_Region_list_item_region);
	if(region)
	{
		return_code =1;	
		region_list_item->region=region;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Region_list_item_region."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Region_list_item_region*/

struct Region_list_item *get_Region_list_item_next(
	struct Region_list_item *region_list_item)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets next  of <region_list_item> 
==============================================================================*/
{
	struct Region_list_item *next=(struct Region_list_item *)NULL;

	ENTER(get_Region_list_item_next);
	if(region_list_item)
	{
		next=region_list_item->next;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Region_list_item_next."
			" invalid arguments");
	}
	LEAVE;
	return (next);
}/* set_Region_list_item_next*/

int set_Region_list_item_next(struct Region_list_item *region_list_item,
	struct Region_list_item *next)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets next of <region_list_item> to 
==============================================================================*/
{
	int return_code;
	ENTER(set_Region_list_item_next);
	if(region_list_item)
	{
		return_code =1;	
		region_list_item->next=next;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Region_list_item_next."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Region_list_item_next*/

struct Rig *create_Rig(char *name,enum Monitoring_status monitoring,
	enum Experiment_status experiment,int number_of_devices,
	struct Device **devices,struct Page_list_item *page_list,
	int number_of_regions,struct Region_list_item *region_list,
	struct Region *current_region
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
											 )
/*******************************************************************************
LAST MODIFIED : 26 June 2000 

DESCRIPTION :
This function allocates memory for a rig and initializes the fields to
specified values.  It returns a pointer to the created rig if successful and
NULL if unsuccessful.
==============================================================================*/
{
	struct Rig *rig;

	ENTER(create_Rig);
	if (ALLOCATE(rig,struct Rig,1))
	{
		if (name)
		{
			if (ALLOCATE(rig->name,char,strlen(name)+1))
			{
				strcpy(rig->name,name);
			}
		}
		else
		{
			rig->name=(char *)NULL;
			assign_empty_string(&(rig->name));
		}
		if (rig->name)
		{
			rig->experiment=experiment;
			rig->monitoring=monitoring;
			rig->number_of_devices=number_of_devices;
			rig->devices=devices;
			rig->page_list=page_list;
			rig->number_of_regions=number_of_regions;
			rig->region_list=region_list;
			rig->current_region=current_region;
#if defined (UNEMAP_USE_3D)
      if (unemap_package)
      {
        rig->unemap_package=ACCESS(Unemap_package)(unemap_package);
      }
      else
      {
        rig->unemap_package = (struct Unemap_package *)NULL;
      }
			rig->all_devices_rig_node_group=(struct GROUP(FE_node) *)NULL;				
#endif /* defined (UNEMAP_USE_3D) */
			rig->signal_file_name=(char *)NULL;
#if defined (OLD_CODE)
???DB.  Only read calibration when doing acquisition */
			rig->calibration_directory=(char *)NULL;
				/*???DB.  Used to be in user_settings.  How to assign ? */
#endif /* defined (OLD_CODE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Rig.  Could not allocate memory for name");
			DEALLOCATE(rig);
			rig=(struct Rig *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Rig.  Could not allocate memory for rig");
	}
	LEAVE;

	return (rig);
} /* create_Rig */

struct Rig *create_standard_Rig(char *name,enum Region_type region_type,
	enum Monitoring_status monitoring,enum Experiment_status experiment,
	int number_of_rows,int *electrodes_in_row,
	int number_of_regions_and_device_numbering,
	int number_of_auxiliary_inputs,float sock_focus
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
	)
/*******************************************************************************
LAST MODIFIED : 10 February 2002

DESCRIPTION :
This function is a specialized version of create_Rig (in rig.c).  It creates a
rig with abs(<number_of_regions_and_device_numbering>) regions with identical
electrode layouts.  In each region the electrodes are equally spaced in
<number_of_rows> rows and <electrodes_in_row> columns.  If
<number_of_regions_and_device_numbering> is positive then the device/channel
number offset between electrodes in a region is
<number_of_regions_and_device_numbering>, otherwise the offset is 1.  There are
<number_of_auxiliary_inputs> auxiliary inputs.  The auxiliaries are single
channel inputs (rather than linear combinations of electrodes).
==============================================================================*/
{
	struct Rig *rig;
	struct Device **device,**devices;
	struct Region *region;
	struct Region_list_item **region_item,*region_list;
	struct Device_description *description;
	struct Channel *channel;
	int column_number,device_number,device_number_offset,electrodes_in_region,
		electrodes_in_row_max,i,number_in_row,number_of_devices,number_of_regions,
		region_number,row_number;
	char *device_name,no_error,*region_name;
	float x,x_max,x_min,x_step,y,y_max,y_step;

	ENTER(create_standard_Rig);
	/* check the arguments */
	if ((number_of_rows>0)&&electrodes_in_row&&
		(0!=number_of_regions_and_device_numbering)&&
		(number_of_auxiliary_inputs>=0)&&((region_type!=SOCK)||(sock_focus>0)))
	{
		if (0<number_of_regions_and_device_numbering)
		{
			number_of_regions=number_of_regions_and_device_numbering;
			device_number_offset=number_of_regions;
		}
		else
		{
			number_of_regions= -number_of_regions_and_device_numbering;
			device_number_offset=1;
		}
		/* create the list of devices */
		electrodes_in_region=0;
		electrodes_in_row_max=0;
		for (i=0;i<number_of_rows;i++)
		{
			if ((number_in_row=electrodes_in_row[i])>0)
			{
				if (number_in_row>electrodes_in_row_max)
				{
					electrodes_in_row_max=number_in_row;
				}
				electrodes_in_region += number_in_row;
			}
		}
		if (electrodes_in_region>0)
		{
			number_of_devices=
				number_of_regions*electrodes_in_region+number_of_auxiliary_inputs;
			if ((ALLOCATE(devices,struct Device *,number_of_devices))&&
				(ALLOCATE(device_name,char,2+(int)log10((double)number_of_devices)))&&
				(ALLOCATE(region_name,char,9+(int)log10((double)number_of_regions))))
			{
				/* create the regions */
				strcpy(region_name,"region ");
				device=devices;
				device_number=0;
				region_number=0;
				region_list=(struct Region_list_item *)NULL;
				region_item= &region_list;
				no_error=1;
				switch (region_type)
				{
					case PATCH:
					{
						x_step=(float)1;					
						y_max=(float)(number_of_rows-1);
						y_step=(float)1;
					} break;
					case SOCK:
					{
						/*???to make sure that first electrode is at the left */
						x_min=(float)-0.00001;
						x_max=(float)(-8*atan((double)1));					
						y_max=(float)2.15;
							/*???general ?*/
						y_step=y_max/(float)number_of_rows;
					} break;
					case TORSO:
					{
						x_max=(float)(4*atan((double)1));
						x_min= -x_max;					
						y_max=(float)(number_of_rows-1);
						y_step=(float)1;
					} break;
				}
				while ((region_number<number_of_regions)&&no_error)
				{
					if (0<number_of_regions_and_device_numbering)
					{
						device_number=region_number;
					}
					/* create the region */
					sprintf(region_name+7,"%d",region_number+1);
					if ((region=create_Region(region_name,region_type,region_number,
						electrodes_in_region
#if defined (UNEMAP_USE_3D)
						,unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
						))&&(*region_item=create_Region_list_item(
						region,(struct Region_list_item *)NULL)))
					{
						switch (region_type)
						{
							case SOCK:
							{
								region->properties.sock.focus=sock_focus;
							} break;
						}
						region_item= &((*region_item)->next);
						/* create the electrodes for the region */
						y=y_max;
						row_number=0;
						while ((row_number<number_of_rows)&&no_error)
						{
							if ((number_in_row=electrodes_in_row[row_number])>0)
							{
								switch (region_type)
								{
									case PATCH:
									{
										x=(float)((electrodes_in_row_max-number_in_row)/2);
									} break;
									case SOCK:
									{
										x=x_min;
										x_step=x_max/(float)number_in_row;
									} break;
									case TORSO:
									{
										x_step=(x_max-x_min)/(float)number_in_row;
										x=x_min+x_step/2;
									} break;
								}
								column_number=0;
								while ((column_number<number_in_row)&&no_error)
								{
									sprintf(device_name,"%d",device_number+1);
									if ((description=create_Device_description(device_name,
										ELECTRODE,region))&&(channel=create_Channel(device_number+1,
										(float)0,(float)1))&&(*device=create_Device(device_number,
										description,channel,(struct Signal *)NULL)))
									{
										/* assign position */
										switch (region_type)
										{
											case PATCH:
											{
												description->properties.electrode.position.x=x;
												description->properties.electrode.position.y=y;
											} break;
											case SOCK:
											{
												prolate_spheroidal_to_cartesian((float)1,y,x,sock_focus,
													&(description->properties.electrode.position.x),
													&(description->properties.electrode.position.y),
													&(description->properties.electrode.position.z),
													(float *)NULL);
											} break;
											case TORSO:
											{
												cylindrical_polar_to_cartesian((float)1,x,y,
													&(description->properties.electrode.position.x),
													&(description->properties.electrode.position.y),
													&(description->properties.electrode.position.z),
													(float *)NULL);
											} break;
										}
										device_number += device_number_offset;
										device++;
									}
									else
									{
										display_message(ERROR_MESSAGE,
									"create_standard_Rig.  Could not allocate memory for device");
										no_error=0;
										destroy_Channel(&channel);
										destroy_Device_description(&description);
									}
									column_number++;
									x += x_step;
								}
							}
							row_number++;
							y -= y_step;
						}
						region_number++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_standard_Rig.  Could not allocate memory for region");
						destroy_Region(&region);
						no_error=0;
					}
				}
				if (no_error)
				{
					device_number=number_of_devices-number_of_auxiliary_inputs;
					/* create the auxiliary devices */
					while (device_number<number_of_devices)
					{
						sprintf(device_name,"%d",device_number+1);
						if ((description=create_Device_description(device_name,AUXILIARY,
							region))&&(channel=create_Channel(device_number+1,(float)0,
							(float)1))&&(*device=create_Device(device_number,description,
							channel,(struct Signal *)NULL)))
						{
							(region->number_of_devices)++;
							device_number++;
							device++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_standard_Rig.  Could not allocate memory for device");
							no_error=0;
							destroy_Channel(&channel);
							destroy_Device_description(&description);
						}
					}
					if (no_error)
					{
						/* create the rig */
						rig=create_Rig(name,monitoring,experiment,number_of_devices,devices,
							(struct Page_list_item *)NULL,number_of_regions,region_list,
							(struct Region *)NULL
#if defined (UNEMAP_USE_3D)
							,unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
							);
					}
					else
					{
						rig=(struct Rig *)NULL;
					}
				}
				else
				{
					device_number=(device_number%number_of_regions)*electrodes_in_region+
						device_number/number_of_regions;
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
					DEALLOCATE(devices);
					/* free regions */
					destroy_Region_list(&region_list);
				}
				DEALLOCATE(device_name);
				DEALLOCATE(region_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_standard_Rig.  Could not allocate memory for device list");
				DEALLOCATE(device_name);
				DEALLOCATE(devices);
				rig=(struct Rig *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"create_standard_Rig.  No electrodes");
			rig=(struct Rig *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_standard_Rig.  Invalid arguments");
		rig=(struct Rig *)NULL;
	}
	LEAVE;

	return (rig);
} /* create_standard_Rig */

int destroy_Rig(struct Rig **rig)
/*******************************************************************************
LAST MODIFIED : 6 October 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**rig>, frees the
memory for <**rig> and changes <*rig> to NULL.
==============================================================================*/
{
	int number_of_devices,return_code;
	struct Device **device;

	ENTER(destroy_Rig);
	return_code=1;
	if (rig&&(*rig))
	{
		DEALLOCATE((*rig)->name);
		device=(*rig)->devices;
		number_of_devices=(*rig)->number_of_devices;
		while (number_of_devices>0)
		{
			destroy_Device(device);
			device++;
			number_of_devices--;
		}
#if defined (UNEMAP_USE_3D) 
		/*free up the package dependent things in the rig*/
		if(((*rig)->all_devices_rig_node_group)&&((*rig)->unemap_package))
		{
			/* following will deaccess the rig_node_group */
			free_unemap_package_rig_node_group((*rig)->unemap_package,
				&((*rig)->all_devices_rig_node_group));
		}	
		DEACCESS(Unemap_package)(&((*rig)->unemap_package));
#endif /* defined (UNEMAP_USE_3D)  */
		destroy_Page_list(&((*rig)->page_list));
		destroy_Region_list(&((*rig)->region_list));
		DEALLOCATE((*rig)->devices);
		DEALLOCATE((*rig)->signal_file_name);
		DEALLOCATE(*rig);	

		*rig=(struct Rig *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Rig */

#if defined (UNEMAP_USE_3D)
struct Unemap_package *get_Rig_unemap_package(struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  unemap_package of <rig> 
==============================================================================*/
{
	struct Unemap_package *unemap_package=(struct Unemap_package *)NULL;

	ENTER(get_Rig_unemap_package);
	if(rig)
	{
		unemap_package=rig->unemap_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Rig_unemap_package."
			" invalid arguments");	
	}
	LEAVE;
	return (unemap_package);
}/* get_Rig_unemap_package*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct GROUP(FE_node) *get_Rig_all_devices_rig_node_group(struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  all_devices_rig_node_group of <rig> 
==============================================================================*/
{
	struct GROUP(FE_node) *rig_node_group=(struct GROUP(FE_node) *)NULL;

	ENTER(get_Rig_all_devices_rig_node_group);
	if(rig)
	{	
		rig_node_group=rig->all_devices_rig_node_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Rig_all_devices_rig_node_group."
			" invalid arguments");	
	}
	LEAVE;
	return (rig_node_group);
}/* get_Rig_all_devices_rig_node_group*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Rig_all_devices_rig_node_group(struct Rig *rig,
	struct GROUP(FE_node) *rig_node_group)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets (and accesses) all_devices_rig_node_group of <rig> to <rig_node_group>
==============================================================================*/
{
	int return_code;
	ENTER(set_Rig_all_devices_rig_node_group);
	if(rig&&rig_node_group)
	{
		return_code =1;	
		REACCESS(GROUP(FE_node))(&(rig->all_devices_rig_node_group),rig_node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Rig_all_devices_rig_node_group."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Rig_all_devices_rig_node_group*/
#endif /* defined (UNEMAP_USE_3D)*/

struct Region *get_Rig_current_region(struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  current_region of <rig> 
==============================================================================*/
{
	struct Region *region=(struct Region *)NULL;

	ENTER(get_Rig_current_region);
	if(rig)
	{	
		region=rig->current_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Rig_current_region."
			" invalid arguments");	
	}
	LEAVE;
	return (region);
}/* get_Rig_current_region*/

int set_Rig_current_region(struct Rig *rig,struct Region *region)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets  current_region of <rig> to <region>
==============================================================================*/
{
	int return_code;
	ENTER(set_Rig_current_region);
	if(rig)
	{
		return_code =1;	
		rig->current_region=region;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Rig_current_region."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Rig_current_region*/

struct Region_list_item *get_Rig_region_list(struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  region_list of <rig> struct Region_list_item *
==============================================================================*/
{
	struct Region_list_item *region_list=(struct Region_list_item *)NULL;

	ENTER(get_Rig_region_list);
	if(rig)
	{	
		region_list=rig->region_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Rig_region_list."
			" invalid arguments");	
	}
	LEAVE;
	return (region_list);
}/* get_Rig_region_list*/

int set_Rig_region_list(struct Rig *rig,struct Region_list_item *region_list)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets  region_list of <rig> to <region>
==============================================================================*/
{
	int return_code;
	ENTER(set_Rig_region_list);
	if(rig)
	{
		return_code =1;	
		rig->region_list=region_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Rig_region_list."
			" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
}/* set_Rig_region_list*/

struct Rig *read_configuration(FILE *input_file,enum Rig_file_type file_type,
	enum Region_type rig_type
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
       )
/*******************************************************************************
LAST MODIFIED : 18 July 2001

DESCRIPTION :
Assumes that the <input_file> has been opened, the <file_type> (binary or text)
has been determined, the <rig_type> has been determined and the file has been
positioned at the beginning of the rig name.  This function creates a rig and
then initializes the fields as specified in the <input_file>.  It returns a
pointer to the rig if successful and NULL if unsuccessful.
==============================================================================*/
{
	char *dummy,finished,found,*name,separator,string[10];
	int channel_number,count,device_number,i,j,number_of_devices,number_of_pages,
		number_of_regions,region_number,region_number_of_devices,string_length;
	enum Device_type device_type;
	enum Region_type region_type;
	float coefficient,*coefficients,focus,sign;
	struct Auxiliary_properties *auxiliary_properties;
	struct Device **device,**electrodes;
	struct Device_list_item *device_item,**device_item_address,*device_list,
		*last_device_item;
	struct Page_list_item **last_page_item;
	struct Region_list_item **region_item_address;
	struct Rig *rig;

	ENTER(read_configuration);
	/* check that the file is open */
	if (input_file)
	{
		/* create the rig */
		if (rig=create_Rig((char *)NULL,MONITORING_OFF,EXPERIMENT_OFF,0,
			(struct Device **)NULL,(struct Page_list_item *)NULL,0,
			(struct Region_list_item *)NULL,(struct Region *)NULL
#if defined (UNEMAP_USE_3D)
			,unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
			))
		{
			/* choose between the file types */
			switch (file_type)
			{
				case TEXT:
				{
					/* read the rig name */
					fscanf(input_file,"%*[ :\n]");
					if (read_string(input_file,"[^\n]",&(rig->name)))
					{
						fscanf(input_file," ");
						/* read the rig type dependent properties */
						switch (rig_type)
						{
							case SOCK:
							{
								fscanf(input_file,"focus for Hammer projection : %f ",&focus);
							} break;
						}
						/* read in the regions */
						number_of_regions=0;
						number_of_devices=0;
						region_item_address= &(rig->region_list);
						device_list=(struct Device_list_item *)NULL;
						device_item_address= &(device_list);
						last_device_item=(struct Device_list_item *)NULL;
						/* read the region heading */
						fscanf(input_file,"%6[^ \n]",string);
						while (rig&&string_to_region_type(string,rig_type,&region_type))
						{
							fscanf(input_file,"%*[ :\n]");
							/* create the region */
							if ((*region_item_address=create_Region_list_item(create_Region(
								(char *)NULL,region_type,number_of_regions,0
#if defined (UNEMAP_USE_3D)
								,unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
								),(struct Region_list_item *)NULL))&&
								((*region_item_address)->region))
							{
								number_of_regions++;
								region_number_of_devices=0;
								/* read the region name */
								if (read_string(input_file,"[^\n]",
									&((*region_item_address)->region->name)))
								{
									fscanf(input_file," ");
									/* read the region type dependent properties */
									if (MIXED==rig_type)
									{
										switch (region_type)
										{
											case SOCK:
											{
												fscanf(input_file,"focus for Hammer projection : %f ",
													&((*region_item_address)->region->properties.sock.
													focus));
											} break;
										}
									}
									else
									{
										switch (rig_type)
										{
											case SOCK:
											{
												(*region_item_address)->region->properties.sock.focus=
													focus;
											} break;
										}
									}
									/* read the devices for the region */
									finished=0;
									while ((!finished)&&rig)
									{
										/* read the device type */
										string[0]='\0';
										fscanf(input_file,"%9[^ \n]",string);
										if ((('e'==string[0])||('E'==string[0]))&&
											(('l'==string[1])||('L'==string[1]))&&
											(('e'==string[2])||('E'==string[2]))&&
											(('c'==string[3])||('C'==string[3]))&&
											(('t'==string[4])||('T'==string[4]))&&
											(('r'==string[5])||('R'==string[5]))&&
											(('o'==string[6])||('O'==string[6]))&&
											(('d'==string[7])||('D'==string[7]))&&
											(('e'==string[8])||('E'==string[8])))
										{
											device_type=ELECTRODE;
										}
										else
										{
											if ((('a'==string[0])||('A'==string[0]))&&
												(('u'==string[1])||('U'==string[1]))&&
												(('x'==string[2])||('X'==string[2]))&&
												(('i'==string[3])||('I'==string[3]))&&
												(('l'==string[4])||('L'==string[4]))&&
												(('i'==string[5])||('I'==string[5]))&&
												(('a'==string[6])||('A'==string[6]))&&
												(('r'==string[7])||('R'==string[7]))&&
												(('y'==string[8])||('Y'==string[8])))
											{
												device_type=AUXILIARY;
											}
											else
											{
												/* have reached the end of the region */
												finished=1;
											}
										}
										/* if valid device type */
										if (rig&&!finished)
										{
											fscanf(input_file,"%*[ :\n]");
											/* add another item to the device list */
											if (last_device_item=create_Device_list_item(
												(struct Device *)NULL,last_device_item,
												(struct Device_list_item *)NULL))
											{
												/* create the device */
												if (last_device_item->device=create_Device(
													region_number_of_devices,
													(struct Device_description *)NULL,
													(struct Channel *)NULL,(struct Signal *)NULL))
												{
													/* create the device description */
													if ((last_device_item->device)->description=
														create_Device_description((char *)NULL,device_type,
														(*region_item_address)->region))
													{
														/* read the device name */
														if (read_string(input_file,"s",
															&(last_device_item->device->description->name)))
														{
															/* read device type dependent properties */
															switch (device_type)
															{
																case AUXILIARY:
																{
																	/* check if single channel or linear
																		combination of devices */
																	string[0]='\0';
																	fscanf(input_file," %7[^ \n]",string);
																	if (('c'==string[0])&&('h'==string[1])&&
																		('a'==string[2])&&('n'==string[3])&&
																		('n'==string[4])&&('e'==string[5])&&
																		('l'==string[6])&&('\0'==string[7]))
																	{
																		/* single channel */
																		if (1==fscanf(input_file," : %d ",
																			&channel_number))
																		{
																			/* single channel */
																			/* create the channel */
																			if (!(last_device_item->device->channel=
																				create_Channel(channel_number,(float)0,
																				(float)1)))
																			{
																				display_message(ERROR_MESSAGE,
															"read_configuration.  Could not create channel");
																				destroy_Device_list(&device_list,1);
																				destroy_Rig(&rig);
																			}
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
													"read_configuration.  Could not read channel number");
																			destroy_Device_list(&device_list,1);
																			destroy_Rig(&rig);
																		}
																	}
																	else
																	{
																		if (('s'==string[0])&&('u'==string[1])&&
																			('m'==string[2])&&('\0'==string[3]))
																		{
																			fscanf(input_file," : ");
																			sign=(float)1;
																			auxiliary_properties=
																				&((last_device_item->device->
																				description->properties).auxiliary);
																			/* linear combination of devices */
																			do
																			{
																				/*???DB.  Stops on a * for the "[^+-\n]" */
																				if (read_string(input_file,"[^\n+-]",
																					&dummy))
																				{
																					if (0<strlen(dummy))
																					{
																						if (name=strchr(dummy,'*'))
																						{
																							*name='\0';
																							name++;
																							if (1!=sscanf(dummy,"%f",
																								&coefficient))
																							{
																								display_message(ERROR_MESSAGE,
							"read_configuration.  Could not read coefficient for auxiliary");
																								destroy_Device_list(
																									&device_list,1);
																								destroy_Rig(&rig);
																							}
																						}
																						else
																						{
																							name=dummy;
																							coefficient=(float)1;
																						}
																						if (rig)
																						{
																							coefficient *= sign;
																							/* trim leading space */
																							while (isspace(*name))
																							{
																								name++;
																							}
																							/* trim trailing space */
																							string_length=strlen(name);
																							while ((0<string_length)&&
																								(isspace(name[string_length-
																								1])))
																							{
																								string_length--;
																							}
																							name[string_length]='\0';
																							/* find the device in the device
																								list */
																							device_item=device_list;
																							found=0;
																							while (device_item&&!found)
																							{
																								if ((device_item->device)&&
																									(device_item->device->
																									description)&&
#if defined (OLD_CODE)
/* relax to only requiring that it has an actual channel */
																									(ELECTRODE==device_item->
																									device->description->type)&&
#endif /* defined (OLD_CODE) */
																									(device_item->device->
																									channel)&&
																									(device_item->device->
																									description->name)&&
																									(0==strcmp(name,device_item->
																									device->description->name)))
																								{
																									found=1;
																								}
																								else
																								{
																									device_item=device_item->next;
																								}
																							}
																							if (found)
																							{
																								if (REALLOCATE(coefficients,
																									auxiliary_properties->
																									electrode_coefficients,float,
																									(auxiliary_properties->
																									number_of_electrodes)+1))
																								{
																									auxiliary_properties->
																										electrode_coefficients=
																										coefficients;
																								}
																								if (REALLOCATE(electrodes,
																									auxiliary_properties->
																									electrodes,struct Device *,
																									(auxiliary_properties->
																									number_of_electrodes)+1))
																								{
																									auxiliary_properties->
																										electrodes=electrodes;
																								}
																								if (coefficients&&electrodes)
																								{
																									coefficients[
																										auxiliary_properties->
																										number_of_electrodes]=
																										coefficient;
																									electrodes[
																										auxiliary_properties->
																										number_of_electrodes]=
																										device_item->device;
																									(auxiliary_properties->
																										number_of_electrodes)++;
																								}
																								else
																								{
																									display_message(ERROR_MESSAGE,
									"read_configuration.  Could not add electrode to auxiliary");
																									destroy_Device_list(
																										&device_list,1);
																									destroy_Rig(&rig);
																								}
																							}
																							else
																							{
																								display_message(ERROR_MESSAGE,
														"read_configuration.  Could not find electrode %s",
																									name);
																								destroy_Device_list(
																									&device_list,1);
																								destroy_Rig(&rig);
																							}
																						}
																					}
																					fscanf(input_file,"%c",
																						&separator);
																					if ('-'==separator)
																					{
																						sign=(float)-1;
																					}
																					else
																					{
																						sign=(float)1;
																					}
																					DEALLOCATE(dummy);
																				}
																				else
																				{
																					display_message(ERROR_MESSAGE,
"read_configuration.  Could not read electrode and coefficient for auxiliary");
																					destroy_Device_list(&device_list,1);
																					destroy_Rig(&rig);
																				}
																			} while (rig&&('\n'!=separator));
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
													"read_configuration.  Invalid auxiliary device %s %s",
																				last_device_item->device->description->
																				name,string);
																			destroy_Device_list(&device_list,1);
																			destroy_Rig(&rig);
																		}
																	}
																} break;
																case ELECTRODE:
																{
																	/* create the channel */
																	if (last_device_item->device->channel=
																		create_Channel(0,(float)0,(float)1))
																	{
																		/* read channel number */
																		if (1==fscanf(input_file," channel : %d ",
																			&(last_device_item->device->channel->
																			number)))
																		{
																			/* read position (dependent on rig
																				type) */
																			switch (region_type)
																			{
																				case SOCK:
																				case TORSO:
																				{
																					if (3!=fscanf(input_file,
																				"position : x = %f , y = %f , z = %f ",
																						&(last_device_item->device->
																						description->properties.electrode.
																						position.x),
																						&(last_device_item->device->
																						description->properties.electrode.
																						position.y),
																						&(last_device_item->device->
																						description->properties.electrode.
																						position.z)))
																					{
																						display_message(ERROR_MESSAGE,
													"read_configuration.  Could not read position");
																						destroy_Device_list(
																							&last_device_item,1);
																						destroy_Rig(&rig);
																					}																				 
																				} break;
																				case PATCH:
																				{
																					if (2!=fscanf(input_file,
																						"position : x = %f , y = %f ",
																						&(last_device_item->device->
																						description->properties.electrode.
																						position.x),
																						&(last_device_item->device->
																						description->properties.electrode.
																						position.y)))
																					{
																						display_message(ERROR_MESSAGE,
													"read_configuration.  Could not read patch position");
																						destroy_Device_list(
																							&last_device_item,1);
																						destroy_Rig(&rig);
																					}
																				} break;
																			}
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
													"read_configuration.  Could not read channel number");
																			destroy_Device_list(&device_list,1);
																			destroy_Rig(&rig);
																		}
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
															"read_configuration.  Could not create channel");
																		destroy_Device_list(&device_list,1);
																		destroy_Rig(&rig);
																	}
																} break;
															}
															if (rig)
															{
																*device_item_address=last_device_item;
																device_item_address= &(last_device_item->next);
																region_number_of_devices++;
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
														"read_configuration.  Could not read device name");
															destroy_Device_list(&device_list,1);
															destroy_Rig(&rig);
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
										"read_configuration.  Could not create device description");
														destroy_Device_list(&device_list,1);
														destroy_Rig(&rig);
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"read_configuration.  Could not create device");
													destroy_Device_list(&device_list,1);
													destroy_Rig(&rig);
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
											"read_configuration.  Could not create device list item");
												destroy_Device_list(&device_list,1);
												destroy_Rig(&rig);
											}
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_configuration.  Could not read region name");
									destroy_Device_list(&device_list,1);
									destroy_Rig(&rig);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_configuration.  Could not create region");
								destroy_Device_list(&device_list,1);
								destroy_Rig(&rig);
							}
							if (rig)
							{
								/* move to the next region */
								(*region_item_address)->region->number_of_devices=
									region_number_of_devices;
								number_of_devices += region_number_of_devices;
								region_item_address= &((*region_item_address)->next);
								/* read the region heading */
								fscanf(input_file,"%6[^ \n]",string);
							}
						}
						if (rig)
						{
							rig->number_of_regions=number_of_regions;
							if (number_of_regions>1)
							{
								rig->current_region=(struct Region *)NULL;
							}
							else
							{
								rig->current_region=rig->region_list->region;
							}
							if (number_of_devices>0)
							{
								if (ALLOCATE(rig->devices,struct Device *,number_of_devices))
								{
									rig->number_of_devices=number_of_devices;
									device=rig->devices;
									device_item=device_list;
									while (device_item)
									{
										*device=device_item->device;
										device_item=device_item->next;
										device++;
									}
									destroy_Device_list(&device_list,0);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_configuration.  Could not create device list");
									destroy_Device_list(&device_list,1);
									destroy_Rig(&rig);
								}
							}
						}
						if (rig&&!feof(input_file))
						{
							if (((('p'==string[0])||('P'==string[0]))&&
								(('a'==string[1])||('A'==string[1]))&&
								(('g'==string[2])||('G'==string[2]))&&
								(('e'==string[3])||('E'==string[3]))&&
								(('s'==string[4])||('S'==string[4]))))
							{
								/* read in the pages */
								last_page_item= &(rig->page_list);
								/* read the first page name */
								if (read_string(input_file,"s",&name))
								{
									fscanf(input_file," %c",&separator);
									while (!feof(input_file)&&rig&&(':'==separator))
									{
										/* add another item to the page list */
										if (*last_page_item=create_Page_list_item(
											(struct Page *)NULL,(struct Page_list_item *)NULL))
										{
											/* create the page */
											if ((*last_page_item)->page=create_Page(name,
												(struct Device_list_item *)NULL))
											{
												/* read the list of devices */
												fscanf(input_file," ");
												separator=',';
												device_item_address=
													&((*last_page_item)->page->device_list);
												last_device_item=(struct Device_list_item *)NULL;
												while (!feof(input_file)&&rig&&(separator!=':'))
												{
													if (read_string(input_file,"[^\n :,]",&dummy))
													{
														if (separator!=',')
														{
															if (ALLOCATE(name,char,strlen(dummy)+2))
															{
																name[0]=separator;
																name[1]='\0';
																strcat(name,dummy);
																DEALLOCATE(dummy);
															}
														}
														else
														{
															name=dummy;
														} /* end if */
														if (name)
														{
															fscanf(input_file," %c ",&separator);
															if (separator!=':')
															{
																/* look for the device with the specified
																	name */
																device=rig->devices;
																number_of_devices=rig->number_of_devices;
																finished=0;
																while ((number_of_devices>0)&&!finished)
																{
																	if (!strcmp(name,
																		(*device)->description->name))
																	{
																		finished=1;
																	}
																	else
																	{
																		device++;
																	} /* end if */
																	number_of_devices--;
																} /* end while */
																if (finished)
																{
																	if (last_device_item=create_Device_list_item(
																		*device,last_device_item,
																		(struct Device_list_item *)NULL))
																	{
																		*device_item_address=last_device_item;
																		device_item_address=
																			&(last_device_item->next);
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
									"read_configuration.  Could not create device item for page");
																		destroy_Rig(&rig);
																	} /* end if */
																} /* end if */
																else
																{
																	display_message(ERROR_MESSAGE,
														"read_configuration.  Invalid device %s in page %s",
																		name,(*last_page_item)->page->name);
																	/*??? important error for user,
																		should continue */
																} /* end if */
															} /* end if */
														}
														else
														{
															display_message(ERROR_MESSAGE,
									"read_configuration.  Could not create device name for page");
															destroy_Rig(&rig);
														} /* end if */
													}
													else
													{
														display_message(ERROR_MESSAGE,
									"read_configuration.  Could not create device name for page");
														destroy_Rig(&rig);
													} /* end if */
												} /* end while */
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"read_configuration.  Could not create page");
												destroy_Rig(&rig);
											}
											last_page_item= &((*last_page_item)->next);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_configuration.  Could not create page list item");
											destroy_Rig(&rig);
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_configuration.  Could not create page name");
									destroy_Rig(&rig);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_configuration.  Invalid device type %s",string);
								destroy_Rig(&rig);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_configuration.  Could not allocate memory for name");
						destroy_Rig(&rig);
					}
				} break;
				case BINARY:
				{
					/* read the rig name */
					BINARY_FILE_READ((char *)&string_length,sizeof(int),1,input_file);
					if (ALLOCATE(rig->name,char,string_length+1))
					{
						if (string_length>0)
						{
							BINARY_FILE_READ(rig->name,sizeof(char),string_length,input_file);
						}
						(rig->name)[string_length]='\0';
						/* read the rig type dependent properties */
						switch (rig_type)
						{
							case SOCK:
							{
								BINARY_FILE_READ((char *)&focus,sizeof(float),1,input_file);
							} break;
						}
						/* read the number of regions */
						BINARY_FILE_READ((char *)&number_of_regions,sizeof(int),1,
							input_file);
						/* read in the regions */
						rig->number_of_regions=number_of_regions;
						number_of_devices=0;
						region_item_address= &(rig->region_list);
						device_list=(struct Device_list_item *)NULL;
						device_item_address= &(device_list);
						last_device_item=(struct Device_list_item *)NULL;
						region_number=0;
						while (rig&&(region_number<number_of_regions))
						{
							/* read the region type */
							if (MIXED==rig_type)
							{
								BINARY_FILE_READ((char *)&region_type,sizeof(enum Region_type),
									1,input_file);
							}
							else
							{
								region_type=rig_type;
							}
							/* create the region */
							if ((*region_item_address=create_Region_list_item(create_Region(
								(char *)NULL,region_type,region_number,0
#if defined (UNEMAP_USE_3D)
								,unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
                ),(struct Region_list_item *)NULL))&&
								((*region_item_address)->region))
							{
								/* read the region name */
								BINARY_FILE_READ((char *)&string_length,sizeof(int),1,
									input_file);
								if (ALLOCATE((*region_item_address)->region->name,char,
									string_length+1))
								{
									BINARY_FILE_READ((*region_item_address)->region->name,
										sizeof(char),string_length,input_file);
									((*region_item_address)->region->name)[string_length]='\0';
									/* read the region type dependent properties */
									if (MIXED==rig_type)
									{
										switch (region_type)
										{
											case SOCK:
											{
												BINARY_FILE_READ((char *)&(((*region_item_address)->
													region->properties).sock.focus),sizeof(float),1,
													input_file);
#if defined (DEBUG)
												/*???debug */
												printf("focus=%g\n",((*region_item_address)->region->
													properties).sock.focus);
												{
													Linear_transformation *lt;

													if (ALLOCATE(lt,Linear_transformation,1))
													{
														((*region_item_address)->region->properties).sock.
															linear_transformation=lt;
														lt->translate_x=412.5;
														lt->translate_y=67.294;
														lt->translate_z= -67.303;
														lt->txx=0;lt->txy=0;lt->txz= -1;
														lt->tyx=0;lt->tyy=1;lt->tyz=0;
														lt->tzx=1;lt->tzy=0;lt->tzz=0;
													}
												}
#endif /* defined (DEBUG) */
#if defined (DEBUG)
/* for IUPS heart usr/people/williams/unemap_notes/iups2001/poster/epi_trsf_new.signal */
												((*region_item_address)->region->properties).sock.focus=
													35.;
												{
													Linear_transformation *lt;
												
													if (ALLOCATE(lt,Linear_transformation,1))
													{
														((*region_item_address)->region->properties).sock.
															linear_transformation=lt;
														lt->translate_x=-19.0514;
														lt->translate_y=-71.9462;
														lt->translate_z=3.1975;

														lt->txx=0.6457;
														lt->txy= 0.6298;
														lt->txz= 0.4316;

														lt->tyx= 0.2155;
														lt->tyy= -0.6927;
														lt->tyz= 0.6883;

														lt->tzx= -0.7325;
														lt->tzy=   0.3514;
														lt->tzz= 0.5830;
													}
												}
#endif /* defined (DEBUG) */
											} break;
										}
									}
									else
									{
										switch (rig_type)
										{
											case SOCK:
											{
												((*region_item_address)->region->properties).sock.focus=
													focus;
											} break;
										}
									}
									/* read the devices for the region */
									/* read the number of inputs */
									BINARY_FILE_READ((char *)&region_number_of_devices,
										sizeof(int),1,input_file);
									(*region_item_address)->region->number_of_devices=
										region_number_of_devices;
									number_of_devices += region_number_of_devices;
									/* read in the inputs */
									while ((region_number_of_devices>0)&&rig)
									{
										BINARY_FILE_READ((char *)&device_type,
											sizeof(enum Device_type),1,input_file);
										/* if valid device type */
										if ((ELECTRODE==device_type)||(AUXILIARY==device_type))
										{
											/* add another item to the device list */
											if ((last_device_item=create_Device_list_item(
												create_Device(0,(struct Device_description *)NULL,
												(struct Channel *)NULL,(struct Signal *)NULL),
												last_device_item,(struct Device_list_item *)NULL))&&
												(last_device_item->device))
											{
												*device_item_address=last_device_item;
												device_item_address= &(last_device_item->next);
												/* create the device description */
												if (last_device_item->device->description=
													create_Device_description((char *)NULL,device_type,
													(*region_item_address)->region))
												{
													/* read the device number */
													BINARY_FILE_READ((char *)&(last_device_item->device->
														number),sizeof(int),1,input_file);
													/* read the device name */
													BINARY_FILE_READ((char *)&string_length,sizeof(int),1,
														input_file);
													if (ALLOCATE(last_device_item->device->description->
														name,char,string_length+1))
													{
														BINARY_FILE_READ(last_device_item->device->
															description->name,sizeof(char),string_length,
															input_file);
														(last_device_item->device->description->name)
															[string_length]='\0';
														/* read channel number */
														BINARY_FILE_READ((char *)&channel_number,
															sizeof(int),1,input_file);
														/* read device type dependent properties */
														switch (device_type)
														{
															case AUXILIARY:
															{
																if (channel_number<0)
																{
																	/* linear combination of devices */
																	channel_number= -channel_number;
																	auxiliary_properties= &((last_device_item->
																		device->description->properties).auxiliary);
																	ALLOCATE(auxiliary_properties->
																		electrode_coefficients,float,
																		channel_number);
																	ALLOCATE(auxiliary_properties->electrodes,
																		struct Device *,channel_number);
																	if ((auxiliary_properties->
																		electrode_coefficients)&&
																		(auxiliary_properties->electrodes))
																	{
																		auxiliary_properties->
																			number_of_electrodes=channel_number;
																		i=0;
																		while (rig&&(i<channel_number))
																		{
																			/* have to create dummy devices because
																				the device with the specified number
																				may not have been read in yet */
																			if ((auxiliary_properties->electrodes)[i]=
																				create_Device(0,
																				(struct Device_description *)NULL,
																				(struct Channel *)NULL,
																				(struct Signal *)NULL))
																			{
																				BINARY_FILE_READ(
																					(char *)((auxiliary_properties->
																					electrode_coefficients)+i),
																					sizeof(float),1,input_file);
																				BINARY_FILE_READ((char *)&(
																					((auxiliary_properties->
																					electrodes)[i])->number),sizeof(int),
																					1,input_file);
																				i++;
																			}
																			else
																			{
																				display_message(ERROR_MESSAGE,
						"read_configuration.  Could not create dummy device for auxiliary");
																				while (i>0)
																				{
																					destroy_Device((auxiliary_properties->
																						electrodes)+i);
																					i--;
																				}
																				destroy_Device_list(&device_list,1);
																				destroy_Rig(&rig);
																			}
																		}
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
"read_configuration.  Could not allocate electrodes and coefficients for auxiliary");
																		destroy_Device_list(&device_list,1);
																		destroy_Rig(&rig);
																	}
																}
																else
																{
																	/* single channel */
																	/* create the channel */
																	if (!(last_device_item->device->channel=
																		create_Channel(channel_number,(float)0,
																		(float)1)))
																	{
																		display_message(ERROR_MESSAGE,
															"read_configuration.  Could not create channel");
																		destroy_Device_list(&device_list,1);
																		destroy_Rig(&rig);
																	}
																}
															} break;
															case ELECTRODE:
															{
																/* create the channel */
																if (last_device_item->device->channel=
																	create_Channel(channel_number,(float)0,
																	(float)1))
																{
																	/* read position (dependent on rig type) */
																	switch (region_type)
																	{
																		case SOCK:
																		case TORSO:
																		{
																			BINARY_FILE_READ(
																				(char *)&(last_device_item->device->
																				description->properties.electrode.
																				position.x),sizeof(float),1,input_file);
																			BINARY_FILE_READ(
																				(char *)&(last_device_item->device->
																				description->properties.electrode.
																				position.y),sizeof(float),1,input_file);
																			BINARY_FILE_READ(
																				(char *)&(last_device_item->device->
																				description->properties.electrode.
																				position.z),sizeof(float),1,input_file);
																		} break;
																		case PATCH:
																		{
																			BINARY_FILE_READ(
																				(char *)&(last_device_item->device->
																				description->properties.electrode.
																				position.x),sizeof(float),1,input_file);
																			BINARY_FILE_READ(
																				(char *)&(last_device_item->device->
																				description->properties.electrode.
																				position.y),sizeof(float),1,input_file);
																		} break;
																	}
																}
																else
																{
																	display_message(ERROR_MESSAGE,
															"read_configuration.  Could not create channel");
																	destroy_Device_list(&device_list,1);
																	destroy_Rig(&rig);
																}
															} break;
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
														"read_configuration.  Could not read device name");
														destroy_Device_list(&device_list,1);
														destroy_Rig(&rig);
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
										"read_configuration.  Could not create device description");
													destroy_Device_list(&device_list,1);
													destroy_Rig(&rig);
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"read_configuration.  Could not create device");
												destroy_Device_list(&device_list,1);
												destroy_Rig(&rig);
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_configuration.  Invalid device type");
											destroy_Device_list(&device_list,1);
											destroy_Rig(&rig);
										}
										region_number_of_devices--;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_configuration.  Could not create region name");
									destroy_Device_list(&device_list,1);
									destroy_Rig(&rig);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_configuration.  Could not create region");
								destroy_Device_list(&device_list,1);
								destroy_Rig(&rig);
							}
							if (rig)
							{
								/* move to the next region */
								region_number++;
								region_item_address= &((*region_item_address)->next);
							}
						}
						if (rig)
						{
							if (number_of_devices>0)
							{
								if (ALLOCATE(rig->devices,struct Device *,number_of_devices))
								{
									rig->number_of_devices=number_of_devices;
									device=rig->devices;
									device_item=device_list;
									while (device_item)
									{
										*device=device_item->device;
										device_item=device_item->next;
										device++;
									}
									destroy_Device_list(&device_list,0);
									/* finish assigning auxiliary devices that are linear
										combinations of electrodes */
									device=rig->devices;
									count=0;
									for (i=0;i<number_of_devices;i++)
									{
										if (device[i]&&(device[i]->description)&&(AUXILIARY==
											device[i]->description->type)&&(0<(auxiliary_properties=
											&((device[i]->description->properties).auxiliary))->
											number_of_electrodes))
										{
											for (j=0;j<auxiliary_properties->number_of_electrodes;j++)
											{
												device_number=((auxiliary_properties->electrodes)[j])->
													number;
												destroy_Device((auxiliary_properties->electrodes)+j);
												if ((0<=device_number)&&
													(device_number<number_of_devices)&&
													device[device_number]&&(device[device_number]->
#if defined (OLD_CODE)
/* relax to only requiring that it has an actual channel */
													description)&&(ELECTRODE==device[device_number]->
													description->type))
#endif /* defined (OLD_CODE) */
													channel))
												{
													(auxiliary_properties->electrodes)[j]=
														device[device_number];
												}
												else
												{
													count++;
												}
											}
										}
									}
									if (count>0)
									{
										display_message(ERROR_MESSAGE,
											"read_configuration.  Invalid electrode for auxiliary");
										destroy_Rig(&rig);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_configuration.  Could not create device list");
									destroy_Device_list(&device_list,1);
									destroy_Rig(&rig);
								}
							}
						}
						if (rig)
						{
							if (number_of_regions>1)
							{
								rig->current_region=(struct Region *)NULL;
							}
							else
							{
								rig->current_region=rig->region_list->region;
							}
							/* read the number of pages */
							BINARY_FILE_READ((char *)&number_of_pages,sizeof(int),1,
								input_file);
							/* read in the pages */
							count=0;
							device=rig->devices;
							last_page_item= &(rig->page_list);
							while ((count<number_of_pages)&&rig)
							{
								/* add another item to the page list */
								if (*last_page_item=create_Page_list_item((struct Page *)NULL,
									(struct Page_list_item *)NULL))
								{
									/* create the page */
									if ((*last_page_item)->page=create_Page((char *)NULL,
										(struct Device_list_item *)NULL))
									{
										/* read the page name */
										BINARY_FILE_READ((char *)&string_length,sizeof(int),1,
											input_file);
										if (ALLOCATE((*last_page_item)->page->name,char,
											string_length+1))
										{
											BINARY_FILE_READ((*last_page_item)->page->name,
												sizeof(char),string_length,input_file);
											((*last_page_item)->page->name)[string_length]='\0';
											/* read the number of devices in the device list */
											BINARY_FILE_READ((char *)&number_of_devices,sizeof(int),1,
												input_file);
											/* read the device list */
											device_item_address=
												&((*last_page_item)->page->device_list);
											last_device_item=(struct Device_list_item *)NULL;
											while ((number_of_devices>0)&&rig)
											{
												BINARY_FILE_READ((char *)&device_number,sizeof(int),1,
													input_file);
												if (last_device_item=create_Device_list_item(
													device[device_number],last_device_item,
													(struct Device_list_item *)NULL))
												{
													*device_item_address=last_device_item;
													device_item_address= &(last_device_item->next);
												}
												else
												{
													display_message(ERROR_MESSAGE,
									"read_configuration.  Could not create device item for page");
													destroy_Rig(&rig);
												}
												number_of_devices--;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_configuration.  Could not create page name");
											destroy_Rig(&rig);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_configuration.  Could not create page");
										destroy_Rig(&rig);
									}
									last_page_item= &((*last_page_item)->next);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_configuration.  Could not create page list item");
									destroy_Rig(&rig);
								}
								count++;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_configuration.  Could not allocate memory for name");
						destroy_Rig(&rig);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"read_configuration.  Invalid file type");
					destroy_Rig(&rig);
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_configuration.  Could not create rig");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_configuration.  input_file is NULL");
		rig=(struct Rig *)NULL;
	}
	LEAVE;

	return (rig);
} /* read_configuration */

int read_calibration_file(char *file_name,void *rig)
/*******************************************************************************
LAST MODIFIED : 11 June 2002

DESCRIPTION :
This function reads in the characteristics of the acquisition channels for the
<rig> from the specified file.
==============================================================================*/
{
	int number,number_of_devices,return_code;
	float gain,offset;
	struct Device **device;
	struct Rig *rig_local;
	FILE *input_file;

	ENTER(read_calibration_file);
	return_code=0;
	/* open the calibration file */
	if (input_file=fopen(file_name,"r"))
	{
		if (rig_local=(struct Rig *)rig)
		{
			/* read the file heading */
			if (EOF!=fscanf(input_file,"%*[^\n]\n"))
			{
				/* read the channel calibrations */
				while (3==fscanf(input_file,"%d %f %f \n",&number,&offset,&gain))
				{
					/* find if there is a device using this channel */
					device=rig_local->devices;
					number_of_devices=rig_local->number_of_devices;
					while ((number_of_devices>0)&&(!((*device)->channel)||
						((*device)->channel->number!=number)))
					{
						number_of_devices--;
						device++;
					}
					if ((number_of_devices>0)&&((*device)->channel->number==number))
					{
						(*device)->channel->offset=offset;
						(*device)->channel->gain=gain;
						(*device)->channel->gain_correction=gain;
					}
				}
				if (feof(input_file))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_calibration_file.  Error reading file");
					return_code=0;
				}
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_calibration_file.  Missing rig");
			return_code=0;
		}
		/* close the calibration file */
		fclose(input_file);
	}
	else
	{
		display_message(WARNING_MESSAGE,"Calibration file, %s, is missing",
			file_name);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_calibration_file */

int read_configuration_file(char *file_name,void *rig_pointer
#if defined (UNEMAP_USE_3D)					
		 ,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D) */
	)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function reads in a rig configuration from the named file, automatically
determining whether it was written as binary or text.  It creates a rig with
the specified configuration and returns a pointer to it.
==============================================================================*/
{
	char rig_type_text[5];
	enum Rig_file_type file_type;
	enum Region_type rig_type;
	FILE *input_file;
	int return_code;
	struct Rig *rig;

	ENTER(read_configuration_file);
	/* determine the file type and rig type */
	rig=(struct Rig *)NULL;
	/* try binary first */
	if (input_file=fopen(file_name,"rb"))
	{
		BINARY_FILE_READ((char *)&rig_type,sizeof(enum Region_type),1,input_file);
		if ((SOCK==rig_type)||(PATCH==rig_type)||(MIXED==rig_type)||
			(TORSO==rig_type))
		{
			file_type=BINARY;
		}
		else
		{
			fclose(input_file);
			input_file=(FILE *)NULL;
		}
	}
	if (!input_file)
	{
		/* try text second */
		if (input_file=fopen(file_name,"r"))
		{
			fscanf(input_file,"%5c",rig_type_text);
			if ((('m'==rig_type_text[0])||('M'==rig_type_text[0]))&&
				(('i'==rig_type_text[1])||('I'==rig_type_text[1]))&&
				(('x'==rig_type_text[2])||('X'==rig_type_text[2]))&&
				(('e'==rig_type_text[3])||('E'==rig_type_text[3]))&&
				(('d'==rig_type_text[4])||('D'==rig_type_text[4])))
			{
				file_type=TEXT;
				rig_type=MIXED;
			}
			else
			{
				if (string_to_region_type(rig_type_text,MIXED,&rig_type))
				{
					file_type=TEXT;
				}
				else
				{
					fclose(input_file);
					input_file=(FILE *)NULL;
				}
			}
		}
	}
	if (input_file)
	{
		if (rig=read_configuration(input_file,file_type,rig_type
#if defined (UNEMAP_USE_3D)
			,unemap_package
#endif /* defined (UNEMAP_USE_3D) */
			))
		{
#if defined (OLD_CODE)
???DB.  Only read calibration when doing acquisition */
			if ((rig->calibration_directory)&&(0<strlen(rig->calibration_directory)))
			{
				if (ALLOCATE(calibration_file_name,char,
					strlen(rig->calibration_directory)+15))
				{
					strcpy(calibration_file_name,rig->calibration_directory);
					strcat(calibration_file_name,"/calibrate.dat");
				}
			}
			else
			{
				if (ALLOCATE(calibration_file_name,char,14))
				{
					strcpy(calibration_file_name,"calibrate.dat");
				}
			}
			if (calibration_file_name)
			{
				read_calibration_file(calibration_file_name,(void *)rig);
				DEALLOCATE(calibration_file_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
		"read_configuration_file.  Insufficient memory for calibration file name");
			}
#endif /* defined (OLD_CODE) */
		}
		fclose(input_file);
	}
	if (rig)
	{
		*((struct Rig **)rig_pointer)=rig;
		return_code=1;
	}
	else
	{
		return_code=0;
	}
#if defined (DEBUG)
	show_config_file("jwconfg",(void *)rig_pointer);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* read_configuration_file */

int write_configuration(struct Rig *rig,FILE *output_file,
	enum Rig_file_type file_type)
/*******************************************************************************
LAST MODIFIED : 28 July 1999

DESCRIPTION :
Assumes that the <output_file> has been opened with the specified <file_type>
(binary or text).  This function writes a description of the <rig> to the
<output_file>.  It returns a non-zero if successful and zero if unsuccessful.
==============================================================================*/
{
	enum Region_type rig_type;
	int i,line_length,number_of_devices,number_of_pages,number_of_regions,
		return_code,string_length;
	struct Auxiliary_properties *auxiliary_properties;
	struct Device **device;
	struct Device_list_item *device_item;
	struct Page_list_item *page_item;
	struct Region *region;
	struct Region_list_item *region_item;

	ENTER(write_configuration);
	return_code=1;
	/* check that the file is open */
	if (output_file)
	{
		/* check the rig */
		if (rig)
		{
			/* choose between the file types */
			switch (file_type)
			{
				case TEXT:
				{
					/* write the rig name */
					fprintf(output_file,"mixed : %s\n",rig->name);
					/* write out the regions */
					region_item=rig->region_list;
					number_of_regions=rig->number_of_regions;
					while (return_code&&(number_of_regions>0))
					{
						region=region_item->region;
						/* write the region type */
						switch (region->type)
						{
							case SOCK:
							{
								fprintf(output_file,"sock : ");
							} break;
							case PATCH:
							{
								fprintf(output_file,"patch : ");
							} break;
							case TORSO:
							{
								fprintf(output_file,"torso : ");
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"write_configuration.  Invalid region type");
								return_code=0;
							} break;
						}
						if (return_code)
						{
							/* write the region name */
							fprintf(output_file,"%s\n",region->name);
							/* write the region type dependent properties */
							switch (region->type)
							{
								case SOCK:
								{
									fprintf(output_file,"focus for Hammer projection : %f\n",
										(region->properties).sock.focus);
								} break;
							}
							/* write out the inputs */
							device=rig->devices;
							number_of_devices=rig->number_of_devices;
							while (number_of_devices>0)
							{
								/* if the device is in the current region write it out */
								if ((*device)->description->region==region)
								{
									/* write the device type */
									switch ((*device)->description->type)
									{
										case ELECTRODE:
										{
											fprintf(output_file,"electrode");
										} break;
										case AUXILIARY:
										{
											fprintf(output_file,"auxiliary");
										} break;
									}
									/* write the device name */
									fprintf(output_file," : %s\n",(*device)->description->name);
									/* write device type dependent properties */
									switch ((*device)->description->type)
									{
										case AUXILIARY:
										{
											auxiliary_properties=
												&((*device)->description->properties.auxiliary);
											if (0<auxiliary_properties->number_of_electrodes)
											{
												fprintf(output_file,"sum : ");
												for (i=0;i<auxiliary_properties->number_of_electrodes;
													i++)
												{
													fprintf(output_file,"%+g*%s",(auxiliary_properties->
														electrode_coefficients)[i],((auxiliary_properties->
														electrodes)[i])->description->name);
												}
												fprintf(output_file,"\n");
											}
											else
											{
												/* write the channel */
												fprintf(output_file,"channel : %d\n",
													(*device)->channel->number);
											}
										} break;
										case ELECTRODE:
										{
											/* write the channel */
											fprintf(output_file,"channel : %d\n",
												(*device)->channel->number);
											/* write position (dependent on region type) */
											switch (region->type)
											{
												case SOCK:
												case TORSO:
												{
													fprintf(output_file,
														"position : x = %f, y = %f, z = %f\n",
														(*device)->description->properties.electrode.
															position.x,
														(*device)->description->properties.electrode.
															position.y,
														(*device)->description->properties.electrode.
															position.z);
												} break;
												case PATCH:
												{
													fprintf(output_file,"position : x = %f, y = %f\n",
														(*device)->description->properties.electrode.
															position.x,
														(*device)->description->properties.electrode.
															position.y);
												} break;
											}
										} break;
									}
								}
								device++;
								number_of_devices--;
							}
						}
						region_item=region_item->next;
						number_of_regions--;
					}
					if (return_code)
					{
						/* write out the pages */
						page_item=rig->page_list;
						if (page_item)
						{
							/* write the pages heading */
							fprintf(output_file,"pages\n");
							while (page_item)
							{
								/* write the page name */
								fprintf(output_file,"%s : ",page_item->page->name);
								line_length=strlen(page_item->page->name)+3;
								/* write the list of devices */
								device_item=page_item->page->device_list;
								while (device_item)
								{
									line_length += strlen(device_item->device->description->name);
									if (line_length>79)
									{
										fprintf(output_file,"\n");
										line_length=strlen(device_item->device->description->name);
									}
									fprintf(output_file,"%s",
										device_item->device->description->name);
									device_item=device_item->next;
									if (device_item)
									{
										fprintf(output_file,",");
										line_length++;
									}
								}
								fprintf(output_file,"\n");
								page_item=page_item->next;
							}
						}
					}
				} break;
				case BINARY:
				{
					rig_type=MIXED;
					BINARY_FILE_WRITE((char *)&rig_type,sizeof(enum Region_type),1,
						output_file);
					/* write the rig name */
					string_length=strlen(rig->name);
					BINARY_FILE_WRITE((char *)&string_length,sizeof(int),1,output_file);
					if (string_length>0)
					{
						BINARY_FILE_WRITE(rig->name,sizeof(char),string_length,output_file);
					}
					/* write the number of regions */
					region_item=rig->region_list;
					number_of_regions=rig->number_of_regions;
					BINARY_FILE_WRITE((char *)&number_of_regions,sizeof(int),1,
						output_file);
					/* write out the regions */
					while (return_code&&(number_of_regions>0))
					{
						region=region_item->region;
						/* write region type */
						if ((SOCK==region->type)||(PATCH==region->type)||
							(TORSO==region->type))
						{
							BINARY_FILE_WRITE((char *)&(region->type),
								sizeof(enum Region_type),1,output_file);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_configuration.  Invalid rig type");
							return_code=0;
						};
						if (return_code)
						{
							/* write the region name */
							string_length=strlen(region->name);
							BINARY_FILE_WRITE((char *)&string_length,sizeof(int),1,
								output_file);
							BINARY_FILE_WRITE(region->name,sizeof(char),string_length,
								output_file);
							/* write the region type dependent properties */
							switch (region->type)
							{
								case SOCK:
								{
									BINARY_FILE_WRITE((char *)&((region->properties).sock.focus),
										sizeof(float),1,output_file);
								} break;
							}
							/* write the number of inputs */
							BINARY_FILE_WRITE((char *)&(region->number_of_devices),
								sizeof(int),1,output_file);
							/* write out the inputs */
							number_of_devices=rig->number_of_devices;
							device=rig->devices;
							while (number_of_devices>0)
							{
								/* if the device is in the current region write it out */
								if ((*device)->description->region==region)
								{
									/* write the device type */
									BINARY_FILE_WRITE((char *)&((*device)->description->type),
										sizeof(enum Device_type),1,output_file);
									/* write the device number */
									BINARY_FILE_WRITE((char *)&((*device)->number),sizeof(int),1,
										output_file);
									/* write the device name */
									string_length=strlen((*device)->description->name);
									BINARY_FILE_WRITE((char *)&string_length,sizeof(int),1,
										output_file);
									BINARY_FILE_WRITE((*device)->description->name,sizeof(char),
										string_length,output_file);
									/* write device type dependent properties */
									switch ((*device)->description->type)
									{
										case AUXILIARY:
										{
											auxiliary_properties=
												&((*device)->description->properties.auxiliary);
											/* assuming that channel numbers are not negative */
											if (0<auxiliary_properties->number_of_electrodes)
											{
												i= -auxiliary_properties->number_of_electrodes;
												BINARY_FILE_WRITE((char *)&i,sizeof(int),1,output_file);
												for (i=0;i<auxiliary_properties->number_of_electrodes;
													i++)
												{
													BINARY_FILE_WRITE((char *)((auxiliary_properties->
														electrode_coefficients)+i),sizeof(float),1,
														output_file);
													BINARY_FILE_WRITE((char *)&(((auxiliary_properties->
														electrodes)[i])->number),sizeof(int),1,output_file);
												}
											}
											else
											{
												/* write the channel */
												BINARY_FILE_WRITE((char *)&((*device)->channel->number),
													sizeof(int),1,output_file);
											}
										} break;
										case ELECTRODE:
										{
											/* write the channel */
											BINARY_FILE_WRITE((char *)&((*device)->channel->number),
												sizeof(int),1,output_file);
											/* write position (dependent on rig type) */
											switch (region->type)
											{
												case SOCK:
												case TORSO:
												{
													BINARY_FILE_WRITE((char *)&((*device)->description->
														properties.electrode.position.x),sizeof(float),1,
														output_file);
													BINARY_FILE_WRITE((char *)&((*device)->description->
														properties.electrode.position.y),sizeof(float),1,
														output_file);
													BINARY_FILE_WRITE((char *)&((*device)->description->
														properties.electrode.position.z),sizeof(float),1,
														output_file);
												} break;
												case PATCH:
												{
													BINARY_FILE_WRITE((char *)&((*device)->description->
														properties.electrode.position.x),sizeof(float),1,
														output_file);
													BINARY_FILE_WRITE((char *)&((*device)->description->
														properties.electrode.position.y),sizeof(float),1,
														output_file);
												} break;
											}
										} break;
									}
								}
								number_of_devices--;
								device++;
							}
							region_item=region_item->next;
							number_of_regions--;
						}
					}
					if (return_code)
					{
						/* write the number of pages */
						number_of_pages=number_in_Page_list(rig->page_list);
						BINARY_FILE_WRITE((char *)&number_of_pages,sizeof(int),1,
							output_file);
						/* write out the pages */
						page_item=rig->page_list;
						while (page_item)
						{
							/* write the page name */
							string_length=strlen(page_item->page->name);
							BINARY_FILE_WRITE((char *)&string_length,sizeof(int),1,
								output_file);
							BINARY_FILE_WRITE(page_item->page->name,sizeof(char),
								string_length,output_file);
							/* write the number of devices */
							number_of_devices=
								number_in_Device_list(page_item->page->device_list);
							BINARY_FILE_WRITE((char *)&number_of_devices,sizeof(int),1,
								output_file);
							/* write out the device numbers */
							device_item=page_item->page->device_list;
							while (device_item)
							{
								BINARY_FILE_WRITE((char *)&(device_item->device->number),
									sizeof(int),1,output_file);
								device_item=device_item->next;
							}
							page_item=page_item->next;
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"write_configuration.  Invalid file type");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"write_configuration.  rig is NULL");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_configuration.  output_file is NULL");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_configuration */

int write_configuration_file(char *file_name,void *rig_pointer)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
Writes the configuration of the <rig> to the named file in text format.  It
returns a non-zero if successful and zero if unsuccessful.
==============================================================================*/
{
	int return_code;
	FILE *output_file;

	ENTER(write_configuration_file);
	if (output_file=fopen(file_name,"w"))
	{
		return_code=
			write_configuration(*((struct Rig **)rig_pointer),output_file,TEXT);
		fclose(output_file);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_configuration_file.  Could not open file: %s",file_name);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_configuration_file */

#if defined (DEBUG)
int show_config(struct Rig *rig,FILE *output_file,
	enum Rig_file_type file_type)
/*******************************************************************************
LAST MODIFIED : 29 July 2000

DESCRIPTION :
Similar to write_configuration() but in more detail. A bit inefficient and
repetative, but the way all this is stored is to be changed 

Assumes that the <output_file> has been opened with the specified <file_type>
(binary or text).  This function writes a description of the <rig> to the
<output_file>.  It returns a non-zero if successful and zero if unsuccessful.
==============================================================================*/
{
	int number_of_devices,return_code,number_of_regions;
	struct Device *device;
	struct Device **devices;	
	struct Device_list_item *device_item;
	struct Page_list_item *page_item;
	struct Region *region;
	struct Region_list_item *region_item;


	ENTER(show_config);
	return_code=1;
	/* check that the file is open */
	if (output_file)
	{
		/* check the rig */
		if (rig)
		{
			/* choose between the file types */
			switch (file_type)
			{
				case TEXT:
				{					
					/* rig->name */
					fprintf(output_file,"Rig :\n");		
					fprintf(output_file,"  ->name : %s\n",rig->name);
					/* rig->Experiment_status */
					fprintf(output_file,"  ->Experiment_status = ");
					switch(rig->experiment)
					{							
						case EXPERIMENT_ON:
						{
							fprintf(output_file,"EXPERIMENT_ON\n");	
						}break;
						case EXPERIMENT_OFF:
						{
							fprintf(output_file,"EXPERIMENT_OFF\n");	
						}break;
						default:	
						{
							fprintf(output_file,"UNDEFINED\n");	
						}break;						
					} /* switch(rig->experiment) */
					/* rig->Monitoring_status */
					fprintf(output_file,"  ->Monitoring_status = ");
					switch(rig->monitoring)
					{							
						case MONITORING_ON:
						{
							fprintf(output_file,"MONITORING_ON\n");	
						}break;
						case MONITORING_OFF:
						{
							fprintf(output_file,"MONITORING_OFF\n");	
						}break;
						default:	
						{
							fprintf(output_file,"UNDEFINED\n");	
						}break;						
					}	/* switch(rig->monitoring) */
					/*rig->number_of_devices */
					fprintf(output_file,"   ->number_of_devices : %d\n",rig->number_of_devices);
					number_of_devices = rig->number_of_devices;	
					/* rig->devices */
					devices = rig->devices;						
					fprintf(output_file,"   ->devices\n");
					while(number_of_devices>0)
					{	
						device = *devices;					
						
						{
							/* (*(rig->devices))->number */
							fprintf(output_file,"    Device number %d\n",device->number);
							/* (*(rig->devices))->description */
							fprintf(output_file,"     ->description \n");
							/* (*(rig->devices))->description->name */
							fprintf(output_file,"      ->name %s\n",device->description->name);
							/* (*(rig->devices))->description->type */
							fprintf(output_file,"      ->type ");
							switch(device->description->type)
							{							
								case ELECTRODE:
								{
									fprintf(output_file,"ELECTRODE\n");	
									/* (*(rig->devices))->description->properties */
									fprintf(output_file,"      ->properties\n");
									fprintf(output_file,"        .electrode.position: x= %f, y= %f, z= %f\n",
														device->description->properties.electrode.position.x,
														device->description->properties.electrode.position.y,
														device->description->properties.electrode.position.z);
								}break;
								case AUXILIARY:
								{		
									fprintf(output_file,"AUXILIARY\n");	
									/* (*(rig->devices))->description->properties */
									fprintf(output_file,"      ->properties\n");
									fprintf(output_file,"        .auxiliary.dummy: = %d\n",
														device->description->properties.auxiliary.dummy);
								}break;
								default:	
								{
									fprintf(output_file,"UNDEFINED\n");	
								}break;						
							}	/*switch(device->description->Device_type) */
							/* (*(rig->devices))->description->region */
							fprintf(output_file,"      ->region\n");
							region=device->description->region;							
							/* (*(rig->devices))->description->region->name  */
							fprintf(output_file,"       ->name : %s\n",region->name);
							/* (*(rig->devices))->description->region->number */
							fprintf(output_file,"       ->number : %d\n",region->number);
							/* (*(rig->devices))->description->region->type */
							fprintf(output_file,"       ->type  ");					
							switch(region->type)
							{							
								case MIXED:
								{
									fprintf(output_file,"MIXED\n");	
								}break;
								case PATCH:
								{
									fprintf(output_file,"PATCH\n");	
								}break;
								case SOCK:
								{	
									/* (*(rig->devices))->description->region->properties.sock.focus */
									fprintf(output_file,"SOCK\n");								
									fprintf(output_file,"        focus for Hammer projection : %f\n",
										(region->properties).sock.focus);														
								}break;				 
								case TORSO:
								{
									fprintf(output_file,"TORSO\n");	
								}break;
								default:	
								{
									fprintf(output_file,"UNDEFINED\n");	
								}break;						
							} /* region->type */
							/* (*(rig->devices))->description->region->number_of_devices */
							fprintf(output_file,"       ->number_of_devices : %d\n",
								region->number_of_devices);
							/* (*(rig->devices))->chanel */
							fprintf(output_file,"     ->channel \n");
							/* (*(rig->devices))->chanel->number */
							fprintf(output_file,"      ->number %d\n",
								device->channel->number);
							/* (*(rig->devices))->chanel->gain */
							fprintf(output_file,"      ->gain %f\n",device->channel->gain);
							/* (*(rig->devices))->chanel->gain_correction */
							fprintf(output_file,"      ->gain_correction %f\n",
								device->channel->gain_correction);
							/* (*(rig->devices))->chanel->offset */
							fprintf(output_file,"      ->offset %f\n",
								device->channel->offset);
							/* (*(rig->devices))->signal */
							fprintf(output_file,"     ->signal ");
							if(device->signal)
							{	
								fprintf(output_file,"\n");
								/* (*(rig->devices))->signal->status */
								fprintf(output_file,"      ->status ");
								switch(device->signal->status)
								{							
									case ACCEPTED:
									{
										fprintf(output_file,"ACCEPTED\n");	
									}break;
									case REJECTED:
									{
										fprintf(output_file,"REJECTED\n");	
									}break;
									case UNDECIDED:
									{									
										fprintf(output_file,"UNDECIDED\n");									 
									}break;				 							
									default:	
									{
										fprintf(output_file,"\n");	
									}break;						
								} /* device->channel->signal->status */
								/* (*(rig->devices))->signal->buffer */
								fprintf(output_file,"      ->buffer\n");
								/* (*(rig->devices))->signal->buffer->number_of_signals */
								fprintf(output_file,"       ->number_of_signals %d\n",
									device->signal->buffer->number_of_signals);
								/* (*(rig->devices))->signal->buffer->number_of_samples */
								fprintf(output_file,"       ->number_of_signals %d\n",
									device->signal->buffer->number_of_samples);
								/* (*(rig->devices))->signal->buffer->frequency */
								fprintf(output_file,"       ->number_of_signals %f\n",
									device->signal->buffer->frequency);
								/* (*(rig->devices))->signal->buffer->times */
								fprintf(output_file,"       *(->times) %d\n",
									*device->signal->buffer->times);
								/* (*(rig->devices))->signal->buffer->value_type */
								fprintf(output_file,"      ->value_type ");
								switch(device->signal->buffer->value_type)
								{							
									case SHORT_INT_VALUE:
									{
										fprintf(output_file,"SHORT_INT_VALUE\n");
										/* (*(rig->devices))->signal->buffer->signals.short_int_values */
										fprintf(output_file,"       *(->signals.short_int_values)%d\n ",
											*(device->signal->buffer->signals.short_int_values) );
									}break;
									case FLOAT_VALUE:
									{
										fprintf(output_file,"FLOAT_VALUE\n");											
										/* (*(rig->devices))->signal->buffer->signals.float_values */
										fprintf(output_file,"       *(->signals.float_values)%f\n ",
											*(device->signal->buffer->signals.float_values) );
									}break;									
									default:	
									{
										fprintf(output_file,"\n");	
									}break;						
								} /* device->signal->buffer->value_type  */
								/* (*(rig->devices))->signal->buffer->start */
								fprintf(output_file,"       ->start %d\n",
									device->signal->buffer->start);
								/* (*(rig->devices))->signal->buffer->end */
								fprintf(output_file,"       ->end %d\n",
									device->signal->buffer->end);
								/* (*(rig->devices))->signal->index */
								fprintf(output_file,"      ->index %d\n ",device->signal->index);
								/* (*(rig->devices))->signal->first_event */
								fprintf(output_file,"      ->first_event\n ");
								/* (*(rig->devices))->signal->first_event->time */
								fprintf(output_file,"       ->time %d\n",
									device->signal->first_event->time);
								/* (*(rig->devices))->signal->first_event->number */
								fprintf(output_file,"       ->number %d\n",
									device->signal->first_event->number);
								/* (*(rig->devices))->signal->first_event->status */
								fprintf(output_file,"       ->status ");
								switch(device->signal->first_event->status)
								{
									case ACCEPTED:
									{
										fprintf(output_file,"ACCEPTED\n");	
									}break;
									case REJECTED:
									{
										fprintf(output_file,"REJECTED\n");	
									}break;
									case UNDECIDED:
									{									
										fprintf(output_file,"UNDECIDED\n");									 
									}break;				 							
									default:	
									{
										fprintf(output_file,"\n");	
									}break;						
								}/* switch(device->signal->first_event->status) */
								/* (*(rig->devices))->signal->first_event->next */
								fprintf(output_file,"       ->next %p\n",
									device->signal->first_event->next);
								/* (*(rig->devices))->signal->first_event->previous */
								fprintf(output_file,"       ->previous %p\n",
									device->signal->first_event->previous);								
								/* (*(rig->devices))->signal->next */
								fprintf(output_file,"      ->next %p \n ",device->signal->next);
								/* (*(rig->devices))->signal->number */
								fprintf(output_file,"      ->number%d\n ",device->signal->number);
							}
							else
							{
								fprintf(output_file," = NULL \n");
							}
							/* (*(rig->devices))->signal_display_maximum */						
							fprintf(output_file,"     ->signal_display_maximum %f\n",
								device->signal_display_maximum);
							/* (*(rig->devices))->signal_display_minimum */
							fprintf(output_file,"     ->signal_miniimum %f\n",
								device->signal_display_minimum);
							/* (*(rig->devices))->highlight */
							fprintf(output_file,"     ->highlight %d\n",device->highlight);
						} /* if (device->description->region==region) */
						devices++;
						number_of_devices--;
					}	/*	while(number_of_devices>0) */
					/* rig->page_list*/
					fprintf(output_file,"  ->page_list \n");	
					page_item = rig->page_list;
					while(page_item)
					{
						/* rig->page_list->page*/
						fprintf(output_file,"   ->page \n");	
						/* rig->page_list->page->name*/
						fprintf(output_file,"    ->name %s\n",page_item->page->name);	
						/* rig->page_list->page->device_list*/
						device_item = page_item->page->device_list;
						while(device_item)
						{							
							device = device_item->device;											
							
							/* rig->page_list->page->device_list->device->number */
							fprintf(output_file,"    Device number %d\n",device->number);
							/* rig->page_list->page->device_list->device->description */
							fprintf(output_file,"     ->description \n");
							/* rig->page_list->page->device_list->device->description->name */
							fprintf(output_file,"      ->name %s\n",device->description->name);
							/* rig->page_list->page->device_list->device->description->type */
							fprintf(output_file,"      ->type ");
							switch(device->description->type)
							{							
								case ELECTRODE:
								{
									fprintf(output_file,"ELECTRODE\n");	
									/* rig->page_list->page->device_list->device->description->properties */
									fprintf(output_file,"      ->properties\n");
									fprintf(output_file,"        .electrode.position: x= %f, y= %f, z= %f\n",
										device->description->properties.electrode.position.x,
										device->description->properties.electrode.position.y,
										device->description->properties.electrode.position.z);
								}break;
								case AUXILIARY:
								{		
									fprintf(output_file,"AUXILIARY\n");	
									/* rig->page_list->page->device_list->device->description->properties */
									fprintf(output_file,"      ->properties\n");
									fprintf(output_file,"        .auxiliary.dummy: = %d\n",
										device->description->properties.auxiliary.dummy);
								}break;
								default:	
								{
									fprintf(output_file,"UNDEFINED\n");	
								}break;						
							}	/*switch(device->description->Device_type) */
							/* rig->page_list->page->device_list->device->description->region */
							fprintf(output_file,"      ->region\n");
							region=device->description->region;							
							/* rig->page_list->page->device_list->device->description->region->name  */
							fprintf(output_file,"       ->name : %s\n",region->name);
							/* rig->page_list->page->device_list->device->description->region->number */
							fprintf(output_file,"       ->number : %d\n",region->number);
							/* rig->page_list->page->device_list->device->description->region->type */
							fprintf(output_file,"       ->type  ");					
							switch(region->type)
							{							
								case MIXED:
								{
									fprintf(output_file,"MIXED\n");	
								}break;
								case PATCH:
								{
									fprintf(output_file,"PATCH\n");	
								}break;
								case SOCK:
								{	
									/* rig->page_list->page->device_list->device
										 ->description->region->properties.sock.focus */
									fprintf(output_file,"SOCK\n");								
									fprintf(output_file,"        focus for Hammer projection : %f\n",
										(region->properties).sock.focus);														
								}break;				 
								case TORSO:
								{
									fprintf(output_file,"TORSO\n");	
								}break;
								default:	
								{
									fprintf(output_file,"UNDEFINED\n");	
								}break;						
							} /* region->type */
							/* rig->page_list->page->device_list->device-
								 >description->region->number_of_devices */
							fprintf(output_file,"       ->number_of_devices : %d\n",
								region->number_of_devices);
							/* rig->page_list->page->device_list->device->chanel */
							fprintf(output_file,"     ->channel \n");
							/* rig->page_list->page->device_list->device->chanel->number */
							fprintf(output_file,"      ->number %d\n",device->channel->number);
							/* rig->page_list->page->device_list->device->chanel->gain */
							fprintf(output_file,"      ->gain %f\n",device->channel->gain);
							/* (*(rig->devices))->chanel->gain_correction */
							fprintf(output_file,"      ->gain_correction %f\n",
								device->channel->gain_correction);
							/* rig->page_list->page->device_list->device->chanel->offset */
							fprintf(output_file,"      ->offset %f\n",device->channel->offset);
							/* rig->page_list->page->device_list->device->signal */
							fprintf(output_file,"     ->signal ");
							if(device->signal)
							{	
								fprintf(output_file,"\n");
								/* rig->page_list->page->device_list->device->signal->status */
								fprintf(output_file,"      ->status ");
								switch(device->signal->status)
								{							
									case ACCEPTED:
									{
										fprintf(output_file,"ACCEPTED\n");	
									}break;
									case REJECTED:
									{
										fprintf(output_file,"REJECTED\n");	
									}break;
									case UNDECIDED:
									{									
										fprintf(output_file,"UNDECIDED\n");									 
									}break;				 							
									default:	
									{
										fprintf(output_file,"\n");	
									}break;						
								} /* device->channel->signal->status */
								/* rig->page_list->page->device_list->device->signal->buffer */
								fprintf(output_file,"      ->buffer\n");
								/* rig->page_list->page->device_list->device
									 ->signal->buffer->number_of_signals */
								fprintf(output_file,"       ->number_of_signals %d\n",
									device->signal->buffer->number_of_signals);
								/* rig->page_list->page->device_list->device
									 ->signal->buffer->number_of_samples */
								fprintf(output_file,"       ->number_of_signals %d\n",
									device->signal->buffer->number_of_samples);
								/* rig->page_list->page->device_list->device->signal->buffer->frequency */
								fprintf(output_file,"       ->number_of_signals %f\n",
									device->signal->buffer->frequency);
								/* rig->page_list->page->device_list->device->signal->buffer->times */
								fprintf(output_file,"       *(->times) %d\n",
									*device->signal->buffer->times);
								/* rig->page_list->page->device_list->device->signal->buffer->value_type */
								fprintf(output_file,"      ->value_type ");
								switch(device->signal->buffer->value_type)
								{							
									case SHORT_INT_VALUE:
									{
										fprintf(output_file,"SHORT_INT_VALUE\n");
										/* rig->page_list->page->device_list->device
											 ->signal->buffer->signals.short_int_values */
										fprintf(output_file,"       *(->signals.short_int_values)%d\n ",
											*(device->signal->buffer->signals.short_int_values) );
									}break;
									case FLOAT_VALUE:
									{
										fprintf(output_file,"FLOAT_VALUE\n");											
										/* rig->page_list->page->device_list->device
											 ->signal->buffer->signals.float_values */
										fprintf(output_file,"       *(->signals.float_values)%f\n ",
											*(device->signal->buffer->signals.float_values) );
									}break;									
									default:	
									{
										fprintf(output_file,"\n");	
									}break;						
								} /* device->signal->buffer->value_type  */
								/* rig->page_list->page->device_list->device->signal->buffer->start */
								fprintf(output_file,"       ->start %d\n",
									device->signal->buffer->start);
								/* rig->page_list->page->device_list->device->signal->buffer->end */
								fprintf(output_file,"       ->end %d\n",
									device->signal->buffer->end);
								/* rig->page_list->page->device_list->device->signal->index */
								fprintf(output_file,"      ->index %d\n ",device->signal->index);
								/* rig->page_list->page->device_list->device->signal->first_event */
								fprintf(output_file,"      ->first_event\n ");
								/* rig->page_list->page->device_list->device->signal->first_event->time */
								fprintf(output_file,"       ->time %d\n",
									device->signal->first_event->time);
								/* rig->page_list->page->device_list->device->signal->first_event->number */
								fprintf(output_file,"       ->number %d\n",
									device->signal->first_event->number);
								/* rig->page_list->page->device_list->device->signal->first_event->status */
								fprintf(output_file,"       ->status ");
								switch(device->signal->first_event->status)
								{
									case ACCEPTED:
									{
										fprintf(output_file,"ACCEPTED\n");	
									}break;
									case REJECTED:
									{
										fprintf(output_file,"REJECTED\n");	
									}break;
									case UNDECIDED:
									{									
										fprintf(output_file,"UNDECIDED\n");									 
									}break;				 							
									default:	
									{
										fprintf(output_file,"\n");	
									}break;						
								}/* switch(device->signal->first_event->status) */
								/* rig->page_list->page->device_list->device->signal->first_event->next */
								fprintf(output_file,"       ->next %p\n",
									device->signal->first_event->next);
								/* rig->page_list->page->device_list->device
									 ->signal->first_event->previous */
								fprintf(output_file,"       ->previous %p\n",
									device->signal->first_event->previous);								
								/* rig->page_list->page->device_list->device->signal->next */
								fprintf(output_file,"      ->next %p \n ",device->signal->next);
								/* rig->page_list->page->device_list->device->signal->number */
								fprintf(output_file,"      ->number%d\n ",device->signal->number);
							}
							else
							{
								fprintf(output_file," = NULL \n");
							}
							/* rig->page_list->page->device_list->device->signal_display_maximum */						
							fprintf(output_file,"     ->signal_display_maximum %f\n",
								device->signal_display_maximum);
							/* rig->page_list->page->device_list->device->signal_display_minimum */
							fprintf(output_file,"     ->signal_miniimum %f\n",
								device->signal_display_minimum);
							/* rig->page_list->page->device_list->device->highlight */
							fprintf(output_file,"     ->highlight %d\n",device->highlight);
							 
							device_item = device_item->next;
						}
						page_item = page_item->next;
					}
					/* rig->number_of_regions */
					fprintf(output_file,"  ->number_of_regions : %d\n",rig->number_of_regions);	
					/* rig->region_list*/
					fprintf(output_file,"  ->region_list \n");	
					region_item=rig->region_list;					
					number_of_regions=rig->number_of_regions;				
					while (number_of_regions>0)
					{	
						fprintf(output_file,"   Region %d\n",number_of_regions);
						region=region_item->region;		
						/* rig->region_list->region->name */
						fprintf(output_file,"    ->name : %s\n",region->name);	
						/* rig->region_list->region->number */
						fprintf(output_file,"    ->number : %d\n",region->number);
						/* rig->region_list->region->type */
						fprintf(output_file,"    ->type  ");
						switch(region->type)
						{							
							case MIXED:
							{
								fprintf(output_file,"MIXED\n");	
							}break;
							case PATCH:
							{
								fprintf(output_file,"PATCH\n");	
							}break;
							case SOCK:
							{
								/* rig->region_list->region->properties.sock.focus */
								fprintf(output_file,"SOCK\n");								
								fprintf(output_file,"    focus for Hammer projection : %f\n",
									(region->properties).sock.focus);														
							}break;				 
							case TORSO:
							{
								fprintf(output_file,"TORSO\n");	
							}break;
							default:	
							{
								fprintf(output_file,"UNDEFINED\n");	
							}break;						
						} /* region->type */	
						/* rig->region_list->region->number_of_devices */
						fprintf(output_file,"    ->number_of_devices : %d\n",region->number_of_devices);
						
						region_item=region_item->next;
						number_of_regions--;
					
					}/* while (number_of_regions>0) */	
					/* rig->current_region */
					region=rig->current_region;
					fprintf(output_file,"  ->current_region\n");	
					/* rig->current_region->name  */
					fprintf(output_file,"   ->name : %s\n",region->name);
					/* rig->current_region->number */
					fprintf(output_file,"   ->number : %d\n",region->number);
					/* rig->current_region->type */
					fprintf(output_file,"   ->type  ");					
					switch(region->type)
					{							
						case MIXED:
						{
							fprintf(output_file,"MIXED\n");	
						}break;
						case PATCH:
						{
							fprintf(output_file,"PATCH\n");	
						}break;
						case SOCK:
						{	
							/* rig->current_region->properties.sock.focus */
							fprintf(output_file,"SOCK\n");								
							fprintf(output_file,"    focus for Hammer projection : %f\n",
								(region->properties).sock.focus);														
						}break;				 
						case TORSO:
						{
							fprintf(output_file,"TORSO\n");	
						}break;
						default:	
						{
							fprintf(output_file,"UNDEFINED\n");	
						}break;						
					} /* region->type */
					/* rig->current_region->number_of_devices */
					fprintf(output_file,"   ->number_of_devices : %d\n",
						region->number_of_devices);
					/* rig->signal_file_name */	
					fprintf(output_file,"  ->signal_file_name : %s\n",
						rig->signal_file_name);
#if defined (OLD_CODE)
???DB.  Only read calibration when doing acquisition */
					/* rig->calibration_directory */
					fprintf(output_file,"  ->calibration_directory : %s\n",
						rig->calibration_directory);					
#endif /* defined (OLD_CODE) */
				} break; /* case TEXT	*/
				default:
				{
					display_message(ERROR_MESSAGE,
						"show_config.  Invalid file type");
					return_code=0;
				} break; /*default: */
			}/* switch (file_type) */
		}	
		else /*	if (rig) */
		{
			display_message(ERROR_MESSAGE,"show_config.  rig is NULL");
			return_code=0;
		}
	}
	else/* if (output_file) */
	{
		display_message(ERROR_MESSAGE,"show_config. output_file is NULL");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* show_config */

int show_config_file(char *file_name,void *rig_pointer)
/*******************************************************************************
LAST MODIFIED : 31 April 1999

DESCRIPTION :
Similar to write_configuration_file() but in more detail.

Writes the configuration of the <rig> to the named file in text format.  It
returns a non-zero if successful and zero if unsuccessful.
==============================================================================*/
{
	int return_code;
	FILE *output_file;

	ENTER(show_config_file);
	if (output_file=fopen(file_name,"w"))
	{
		return_code=
			show_config(*((struct Rig **)rig_pointer),output_file,TEXT);
		fclose(output_file);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"show_config_file.  Could not open file: %s",file_name);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* show_config_file */
#endif /* DEBUG */

struct Signal *create_Signal(int index,struct Signal_buffer *buffer,
	enum Event_signal_status status,int number)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
This function allocates memory for a signal and initializes the fields to the
specified values.  It returns a pointer to the created signal if successful and
NULL if unsuccessful.
==============================================================================*/
{
	struct Signal *signal;

	ENTER(create_Signal);
	if (ALLOCATE(signal,struct Signal,1))
	{
		signal->index=index;
		signal->buffer=buffer;
		signal->first_event=(struct Event *)NULL;
		signal->status=status;
		signal->next=(struct Signal *)NULL;
		signal->number=number;
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Signal.  Could not allocate signal");
	}
	LEAVE;

	return (signal);
} /* create_Signal */

int destroy_Signal(struct Signal **signal)
/*******************************************************************************
LAST MODIFIED : 30 May 1992

DESCRIPTION :
This function frees the memory for <**signal> and changes <*signal> to NULL.
??? To free the buffer would need to chnage to a handle (**) ?
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Signal);
	return_code=1;
	if (*signal)
	{
		DEALLOCATE(*signal);
	}
	LEAVE;

	return (return_code);
} /* destroy_Signal */

int Signal_get_min_max(struct Signal *signal,float *min,float *max,int time_range)
/*******************************************************************************
LAST MODIFIED : 4 August 2000

DESCRIPTION :
Get the minimum and maximun of <signal>, returned in <min> <max>.
If <time_range> is set, determine min, max over the selected time range.
If <time_range> is 0, determine min, max over the signal's entire time range.
==============================================================================*/
{
	float value,minimum,maximum;
	int count,end_count,number_of_samples,number_of_signals,return_code,start_count,index;
	struct Signal_buffer *signal_buffer;

	ENTER(Signal_get_min_max);
	return_code=0;
	if(signal&&(signal_buffer=signal->buffer))
	{
		return_code=1;	
		number_of_samples=signal_buffer->number_of_samples;
		number_of_signals=signal_buffer->number_of_signals;		
		if(time_range)
		{
			start_count=signal_buffer->start;
			end_count=signal_buffer->end;
		}
		else
		{
			start_count=0;
			end_count=number_of_samples;
		}
		index=signal->index+(start_count*number_of_signals);
		switch(signal_buffer->value_type)
		{
			case SHORT_INT_VALUE:
			{
				minimum=(float)(signal_buffer->signals.short_int_values[index]);
			}break;
			case FLOAT_VALUE:
			{
				minimum=signal_buffer->signals.float_values[index];				
			}break;			
		}
		maximum=minimum;
		for(count=start_count;count<end_count;count++)
		{			
			switch(signal_buffer->value_type)
			{
				case SHORT_INT_VALUE:
				{
					value=(float)(signal_buffer->signals.short_int_values[index]);
				}break;
				case FLOAT_VALUE:
				{
					value=signal_buffer->signals.float_values[index];									
				}break;
			}		
			if(value>maximum)
			{
				maximum=value;			
			}
			if(value<minimum)
			{
				minimum=value;			
			}				
			index=index+number_of_signals;
		}		
		*min=minimum;
		*max=maximum;	
	}
	else
	{
		*min=0;
		*max=0;
		display_message(ERROR_MESSAGE,
			"Signal_get_min_max.  Invalid arguments");
	}
	LEAVE;
	return(return_code);
}/* Signal_get_min_max */

struct Signal_buffer *create_Signal_buffer(enum Signal_value_type value_type,
	int number_of_signals,int number_of_samples,float frequency)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
This function allocates memory for a signal buffer to hold the specified
<value_type>, <number_of_signals> and <number_of_samples>.  It returns a pointer
to the created signal buffer if successful and NULL if unsuccessful.
==============================================================================*/
{
	struct Signal_buffer *buffer;

	ENTER(create_Signal_buffer);
	/* check arguments */
	if (((SHORT_INT_VALUE==value_type)||(FLOAT_VALUE==value_type))&&
		(number_of_signals>0)&&(number_of_samples>0)&&(frequency>0))
	{
		if (ALLOCATE(buffer,struct Signal_buffer,1))
		{
			if (ALLOCATE(buffer->times,int,number_of_samples))
			{
				if (((SHORT_INT_VALUE==value_type)&&
					ALLOCATE(buffer->signals.short_int_values,short int,
					number_of_samples*number_of_signals))||
					((FLOAT_VALUE==value_type)&&
					ALLOCATE(buffer->signals.float_values,float,
					number_of_samples*number_of_signals)))
				{
					buffer->number_of_signals=number_of_signals;
					buffer->number_of_samples=number_of_samples;
					buffer->frequency=frequency;
					buffer->start=0;
					buffer->end=number_of_samples-1;
					buffer->value_type=value_type;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Signal_buffer.  Could not allocate signals");
					DEALLOCATE(buffer->times);
					DEALLOCATE(buffer);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Signal_buffer.  Could not allocate times");
				DEALLOCATE(buffer);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Signal_buffer.  Could not allocate buffer");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Signal_buffer.  Could not allocate buffer");
		buffer=(struct Signal_buffer *)NULL;
	}
	LEAVE;

	return (buffer);
} /* create_Signal_buffer */

struct Signal_buffer *reallocate_Signal_buffer(
	struct Signal_buffer *signal_buffer,enum Signal_value_type value_type,
	int number_of_signals,int number_of_samples,float frequency)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
This function reallocates memory for a signal <buffer> to hold the specified
<value_type>, <number_of_signals> and <number_of_samples>.  It returns a pointer
to the created signal buffer if successful and NULL if unsuccessful.
==============================================================================*/
{
	float *float_values;
	int *new_times,*times;
	short int *short_int_values;
	struct Signal_buffer *buffer;

	ENTER(reallocate_Signal_buffer);
	/* check arguments */
	if (((SHORT_INT_VALUE==value_type)||(FLOAT_VALUE==value_type))&&
		(number_of_signals>0)&&(number_of_samples>0)&&(frequency>0))
	{
		if (buffer=signal_buffer)
		{
			if (number_of_samples!=buffer->number_of_samples)
			{
				ALLOCATE(new_times,int,number_of_samples);
				times=new_times;
			}
			else
			{
				new_times=(int *)NULL;
				times=buffer->times;
			}
			if (times)
			{
				if ((value_type!=buffer->value_type)||
					(number_of_signals!=buffer->number_of_signals)||
					(number_of_samples!=buffer->number_of_samples))
				{
					if (SHORT_INT_VALUE==value_type)
					{
						if (SHORT_INT_VALUE==buffer->value_type)
						{
							float_values=(float *)NULL;
							REALLOCATE(short_int_values,buffer->signals.short_int_values,
								short int,number_of_samples*number_of_signals);
						}
						else
						{
							float_values=(float *)NULL;
							REALLOCATE(short_int_values,buffer->signals.float_values,
								short int,number_of_samples*number_of_signals);
						}
					}
					else
					{
						if (SHORT_INT_VALUE==buffer->value_type)
						{
							REALLOCATE(float_values,buffer->signals.short_int_values,float,
								number_of_samples*number_of_signals);
							short_int_values=(short int *)NULL;
						}
						else
						{
							REALLOCATE(float_values,buffer->signals.float_values,float,
								number_of_samples*number_of_signals);
							short_int_values=(short int *)NULL;
						}
					}
				}
				else
				{
					if (SHORT_INT_VALUE==value_type)
					{
						float_values=(float *)NULL;
						short_int_values=buffer->signals.short_int_values;
					}
					else
					{
						float_values=buffer->signals.float_values;
						short_int_values=(short int *)NULL;
					}
				}
				if (float_values||short_int_values)
				{
					buffer->number_of_signals=number_of_signals;
					buffer->number_of_samples=number_of_samples;
					buffer->frequency=frequency;
					buffer->start=0;
					buffer->end=number_of_samples-1;
					buffer->value_type=value_type;
					if (new_times)
					{
						DEALLOCATE(buffer->times);
						buffer->times=new_times;
					}
					if (SHORT_INT_VALUE==value_type)
					{
						buffer->signals.short_int_values=short_int_values;
					}
					else
					{
						buffer->signals.float_values=float_values;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"reallocate_Signal_buffer.  Could not allocate signals");
					DEALLOCATE(new_times);
					buffer=(struct Signal_buffer *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"reallocate_Signal_buffer.  Could not allocate times");
				buffer=(struct Signal_buffer *)NULL;
			}
		}
		else
		{
			buffer=create_Signal_buffer(value_type,number_of_signals,
				number_of_samples,frequency);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"reallocate_Signal_buffer.  Invalid argument(s).  %d (%d %d) %d %d %g",
			value_type,SHORT_INT_VALUE,FLOAT_VALUE,number_of_signals,
			number_of_samples,frequency);
		buffer=(struct Signal_buffer *)NULL;
	}
	LEAVE;

	return (buffer);
} /* reallocate_Signal_buffer */

int destroy_Signal_buffer(struct Signal_buffer **buffer)
/*******************************************************************************
LAST MODIFIED : 31 August 1993

DESCRIPTION :
This function frees the memory associated with the fields of <**buffer>, frees
the memory for <**buffer> and changes <*buffer> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Signal_buffer);
	return_code=1;
	if (*buffer)
	{
		DEALLOCATE((*buffer)->times);
		switch ((*buffer)->value_type)
		{
			case SHORT_INT_VALUE:
			{
				DEALLOCATE((*buffer)->signals.short_int_values);
			} break;
			case FLOAT_VALUE:
			{
				DEALLOCATE((*buffer)->signals.float_values);
			} break;
		}
		DEALLOCATE(*buffer);
	}
	LEAVE;

	return (return_code);
} /* destroy_Signal_buffer */

int read_signal_file(FILE *input_file,struct Rig **rig_pointer
#if defined (UNEMAP_USE_3D)
			,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
			)
/*******************************************************************************
LAST MODIFIED : 29 July 2000

DESCRIPTION :
This function reads in a rig configuration and an interval of signal data from
the <input_file>.
???DB.  What about multiple signals ?
==============================================================================*/
{
	enum Region_type rig_type;
	enum Signal_value_type signal_value_type;
	float frequency,*buffer_value;
	int fread_result,i,index,number_of_devices,number_of_samples,
		number_of_signals,return_code,temp_int;
	struct Device **device;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(read_signal_file);
	/* check the arguments */
	if (input_file&&rig_pointer)
	{
		/* read the rig type */
		if (1==BINARY_FILE_READ((char *)&rig_type,sizeof(enum Region_type),1,
			input_file))
		{
#if defined (DEBUG)
			/*???debug */
			printf("read_signal_file %d %d %d %d %d\n",rig_type,MIXED,SOCK,PATCH,
				TORSO);
#endif /* defined (DEBUG) */
			/* if the rig type is valid */
			if ((SOCK==rig_type)||(PATCH==rig_type)||(MIXED==rig_type)||
				(TORSO==rig_type))
			{
				/* read the the configuration file */
				if (rig=read_configuration(input_file,BINARY,rig_type
#if defined (UNEMAP_USE_3D)
					,unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
					))
				{
					/* read the number of signals */
					if (1==BINARY_FILE_READ((char *)&number_of_signals,sizeof(int),1,
						input_file))
					{
						/*???DB.  Done this way to maintain backward compatibility */
						if (number_of_signals<0)
						{
							signal_value_type=FLOAT_VALUE;
							number_of_signals= -number_of_signals;
						}
						else
						{
							signal_value_type=SHORT_INT_VALUE;
						}
						/* read the number of samples */
						if (1==BINARY_FILE_READ((char *)&number_of_samples,sizeof(int),1,
							input_file))
						{
							/* read the sampling frequency (Hz) */
							if (1==BINARY_FILE_READ((char *)&frequency,sizeof(float),1,
								input_file))
							{
								/* create the signal buffer */
								if (buffer=create_Signal_buffer(signal_value_type,
									number_of_signals,number_of_samples,frequency))
								{
									/* read the sample times */
									if (number_of_samples==(int)BINARY_FILE_READ(
										(char *)buffer->times,sizeof(int),number_of_samples,
										input_file))
									{
										/* read the signals */
										switch (signal_value_type)
										{
											case SHORT_INT_VALUE:
											{
												fread_result=BINARY_FILE_READ((char *)buffer->signals.
													short_int_values,sizeof(short int),
													number_of_samples*number_of_signals,input_file);
											} break;
											case FLOAT_VALUE:
											{
												fread_result=BINARY_FILE_READ((char *)buffer->signals.
													float_values,sizeof(float),
													number_of_samples*number_of_signals,input_file);
												/* check signal values.  If it's not a valid float, set
													 it to 0  */																						
												buffer_value=buffer->signals.float_values;
												for (i=0;i<number_of_samples*number_of_signals;i++)
												{			
													/* check if data is valid finite() checks inf and
														nan */
													/* finite() in math.h for Linux, ieeefp.h for Irix */
#if defined (WIN32)
													if (!_finite((double)(*buffer_value)))
#else /* defined (WIN32) */
													if (!finite((double)(*buffer_value)))
#endif /* defined (WIN32) */
													{
														*buffer_value=(float)0;
														display_message(ERROR_MESSAGE,
			"read_signal_file.  Signal value is infinite or not a number.  Set to 0");
													}
													buffer_value++;
												}							
											} break;
										}
										if (fread_result==number_of_samples*number_of_signals)
										{
											/* read the device signal indicies and channel
												characteristics */
											number_of_devices=rig->number_of_devices;
											device=rig->devices;
											return_code=1;
											while (return_code&&(number_of_devices>0))
											{
												/* don't read anything for auxiliary devices that are a
													linear combination */
												if ((*device)->channel)
												{
													if ((1==BINARY_FILE_READ((char *)&temp_int,
														sizeof(int),1,input_file))&&(1==BINARY_FILE_READ(
														(char *)&((*device)->channel->offset),sizeof(float),
														1,input_file))&&(1==BINARY_FILE_READ(
														(char *)&((*device)->channel->gain),sizeof(float),1,
														input_file)))
													{
														(*device)->channel->gain_correction=(float)1;
														/* allow multiple signals for each device */
														if (temp_int<0)
														{
															index= -(temp_int+1);
														}
														else
														{
															index=temp_int;
														}
														if (0==(*device)->channel->gain)
														{
															(*device)->channel->gain=(float)1;
															(*device)->channel->offset=(float)0;
															display_message(ERROR_MESSAGE,
						"read_signal_file.  Zero gain - setting offset to 0 and gain to 1");
														}
														if (ELECTRODE==(*device)->description->type)
														{
															if (!((*device)->signal=
																create_Signal(index,buffer,UNDECIDED,0)))
															{
																return_code=0;
																destroy_Signal_buffer(&buffer);
																destroy_Rig(&rig);
																return_code=0;
															}
														}
														else
														{
															if (!((*device)->signal=
																create_Signal(index,buffer,REJECTED,0)))
															{
																return_code=0;
																destroy_Signal_buffer(&buffer);
																destroy_Rig(&rig);
																return_code=0;
															}
														}
														/* allow multiple signals for each device */
														if (return_code)
														{
															signal=(*device)->signal;
															while (return_code&&(temp_int<0))
															{
																if (1==BINARY_FILE_READ((char *)&temp_int,
																	sizeof(int),1,input_file))
																{
																	if (temp_int<0)
																	{
																		index= -(temp_int+1);
																	}
																	else
																	{
																		index=temp_int;
																	}
																	if (ELECTRODE==(*device)->description->type)
																	{
																		if (signal->next=
																			create_Signal(index,buffer,UNDECIDED,0))
																		{
																			signal=signal->next;
																		}
																		else
																		{
																			return_code=0;
																			destroy_Signal_buffer(&buffer);
																			destroy_Rig(&rig);
																			return_code=0;
																		}
																	}
																	else
																	{
																		if (signal->next=
																			create_Signal(index,buffer,REJECTED,0))
																		{
																			signal=signal->next;
																		}
																		else
																		{
																			return_code=0;
																			destroy_Signal_buffer(&buffer);
																			destroy_Rig(&rig);
																			return_code=0;
																		}
																	}
																}
																else
																{
																	display_message(ERROR_MESSAGE,
												"read_signal_file.  Error reading file - signal index");
																	destroy_Signal_buffer(&buffer);
																	destroy_Rig(&rig);
																	return_code=0;
																}
															}
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
									"read_signal_file.  Error reading file - offsets and gains");
														destroy_Signal_buffer(&buffer);
														destroy_Rig(&rig);
														return_code=0;
													}
												}
												device++;
												number_of_devices--;
											}
											if (return_code&&(number_of_devices>0))
											{
												display_message(ERROR_MESSAGE,
													"read_signal_file.  Missing devices");
												destroy_Signal_buffer(&buffer);
												destroy_Rig(&rig);
												return_code=0;
											}
											if (return_code)
											{
												*rig_pointer=rig;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_signal_file.  Error reading file - signals");
											destroy_Signal_buffer(&buffer);
											destroy_Rig(&rig);
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_signal_file.  Error reading file - times");
										destroy_Signal_buffer(&buffer);
										destroy_Rig(&rig);
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_signal_file.  Could not create signal buffer");
									destroy_Rig(&rig);
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_signal_file.  Error reading file - frequency");
								destroy_Rig(&rig);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_signal_file.  Error reading file - number of samples");
							destroy_Rig(&rig);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_signal_file.  Error reading file - number of signals");
						destroy_Rig(&rig);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_signal_file.  Could not read configuration");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"read_signal_file.  Invalid rig type");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_signal_file.  Error reading file - rig type");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_signal_file.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_signal_file */

#if defined (SAVE_WHOLE_BUFFER)
int write_signal_file(char *file_name,void *rig_pointer)
/*******************************************************************************
LAST MODIFIED : 4 March 2001

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
???DB.  What about multiple signal buffers ?
==============================================================================*/
{
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *buffer;
	FILE *output_file;
	int index,number_of_devices,number_of_samples,number_of_signals,return_code,
		temp_int;
	struct Device **device;

	ENTER(write_signal_file);
	/* check that the rig exists */
	if (rig= *((struct Rig **)rig_pointer))
	{
		/* open the output file */
		if (output_file=fopen(file_name,"wb"))
		{
			/* write the the configuration file */
			if (write_configuration(rig,output_file,BINARY))
			{
				/* check the rig device list */
				if (device=rig->devices)
				{
					/* check the  signal buffer */
					if (buffer=get_Device_signal_buffer(*device))
					{
						number_of_signals=buffer->number_of_signals;
						number_of_samples=(buffer->end)-(buffer->start)+1;
						/*???DB.  Done this way to maintain backward compatability */
						switch (buffer->value_type)
						{
							case SHORT_INT_VALUE:
							{
								temp_int=number_of_signals;
							} break;
							case FLOAT_VALUE:
							{
								temp_int= -number_of_signals;
							} break;
						}
						/*???DB.  Change for circular ? */
						if ((1==BINARY_FILE_WRITE((char *)&temp_int,sizeof(int),1,
							output_file))&&(1==BINARY_FILE_WRITE((char *)&number_of_samples,
							sizeof(int),1,output_file))&&
							(1==BINARY_FILE_WRITE((char *)&(buffer->frequency),sizeof(float),
							1,output_file)))
						{
							if (number_of_samples==
								BINARY_FILE_WRITE((char *)((buffer->times)+(buffer->start)),
								sizeof(int),number_of_samples,output_file))
							{
								switch (buffer->value_type)
								{
									case SHORT_INT_VALUE:
									{
										temp_int=BINARY_FILE_WRITE((char *)((buffer->signals.
											short_int_values)+(buffer->start)*number_of_signals),
											sizeof(short int),number_of_samples*number_of_signals,
											output_file);
									} break;
									case FLOAT_VALUE:
									{
										temp_int=BINARY_FILE_WRITE((char *)((buffer->signals.
											float_values)+(buffer->start)*number_of_signals),
											sizeof(float),number_of_samples*number_of_signals,
											output_file);
									} break;
								}
								if (number_of_signals*number_of_samples==temp_int)
								{
									/* write out the device signal indicies and channel
										characteristics */
									number_of_devices=rig->number_of_devices;
									return_code=1;
									while (return_code&&(number_of_devices>0))
									{
										/* don't write anything for an auxiliary device that is a
											linear combination */
										if ((*device)->channel)
										{
											/* a negative index indicates that there is another signal
												for the device */
											index=(*device)->signal->index;
											if (signal=(*device)->signal->next)
											{
												index= -index;
											}
											if ((1==BINARY_FILE_WRITE((char *)&index,sizeof(int),1,
												output_file))&&
												(1==BINARY_FILE_WRITE((char *)&((*device)->channel->
												offset),sizeof(float),1,output_file))&&
												(1==BINARY_FILE_WRITE((char *)&((*device)->channel->
												gain),sizeof(float),1,output_file)))
											{
												/* write multiple signals */
												while (return_code&&signal)
												{
													index=signal->index;
													if (signal=signal->next)
													{
														index= -(index+1);
													}
													if (1!=BINARY_FILE_WRITE((char *)&index,sizeof(int),1,
														output_file))
													{
														display_message(ERROR_MESSAGE,
															"write_signal_file.  Error writing index");
														return_code=0;
													}
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
												"write_signal_file.  Error writing offsets and gains");
												return_code=0;
											}
										}
										device++;
										number_of_devices--;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"write_signal_file.  Error writing signals");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"write_signal_file.  Error writing times");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_signal_file.  Error writing %s",
								"number of samples, number of signals or frequency");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"write_signal_file.  signal buffer is missing");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_signal_file.  rig device list is missing");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_signal_file.  Could not write configuration");
				return_code=0;
			}
			fclose(output_file);
			if (!return_code)
			{
				remove(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"write_signal_file.  Invalid file: %s",
				file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_signal_file.  rig missing");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_signal_file */
#else /* defined (SAVE_WHOLE_BUFFER) */
int write_signal_file(FILE *output_file,struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 4 March 2001

DESCRIPTION :
This function writes the <rig> configuration and interval of signal data to the
<output_file>.
???DB.  What about multiple signal buffers ?
==============================================================================*/
{
	float *float_sample;
	int i,index,j,number_of_devices,number_of_samples,number_of_signals,
		return_code,temp_int;
	short int *short_int_sample;
	struct Device **device;
	struct Signal *signal;
	struct Signal_buffer *buffer;

	ENTER(write_signal_file);
	/* check the arguments */
	if (output_file&&rig)
	{
		/* write the the configuration file */
		if (write_configuration(rig,output_file,BINARY))
		{
			/* check the rig device list */
			if (device=rig->devices)
			{
				/* check the signal buffer */
				if ((buffer=get_Device_signal_buffer(*device))&&
					((number_of_devices=rig->number_of_devices)>0))
				{
					/* count the number of signals to be output */
					temp_int=0;
					device=rig->devices;
					for (j=number_of_devices;j>0;j--)
					{
						signal=(*device)->signal;
						while (signal)
						{
							temp_int++;
							signal=signal->next;
						}
						device++;
					}
					number_of_signals=buffer->number_of_signals;
					/*???DB.  Done this way to maintain backward compatability */
					switch (buffer->value_type)
					{
						case FLOAT_VALUE:
						{
							temp_int= -temp_int;
						} break;
					}
					if (buffer->end>=buffer->start)
					{
						number_of_samples=(buffer->end)-(buffer->start)+1;
						if ((1==BINARY_FILE_WRITE((char *)&temp_int,sizeof(int),1,
							output_file))&&(1==BINARY_FILE_WRITE((char *)&number_of_samples,
							sizeof(int),1,output_file))&&
							(1==BINARY_FILE_WRITE((char *)&(buffer->frequency),sizeof(float),
							1,output_file)))
						{
							/* write out the times */
							if (number_of_samples==(int)BINARY_FILE_WRITE(
								(char *)((buffer->times)+(buffer->start)),sizeof(int),
								number_of_samples,output_file))
							{
								/* write out the samples */
								switch (buffer->value_type)
								{
									case SHORT_INT_VALUE:
									{
										return_code=1;
										short_int_sample=(buffer->signals.short_int_values)+
											(buffer->start)*number_of_signals;
										i=number_of_samples;
										while (return_code&&(i>0))
										{
											device=rig->devices;
											j=number_of_devices;
											while (return_code&&(j>0))
											{
												signal=(*device)->signal;
												while (return_code&&signal)
												{
													if (1==BINARY_FILE_WRITE((char *)(short_int_sample+
														(signal->index)),sizeof(short int),1,output_file))
													{
														signal=signal->next;
													}
													else
													{
														return_code=0;
													}
												}
												if (return_code)
												{
													device++;
													j--;
												}
											}
											short_int_sample += number_of_signals;
											i--;
										}
									} break;
									case FLOAT_VALUE:
									{
										return_code=1;
										float_sample=(buffer->signals.float_values)+
											(buffer->start)*number_of_signals;
										i=number_of_samples;
										while (return_code&&(i>0))
										{
											device=rig->devices;
											j=number_of_devices;
											while (return_code&&(j>0))
											{
												signal=(*device)->signal;
												while (return_code&&signal)
												{
													if (1==BINARY_FILE_WRITE((char *)(float_sample+
														(signal->index)),sizeof(float),1,output_file))
													{
														signal=signal->next;
													}
													else
													{
														return_code=0;
													}
												}
												if (return_code)
												{
													device++;
													j--;
												}
											}
											float_sample += number_of_signals;
											i--;
										}
									} break;
									default:
									{
										return_code=0;
									} break;
								}
								if (!return_code)
								{
									display_message(ERROR_MESSAGE,
										"write_signal_file.  Error writing signals");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"write_signal_file.  Error writing times");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_signal_file.  Error writing %s",
								"number of samples, number of signals or frequency");
							return_code=0;
						}
					}
					else
					{
						number_of_samples=
							(buffer->number_of_samples)-(buffer->start)+(buffer->end)+1;
						if ((1==BINARY_FILE_WRITE((char *)&temp_int,sizeof(int),1,
							output_file))&&(1==BINARY_FILE_WRITE((char *)&number_of_samples,
							sizeof(int),1,output_file))&&
							(1==BINARY_FILE_WRITE((char *)&(buffer->frequency),sizeof(float),
							1,output_file)))
						{
							/* write out the times */
							if (((buffer->number_of_samples)-(buffer->start)==
								(int)BINARY_FILE_WRITE(
								(char *)((buffer->times)+(buffer->start)),sizeof(int),
								(buffer->number_of_samples)-(buffer->start),output_file))&&
								((buffer->end)+1==(int)BINARY_FILE_WRITE(
								(char *)(buffer->times),sizeof(int),(buffer->end)+1,
								output_file)))
							{
								/* write out the samples */
								switch (buffer->value_type)
								{
									case SHORT_INT_VALUE:
									{
										return_code=1;
										short_int_sample=(buffer->signals.short_int_values)+
											(buffer->start)*number_of_signals;
										i=(buffer->number_of_samples)-(buffer->start);
										while (return_code&&(i>0))
										{
											device=rig->devices;
											j=number_of_devices;
											while (return_code&&(j>0))
											{
												signal=(*device)->signal;
												while (return_code&&signal)
												{
													if (1==BINARY_FILE_WRITE((char *)(short_int_sample+
														(signal->index)),sizeof(short int),1,output_file))
													{
														signal=signal->next;
													}
													else
													{
														return_code=0;
													}
												}
												if (return_code)
												{
													device++;
													j--;
												}
											}
											short_int_sample += number_of_signals;
											i--;
										}
										short_int_sample=buffer->signals.short_int_values;
										i=(buffer->end)+1;
										while (return_code&&(i>0))
										{
											device=rig->devices;
											j=number_of_devices;
											while (return_code&&(j>0))
											{
												signal=(*device)->signal;
												while (return_code&&signal)
												{
													if (1==BINARY_FILE_WRITE((char *)(short_int_sample+
														(signal->index)),sizeof(short int),1,output_file))
													{
														signal=signal->next;
													}
													else
													{
														return_code=0;
													}
												}
												if (return_code)
												{
													device++;
													j--;
												}
											}
											short_int_sample += number_of_signals;
											i--;
										}
									} break;
									case FLOAT_VALUE:
									{
										return_code=1;
										float_sample=(buffer->signals.float_values)+
											(buffer->start)*number_of_signals;
										i=(buffer->number_of_samples)-(buffer->start);
										while (return_code&&(i>0))
										{
											device=rig->devices;
											j=number_of_devices;
											while (return_code&&(j>0))
											{
												signal=(*device)->signal;
												while (return_code&&signal)
												{
													if (1==BINARY_FILE_WRITE((char *)(float_sample+
														(signal->index)),sizeof(float),1,output_file))
													{
														signal=signal->next;
													}
													else
													{
														return_code=0;
													}
												}
												if (return_code)
												{
													device++;
													j--;
												}
											}
											float_sample += number_of_signals;
											i--;
										}
										float_sample=buffer->signals.float_values;
										i=(buffer->end)+1;
										while (return_code&&(i>0))
										{
											device=rig->devices;
											j=number_of_devices;
											while (return_code&&(j>0))
											{
												signal=(*device)->signal;
												while (return_code&&signal)
												{
													if (1==BINARY_FILE_WRITE((char *)(float_sample+
														(signal->index)),sizeof(float),1,output_file))
													{
														signal=signal->next;
													}
													else
													{
														return_code=0;
													}
												}
												if (return_code)
												{
													device++;
													j--;
												}
											}
											float_sample += number_of_signals;
											i--;
										}
									} break;
									default:
									{
										return_code=0;
									} break;
								}
								if (!return_code)
								{
									display_message(ERROR_MESSAGE,
										"write_signal_file.  Error writing signals");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"write_signal_file.  Error writing times");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_signal_file.  Error writing %s",
								"number of samples, number of signals or frequency");
							return_code=0;
						}
					}
					if (return_code)
					{
						/* write out the device signal indicies and channel
							characteristics */
						device=rig->devices;
						i=0;
						j=0;
						while (return_code&&(j<number_of_devices))
						{
							/* don't write anything for an auxiliary device that is a linear
								combination */
							if ((*device)->channel)
							{
								/* a negative index indicates that there is another signal for
									the device */
								index=i;
								if (signal=(*device)->signal->next)
								{
									index= -(index+1);
								}
								if ((1==BINARY_FILE_WRITE((char *)&index,sizeof(int),1,
									output_file))&&
									(1==BINARY_FILE_WRITE((char *)&((*device)->channel->offset),
									sizeof(float),1,output_file))&&
									(1==BINARY_FILE_WRITE((char *)&((*device)->channel->gain),
									sizeof(float),1,output_file)))
								{
									i++;
									/* write multiple signals */
									while (return_code&&signal)
									{
										index=i;
										if (signal=signal->next)
										{
											index= -(index+1);
										}
										if (1==BINARY_FILE_WRITE((char *)&index,sizeof(int),1,
											output_file))
										{
											i++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"write_signal_file.  Error writing index");
											return_code=0;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"write_signal_file.  Error writing offsets and gains");
									return_code=0;
								}
							}
							device++;
							j++;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_signal_file.  signal buffer is missing");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_signal_file.  rig device list is missing");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_signal_file.  Could not write configuration");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_signal_file.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_signal_file */
#endif /* defined (SAVE_WHOLE_BUFFER) */

struct Event *create_Event(int time,int number,enum Event_signal_status status,
	struct Event *previous,struct Event *next)
/*******************************************************************************
LAST MODIFIED : 31 December 1996

DESCRIPTION :
This function allocates memeory for an event and initializes the fields to the
specified values.  It returns a pointer to the created event if successful and
NULL if unsuccessful.
==============================================================================*/
{
	struct Event *event;

	ENTER(create_Event);
	if (ALLOCATE(event,struct Event,1))
	{
		event->time=time;
		event->number=number;
		event->status=status;
		event->previous=previous;
		event->next=next;
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Event.  Could not allocate memory");
	}
	LEAVE;

	return (event);
} /* create_Event */

int destroy_Event_list(struct Event **first_event)
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
This function frees the memory associated with the event list starting at
<**first_event> and sets <*first_event> to NULL.
==============================================================================*/
{
	int return_code;
	struct Event *event,*next_event;

	ENTER(destroy_Event_list);
	return_code=1;
	if (first_event&&(event= *first_event))
	{
		do
		{
			next_event=event->next;
			DEALLOCATE(event);
		} while (event=next_event);
		*first_event=(struct Event *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Event_list */

int destroy_all_events(struct Rig *rig)
/*******************************************************************************
LAST MODIFIED : 3 July 1993

DESCRIPTION :
==============================================================================*/
{
	int number_of_devices,return_code;
	struct Device **device;

	ENTER(destroy_all_events);
	if (rig)
	{
		device=rig->devices;
		number_of_devices=rig->number_of_devices;
		while (number_of_devices>0)
		{
			destroy_Event_list(&((*device)->signal->first_event));
			device++;
			number_of_devices--;
		}
	}
	return_code=1;
	LEAVE;

	return (return_code);
} /* destroy_all_events */

struct Electrical_imaging_event *create_Electrical_imaging_event(int time) 
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION : create a Electrical_imaging_event at <time>
==============================================================================*/
{
	struct Electrical_imaging_event *event;

	ENTER(create_Electrical_imaging_event);
	if (ALLOCATE(event,struct Electrical_imaging_event,1))
	{	
		event->time=time;
		event->is_current_event=0;
		event->previous=(struct Electrical_imaging_event *)NULL;
		event->next=(struct Electrical_imaging_event *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Electrical_imaging_event.  Could not allocate memory");
	}
	LEAVE;
	return (event);
} /* create_Electrical_imaging_event */

int print_Electrical_imaging_event_list(
	struct Electrical_imaging_event *first_list_event)
/*******************************************************************************
LAST MODIFIED :  31 May 2001

DESCRIPTION : Debugging function.
==============================================================================*/
{
	int return_code;
	struct Electrical_imaging_event *event;

	ENTER(print_Electrical_imaging_event_list);
	event=(struct Electrical_imaging_event *)NULL;
	if(event=first_list_event)
	{
		return_code=1;	
		while(event)
		{
			printf("Electrical_imaging_event time = %d\n",
				event->time);
			event=event->next;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"print_Electrical_imaging_event_list. no list");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* print_Electrical_imaging_event_list */

int count_Electrical_imaging_events(
	struct Electrical_imaging_event *first_list_event)
/*******************************************************************************
LAST MODIFIED :  4 July 2001

DESCRIPTION : Count and return the number of evens in the list beginning at 
<first_list_event>
==============================================================================*/
{
	int num_events;
	struct Electrical_imaging_event *event;

	ENTER(count_Electrical_imaging_events);
	event=(struct Electrical_imaging_event *)NULL;
	if(event=first_list_event)
	{
		num_events=0;	
		while(event)
		{
			num_events++;
			event=event->next;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"count_Electrical_imaging_events. no list");
		num_events=0;
	}
	LEAVE;
	return(num_events);
}/* count_Electrical_imaging_events */

int add_Electrical_imaging_event_to_sorted_list(
	struct Electrical_imaging_event **first_list_event,
	struct Electrical_imaging_event *new_event)
/*******************************************************************************
LAST MODIFIED :  31 May 2001

DESCRIPTION : adds <new_event> to the event list <first_list_event>,
inserting it so that the list is in order of event->time.
If 2 events have the same time, they will both exist in the list, stored 
consecutively. 
==============================================================================*/
{
	int placed,return_code;
	struct Electrical_imaging_event *event,*next_event;

	ENTER(add_Electrical_imaging_event_to_sorted_list);
	event=(struct Electrical_imaging_event *)NULL;
	next_event=(struct Electrical_imaging_event *)NULL;
	if(new_event)
	{
		return_code=1;
		placed=0;
		if(event=*first_list_event)
		{	
			/* find place to insert event */
			/*at beginning of list?*/
			if(new_event->time<
				event->time)
			{	
				new_event->previous=(struct Electrical_imaging_event *)NULL;
				new_event->next=event;
				event->previous=new_event;
				*first_list_event=new_event;
				placed=1;
			}
			else
			{
				/*in middle or at end of list?*/
				do
				{
					if(new_event->time>=
						event->time)
					{				
						if(!event->next)
						{
							/* add the new one on the end of list */		
							event->next=new_event;
							new_event->previous=event;
							new_event->next=(struct Electrical_imaging_event *)NULL;
							placed=1;
						}
						else
						{
							next_event=event->next;
							if(new_event->time<next_event->time)
							{
								/* insert in the middle of list */								
								event->next=new_event;
								new_event->previous=event;
								new_event->next=next_event;
								next_event->previous=new_event;
								placed=1;
							}
						}
					}				
				}while((event=event->next)&&(!placed));
			}
		}
		else
		{	
			/* first entry in the list */
			*first_list_event=new_event;
			new_event->previous=(struct Electrical_imaging_event *)NULL;
			new_event->next=(struct Electrical_imaging_event *)NULL;
			placed=1;
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"add_Electrical_imaging_event_to_sorted_list. invalid arguments");
	}
	if(!placed)
	{
		return_code=0;
		return_code=0;
		display_message(ERROR_MESSAGE,
			"add_Electrical_imaging_event_to_sorted_list. error adding to list");
	}
	LEAVE;
	return(return_code);
} /* add_Electrical_imaging_event_to_sorted_list */

int remove_Electrical_imaging_event_from_list(
	struct Electrical_imaging_event **first_list_event,
	struct Electrical_imaging_event *event_to_remove)
/*******************************************************************************
LAST MODIFIED :  14 June 2001

DESCRIPTION : remove the <event_to_remove> from the list of events whose first 
event is <first_list_event>.
==============================================================================*/
{
	int return_code,found;
	struct Electrical_imaging_event *event,*next_event,*prev_event;

	ENTER(remove_Electrical_imaging_event_from_list);
	event=(struct Electrical_imaging_event *)NULL;
	next_event=(struct Electrical_imaging_event *)NULL;
	prev_event=(struct Electrical_imaging_event *)NULL;
	if(event_to_remove&&(first_list_event)&&(event=*first_list_event))
	{
		return_code=0;	
		found=0;
		/*find event in list*/
		while(event&&(!found))
		{
			if(event_to_remove==event)
			{
				found=1;
			}
			event=event->next;
		}
		if(!found)
		{
			/* should this be an error? I think so.*/
			return_code=0;
			display_message(ERROR_MESSAGE,
				"remove_Electrical_imaging_event_from_list. event not in list ");
		}
		else
		{
			return_code=1;
			
			/* remove event from list */
			prev_event=event_to_remove->previous;
			next_event=event_to_remove->next;
			if(prev_event==NULL)
			{
				/* first event in list*/
				if(next_event)
				{
					next_event->previous=(struct Electrical_imaging_event *)NULL;
				}
				*first_list_event=next_event;
			}
			else if(next_event==NULL)
			{
				/* last event in list*/
				prev_event->next=(struct Electrical_imaging_event *)NULL;
			}
			else
			{
				/* in the middle of the list */
				prev_event->next=next_event;
				next_event->previous=prev_event;
			}
			DEALLOCATE(event_to_remove);
		}	
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"remove_Electrical_imaging_event_from_list. invalid arguments");
	}
	LEAVE;
	return(return_code);
} /* remove_Electrical_imaging_event_from_list */

int add_Electrical_imaging_event_to_unsorted_list(
	struct Electrical_imaging_event **first_list_event,
	struct Electrical_imaging_event *new_event)
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :adds <new_event> to the end of the event list 
<first_list_event>. See also add_Electrical_imaging_event_to_sorted_list
==============================================================================*/
{
	int return_code;
	struct Electrical_imaging_event *event;

	ENTER(add_Electrical_imaging_event_to_unsorted_list);
	event=(struct Electrical_imaging_event *)NULL;
	if(new_event)
	{
		return_code=1;
		if(event=*first_list_event)
		{
			/* find last event*/
			while(event->next)
			{
				event=event->next;
			}							
			/* add the new one on the end */		
			event->next=new_event;
			new_event->previous=event;
		}
		else
		{	
			/* first entry in the list */
			*first_list_event=new_event;
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"add_Electrical_imaging_event_to_unsorted_list. invalid arguments");
	}
	LEAVE;
	return(return_code);
} /* add_Electrical_imaging_event_to_unsorted_list */

int destroy_Electrical_imaging_event_list(
	struct Electrical_imaging_event **first_event)
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
This function frees the memory associated with the event list starting at
<**first_event> and sets <*first_event> to NULL.
==============================================================================*/
{
	int return_code;
	struct Electrical_imaging_event *event,*next_event;

	ENTER(destroy_Electrical_imaging_event_list);
	return_code=1;
	if (first_event&&(event= *first_event))
	{
		while(event)
		{
			next_event=event->next;
			DEALLOCATE(event);
			event=next_event;
		} 
		*first_event=(struct Electrical_imaging_event *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_Electrical_imaging_event_list */
