/*******************************************************************************
FILE : element_point_ranges.c

LAST MODIFIED : 30 May 2000

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

char **Xi_discretization_mode_get_valid_strings_for_Element_point_ranges(
	int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Xi_discretization_modes that can be used for Element_point_ranges, obtained
from function Xi_discretization_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;

	ENTER(Xi_discretization_mode_get_valid_strings_for_Element_point_ranges);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=2;
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			valid_strings[0]=
				Xi_discretization_mode_string(XI_DISCRETIZATION_CELL_CENTRES);
			valid_strings[1]=
				Xi_discretization_mode_string(XI_DISCRETIZATION_CELL_CORNERS);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Xi_discretization_mode_get_valid_strings_for_Element_point_ranges.  "
				"Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Xi_discretization_mode_get_valid_strings_for_Element_point_ranges.  "
			"Invalid argument(s)");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Xi_discretization_mode_get_valid_strings_for_Element_point_ranges */

int Element_point_ranges_identifier_is_valid(
	struct Element_point_ranges_identifier *identifier)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Returns true if <identifier> has a valid element, Xi_discretization_mode and
number_in_xi for being used in an Element_point_ranges structure.
Writes what is invalid about the identifier.
==============================================================================*/
{
	int dimension,i,return_code;

	ENTER(Element_point_ranges_identifier_is_valid);
	if (identifier)
	{
		if (identifier->element)
		{
			if ((XI_DISCRETIZATION_CELL_CENTRES==identifier->xi_discretization_mode)||
				(XI_DISCRETIZATION_CELL_CORNERS==identifier->xi_discretization_mode))
			{
				return_code=1;
				dimension=get_FE_element_dimension(identifier->element);
				for (i=0;i<dimension;i++)
				{
					if (1 > identifier->number_in_xi[i])
					{
						display_message(ERROR_MESSAGE,
							"Element_point_ranges_identifier_is_valid.  "
							"Invalid number_in_xi[%d] of %d",i,identifier->number_in_xi[i]);
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_identifier_is_valid.  "
					"Invalid Xi_discretization_mode: %p",
					Xi_discretization_mode_string(identifier->xi_discretization_mode));
				return_code=0;
			}
		}
		else
		{
			printf("Element_point_ranges_identifier_is_valid.  Missing element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_identifier_is_valid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* Element_point_ranges_identifier_is_valid */

int Element_point_ranges_identifier_element_point_number_is_valid(
	struct Element_point_ranges_identifier *identifier,int element_point_number)
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION :
Returns true if <element_point_number> is in the number_in_xi range for
<identifier>. Assumes <identifier> is already validated by
Element_point_ranges_identifier_is_valid.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_identifier_element_point_number_is_valid);
	if (identifier)
	{
		return_code = ((0<=element_point_number)&&(element_point_number<
			Xi_discretization_mode_get_number_of_xi_points(
				identifier->xi_discretization_mode,
				get_FE_element_dimension(identifier->element),
				identifier->number_in_xi)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_identifier_element_point_number_is_valid.  "
			"Missing identifier");
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* Element_point_ranges_identifier_element_point_number_is_valid */

PROTOTYPE_COPY_OBJECT_FUNCTION(Element_point_ranges_identifier)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
syntax: COPY(Element_point_ranges_identifier)(destination,source)
Copies the contents of source to destination.
Note! No accessing of elements is assumed or performed by this function; it is
purely a copy. [DE]ACCESSing must be handled by calling function if required.
==============================================================================*/
{
	int i,return_code;

	ENTER(COPY(Element_point_ranges_identifier));
	if (destination&&source&&(destination!=source))
	{
		destination->element=source->element;
		destination->xi_discretization_mode=source->xi_discretization_mode;
		for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
		{
			destination->number_in_xi[i]=source->number_in_xi[i];
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"COPY(Element_point_ranges_identifier).  Invalid argument(s)");
		/* error defaults to the same? */
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* COPY(Element_point_ranges_identifier) */

struct Element_point_ranges *CREATE(Element_point_ranges)(
	struct Element_point_ranges_identifier *identifier)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Creates an Element_point_ranges object that can store ranges of points in the
element:Xi_discretization_mode of the <identifier>.
==============================================================================*/
{
	struct Element_point_ranges *element_point_ranges;

	ENTER(CREATE(Element_point_ranges));
	element_point_ranges=(struct Element_point_ranges *)NULL;
	if (Element_point_ranges_identifier_is_valid(identifier))
	{
		if (ALLOCATE(element_point_ranges,struct Element_point_ranges,1)&&
			(element_point_ranges->ranges=CREATE(Multi_range)()))
		{
			/* ensure identifier points at id for indexed lists */
			element_point_ranges->identifier = &(element_point_ranges->id);
			COPY(Element_point_ranges_identifier)(
				element_point_ranges->identifier,identifier);
			/* struct Element_point_ranges ACCESSes the element in the identifier */
			ACCESS(FE_element)(element_point_ranges->identifier->element);
			element_point_ranges->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_point_ranges).  Not enough memory");
			DEALLOCATE(element_point_ranges);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_point_ranges).  Invalid identifier");
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

int Element_point_ranges_has_ranges(
	struct Element_point_ranges *element_point_ranges)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Returns true if <element_point_ranges> has ranges, ie. is not empty.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_has_ranges);
	if (element_point_ranges)
	{
		return_code=
			(0<Multi_range_get_number_of_ranges(element_point_ranges->ranges));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_has_ranges.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_has_ranges */

int Element_point_ranges_add_to_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Ensures the <element_point_ranges> are in <element_point_ranges_list>.
==============================================================================*/
{
	int i,number_of_ranges,return_code,start,stop;
	struct Element_point_ranges *existing_element_point_ranges,
		*new_element_point_ranges;
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
			if ((new_element_point_ranges=CREATE(Element_point_ranges)(
				element_point_ranges->identifier))&&
				Multi_range_copy(new_element_point_ranges->ranges,
					element_point_ranges->ranges)&&
				ADD_OBJECT_TO_LIST(Element_point_ranges)(
					new_element_point_ranges,element_point_ranges_list))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_add_to_list.  Not add ranges");
				if (new_element_point_ranges)
				{
					DESTROY(Element_point_ranges)(&new_element_point_ranges);
				}
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
LAST MODIFIED : 12 May 2000

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
					" cell_centres|cell_corners #xi1 #xi2.. #xiN "
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

struct Element_point_ranges *Element_point_ranges_from_grid_field_ranges(
	struct FE_element *element,struct FE_field *grid_field,
	struct Multi_range *ranges)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
If <grid_field> is a single component grid-based field in <element>, creates and
returns an Element_point_ranges containing all the grid points at which the
value of <grid_field> is in the <ranges>.
No Element_point_ranges object is returned without error if:
- <grid_field> is not grid-based in <element>.
- No grid points in <element> have <grid_field> value in the given <ranges>.
==============================================================================*/
{
	int grid_value_in_range,i,number_of_grid_values,*values;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier identifier;

	ENTER(Element_point_ranges_from_grid_field_ranges);
	element_point_ranges=(struct Element_point_ranges *)NULL;
	if (element&&grid_field&&ranges&&
		(1==get_FE_field_number_of_components(grid_field)&&
		(INT_VALUE==get_FE_field_value_type(grid_field))))
	{
		if (FE_element_field_is_grid_based(element,grid_field))
		{
			if (get_FE_element_field_component_grid_int_values(element,
				grid_field,/*component_number*/0,&values))
			{
				number_of_grid_values=
					get_FE_element_field_number_of_grid_values(element,grid_field);
				/* work out if any values are in the given ranges */
				grid_value_in_range=0;
				for (i=0;(i<number_of_grid_values)&&(!grid_value_in_range);i++)
				{
					grid_value_in_range=Multi_range_is_value_in_range(ranges,values[i]);
				}
				if (grid_value_in_range)
				{
					identifier.element=element;
					identifier.xi_discretization_mode=XI_DISCRETIZATION_CELL_CORNERS;
					get_FE_element_field_grid_map_number_in_xi(element,grid_field,
						identifier.number_in_xi);
					if (element_point_ranges=CREATE(Element_point_ranges)(&identifier))
					{
						for (i=0;i<number_of_grid_values;i++)
						{
							if (Multi_range_is_value_in_range(ranges,values[i]))
							{
								Element_point_ranges_add_range(element_point_ranges,i,i);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_point_ranges_from_grid_field_ranges.  "
							"Could not create Element_point_ranges");
					}
				}
				DEALLOCATE(values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_ranges_from_grid_field_ranges.  Invalid grid field");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_from_grid_field_ranges.  Invalid argument(s)");
	}
	LEAVE;

	return (element_point_ranges);
} /* Element_point_ranges_from_grid_field_ranges */

int FE_element_grid_to_Element_point_ranges_list(struct FE_element *element,
	void *grid_to_list_data_void)
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Iterator function that gets an Element_point_ranges structure representing all
the grid_points in <element> with discretization of the single component
integer <grid_field>, for which the field value is in the given <ranges>.
Note that there may legitimately be none if <grid_field> is not grid-based in
<element> or the ranges do not intersect with the values in the field.
The structure is then added to the <element_point_ranges_list>.
select_data_void should point to a
struct FE_element_grid_to_Element_point_ranges_list_data.
Uses only top level elements, type CM_ELEMENT.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_grid_to_Element_point_ranges_list_data *grid_to_list_data;

	ENTER(FE_element_grid_to_Element_point_ranges_list);
	if (element&&(grid_to_list_data=
		(struct FE_element_grid_to_Element_point_ranges_list_data *)
		grid_to_list_data_void))
	{
		if ((CM_ELEMENT == element->cm.type)&&
			(element_point_ranges=Element_point_ranges_from_grid_field_ranges(
				element,grid_to_list_data->grid_fe_field,
				grid_to_list_data->grid_value_ranges)))
		{
			return_code=Element_point_ranges_add_to_list(element_point_ranges,
				(void *)grid_to_list_data->element_point_ranges_list);
			DESTROY(Element_point_ranges)(&element_point_ranges);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_grid_to_Element_point_ranges_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_grid_to_Element_point_ranges_list */
