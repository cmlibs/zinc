/*******************************************************************************
FILE : creating_FE_nodes.c

LAST MODIFIED : 20 December 1994

DESCRIPTION :
Created to show Paul Charette how to set up the node field info when creating
nodes graphically.
???DB.  Use as the basis of a create node command in the front end ?
==============================================================================*/
#include "general/debug.h"
#include "general/system.h"
#include "general/user_interface.h"
#include "graphics/finite_element.h"

main()
{
	char *component_names[]=
	{
		"x",
		"y"
	};
	FE_value nodal_values[]={0,0};
	struct FE_field *field;
	struct FE_node *node;
	struct FE_node_field *node_field;
	struct FE_node_field_info *node_field_info;
	struct FE_node_field_list_item *coordinate_node_field_list=
		(struct FE_node_field_list_item *)NULL;

	/* create the 2d coordinate field (automatically checks if it already
		exists) */
		/*???DB.  We may want to create it as 3d and let the user specify the z
			component ? */
		/*???DB.  field is a local variable and is destroyed at the end of the then
			clause.  Alternatively there could be an access at the beginning and a
			deaccess at the end of the then clause.  Should create_ automatically
			access ? */
	if (field=create_FE_field("coordinate_2d",COORDINATE,0,(FE_value *)NULL,
		RECTANGULAR_CARTESIAN,2,component_names))
	{
		/* create the structure for accessing the 2d coordinate field values at a
			node */
		if (node_field=create_FE_node_field(field))
		{
			(node_field->components)[0].value=0;
			(node_field->components)[0].number_of_derivatives=0;
			(node_field->components)[1].value=1;
			(node_field->components)[1].number_of_derivatives=0;
			/* add node field to the temporary list of coordinate node fields */
			if (add_FE_node_field_to_list(node_field,coordinate_node_field_list))
			{
				/* create the structure for accessing all the field values at a node */
				if (node_field_info=create_FE_node_field_info(3,
					coordinate_node_field_list,(struct FE_node_field_list_item *)NULL,
					(struct FE_node_field_list_item *)NULL))
				{
					/* create a node */
						/*???DB.  This will have to be a loop for graphical creation */
					if (node=create_FE_node(1,(struct FE_node_group *)NULL,
						node_field_info,nodal_values))
					{
/*???debug */
printf("created node\n");
					}
					else
					{
						write_error(CM_ERROR_MESSAGE,"main.  Could not create node");
					}
					destroy_FE_node_field_info(&node_field_info);
				}
				else
				{
					write_error(CM_ERROR_MESSAGE,
						"main.  Could not create node field information");
				}
				destroy_FE_node_field_list(&coordinate_node_field_list);
			}
			else
			{
				write_error(CM_ERROR_MESSAGE,
					"main.  Could not add coordinate_2d node field to list");
			}
			destroy_FE_node_field(&node_field);
		}
		else
		{
			write_error(CM_ERROR_MESSAGE,
				"main.  Could not create coordinate_2d node field");
		}
		destroy_FE_field(&field);
	}
	else
	{
		write_error(CM_ERROR_MESSAGE,"main.  Could not create coordinate_2d field");
	}
} /* main */
