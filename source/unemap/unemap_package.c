/*******************************************************************************
FILE : unemap_package.c

LAST MODIFIED : 13 May 2003

DESCRIPTION :
Contains function definitions for unemap package.
==============================================================================*/
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <string.h>
#include "element/element_operations.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/import_finite_element.h"
#include "general/mystring.h"
#include "general/debug.h"
#include "graphics/colour.h"
#include "node/node_operations.h"
#include "region/cmiss_region.h"
#include "time/time.h"
#include "unemap/unemap_package.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#if defined (UNEMAP_USE_NODES)
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "graphics/graphical_element.h"
#endif /* defined (UNEMAP_USE_NODES) */

/*
Module functions
------------
*/
#if defined (UNEMAP_USE_3D)
static int rig_node_has_electrode_defined(struct FE_node *node,
	void *package_void)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Determines if the <node> (assumed to be a member of a unemap_package rig_node_group)
contains a device_type field with the value ELECTRODE.
Called  by (see also) unemap_package_rig_node_group_has_electrodes.
==============================================================================*/
{
	int return_code;
	char *device_type_string=(char *)NULL;
	struct Unemap_package *package=(struct Unemap_package *)NULL;

	ENTER(rig_node_has_electrode_defined);
	return_code=0;
	if (node&&(package=(struct Unemap_package *)package_void))
	{
		if(FE_field_is_defined_at_node(package->device_type_field,node))
		{
			get_FE_nodal_string_value(node,package->device_type_field,
				0,0,FE_NODAL_VALUE,&device_type_string);
			if(!strcmp(device_type_string,"ELECTRODE"))
			{
				/*success!*/
				return_code=1;
			}
			DEALLOCATE(device_type_string);			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"rig_node_has_electrode_defined. invalid arguments");		
	}
	LEAVE;
	return (return_code);
}/* rig_node_has_electrode_defined */
#endif /* defined (UNEMAP_USE_3D)*/

/*
Global functions
------------
*/
#if defined (UNEMAP_USE_3D)
DECLARE_OBJECT_FUNCTIONS(Unemap_package)

struct Unemap_package *CREATE(Unemap_package)(
	struct MANAGER(FE_basis) *fe_basis_manager,
	struct Cmiss_region *root_cmiss_region,
	struct Cmiss_region *data_root_cmiss_region,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct FE_node_selection *node_selection,
	struct IO_stream_package *io_stream_package)
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION:
Create a Unemap package, and fill in the managers.
The fields are filled in with set_unemap_package_fields()
==============================================================================*/
{	
	struct Unemap_package *package;

	ENTER(CREATE(Unemap_package));
	if (fe_basis_manager && root_cmiss_region && data_root_cmiss_region &&		
		computed_field_manager&&interactive_tool_manager&&node_selection)
	{
		if (ALLOCATE(package,struct Unemap_package,1))
		{
			package->fe_basis_manager=fe_basis_manager;	
			package->root_cmiss_region = ACCESS(Cmiss_region)(root_cmiss_region);
			package->data_root_cmiss_region =
				ACCESS(Cmiss_region)(data_root_cmiss_region);
			package->computed_field_manager=computed_field_manager;
			package->interactive_tool_manager=interactive_tool_manager;
			package->node_selection=node_selection;
			package->io_stream_package=io_stream_package;
			/* fields of the rig_nodes */
			package->device_name_field=(struct FE_field *)NULL;
			package->device_type_field=(struct FE_field *)NULL;
#if defined (UNEMAP_USE_NODES)
			package->display_end_time_field=(struct FE_field *)NULL;
			package->display_start_time_field=(struct FE_field *)NULL;
			package->highlight_field=(struct FE_field *)NULL;		
#endif /* defined (UNEMAP_USE_NODES)*/	
			package->torso_group_name=(char *)NULL;
			package->channel_number_field=(struct FE_field *)NULL;
			package->read_order_field=(struct FE_field *)NULL;
			package->map_fit_field=(struct FE_field *)NULL;			
			package->signal_field=(struct FE_field *)NULL;
			package->delaunay_signal_field=(struct FE_field *)NULL;
			package->signal_minimum_field=(struct FE_field *)NULL;
			package->signal_maximum_field=(struct FE_field *)NULL;	
			package->signal_status_field=(struct FE_field *)NULL;
			package->channel_gain_field=(struct FE_field *)NULL;
			package->channel_offset_field=(struct FE_field *)NULL;
			package->signal_value_at_time_field=(struct Computed_field *)NULL;
			package->offset_signal_value_at_time_field=
				(struct Computed_field *)NULL;
			package->scaled_offset_signal_value_at_time_field=
				(struct Computed_field *)NULL;
			package->potential_time_object=(struct Time_object *)NULL;
			package->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Unemap_package).  Could not allocate memory ");
			DEALLOCATE(package);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Unemap_package).  Invalid argument(s)");
		package=(struct Unemap_package *)NULL;
	}
	LEAVE;

	return (package);
} /* CREATE(Unemap_package) */

