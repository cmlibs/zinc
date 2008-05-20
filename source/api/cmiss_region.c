/*******************************************************************************
FILE : cmiss_region.c

LAST MODIFIED : 02 March 2005

DESCRIPTION :
The public interface to the Cmiss_regions.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <stdlib.h>
#include "api/cmiss_region.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/import_finite_element.h"
#include "general/io_stream.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

struct Cmiss_region *Cmiss_region_create(void)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Creates an empty Cmiss_region.
==============================================================================*/
{
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct LIST(FE_element_shape) *element_shape_list;
	struct MANAGER(FE_basis) *basis_manager;

	ENTER(Cmiss_region_create);
	if (region = CREATE(Cmiss_region)())
	{
		ACCESS(Cmiss_region)(region);
		if ((basis_manager=CREATE_MANAGER(FE_basis)()) && 
			(element_shape_list=CREATE(LIST(FE_element_shape))()))
		{
			if (fe_region=CREATE(FE_region)((struct FE_region *)NULL,basis_manager,
					 element_shape_list))
			{
				if (!Cmiss_region_attach_FE_region(region,fe_region))
				{
					DEACCESS(Cmiss_region)(&region);
				}
			}
			else
			{
				DEACCESS(Cmiss_region)(&region);
			}
		}
		else
		{
			DEACCESS(Cmiss_region)(&region);
		}

	}
	LEAVE;

	return (region);
} /* Cmiss_region_create */

int Cmiss_region_destroy(Cmiss_region_id *region)
/*******************************************************************************
LAST MODIFIED : 3 January 2008

DESCRIPTION :
Destroys the <region> and sets the pointer to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_destroy);
	return_code = 0;
	if (region && *region)
	{
		return_code = DEACCESS(Cmiss_region)(region);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_destroy */

