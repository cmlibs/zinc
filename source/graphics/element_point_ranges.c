/*******************************************************************************
FILE : element_point_ranges.c

LAST MODIFIED : 28 March 2000

DESCRIPTION :
==============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "graphics/element_point_ranges.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Element_point_ranges
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Stores ranges of element/grid points in an element, used for selection.
==============================================================================*/
{
	/* identifier points at id for indexing in lists */
	struct Element_point_ranges_identifier id,*identifier;
	struct Multi_range *ranges;
	int access_count;
}; /* struct Element_point_ranges */

FULL_DECLARE_INDEXED_LIST_TYPE(Element_point_ranges);

/*
Module functions
----------------
*/

static int compare_Element_point_ranges_identifier(
	struct Element_point_ranges_identifier *identifier1,
	struct Element_point_ranges_identifier *identifier2)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Returns -1 (identifier1 less), 0 (equal) or +1 (identifier1 greater) for
indexing lists of Element_point_ranges.
First the element is compared, then the Xi_discretization_mode, then the
identifying values depending on this mode.
==============================================================================*/
{
	int dimension,i,return_code;

	ENTER(compare_Element_point_ranges_identifier);
	if (identifier1&&identifier2)
	{
		if (identifier1->element < identifier2->element)
		{
			return_code = -1;
		}
		else if (identifier1->element > identifier2->element)
		{
			return_code = 1;
		}
		else
		{
			/* same element; now compare xi_discretization_mode */
			if (identifier1->xi_discretization_mode <
				identifier2->xi_discretization_mode)
			{
				return_code = -1;
			}
			else if (identifier1->xi_discretization_mode >
				identifier2->xi_discretization_mode)
			{
				return_code = 1;
			}
			else
			{
				return_code=0;
				/* same xi_discretization mode; now compare identifying values
					 depending on this mode */
				dimension=get_FE_element_dimension(identifier1->element);
				for (i=0;!return_code&&(i<dimension);i++)
				{
					if (identifier1->number_in_xi[i] < identifier2->number_in_xi[i])
					{
						return_code = -1;
					}
					else if (identifier1->number_in_xi[i] > identifier2->number_in_xi[i])
					{
						return_code = 1;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compare_Element_point_ranges_identifier.  Invalid argument(s)");
		/* error defaults to the same? */
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compare_Element_point_ranges_identifier */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Element_point_ranges,identifier, \
	struct Element_point_ranges_identifier *, \
	compare_Element_point_ranges_identifier)

/*
Global functions
----------------
*/

struct Element_point_ranges *CREATE(Element_point_ranges)(
	struct Element_point_ranges_identifier *identifier)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Creates an Element_point_ranges object that can store ranges of points in the
element:Xi_discretization_mode of the <identifier>.
==============================================================================*/
{
	int dimension,i,return_code;
	struct Element_point_ranges *element_point_ranges;

	ENTER(CREATE(Element_point_ranges));
	if (identifier&&identifier->element&&(
		(XI_DISCRETIZATION_CELL_CENTRES==identifier->xi_discretization_mode)||
		(XI_DISCRETIZATION_CELL_CORNERS==identifier->xi_discretization_mode)))
	{
		dimension=get_FE_element_dimension(identifier->element);
		return_code=1;
		for (i=0;(i<dimension)&&return_code;i++)
		{
			if (1>identifier->number_in_xi[i])
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Element_point_ranges).  Invalid number_in_xi");
				return_code=0;
			}
		}
		if (return_code)
		{
			if (ALLOCATE(element_point_ranges,struct Element_point_ranges,1)&&
				(element_point_ranges->ranges=CREATE(Multi_range)()))
			{
				element_point_ranges->id.element=
					ACCESS(FE_element)(identifier->element);
				element_point_ranges->id.xi_discretization_mode=
					identifier->xi_discretization_mode;
				for (i=0;i<dimension;i++)
				{
					element_point_ranges->id.number_in_xi[i]=identifier->number_in_xi[i];
				}
				/* ensure identifier points at id for indexed lists */
				element_point_ranges->identifier = &(element_point_ranges->id);
				element_point_ranges->access_count=0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Element_point_ranges).  Not enough memory");
				DEALLOCATE(element_point_ranges);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_point_ranges).  Invalid argument(s)");
	}
	LEAVE;

	return (element_point_ranges);
} /* CREATE(Element_point_ranges) */

int DESTROY(Element_point_ranges)(
	struct Element_point_ranges **element_point_ranges_address)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Destroys the Element_point_ranges.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges *element_point_ranges;

	ENTER(DESTROY(Element_point_ranges));
	if (element_point_ranges_address&&
		(element_point_ranges= *element_point_ranges_address))
	{
		if (0==element_point_ranges->access_count)
		{
			DEACCESS(FE_element)(&(element_point_ranges->id.element));
			DESTROY(Multi_range)(&(element_point_ranges->ranges));
			DEALLOCATE(*element_point_ranges_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Element_point_ranges).  Non-zero access count!");
			*element_point_ranges_address=(struct Element_point_ranges *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_point_ranges).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Element_point_ranges) */

DECLARE_OBJECT_FUNCTIONS(Element_point_ranges)
DECLARE_INDEXED_LIST_FUNCTIONS(Element_point_ranges)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Element_point_ranges, \
	identifier,struct Element_point_ranges_identifier *, \
	compare_Element_point_ranges_identifier)

int Element_point_ranges_get_identifier(
	struct Element_point_ranges *element_point_ranges,
	struct Element_point_ranges_identifier *identifier)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Puts the contents of the identifier for <element_point_ranges> in the
caller-supplied <identifier>.
==============================================================================*/
{
	int dimension,i,return_code;

	ENTER(Element_point_ranges_get_identifier);
	if (element_point_ranges&&identifier)
	{
		identifier->element=element_point_ranges->id.element;
		identifier->xi_discretization_mode=
			element_point_ranges->id.xi_discretization_mode;
		dimension=get_FE_element_dimension(element_point_ranges->id.element);
		for (i=0;i<dimension;i++)
		{
			identifier->number_in_xi[i]=element_point_ranges->id.number_in_xi[i];
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_get_identifier.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_get_identifier */

int Element_point_ranges_add_range(
	struct Element_point_ranges *element_point_ranges,int start,int stop)
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Adds the range from <start> to <stop> to the ranges in <element_point_ranges>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_add_range);
	if (element_point_ranges)
	{
		return_code=Multi_range_add_range(element_point_ranges->ranges,start,stop);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_add_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_add_range */

struct Multi_range *Element_point_ranges_get_ranges(
	struct Element_point_ranges *element_point_ranges)
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Returns a pointer to the ranges in <element_point_ranges>. This should not be
modified in any way.
==============================================================================*/
{
	struct Multi_range *ranges;

	ENTER(Element_point_ranges_get_ranges);
	if (element_point_ranges)
	{
		ranges=element_point_ranges->ranges;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_get_ranges.  Invalid argument(s)");
		ranges=(struct Multi_range *)NULL;
	}
	LEAVE;

	return (ranges);
} /* Element_point_ranges_get_ranges */

int Element_point_ranges_add_to_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void)
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Ensures the <element_point_ranges> are in <element_point_ranges_list>.
==============================================================================*/
{
	int i,number_of_ranges,return_code,start,stop;
	struct Element_point_ranges *existing_element_point_ranges;
	struct LIST(Element_point_ranges) *element_point_ranges_list;

	ENTER(Element_point_ranges_add_to_list);
	if (element_point_ranges&&(element_point_ranges_list=
		(struct LIST(Element_point_ranges) *)element_point_ranges_list_void)&&
		(0<(number_of_ranges=
			Multi_range_get_number_of_ranges(element_point_ranges->ranges))))
	{
		if (existing_element_point_ranges=
			FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
				element_point_ranges->identifier,element_point_ranges_list))
		{
			return_code=1;
			for (i=0;(i<number_of_ranges)&&return_code;i++)
			{
				if (!(Multi_range_get_range(element_point_ranges->ranges,i,
					&start,&stop)&&
					Multi_range_add_range(existing_element_point_ranges->ranges,
						start,stop)))
				{
					display_message(ERROR_MESSAGE,
						"Element_point_ranges_add_to_list.  Could not add range");
					return_code=0;
				}
			}
		}
		else
		{
			if ((existing_element_point_ranges=CREATE(Element_point_ranges)(
				element_point_ranges->identifier))&&
				Multi_range_copy(existing_element_point_ranges->ranges,
					element_point_ranges->ranges)&&
				ADD_OBJECT_TO_LIST(Element_point_ranges)(
					existing_element_point_ranges,element_point_ranges_list))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_add_to_list.  Not enough memory");
				DESTROY(Element_point_ranges)(&existing_element_point_ranges);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_add_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_add_to_list */

int Element_point_ranges_remove_from_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Ensures the <element_point_ranges> is not in <element_point_ranges_list>.
==============================================================================*/
{
	int i,number_of_ranges,return_code,start,stop;
	struct Element_point_ranges *existing_element_point_ranges;
	struct LIST(Element_point_ranges) *element_point_ranges_list;

	ENTER(Element_point_ranges_remove_from_list);
	if (element_point_ranges&&(element_point_ranges_list=
		(struct LIST(Element_point_ranges) *)element_point_ranges_list_void)&&
		(0<(number_of_ranges=
			Multi_range_get_number_of_ranges(element_point_ranges->ranges))))
	{
		if (existing_element_point_ranges=
			FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
				element_point_ranges->identifier,element_point_ranges_list))
		{
			return_code=1;
			for (i=0;(i<number_of_ranges)&&return_code;i++)
			{
				if (!(Multi_range_get_range(element_point_ranges->ranges,i,
					&start,&stop)&&
					Multi_range_remove_range(existing_element_point_ranges->ranges,
						start,stop)))
				{
					display_message(ERROR_MESSAGE,
						"Element_point_ranges_remove_from_list.  Could not remove range");
					return_code=0;
				}
			}
			/* remove existing_element_point_ranges if empty */
			if (0==Multi_range_get_number_of_ranges(
				existing_element_point_ranges->ranges))
			{
				REMOVE_OBJECT_FROM_LIST(Element_point_ranges)(
					existing_element_point_ranges,element_point_ranges_list);
				return_code=1;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_remove_from_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_remove_from_list */

int Element_point_ranges_toggle_in_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Toggles the <element_point_ranges> in <element_point_ranges_list>.
==============================================================================*/
{
	int i,number_of_ranges,return_code,start,stop;
	struct Element_point_ranges *existing_element_point_ranges;
	struct LIST(Element_point_ranges) *element_point_ranges_list;

	ENTER(Element_point_ranges_toggle_in_list);
	if (element_point_ranges&&(element_point_ranges_list=
		(struct LIST(Element_point_ranges) *)element_point_ranges_list_void)&&
		(0<(number_of_ranges=
			Multi_range_get_number_of_ranges(element_point_ranges->ranges))))
	{
		if (existing_element_point_ranges=
			FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
				element_point_ranges->identifier,element_point_ranges_list))
		{
			return_code=1;
			for (i=0;(i<number_of_ranges)&&return_code;i++)
			{
				if (!(Multi_range_get_range(element_point_ranges->ranges,i,
					&start,&stop)&&
					Multi_range_toggle_range(existing_element_point_ranges->ranges,
						start,stop)))
				{
					display_message(ERROR_MESSAGE,
						"Element_point_ranges_toggle_in_list.  Could not toggle range");
					return_code=0;
				}
			}
		}
		else
		{
			if ((existing_element_point_ranges=CREATE(Element_point_ranges)(
				element_point_ranges->identifier))&&
				Multi_range_copy(existing_element_point_ranges->ranges,
					element_point_ranges->ranges)&&
				ADD_OBJECT_TO_LIST(Element_point_ranges)(
					existing_element_point_ranges,element_point_ranges_list))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_toggle_in_list.  Not enough memory");
				DESTROY(Element_point_ranges)(&existing_element_point_ranges);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_toggle_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_toggle_in_list */

int set_Element_point_ranges(struct Parse_state *state,
	void *element_point_ranges_address_void,void *element_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION :
Modifier function to set an element_point_ranges. <element_point_ranges_address>
should point to a currently-NULL pointer to a struct Element_point_ranges. Upon
successful return an Element_point_ranges will be created and the pointer to it
returned in this location, for the calling function to use or destroy.
==============================================================================*/
{
	char *current_token,**valid_strings,*xi_discretization_mode_string;
	int dimension,i,number_of_xi_points,number_of_valid_strings,return_code,start,
		stop;
	struct CM_element_information cm;
	struct Element_point_ranges *element_point_ranges,
		**element_point_ranges_address;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct MANAGER(FE_element) *element_manager;
	struct Option_table *option_table;

	ENTER(set_Element_point_ranges);
	if (state&&(element_point_ranges_address=
		(struct Element_point_ranges **)element_point_ranges_address_void)&&
		((struct Element_point_ranges *)NULL == *element_point_ranges_address)&&
		(element_manager=(struct MANAGER(FE_element) *)element_manager_void))
	{
		return_code=1;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* element type */
				if (fuzzy_string_compare(current_token,"element"))
				{
					cm.type=CM_ELEMENT;
				}
				else if (fuzzy_string_compare(current_token,"face"))
				{
					cm.type=CM_FACE;
				}
				else if (fuzzy_string_compare(current_token,"line"))
				{
					cm.type=CM_LINE;
				}
				else
				{
					display_message(WARNING_MESSAGE,"Missing element|face|line");
					display_parse_state_location(state);
					return_code=0;
				}
				/* element number */
				if (return_code)
				{
					shift_Parse_state(state,1);
					if (current_token=state->current_token)
					{
						if (strcmp(PARSER_HELP_STRING,current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
						{
							cm.number=atoi(current_token);
							if (element_point_ranges_identifier.element=
								FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(&cm,
									element_manager))
							{
								shift_Parse_state(state,1);
							}
							else
							{
								display_message(WARNING_MESSAGE,"Unknown element: %s %d",
									CM_element_type_string(cm.type),cm.number);
								return_code=0;
							}
						}
						else
						{
							display_message(INFORMATION_MESSAGE," NUMBER");
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing %s number",
							CM_element_type_string(cm.type));
						display_parse_state_location(state);
						return_code=0;
					}
				}
				/* xi_discretization_mode */
				if (return_code)
				{
					option_table=CREATE(Option_table)();
					xi_discretization_mode_string=
						Xi_discretization_mode_string(XI_DISCRETIZATION_CELL_CENTRES);
					valid_strings=
						Xi_discretization_mode_get_valid_strings(&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&xi_discretization_mode_string);
					DEALLOCATE(valid_strings);
					if (return_code=Option_table_parse(option_table,state))
					{
						element_point_ranges_identifier.xi_discretization_mode=
							Xi_discretization_mode_from_string(xi_discretization_mode_string);
					}
					DESTROY(Option_table)(&option_table);
				}
				/* number_in_xi */
				if (return_code)
				{
					dimension=
						get_FE_element_dimension(element_point_ranges_identifier.element);
					for (i=0;i<dimension;i++)
					{
						element_point_ranges_identifier.number_in_xi[0]=1;
					}
					if (return_code=set_int_vector(state,
						(void *)element_point_ranges_identifier.number_in_xi,
						(void *)&dimension))
					{
						/* check number_in_xi are all > 0 */
						if (1>(number_of_xi_points=
							Xi_discretization_mode_get_number_of_xi_points(
								element_point_ranges_identifier.xi_discretization_mode,
								dimension,element_point_ranges_identifier.number_in_xi)))
						{
							display_message(WARNING_MESSAGE,"Invalid number in xi");
							display_parse_state_location(state);
							return_code=0;
						}
					}
				}
				/* ranges */
				if (return_code)
				{
					/* create the element_point_ranges */
					if (element_point_ranges=CREATE(Element_point_ranges)(
						&element_point_ranges_identifier))
					{
						if (set_Multi_range(state,(void *)(element_point_ranges->ranges),
							(void *)NULL))
						{
							if ((0<Multi_range_get_number_of_ranges(
								element_point_ranges->ranges))&&
								(!Multi_range_get_last_start_value(
									element_point_ranges->ranges,0,&start))&&
								(!Multi_range_get_next_stop_value(
									element_point_ranges->ranges,number_of_xi_points-1,&stop)))
							{
								*element_point_ranges_address=element_point_ranges;
							}
							else
							{
								display_message(WARNING_MESSAGE,"Invalid ranges");
								display_parse_state_location(state);
								DESTROY(Element_point_ranges)(&element_point_ranges);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Element_point_ranges.  Could not build ranges");
							DESTROY(Element_point_ranges)(&element_point_ranges);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_Element_point_ranges.  Could not create object");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," element|face|line #"
					" cell_centres|cell_corners etc. #xi1 #xi2.. #xiN "
					"[#|#..#[,#|#..#[,etc.]]]");
				return_code=0;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing element|face|line");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Element_point_ranges.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Element_point_ranges */

int Element_point_ranges_element_is_in_group(
	struct Element_point_ranges *element_point_ranges,void *element_group_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION :
Returns true if the element for <element_point_ranges> is in <element_group>.
==============================================================================*/
{
	int return_code;
	struct GROUP(FE_element) *element_group;

	ENTER(Element_point_ranges_element_is_in_group);
	if (element_point_ranges&&
		(element_group=(struct GROUP(FE_element) *)element_group_void))
	{
		return_code=(struct FE_element *)NULL != IS_OBJECT_IN_GROUP(FE_element)(
				element_point_ranges->id.element,element_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_element_is_in_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_element_is_in_group */