int DESTROY(Unemap_package)(struct Unemap_package **package_address)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Frees the memory for the Unemap_package node field and sets <*package_address>
to NULL.
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *torso_cmiss_region;
	struct Unemap_package *package;

	ENTER(DESTROY(Unemap_package));
	torso_cmiss_region = (struct Cmiss_region *)NULL;
	if ((package_address)&&(package= *package_address))
	{		
		if ((package->torso_group_name) && (torso_cmiss_region =
			Cmiss_region_get_child_region_from_name(package->root_cmiss_region,
				package->torso_group_name)))
		{
			FE_region_clear(Cmiss_region_get_FE_region(torso_cmiss_region),
				/*destroy_in_master*/1);
			Cmiss_region_remove_child_region(package->root_cmiss_region,
				torso_cmiss_region);
		}

		DEACCESS(Cmiss_region)(&(package->root_cmiss_region));
		DEACCESS(Cmiss_region)(&(package->data_root_cmiss_region));
		DEACCESS(FE_field)(&(package->device_name_field));
		DEACCESS(FE_field)(&(package->device_type_field));
		DEACCESS(FE_field)(&(package->channel_number_field));
#if defined (UNEMAP_USE_NODES)
		DEACCESS(FE_field)(&(package->display_start_time_field));
		DEACCESS(FE_field)(&(package->display_end_time_field));
		DEACCESS(FE_field)(&(package->highlight_field));
#endif /* defined (UNEMAP_USE_NODES)*/			
		/* Note: Unemap and Cmgui don't destroy the rig (but should!). (The rig contains */
		/* map_3d_packages which contain mapped_torso_node{element}_groups */
		/* which contain a subset of the nodes,elements in torso_node_group */					
		DEALLOCATE(package->torso_group_name);
		DEACCESS(FE_field)(&(package->read_order_field));
		DEACCESS(FE_field)(&(package->map_fit_field));
		DEACCESS(FE_field)(&(package->signal_field));	
		DEACCESS(FE_field)(&(package->delaunay_signal_field));
		DEACCESS(FE_field)(&(package->signal_minimum_field));
		DEACCESS(FE_field)(&(package->signal_maximum_field));	
		DEACCESS(FE_field)(&(package->signal_status_field));
		DEACCESS(FE_field)(&(package->channel_gain_field));
		DEACCESS(FE_field)(&(package->channel_offset_field));	
		DEACCESS(Computed_field)(&(package->scaled_offset_signal_value_at_time_field));
		DEACCESS(Computed_field)(&(package->offset_signal_value_at_time_field));
		DEACCESS(Computed_field)(&(package->signal_value_at_time_field));
		DEACCESS(Time_object)(&(package->potential_time_object));		
		DEALLOCATE(*package_address);		
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Unemap_package) */
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_device_name_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *device_name_field;
	ENTER(get_unemap_package_device_name_field);
	if(package)
	{
		device_name_field=package->device_name_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_device_name_field."
				" invalid arguments");
		device_name_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (device_name_field);
} /* get_unemap_package_device_name_field */

int set_unemap_package_device_name_field(struct Unemap_package *package,
	struct FE_field *device_name_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_device_name_field);
	if(package)
	{
		return_code =1;		 
		REACCESS(FE_field)(&(package->device_name_field),device_name_field);	
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_device_name_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_device_name_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_device_type_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *device_type_field;
	ENTER(get_unemap_package_device_type_field);
	if(package)
	{
		device_type_field=package->device_type_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_device_type_field."
				" invalid arguments");
		device_type_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (device_type_field);
} /* get_unemap_package_device_type_field */

