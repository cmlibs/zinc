/*******************************************************************************
FILE : unemap_package.c

LAST MODIFIED : 26 July 2000

DESCRIPTION :
Contains function definitions for unemap package.
==============================================================================*/
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "finite_element/finite_element.h"
#if defined (UNEMAP_USE_NODES)
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "graphics/graphical_element.h"
#endif /* defined (UNEMAP_USE_NODES) */
#include "general/debug.h"
#include "graphics/colour.h"
#include "unemap/unemap_package.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#if defined (UNEMAP_USE_3D)

DECLARE_OBJECT_FUNCTIONS(Unemap_package)

struct Unemap_package *CREATE(Unemap_package)(
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_basis) *fe_basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct FE_node_selection *node_selection)
/*******************************************************************************
LAST MODIFIED : 31 AUGUST 2000

DESCRIPTION:
Create a Unemap package, and fill in the managers.
The fields are filed in with set_unemap_package_fields()
==============================================================================*/
{	
	struct Unemap_package *package;

	ENTER(CREATE(Unemap_package));
	if (fe_field_manager&&element_group_manager&&node_manager&&
		data_group_manager&&node_group_manager&&fe_basis_manager&&element_manager&&		
		computed_field_manager&&interactive_tool_manager&&node_selection)
	{
		if (ALLOCATE(package,struct Unemap_package,1))
		{
			package->fe_field_manager=fe_field_manager;
			package->element_group_manager=element_group_manager;
			package->node_manager=node_manager;
			package->data_manager=data_manager;
			package->data_group_manager=data_group_manager;
			package->node_group_manager=node_group_manager;
			package->fe_basis_manager=fe_basis_manager;	
			package->element_manager=element_manager;
			package->computed_field_manager=computed_field_manager;
			package->interactive_tool_manager=interactive_tool_manager;
			package->node_selection=node_selection;
			/* fields of the rig_nodes */
			package->device_name_field=(struct FE_field *)NULL;
			package->device_type_field=(struct FE_field *)NULL;
#if defined (UNEMAP_USE_NODES)
			package->display_end_time_field=(struct FE_field *)NULL;
			package->display_start_time_field=(struct FE_field *)NULL;
			package->highlight_field=(struct FE_field *)NULL;
#endif /* defined (UNEMAP_USE_NODES)*/
			package->channel_number_field=(struct FE_field *)NULL;
			package->read_order_field=(struct FE_field *)NULL;			
			package->signal_field=(struct FE_field *)NULL;
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
			package->time_field=(struct Computed_field *)NULL;
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
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Frees the memory for the Unemap_package node field and sets <*package_address>
to NULL.
==============================================================================*/
{
	int return_code;
	struct Unemap_package *package;

	ENTER(DESTROY(Unemap_package));
	if ((package_address)&&(package= *package_address))
	{		
		DEACCESS(FE_field)(&(package->device_name_field));
		DEACCESS(FE_field)(&(package->device_type_field));
		DEACCESS(FE_field)(&(package->channel_number_field));
#if defined (UNEMAP_USE_NODES)
		DEACCESS(FE_field)(&(package->display_start_time_field));
		DEACCESS(FE_field)(&(package->display_end_time_field));
		DEACCESS(FE_field)(&(package->highlight_field));
#endif /* defined (UNEMAP_USE_NODES)*/
		DEACCESS(FE_field)(&(package->read_order_field));		
		DEACCESS(FE_field)(&(package->signal_field));
		DEACCESS(FE_field)(&(package->signal_minimum_field));
		DEACCESS(FE_field)(&(package->signal_maximum_field));	
		DEACCESS(FE_field)(&(package->signal_status_field));
		DEACCESS(FE_field)(&(package->channel_gain_field));
		DEACCESS(FE_field)(&(package->channel_offset_field));	
		DEACCESS(Computed_field)(&(package->scaled_offset_signal_value_at_time_field));
		DEACCESS(Computed_field)(&(package->offset_signal_value_at_time_field));
		DEACCESS(Computed_field)(&(package->signal_value_at_time_field));
		DEACCESS(Computed_field)(&(package->time_field));		
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

		if (FE_field_can_be_destroyed(package->device_name_field))
		{
			printf("bibble\n");
		}
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

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_unemap_package_highlight_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 12 1999

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
struct Computed_field *get_unemap_package_time_field(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
{
	struct Computed_field *time_field;
	ENTER(get_unemap_package_time_field);
	if(package)
	{
		time_field=package->time_field; 
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_time_field."
				" invalid arguments");
		time_field = (struct Computed_field *)NULL;
	}
	LEAVE;
	return (time_field);
} /* get_unemap_package_time_field */

int set_unemap_package_time_field(struct Unemap_package *package,
	struct Computed_field *time_field)
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
{
	int return_code;

	ENTER(set_unemap_package_time_field);
	if(package)
	{
		return_code =1;	
		REACCESS(Computed_field)(&(package->time_field),
			time_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_unemap_package_time_field."
				" invalid arguments");
		return_code =0;
	}
	LEAVE;
	return (return_code);
} /* set_unemap_package_time_field */
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
struct MANAGER(FE_field) *get_unemap_package_FE_field_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_field) *fe_field_manager;

	ENTER(get_unemap_package_FE_field_manager);
	if(package)
	{
		fe_field_manager=package->fe_field_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_FE_field_manager."
			" invalid arguments");
		fe_field_manager = (struct MANAGER(FE_field) *)NULL;
	}
	LEAVE;
	return(fe_field_manager);
}/* get_unemap_package_FE_field_manager */
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
struct MANAGER(GROUP(FE_element)) *get_unemap_package_element_group_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(GROUP(FE_element)) *element_group_manager;

	ENTER(get_unemap_package_element_group_manager);
	if(package)
	{
		element_group_manager=package->element_group_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_element_group_manager."
			" invalid arguments");
		element_group_manager = (struct MANAGER(GROUP(FE_element)) *)NULL;
	}
	LEAVE;
	return(element_group_manager);
}/* get_unemap_package_element_group_manager */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_node) *get_unemap_package_node_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_node) *node_manager;

	ENTER(get_unemap_package_node_manager);
	if(package)
	{
		node_manager=package->node_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_node_manager."
			" invalid arguments");
		node_manager = (struct MANAGER(FE_node) *)NULL;
	}
	LEAVE;
	return(node_manager);
}/* get_unemap_package_node_manager */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_node) *get_unemap_package_data_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_node) *data_manager;

	ENTER(get_unemap_package_data_manager);
	if(package)
	{
		data_manager=package->data_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_data_manager."
			" invalid arguments");
		data_manager = (struct MANAGER(FE_node) *)NULL;
	}
	LEAVE;
	return(data_manager);
}/* get_unemap_package_data_manager */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_element) *get_unemap_package_element_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(FE_element) *element_manager;

	ENTER(get_unemap_package_element_manager);
	if(package)
	{
		element_manager=package->element_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_element_manager."
			" invalid arguments");
		element_manager = (struct MANAGER(FE_element) *)NULL;
	}
	LEAVE;
	return(element_manager);
}/* get_unemap_package_element_manager */
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
struct MANAGER(GROUP(FE_node)) *get_unemap_package_data_group_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(GROUP(FE_node)) *data_group_manager;

	ENTER(get_unemap_package_data_group_manager);
	if(package)
	{
		data_group_manager=package->data_group_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_data_group_manager."
			" invalid arguments");
		data_group_manager = (struct MANAGER(GROUP(FE_node)) *)NULL;
	}
	LEAVE;
	return(data_group_manager);
}/* get_unemap_package_data_group_manager */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(GROUP(FE_node)) *get_unemap_package_node_group_manager(
	struct Unemap_package *package)
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
{
	struct MANAGER(GROUP(FE_node)) *node_group_manager;

	ENTER(get_unemap_package_node_group_manager);
	if(package)
	{
		node_group_manager=package->node_group_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_unemap_package_node_group_manager."
			" invalid arguments");
		node_group_manager = (struct MANAGER(GROUP(FE_node)) *)NULL;
	}
	LEAVE;
	return(node_group_manager);
}/* get_unemap_package_node_group_manager */
#endif /* defined (UNEMAP_USE_3D)*/

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