int Cmiss_region_begin_change_API(Cmiss_region_id region)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Changes made to the <region> between Cmiss_region_begin_change and
Cmiss_region_end_change do not generate events in the rest of cmgui until
the change count returns to zero.  This allows many changes to be made 
efficiently, resulting in only one update of the dependent objects.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_begin_change_API);
	return_code = 0;
	if (region)
	{
		Cmiss_region_begin_change(region);
		if (fe_region=Cmiss_region_get_FE_region(region))
		{
			return_code = FE_region_begin_change(fe_region);
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_begin_change_API */

int Cmiss_region_end_change_API(Cmiss_region_id region)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Changes made to the <region> between Cmiss_region_begin_change and
Cmiss_region_end_change do not generate events in the rest of cmgui until
the change count returns to zero.  This allows many changes to be made 
efficiently, resulting in only one update of the dependent objects.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_end_change_API);
	return_code = 0;
	if (region)
	{
		Cmiss_region_end_change(region);
		if (fe_region=Cmiss_region_get_FE_region(region))
		{
			return_code = FE_region_end_change(fe_region);
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_end_change_API */

int Cmiss_region_read_file(struct Cmiss_region *region, char *file_name)
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *temp_region;
	struct FE_region *fe_region;
	struct IO_stream_package *io_stream_package;
	struct LIST(FE_element_shape) *element_shape_list;
	struct MANAGER(FE_basis) *basis_manager;

	ENTER(Cmiss_region_read_file);
	return_code = 0;
	if (region&&file_name&&(fe_region=Cmiss_region_get_FE_region(region))&&
		(basis_manager=FE_region_get_basis_manager(fe_region))&&
		(element_shape_list=FE_region_get_FE_element_shape_list(fe_region)) && 
		(io_stream_package=CREATE(IO_stream_package)()))
	{
		if (temp_region=read_exregion_file_of_name(file_name,io_stream_package,
			basis_manager,element_shape_list,(struct FE_import_time_index *)NULL))
		{
			ACCESS(Cmiss_region)(temp_region);
			if (Cmiss_regions_FE_regions_can_be_merged(region,temp_region))
			{
				return_code=Cmiss_regions_merge_FE_regions(region,temp_region);
			}
			DEACCESS(Cmiss_region)(&temp_region);
		}
		DESTROY(IO_stream_package)(&io_stream_package);
	}
	LEAVE;

	return(return_code);
}

struct Cmiss_region *Cmiss_region_get_sub_region(struct Cmiss_region *region,
	const char *path)
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Returns the sub_region specified by the string <path> in <region>.
==============================================================================*/
{
	struct Cmiss_region *return_region;

	ENTER(Cmiss_region_get_sub_region);
	if (region && path)
	{
		if (Cmiss_region_get_region_from_path(region, (char *)path, &return_region))
		{
			/* ACCESS(Cmiss_region)(RETVAL); */
		}
	}
	else
	{
		return_region = (struct Cmiss_region *)NULL;
	}
	LEAVE;

	return (return_region);
} /* Cmiss_region_get_sub_region */

struct Cmiss_element *Cmiss_region_get_element(struct Cmiss_region *region,
	const char *name)
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns element with <name> in <region> if it exists.
==============================================================================*/
{
	int name_length;
	struct CM_element_information identifier;
	struct Cmiss_element *return_element;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_get_element);
	return_element = (struct Cmiss_element *)NULL;
	if (region&&name)
	{
		if (fe_region=Cmiss_region_get_FE_region(region))
		{
			if (((1==sscanf(name," %d %n",&(identifier.number),&name_length))
				||(1==sscanf(name," E%d %n",&(identifier.number),&name_length))
				||(1==sscanf(name," Element%d %n",&(identifier.number),&name_length)))
				&&((unsigned int)name_length==strlen(name)))
			{
				identifier.type = CM_ELEMENT;
				return_element = FE_region_get_FE_element_from_identifier(fe_region,
					&identifier);
			}
			else if (((1==sscanf(name," F%d %n",&(identifier.number),&name_length))
				||(1==sscanf(name," Face%d %n",&(identifier.number),&name_length)))
				&&((unsigned int)name_length==strlen(name)))
			{
				identifier.type = CM_FACE;
				return_element = FE_region_get_FE_element_from_identifier(fe_region,
					&identifier);
			}
			else if (((1==sscanf(name," L%d %n",&(identifier.number),&name_length))
				||(1==sscanf(name," Line%d %n",&(identifier.number),&name_length)))
				&&((unsigned int)name_length==strlen(name)))
			{
				identifier.type = CM_LINE;
				return_element = FE_region_get_FE_element_from_identifier(fe_region,
					&identifier);
			}
		}
	}
	LEAVE;

	return (return_element);
} /* Cmiss_region_get_element */

struct Cmiss_node *Cmiss_region_get_node(struct Cmiss_region *region,
	const char *name)
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns element with <name> in <region> if it exists.
==============================================================================*/
{
	int name_length, node_number;
	struct Cmiss_node *return_node;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_get_node);
	return_node = (struct Cmiss_node *)NULL;
	if (region&&name)
	{
		if (fe_region = Cmiss_region_get_FE_region(region))
		{
			if (1==sscanf(name," %d %n",&node_number,&name_length))
			{
				return_node = ACCESS(FE_node)(
					FE_region_get_FE_node_from_identifier(fe_region, node_number));
			}
		}
	}
	LEAVE;

	return (return_node);
} /* Cmiss_region_get_node */

int Cmiss_region_get_number_of_nodes_in_region(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns the number of nodes in the <region>.
==============================================================================*/
{
	int number_of_nodes;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_get_number_of_nodes_in_region);
	number_of_nodes = 0;
	if (region)
	{
		if (fe_region = Cmiss_region_get_FE_region(region))
		{
			number_of_nodes = FE_region_get_number_of_FE_nodes(fe_region);
		}
	}
	LEAVE;

	return (number_of_nodes);
} /* Cmiss_region_get_number_of_elements */

int Cmiss_region_get_number_of_elements_in_region(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 02 March 2005

DESCRIPTION :
Returns the number of elements in the <region>.
==============================================================================*/
{
	int number_of_elements;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_get_number_of_elements_in_region);
	number_of_elements = 0;
	if (region)
	{
		if (fe_region = Cmiss_region_get_FE_region(region))
		{
			number_of_elements = FE_region_get_number_of_FE_elements(fe_region);
		}
	}
	LEAVE;

	return (number_of_elements);
} /* Cmiss_region_get_number_of_elements */

Cmiss_node_id Cmiss_region_merge_Cmiss_node(Cmiss_region_id region,
	Cmiss_node_id node)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Checks <node> is compatible with <region> and any existing Cmiss_node