int set_unemap_package_device_type_field(struct Unemap_package *package,
	struct FE_field *device_type_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_device_type_field);
	if(package)
	{
		return_code =1;
		REACCESS(FE_field)(&(package->device_type_field),device_type_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_device_type_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_device_type_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_channel_number_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *channel_number_field;
	ENTER(get_unemap_package_channel_number_field);
	if(package)
	{
		channel_number_field=package->channel_number_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_channel_number_field."
				" invalid arguments");
		channel_number_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (channel_number_field);
} /* get_unemap_package_channel_number_field */

int set_unemap_package_channel_number_field(struct Unemap_package *package,
	struct FE_field *channel_number_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_channel_number_field);
	if(package)
	{
		return_code =1;		
		REACCESS(FE_field)(&(package->channel_number_field),channel_number_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_channel_number_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_channel_number_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Time_object *get_unemap_package_potential_time_object(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 25 January 2002

DESCRIPTION :
Get the potential time object.
==============================================================================*/
{
	struct Time_object *time_object;
	ENTER(get_unemap_package_potential_time_object);
	if(package)
	{
		time_object=package->potential_time_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_potential_time_object."
				" invalid arguments");
		time_object = (struct Time_object *)NULL;
	}
	LEAVE;
	return (time_object);
} /* get_unemap_package_potential_time_object */

int set_unemap_package_potential_time_object(struct Unemap_package *package,
	struct Time_object *potential_time_object)
/*******************************************************************************
LAST MODIFIED : 25 January 2002

DESCRIPTION :
Sets the potential time object.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_potential_time_object);
	if(package)
	{
		return_code =1;		
		REACCESS(Time_object)(&(package->potential_time_object),
			potential_time_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_potential_time_object."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_potential_time_object */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Cmiss_region *get_unemap_package_root_Cmiss_region(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Gets the root_cmiss_region from the unemap package.
==============================================================================*/
{
	struct Cmiss_region *root_cmiss_region;

	ENTER(get_unemap_package_root_Cmiss_region);
	if(package)
	{
		root_cmiss_region = package->root_cmiss_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_unemap_package_root_Cmiss_region.  Invalid argument(s)");
		root_cmiss_region = (struct Cmiss_region *)NULL;
	}
	LEAVE;

	return (root_cmiss_region);
}/* get_unemap_package_root_Cmiss_region */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Cmiss_region *get_unemap_package_data_root_Cmiss_region(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Gets the data_root_cmiss_region from the unemap package.
==============================================================================*/
{
	struct Cmiss_region *data_root_cmiss_region;

	ENTER(get_unemap_package_data_root_Cmiss_region);
	if(package)
	{
		data_root_cmiss_region = package->data_root_cmiss_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_unemap_package_data_root_Cmiss_region.  Invalid argument(s)");
		data_root_cmiss_region = (struct Cmiss_region *)NULL;
	}
	LEAVE;

	return (data_root_cmiss_region);
}/* get_unemap_package_data_root_Cmiss_region */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_unemap_package_display_start_time_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *display_start_time_field;
	ENTER(get_unemap_package_display_start_time_field);
	if(package)
	{
		display_start_time_field=package->display_start_time_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_display_start_time_field."
				" invalid arguments");
		display_start_time_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (display_start_time_field);
} /* get_unemap_package_display_start_time_field */

int set_unemap_package_display_start_time_field(struct Unemap_package *package,
	struct FE_field *display_start_time_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_display_start_time_field);
	if(package)
	{
		return_code =1;		
		REACCESS(FE_field)(&(package->display_start_time_field),display_start_time_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_display_start_time_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_display_start_time_field */
#endif /* defined (UNEMAP_USE_NODES)*/

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_unemap_package_display_end_time_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *display_end_time_field;
	ENTER(get_unemap_package_display_end_time_field);
	if(package)
	{
		display_end_time_field=package->display_end_time_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_display_end_time_field."
				" invalid arguments");
		display_end_time_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (display_end_time_field);
} /* get_unemap_package_display_end_time_field */

int set_unemap_package_display_end_time_field(struct Unemap_package *package,
	struct FE_field *display_end_time_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_display_end_time_field);
	if(package)
	{
		return_code =1;		
		REACCESS(FE_field)(&(package->display_end_time_field),display_end_time_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_display_end_time_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_display_end_time_field */
#endif /* defined (UNEMAP_USE_NODES)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_read_order_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *read_order_field;
	ENTER(get_unemap_package_read_order_field);
	if(package)
	{
		read_order_field=package->read_order_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_read_order_field."
				" invalid arguments");
		read_order_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (read_order_field);
} /* get_unemap_package_read_order_field */

int set_unemap_package_read_order_field(struct Unemap_package *package,
	struct FE_field *read_order_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_read_order_field);
	if(package)
	{
		return_code =1;		
		REACCESS(FE_field)(&(package->read_order_field),read_order_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_read_order_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_read_order_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_map_fit_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
gets the field of the unemap package.
map_fit_field also in map_3d_package. Need to store here between creation
of map_fit_field and creation of map_3d_package. ??JW Remove from map_3d_package?
==============================================================================*/
{
	struct FE_field *map_fit_field;
	ENTER(get_unemap_package_map_fit_field);
	if(package)
	{
		map_fit_field=package->map_fit_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_map_fit_field."
				" invalid arguments");
		map_fit_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (map_fit_field);
} /* get_unemap_package_map_fit_field */

int set_unemap_package_map_fit_field(struct Unemap_package *package,
	struct FE_field *map_fit_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.	
map_fit_field also in map_3d_package. Need to store here between creation
of map_fit_field and creation of map_3d_package. ??JW Remove from map_3d_package?
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_map_fit_field);
	if(package)
	{
		return_code =1;		
		REACCESS(FE_field)(&(package->map_fit_field),map_fit_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_map_fit_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_map_fit_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_delaunay_signal_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *delaunay_signal_field;
	ENTER(get_unemap_package_delaunay_signal_field);
	if(package)
	{
		delaunay_signal_field=package->delaunay_signal_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_delaunay_signal_field."
				" invalid arguments");
		delaunay_signal_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (delaunay_signal_field);
} /* get_unemap_package_delaunay_signal_field */

int set_unemap_package_delaunay_signal_field(struct Unemap_package *package,
	struct FE_field *delaunay_signal_field)
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
Sets the field of the unemap package.	
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_delaunay_signal_field);
	if(package)
	{
		return_code =1;		
		REACCESS(FE_field)(&(package->delaunay_signal_field),delaunay_signal_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_delaunay_signal_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_delaunay_signal_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_unemap_package_highlight_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *highlight_field;
	ENTER(get_unemap_package_highlight_field);
	if(package)
	{
		highlight_field=package->highlight_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_highlight_field."
				" invalid arguments");
		highlight_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (highlight_field);
} /* get_unemap_package_highlight_field */

int set_unemap_package_highlight_field(struct Unemap_package *package,
	struct FE_field *highlight_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_highlight_field);
	if(package)
	{
		return_code =1;		
		REACCESS(FE_field)(&(package->highlight_field),highlight_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_highlight_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_highlight_field */
#endif /* defined (UNEMAP_USE_NODES)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_signal_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *signal_field;
	ENTER(get_unemap_package_signal_field);
	if(package)
	{
		signal_field=package->signal_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_field."
				" invalid arguments");
		signal_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (signal_field);
} /* get_unemap_package_signal_field */

int set_unemap_package_signal_field(struct Unemap_package *package,
	struct FE_field *signal_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->signal_field),signal_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_field */
#endif /* defined (UNEMAP_USE_NODES)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_signal_minimum_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *signal_minimum_field;
	ENTER(get_unemap_package_signal_minimum_field);
	if(package)
	{
		signal_minimum_field=package->signal_minimum_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_minimum_field."
				" invalid arguments");
		signal_minimum_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (signal_minimum_field);
} /* get_unemap_package_signal_minimum_field */

int set_unemap_package_signal_minimum_field(struct Unemap_package *package,
	struct FE_field *signal_minimum_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_minimum_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->signal_minimum_field),signal_minimum_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_minimum_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_minimum_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_signal_maximum_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *signal_maximum_field;
	ENTER(get_unemap_package_signal_maximum_field);
	if(package)
	{
		signal_maximum_field=package->signal_maximum_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_maximum_field."
				" invalid arguments");
		signal_maximum_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (signal_maximum_field);
} /* get_unemap_package_signal_maximum_field */

int set_unemap_package_signal_maximum_field(struct Unemap_package *package,
	struct FE_field *signal_maximum_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_maximum_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->signal_maximum_field),signal_maximum_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_maximum_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_maximum_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_signal_status_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : October 8 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *signal_status_field;
	ENTER(get_unemap_package_signal_status_field);
	if(package)
	{
		signal_status_field=package->signal_status_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_status_field."
				" invalid arguments");
		signal_status_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (signal_status_field);
} /* get_unemap_package_signal_status_field */

int set_unemap_package_signal_status_field(struct Unemap_package *package,
	struct FE_field *signal_status_field)
/*******************************************************************************
LAST MODIFIED : October 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_status_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->signal_status_field),signal_status_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_status_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_status_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Computed_field *get_unemap_package_signal_value_at_time_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct Computed_field *signal_value_at_time_field;
	ENTER(get_unemap_package_signal_value_at_time_field);
	if(package)
	{
		signal_value_at_time_field=package->signal_value_at_time_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_signal_value_at_time_field."
				" invalid arguments");
		signal_value_at_time_field = (struct Computed_field *)NULL;
	}
	LEAVE;
	return (signal_value_at_time_field);
} /* get_unemap_package_signal_value_at_time_field */

int set_unemap_package_signal_value_at_time_field(struct Unemap_package *package,
	struct Computed_field *signal_value_at_time_field)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_signal_value_at_time_field);
	if(package)
	{
		return_code =1;	
		REACCESS(Computed_field)(&(package->signal_value_at_time_field),
			signal_value_at_time_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_signal_value_at_time_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_signal_value_at_time_field */
#endif /* defined (UNEMAP_USE_3D)*/


#if defined (UNEMAP_USE_3D)
struct Computed_field *get_unemap_package_offset_signal_value_at_time_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct Computed_field *offset_signal_value_at_time_field;
	ENTER(get_unemap_package_offset_signal_value_at_time_field);
	if(package)
	{
		offset_signal_value_at_time_field=package->offset_signal_value_at_time_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_offset_signal_value_at_time_field."
				" invalid arguments");
		offset_signal_value_at_time_field = (struct Computed_field *)NULL;
	}
	LEAVE;
	return (offset_signal_value_at_time_field);
} /* get_unemap_package_offset_signal_value_at_time_field */

int set_unemap_package_offset_signal_value_at_time_field(struct Unemap_package *package,
	struct Computed_field *offset_signal_value_at_time_field)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_offset_signal_value_at_time_field);
	if(package)
	{
		return_code =1;	
		REACCESS(Computed_field)(&(package->offset_signal_value_at_time_field),
			offset_signal_value_at_time_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_offset_signal_value_at_time_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_offset_signal_value_at_time_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Computed_field *get_unemap_package_scaled_offset_signal_value_at_time_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct Computed_field *scaled_offset_signal_value_at_time_field;
	ENTER(get_unemap_package_scaled_offset_signal_value_at_time_field);
	if(package)
	{
		scaled_offset_signal_value_at_time_field=
			package->scaled_offset_signal_value_at_time_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_unemap_package_scaled_offset_signal_value_at_time_field."
				" invalid arguments");
		scaled_offset_signal_value_at_time_field = (struct Computed_field *)NULL;
	}
	LEAVE;
	return (scaled_offset_signal_value_at_time_field);
} /* get_unemap_package_scaled_offset_signal_value_at_time_field */

int set_unemap_package_scaled_offset_signal_value_at_time_field(
	struct Unemap_package *package,
	struct Computed_field *scaled_offset_signal_value_at_time_field)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_scaled_offset_signal_value_at_time_field);
	if(package)
	{
		return_code =1;	
		REACCESS(Computed_field)(&(package->scaled_offset_signal_value_at_time_field),
			scaled_offset_signal_value_at_time_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_unemap_package_scaled_offset_signal_value_at_time_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_scaled_offset_signal_value_at_time_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_channel_offset_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *channel_offset_field;
	ENTER(get_unemap_package_channel_offset_field);
	if(package)
	{
		channel_offset_field=package->channel_offset_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_channel_offset_field."
				" invalid arguments");
		channel_offset_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (channel_offset_field);
} /* get_unemap_package_channel_offset_field */

int set_unemap_package_channel_offset_field(struct Unemap_package *package,
	struct FE_field *channel_offset_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_channel_offset_field);
	if(package)
	{
		return_code =1;	
		REACCESS(FE_field)(&(package->channel_offset_field),channel_offset_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_channel_offset_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_channel_offset_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_channel_gain_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct FE_field *channel_gain_field;
	ENTER(get_unemap_package_channel_gain_field);
	if(package)
	{
		channel_gain_field=package->channel_gain_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_channel_gain_field."
				" invalid arguments");
		channel_gain_field = (struct FE_field *)NULL;
	}
	LEAVE;
	return (channel_gain_field);
} /* get_unemap_package_channel_gain_field */

int set_unemap_package_channel_gain_field(struct Unemap_package *package,
	struct FE_field *channel_gain_field)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_channel_gain_field);
	if(package)
	{
		return_code=1;		
		REACCESS(FE_field)(&(package->channel_gain_field),channel_gain_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_channel_gain_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_channel_gain_field */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(Computed_field) *get_unemap_package_Computed_field_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(get_unemap_package_Computed_field_manager);
	if(package)
	{
		computed_field_manager=package->computed_field_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_Computed_field_manager."
			" invalid arguments");
		computed_field_manager = (struct MANAGER(Computed_field) *)NULL;
	}
	LEAVE;
	return(computed_field_manager);
}/* get_unemap_package_Computed_field_manager */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_node_selection *get_unemap_package_FE_node_selection(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 31 AUGUST 2000

DESCRIPTION :
gets a FE_node_selection of the unemap package.
==============================================================================*/
{
	struct FE_node_selection *node_selection;

	ENTER(get_unemap_package_FE_node_selection);
	if(package)
	{
		node_selection=package->node_selection;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_FE_node_selection."
			" invalid arguments");
		node_selection = (struct FE_node_selection *)NULL;
	}
	LEAVE;
	return(node_selection);
}/* get_unemap_package_FE_node_selection */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(Interactive_tool) *get_unemap_package_interactive_tool_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *interactive_tool_manager;

	ENTER(get_unemap_package_interactive_tool_manager);
	if(package)
	{
		interactive_tool_manager=package->interactive_tool_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_interactive_tool_manager."
			" invalid arguments");
		interactive_tool_manager = (struct MANAGER(Interactive_tool) *)NULL;
	}
	LEAVE;
	return(interactive_tool_manager);
}/* get_unemap_package_interactive_tool_manager */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_basis) *get_unemap_package_basis_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_basis) *basis_manager;

	ENTER(get_unemap_package_basis_manager);
	if(package)
	{
		basis_manager=package->fe_basis_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_basis_manager."
			" invalid arguments");
		basis_manager = (struct MANAGER(FE_basis) *)NULL;
	}
	LEAVE;
	return(basis_manager);
}/* get_unemap_package_basis_manager */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int unemap_package_rig_node_group_has_electrodes(struct Unemap_package *package,
	struct FE_region *rig_node_group)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :determines if the  <rig_node group> 
 contains at least one node with a device_type field