#if defined (UNEMAP_USE_3D)
int unemap_package_rig_node_group_has_electrodes(struct Unemap_package *package,
	struct GROUP(FE_node) *rig_node_group)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

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
			node=FIRST_OBJECT_IN_GROUP_THAT(FE_node)(rig_node_has_electrode_defined, 
				(void *)package,rig_node_group);
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
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Frees the <unemap_package> rig's computed and fe fields
==============================================================================*/
{
	int return_code;
	struct MANAGER(Computed_field) *computed_field_manager=
		(struct MANAGER(Computed_field) *)NULL;
	struct MANAGER(FE_field) *fe_field_manager=
		(struct MANAGER(FE_field) *)NULL;
	struct FE_field *temp_field=(struct FE_field *)NULL;

	ENTER(free_unemap_package_rig_fields)
	if(unemap_package)
	{
		return_code=1;
		computed_field_manager=unemap_package->computed_field_manager;
		fe_field_manager=unemap_package->fe_field_manager;
		if(unemap_package->device_name_field)
		{
			temp_field=unemap_package->device_name_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->device_name_field))
			{
				unemap_package->device_name_field=(struct FE_field *)NULL;
			}
		}
		if(unemap_package->device_type_field)
		{
			temp_field=unemap_package->device_type_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->device_type_field))
			{
				unemap_package->device_type_field=(struct FE_field *)NULL;
			}
		}
		if(unemap_package->channel_number_field)
		{
			temp_field=unemap_package->channel_number_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->channel_number_field))
			{
				unemap_package->channel_number_field=(struct FE_field *)NULL;
			}
		}	