using the same identifier, then merges it into <region>.
If no FE_node of the same identifier exists in FE_region, <node> is added
to <fe_region> and returned by this function, otherwise changes are merged into
the existing FE_node and it is returned.
==============================================================================*/
{
	Cmiss_node_id returned_node;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_merge_Cmiss_node);
	returned_node = (Cmiss_node_id)NULL;
	if (region&&node)
	{
		if (fe_region=Cmiss_region_get_FE_region(region))
		{
			returned_node = ACCESS(FE_node)(FE_region_merge_FE_node(fe_region, node));
		}
	}
	LEAVE;

	return (returned_node);
} /* Cmiss_region_merge_Cmiss_node */

int Cmiss_region_for_each_node_in_region(struct Cmiss_region *region,
	Cmiss_node_iterator_function iterator_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Iterates over each node in <region>.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_for_each_node_in_region);
	return_code = 0;
	if (region&&iterator_function)
	{
		if (fe_region=Cmiss_region_get_FE_region(region))
		{
			return_code=FE_region_for_each_FE_node(fe_region,iterator_function,
				user_data);
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_for_each_node_in_region */

int Cmiss_region_for_each_element_in_region(struct Cmiss_region *region,
	Cmiss_element_iterator_function iterator_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 02 March 2005

DESCRIPTION :
Iterates over each element in <region>.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_for_each_element_in_region);
	return_code = 0;
	if (region&&iterator_function)
	{
		if (fe_region=Cmiss_region_get_FE_region(region))
		{
			return_code=FE_region_for_each_FE_element(fe_region,iterator_function,
				user_data);
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_for_each_element_in_region */

Cmiss_element_id Cmiss_region_merge_Cmiss_element(Cmiss_region_id region,
	Cmiss_element_id element)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Checks <element> is compatible with <region> and any existing Cmiss_element
using the same identifier, then merges it into <region>.
If no Cmiss_element of the same identifier exists in Cmiss_region, <element> is added
to <region> and returned by this function, otherwise changes are merged into
the existing Cmiss_element and it is returned.
==============================================================================*/
{
	Cmiss_element_id returned_element;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_merge_Cmiss_element);
	returned_element = (Cmiss_element_id)NULL;
	if (region&&element)
	{
		if (fe_region=Cmiss_region_get_FE_region(region))
		{
			returned_element = ACCESS(FE_element)(
				FE_region_merge_FE_element(fe_region, element));
		}
	}
	LEAVE;

	return (returned_element);
} /* Cmiss_region_merge_Cmiss_element */

Cmiss_field_id Cmiss_region_add_field(Cmiss_region_id region,
	Cmiss_field_id field)
/*******************************************************************************
LAST MODIFIED : 13 May 2008

DESCRIPTION :
Adds the <field> to <region>.  The <field> pointer is returned on success
as a convenience.
==============================================================================*/
{
	char *field_name;
	struct MANAGER(Computed_field) *manager;

	ENTER(Cmiss_region_add_field);
	if (region && 
		(manager = Cmiss_region_get_Computed_field_manager(region))
		&& field)
	{
		if (GET_NAME(Computed_field)(field, &field_name))
		{
			if (0 == field_name[0])
			{
				char name[100];
				int number = NUMBER_IN_MANAGER(Computed_field)(manager);
			
				/* Make a 'unique' name based on the number_of_objects in the manager */
				sprintf(name, "temp%d", number);
				while(FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							name, manager))
				{
					number++;
					sprintf(name, "temp%d", number);
				}
				Computed_field_set_name(field, name);

				/* We assume by default that fields made this way are
					intermediaries which will get destroyed when only
					the manager is referencing them. */
				Computed_field_set_intermediary_managed_field_flag(
					field, 1);
			}
			DEALLOCATE(field_name);

			if (!ADD_OBJECT_TO_MANAGER(Computed_field)(field, manager))
			{
				field = (struct Cmiss_field *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_add_field.  Invalid argument(s)");
		field = (struct Cmiss_field *)NULL;
	}
	LEAVE;

	return (field);
} /* Cmiss_region_add_field */

Cmiss_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region, 
	const char *field_name)
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Returns the field of <field_name> from <region> if it is defined.
==============================================================================*/
{
	struct Cmiss_field *field;
	struct MANAGER(Computed_field) *manager;

	ENTER(Cmiss_region_find_field_by_name);
	if (region && field_name && 
		(manager = Cmiss_region_get_Computed_field_manager(region)))
	{
		field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			(char *)field_name, manager);
		if (field)
		{
			ACCESS(Computed_field)(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_find_field_by_name.  Invalid argument(s)");
		field = (struct Cmiss_field *)NULL;
	}
	LEAVE;

	return (field);
} /* Cmiss_region_find_field_by_name */