set to "ELECTRODE". See also rig_node_has_electrode_defined
==============================================================================*/
{
	int return_code; 
	struct FE_node *node =(struct FE_node *)NULL;
	
	ENTER(unemap_package_rig_node_group_has_electrodes)
	return_code =0;
	if(package&&rig_node_group)
	{
		if(package->device_type_field)
		{				
			node = FE_region_get_first_FE_node_that(rig_node_group,
				rig_node_has_electrode_defined, (void *)package);
			if(node)
			{
				/*we've found one!*/
				return_code =1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_package_rig_node_group_has_electrodes.\n"
				" no device_type_field defined");
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_package_rig_node_group_has_electrodes.\n"
			" invalid arguments");	
	}
	LEAVE;
	return(return_code);
}/* unemap_package_rig_node_group_has_electrodes. */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_rig_fields(struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Frees the <unemap_package> rig's computed and fe fields
NOTE: map_fit_field ISN'T and SHOULDN'T be destroyed here. It isn't a property of 
the rig. 
==============================================================================*/
{
	int return_code;
	struct MANAGER(Computed_field) *computed_field_manager=
		(struct MANAGER(Computed_field) *)NULL;

	ENTER(free_unemap_package_rig_fields)
	if (unemap_package)
	{
		return_code=1;
		computed_field_manager=unemap_package->computed_field_manager;
		if (unemap_package->device_name_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->device_name_field);
			DEACCESS(FE_field)(&(unemap_package->device_name_field));
			unemap_package->device_name_field = (struct FE_field *)NULL;
		}
		if (unemap_package->device_type_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->device_type_field);
			DEACCESS(FE_field)(&(unemap_package->device_type_field));
			unemap_package->device_type_field = (struct FE_field *)NULL;
		}
		if (unemap_package->channel_number_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->channel_number_field);
			DEACCESS(FE_field)(&(unemap_package->channel_number_field));
			unemap_package->channel_number_field = (struct FE_field *)NULL;
		}