#if  defined (UNEMAP_USE_NODES)
		if(unemap_package->display_start_time_field)
		{
			temp_field=unemap_package->display_start_time_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->display_start_time_field))
			{
				unemap_package->display_start_time_field=(struct FE_field *)NULL;
			}
		}	
		if(unemap_package->display_end_time_field)
		{
			temp_field=unemap_package->display_end_time_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->display_end_time_field))
			{
				unemap_package->display_end_time_field=(struct FE_field *)NULL;
			}
		}	
		if(unemap_package->highlight_field)
		{
			temp_field=unemap_package->highlight_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->highlight_field))
			{
				unemap_package->highlight_field=(struct FE_field *)NULL;
			}
		}	
#endif /* defined (UNEMAP_USE_NODES) */
		if(unemap_package->read_order_field)
		{
			temp_field=unemap_package->read_order_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->read_order_field))
			{
				unemap_package->read_order_field=(struct FE_field *)NULL;
			}
		}				
		if(unemap_package->signal_field)
		{
			temp_field=unemap_package->signal_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->signal_field))
			{
				unemap_package->signal_field=(struct FE_field *)NULL;
			}
		}
		if(unemap_package->signal_minimum_field)
		{
			temp_field=unemap_package->signal_minimum_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->signal_minimum_field))
			{
				unemap_package->signal_minimum_field=(struct FE_field *)NULL;
			}
		}
		if(unemap_package->signal_maximum_field)
		{
			temp_field=unemap_package->signal_maximum_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->signal_maximum_field))
			{
				unemap_package->signal_maximum_field=(struct FE_field *)NULL;
			}
		}	
		if(unemap_package->signal_status_field)
		{
			temp_field=unemap_package->signal_status_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->signal_status_field))
			{
				unemap_package->signal_status_field=(struct FE_field *)NULL;
			}
		}	
		if(unemap_package->channel_gain_field)
		{
			temp_field=unemap_package->channel_gain_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->channel_gain_field))
			{
				unemap_package->channel_gain_field=(struct FE_field *)NULL;
			}
		}
		if(unemap_package->channel_offset_field)
		{
			temp_field=unemap_package->channel_offset_field;
			DEACCESS(FE_field)(&temp_field);
			if(destroy_computed_field_given_fe_field(computed_field_manager,fe_field_manager,
				unemap_package->channel_offset_field))
			{
				unemap_package->channel_offset_field=(struct FE_field *)NULL;
			}
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
LAST MODIFIED : 4 May 2000

DESCRIPTION :
Frees the time related computed fields (used by the map electrode glyphs) 
stored in the unemap package. Also frees any associated fe_fields
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_field;
	struct FE_field *fe_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(FE_field) *fe_field_manager;

	ENTER(free_unemap_package_time_computed_fields);
	if(unemap_package)
	{
		computed_field=(struct Computed_field *)NULL;	
		fe_field=(struct FE_field *)NULL;
		computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
		fe_field_manager=(struct MANAGER(FE_field) *)NULL;
		return_code=1;
		computed_field_manager=get_unemap_package_Computed_field_manager(unemap_package);
		computed_field=get_unemap_package_scaled_offset_signal_value_at_time_field(
			unemap_package);
		/* following does deaccess*/
		set_unemap_package_scaled_offset_signal_value_at_time_field
						(unemap_package,(struct Computed_field *)NULL);
		if(computed_field)
		{				
			if (Computed_field_can_be_destroyed(computed_field))
			{			
				/* also want to destroy any wrapped FE_field */
				fe_field=(struct FE_field *)NULL;
				if (Computed_field_is_type_finite_element(computed_field))
				{
				  Computed_field_get_type_finite_element(computed_field,
					 &fe_field);
				}
				if(REMOVE_OBJECT_FROM_MANAGER(Computed_field)
					(computed_field,computed_field_manager))
				{
					if (fe_field)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
							fe_field,fe_field_manager);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
						" Couldn't remove scaled_offset_signal_value_at_time_field from manager");
				}
				
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
					"Couldn't destroy scaled_offset_signal_value_at_time_field");
			}				
		}/* if(computed_field) */
		computed_field=get_unemap_package_offset_signal_value_at_time_field(unemap_package);
		/* following does deaccess*/
		set_unemap_package_offset_signal_value_at_time_field
						(unemap_package,(struct Computed_field *)NULL);
		if(computed_field)
		{				
			if (Computed_field_can_be_destroyed(computed_field))
			{			
				/* also want to destroy any wrapped FE_field */
				fe_field=(struct FE_field *)NULL;
				if (Computed_field_is_type_finite_element(computed_field))
				{
				  Computed_field_get_type_finite_element(computed_field,
					 &fe_field);
				}
				if(REMOVE_OBJECT_FROM_MANAGER(Computed_field)
					(computed_field,computed_field_manager))
				{
					if (fe_field)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
							fe_field,fe_field_manager);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
						" Couldn't remove offset_signal_value_at_time_field from manager");
				}				
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
					"Couldn't destroy offset_signal_value_at_time_field");
			}				
		}/* if(computed_field) */
		computed_field=get_unemap_package_signal_value_at_time_field(unemap_package);
		/* following does deaccess*/
		set_unemap_package_signal_value_at_time_field
						(unemap_package,(struct Computed_field *)NULL);
		if(computed_field)
		{				
			if (Computed_field_can_be_destroyed(computed_field))
			{			

				/* also want to destroy any wrapped FE_field */
				fe_field=(struct FE_field *)NULL;
				if (Computed_field_is_type_finite_element(computed_field))
				{
				  Computed_field_get_type_finite_element(computed_field,
					 &fe_field);
				}
				if(REMOVE_OBJECT_FROM_MANAGER(Computed_field)
					(computed_field,computed_field_manager))
				{
					if (fe_field)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
							fe_field,fe_field_manager);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
						" Couldn't remove signal_value_at_time_field from manager");
				}
				
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
					"Couldn't destroy signal_value_at_time_field");
			}				
		}/* if(computed_field) */
		computed_field=get_unemap_package_time_field(unemap_package);	
		/* following does deaccess */
		set_unemap_package_time_field
						(unemap_package,(struct Computed_field *)NULL);
		if(computed_field)
		{	
			if (Computed_field_can_be_destroyed
				(computed_field))
			{
				fe_field=(struct FE_field *)NULL;
				if (Computed_field_is_type_finite_element(computed_field))
				{
				  Computed_field_get_type_finite_element(computed_field,
					 &fe_field);
				}
				if(REMOVE_OBJECT_FROM_MANAGER(Computed_field)
					(computed_field,computed_field_manager))
				{
					if (fe_field)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
							fe_field,fe_field_manager);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields"
						" Couldn't remove time_field from manager");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"free_unemap_package_time_computed_fields. "
					"Couldn't destroy time_field ");
			}
		}/* if(computed_field) */
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
	struct GROUP(FE_node) *rig_node_group)
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Frees up any glyphs used by the nodes in the rig_node_group
==============================================================================*/
{
	char *group_name;
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;
	struct Scene *scene;
	struct GROUP(FE_element) *rig_element_group;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;

	ENTER(free_unemap_package_rig_node_group_glyphs);	
	gt_element_group=(struct GT_element_group *)NULL;
	settings=(struct GT_element_settings *)NULL;
	scene=(struct Scene *)NULL;
	rig_element_group=(struct GROUP(FE_element) *)NULL;
	element_group_manager=(struct MANAGER(GROUP(FE_element)) *)NULL;
	if (package&&rig_node_group&&drawing_information)
	{
		if((scene=get_map_drawing_information_scene(drawing_information))&&
			(element_group_manager=
				get_unemap_package_element_group_manager(package)))
		{
			GET_NAME(GROUP(FE_node))(rig_node_group,&group_name);	 
			rig_element_group=FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)
				(group_name,element_group_manager);
			if (rig_element_group&&(gt_element_group=
				Scene_get_graphical_element_group(scene,rig_element_group)))
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
					"free_unemap_package_rig_node_group_glyphs. rig_element_group/gt_element_group"
						"not found");
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
	DEALLOCATE(group_name);	
	LEAVE;

	return (return_code);
} /* free_unemap_package_rig_node_group_glyphs */
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_rig_node_group(struct Unemap_package *package,	
	struct GROUP(FE_node) **rig_node_group)
/*******************************************************************************
LAST MODIFIED : 17 July 2000 

DESCRIPTION :Frees the node, element and data groups of <rig_node_group>
Note: DOESN't free the glyphs of the node group. See 
free_unemap_package_rig_node_group_glyphs for this
==============================================================================*/
{
	int return_code;	
	
	ENTER(free_unemap_package_rig_node_group);
	if(package&&rig_node_group&&(*rig_node_group))
	{			
		return_code=free_node_and_element_and_data_groups(
			rig_node_group,package->element_manager,
			package->element_group_manager,package->data_manager,
			package->data_group_manager,package->node_manager,
			package->node_group_manager);	
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