#if  defined (UNEMAP_USE_NODES)
		if (unemap_package->display_start_time_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->display_start_time_field);
			DEACCESS(FE_field)(&(unemap_package->display_start_time_field));
			unemap_package->display_start_time_field = (struct FE_field *)NULL;
		}
		if (unemap_package->display_end_time_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->display_end_time_field);
			DEACCESS(FE_field)(&(unemap_package->display_end_time_field));
			unemap_package->display_end_time_field = (struct FE_field *)NULL;
		}
		if (unemap_package->highlight_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->highlight_field);
			DEACCESS(FE_field)(&(unemap_package->highlight_field));
			unemap_package->highlight_field = (struct FE_field *)NULL;
		}
#endif /* defined (UNEMAP_USE_NODES) */		
		if (unemap_package->read_order_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->read_order_field);
			DEACCESS(FE_field)(&(unemap_package->read_order_field));
			unemap_package->read_order_field = (struct FE_field *)NULL;
		}
		if (unemap_package->signal_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->signal_field);
			DEACCESS(FE_field)(&(unemap_package->signal_field));
			unemap_package->signal_field = (struct FE_field *)NULL;
		}
		if (unemap_package->signal_minimum_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->signal_minimum_field);
			DEACCESS(FE_field)(&(unemap_package->signal_minimum_field));
			unemap_package->signal_minimum_field = (struct FE_field *)NULL;
		}
		if (unemap_package->signal_maximum_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->signal_maximum_field);
			DEACCESS(FE_field)(&(unemap_package->signal_maximum_field));
			unemap_package->signal_maximum_field = (struct FE_field *)NULL;
		}
		if (unemap_package->signal_status_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->signal_status_field);
			DEACCESS(FE_field)(&(unemap_package->signal_status_field));
			unemap_package->signal_status_field = (struct FE_field *)NULL;
		}
		if (unemap_package->channel_gain_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->channel_gain_field);
			DEACCESS(FE_field)(&(unemap_package->channel_gain_field));
			unemap_package->channel_gain_field = (struct FE_field *)NULL;
		}
		if (unemap_package->channel_offset_field)
		{		 
			Computed_field_manager_destroy_FE_field(computed_field_manager,
				unemap_package->channel_offset_field);
			DEACCESS(FE_field)(&(unemap_package->channel_offset_field));
			unemap_package->channel_offset_field = (struct FE_field *)NULL;
		}
	}
	else
	{	
		display_message(ERROR_MESSAGE,"free_unemap_package_rig_fields."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	return(return_code);		
}/* free_unemap_package_rig_fields*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_time_computed_fields(struct Unemap_package *unemap_package)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Frees the time related computed fields (used by the map electrode glyphs) 
stored in the unemap package. Also frees any associated fe_fields
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_field;
	struct FE_field *fe_field;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(free_unemap_package_time_computed_fields);
	if(unemap_package)
	{
		computed_field=(struct Computed_field *)NULL;	
		fe_field=(struct FE_field *)NULL;
		computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
		return_code=1;
		computed_field_manager =
			get_unemap_package_Computed_field_manager(unemap_package);
		computed_field =
			get_unemap_package_scaled_offset_signal_value_at_time_field(
				unemap_package);
		/* following does deaccess*/
		set_unemap_package_scaled_offset_signal_value_at_time_field
						(unemap_package,(struct Computed_field *)NULL);
		if (computed_field)
		{				
			if (MANAGED_OBJECT_NOT_IN_USE(Computed_field)(computed_field,
				computed_field_manager))
			{			
				/* also want to destroy any wrapped FE_field */
				if (Computed_field_is_type_finite_element(computed_field))
				{
				  if (!(Computed_field_get_type_finite_element(computed_field,
						&fe_field) &&
						Computed_field_manager_destroy_FE_field(computed_field_manager,
							fe_field)))
					{
						return_code = 0;
					}
				}
				else
				{
					if (!REMOVE_OBJECT_FROM_MANAGER(Computed_field)(
						computed_field, computed_field_manager))
					{
						return_code = 0;
					}
				}
				if (!return_code)
				{
					display_message(WARNING_MESSAGE,
						"free_unemap_package_time_computed_fields.  "
						"Couldn't remove scaled_offset_signal_value_at_time_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"free_unemap_package_time_computed_fields.  "
					"Couldn't destroy scaled_offset_signal_value_at_time_field");
			}				
		}/* if(computed_field) */
		computed_field =
			get_unemap_package_offset_signal_value_at_time_field(unemap_package);
		/* following does deaccess*/
		set_unemap_package_offset_signal_value_at_time_field(
			unemap_package,(struct Computed_field *)NULL);
		if (computed_field)
		{
			if (MANAGED_OBJECT_NOT_IN_USE(Computed_field)(computed_field,
				computed_field_manager))
			{			
				/* also want to destroy any wrapped FE_field */
				if (Computed_field_is_type_finite_element(computed_field))
				{
				  if (!(Computed_field_get_type_finite_element(computed_field,
						&fe_field) &&
						Computed_field_manager_destroy_FE_field(computed_field_manager,
							fe_field)))
					{
						return_code = 0;
					}
				}
				else
				{
					if (!REMOVE_OBJECT_FROM_MANAGER(Computed_field)(
						computed_field, computed_field_manager))
					{
						return_code = 0;
					}
				}
				if (!return_code)
				{
					display_message(WARNING_MESSAGE,
						"free_unemap_package_time_computed_fields.  "
						"Couldn't remove offset_signal_value_at_time_field from manager");
				}				
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"free_unemap_package_time_computed_fields.  "
					"Couldn't destroy offset_signal_value_at_time_field");
			}				
		}/* if(computed_field) */
		/* signal_value_at_time_field is now just a pointer to an fe_field wrapper and
			so we no longer destroy it here, just deaccess it */
		/* following does deaccess*/
		set_unemap_package_signal_value_at_time_field(
			unemap_package, (struct Computed_field *)NULL);
		/* and the time_field has gone altogether */
	}/* if(unemap_package) */
	else
	{
		display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields. "
				"Invalid arguments ");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/*free_unemap_package_time_computed_fields */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_rig_node_group_glyphs(
	struct Map_drawing_information *drawing_information,
	struct Unemap_package *package,
	struct FE_region *rig_node_group)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Frees up any glyphs used by the nodes in the rig_node_group
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *rig_node_cmiss_region;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;
	struct Scene *scene;

	ENTER(free_unemap_package_rig_node_group_glyphs);	
	gt_element_group=(struct GT_element_group *)NULL;
	settings=(struct GT_element_settings *)NULL;
	scene=(struct Scene *)NULL;
	if (package && rig_node_group && drawing_information)
	{
		if ((scene = get_map_drawing_information_scene(drawing_information)) &&
			(FE_region_get_Cmiss_region(rig_node_group, &rig_node_cmiss_region)))
		{
			if (gt_element_group =
				Scene_get_graphical_element_group(scene, rig_node_cmiss_region))
			{
				return_code=1;
				while (return_code&&(settings=first_settings_in_GT_element_group_that(
					gt_element_group,GT_element_settings_type_matches,
					(void *)GT_ELEMENT_SETTINGS_NODE_POINTS)))
				{
					if(!(return_code=GT_element_group_remove_settings(gt_element_group,settings)))
					{
						display_message(ERROR_MESSAGE,
							"free_unemap_package_rig_node_group_glyphs. couldn't remove settings");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"free_unemap_package_rig_node_group_glyphs.  "
					"gt_element_group not found");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"free_unemap_package_rig_node_group_glyphs.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* free_unemap_package_rig_node_group_glyphs */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_rig_node_group(struct Unemap_package *package,	
	struct FE_region **rig_node_group)
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :Frees the node, element and data groups of <rig_node_group>
Note: DOESN't free the glyphs of the node group. See 
free_unemap_package_rig_node_group_glyphs for this
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *rig_node_cmiss_region;
	
	ENTER(free_unemap_package_rig_node_group);
	if (package && rig_node_group && (*rig_node_group) &&
		(FE_region_get_Cmiss_region(*rig_node_group, &rig_node_cmiss_region)))
	{
		FE_region_clear(*rig_node_group, /*destroy_in_master*/1);
		return_code = Cmiss_region_remove_child_region(package->root_cmiss_region,
			rig_node_cmiss_region);
		DEACCESS(FE_region)(rig_node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,"free_unemap_package_rig_node_group."
			" invalid arguments");
		return_code =0;	
	}		
	LEAVE;
	return (return_code);
}/* free_unemap_package_rig_node_group */
#endif /* #if defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
char *get_unemap_package_torso_group_name(struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 16 July 2002

DESCRIPTION :
Returns the <torso_group_name> of the unemap_package.
==============================================================================*/
{
	char *torso_group_name;

	ENTER(get_unemap_package_torso_group_name);
	if(package)
	{
		torso_group_name=package->torso_group_name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_unemap_package_torso_group_name.  Invalid arguments");
		torso_group_name = (char *)NULL;
	}
	LEAVE;

	return (torso_group_name);
} /* get_unemap_package_torso_group_name */

int unemap_package_read_torso_file(struct Unemap_package *package,
	char *torso_file_name)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Reads in the default torso node and element groups from <torso_file_name>.
The group name is then stored in the unemap_package.
==============================================================================*/
{
	char exnode_ext[] = ".exnode";
	char exelem_ext[] = ".exelem";
	char *exnode_name_str, *exelem_name_str, *torso_group_name;
	int exnode_name_str_len, exelem_name_str_len, last_identifier,
		number_of_elements, number_of_nodes, return_code, string_len;
	struct Cmiss_region *element_root_cmiss_region, *torso_root_cmiss_region,
		*root_cmiss_region;
	struct Computed_field *cmiss_number_field;
	struct FE_region *torso_root_fe_region, *torso_element_group;
	struct MANAGER(FE_basis) *basis_manager;

	ENTER(unemap_package_read_torso_file);
	if (package && torso_file_name &&
		(root_cmiss_region = get_unemap_package_root_Cmiss_region(package)) &&
		(basis_manager = get_unemap_package_basis_manager(package)))
	{
		return_code = 1;
		if (package->torso_group_name)
		{
			DEALLOCATE(package->torso_group_name);
			package->torso_group_name = (char *)NULL;
		}
		/* construct full names for exnode and exelem files */
		string_len = strlen(torso_file_name);
		exnode_name_str_len = string_len + strlen(exnode_ext) + 1; 
		exelem_name_str_len = string_len + strlen(exelem_ext) + 1;
		ALLOCATE(exnode_name_str, char, exnode_name_str_len);
		ALLOCATE(exelem_name_str, char, exelem_name_str_len);
		if (exnode_name_str && exelem_name_str)
		{
			/* add the extensions to the file name */
			strcpy(exnode_name_str, torso_file_name);
			strcat(exnode_name_str, exnode_ext);
			strcpy(exelem_name_str, torso_file_name);
			strcat(exelem_name_str, exelem_ext);
			/* read the node and element files */
			torso_root_cmiss_region = read_exregion_file_of_name(exnode_name_str,
				package->io_stream_package, basis_manager,
				FE_region_get_FE_element_shape_list(Cmiss_region_get_FE_region(root_cmiss_region)),
				(struct FE_import_time_index *)NULL);
			element_root_cmiss_region = read_exregion_file_of_name(exelem_name_str,
				package->io_stream_package, basis_manager,
				FE_region_get_FE_element_shape_list(Cmiss_region_get_FE_region(root_cmiss_region)),
				(struct FE_import_time_index *)NULL);
			/* check the elements merge into the nodes and get the first child name */
			if (torso_root_cmiss_region && element_root_cmiss_region &&
				Cmiss_regions_FE_regions_can_be_merged(torso_root_cmiss_region,
					element_root_cmiss_region) &&
				Cmiss_regions_merge_FE_regions(torso_root_cmiss_region,
					element_root_cmiss_region) &&
				Cmiss_region_get_child_region_name(torso_root_cmiss_region,
					/*child_number*/0, &torso_group_name))
			{
				/* offset torso node and element numbers to be hard against INT_MAX/2,
					 the internal limit imposed for the benefit of encoding element
					 numbers in a single integer including type */
				if ((cmiss_number_field = CREATE(Computed_field)("cmiss_number")) &&
					Computed_field_set_type_cmiss_number(cmiss_number_field))
				{
					torso_root_fe_region =
						Cmiss_region_get_FE_region(torso_root_cmiss_region);
					last_identifier = INT_MAX/2;
					FE_region_begin_change(torso_root_fe_region);
					number_of_nodes =
						FE_region_get_number_of_FE_nodes(torso_root_fe_region);
					/* offset default torso node and element groups */
					FE_region_change_node_identifiers(torso_root_fe_region,
						/*node_offset*/(last_identifier - number_of_nodes),
						/*sort_by_field*/cmiss_number_field, /*time*/0.0);
					number_of_elements =
						FE_region_get_number_of_FE_elements(torso_root_fe_region);
					FE_region_change_element_identifiers(torso_root_fe_region, CM_ELEMENT,
						/*element_offset*/(last_identifier - number_of_elements),
						/*sort_by_field*/cmiss_number_field, /*time*/0.0);
					FE_region_change_element_identifiers(torso_root_fe_region, CM_FACE,
						/*element_offset*/(last_identifier - number_of_elements),
						/*sort_by_field*/cmiss_number_field, /*time*/0.0);
					FE_region_change_element_identifiers(torso_root_fe_region, CM_LINE,
						/*element_offset*/(last_identifier - number_of_elements),
						/*sort_by_field*/cmiss_number_field, /*time*/0.0);
					FE_region_end_change(torso_root_fe_region);
				}
				DESTROY(Computed_field)(&cmiss_number_field);

				/* merge into the root_cmiss_region */
				if (Cmiss_regions_FE_regions_can_be_merged(root_cmiss_region,
					torso_root_cmiss_region) &&
					Cmiss_regions_merge_FE_regions(root_cmiss_region,
						torso_root_cmiss_region))
				{
					/* store the group name in the unemap_package */
					package->torso_group_name = torso_group_name;
					/* define the fit field on the default torso */
					torso_element_group = Cmiss_region_get_FE_region(
						Cmiss_region_get_child_region_from_name(root_cmiss_region,
							torso_group_name));
					define_fit_field_at_quad_elements_and_nodes(torso_element_group,
						package->map_fit_field);
					/* add cylindrical field info for texture mapping to default torso */
					add_cylindrical_info_to_cartesian_torso(torso_group_name, package);
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE, "unemap_package_read_torso_file.  "
						"Torso file '%s' not compatible with global objects",
						torso_file_name);
					DEALLOCATE(torso_group_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_package_read_torso_file.  Invalid torso file '%s'",
					torso_file_name);
				return_code = 0;
			}
			DESTROY(Cmiss_region)(&element_root_cmiss_region);
			DESTROY(Cmiss_region)(&torso_root_cmiss_region);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_package_read_torso_file.  Could not allocate filenames");
			return_code = 0;
		}
		DEALLOCATE(exelem_name_str);
		DEALLOCATE(exnode_name_str);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_package_read_torso_file.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* unemap_package_read_torso_file */
#endif /* defined (UNEMAP_USE_3D)*/

