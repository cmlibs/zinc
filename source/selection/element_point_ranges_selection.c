/*******************************************************************************
FILE : element_point_ranges_selection.c

LAST MODIFIED : 18 May 2000

DESCRIPTION :
Global store of selected element_point_ranges for group actions and
highlighting.
==============================================================================*/
#include <stdio.h>
#include "general/callback_private.h"
#include "general/debug.h"
#include "selection/element_point_ranges_selection.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

FULL_DECLARE_CALLBACK_TYPES(Element_point_ranges_selection_change, \
	struct Element_point_ranges_selection *, \
	struct Element_point_ranges_selection_changes *);

struct Element_point_ranges_selection
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Global store of selected element_point_ranges for group actions and
highlighting.
==============================================================================*/
{
	/* flag indicating whether the cache is on */
	int cache;
	/* list of all element_point_ranges currently selected */
	struct LIST(Element_point_ranges) *element_point_ranges_list;
	/* list of all element_point_ranges selected since the last update;
		 includes those already selected; excludes those subsequently unselected */
	struct LIST(Element_point_ranges)
		*newly_selected_element_point_ranges_list;
	/* list of all element_point_ranges unselected since the last update;
		 excludes those not already selected AND those subsequently selected */
	struct LIST(Element_point_ranges)
		*newly_unselected_element_point_ranges_list;
	/* list of callbacks requested by other objects when selction changes */
	struct LIST(CALLBACK_ITEM(Element_point_ranges_selection_change))
		*change_callback_list;
}; /* struct Element_point_ranges_selection */

/*
Module functions
----------------
*/
DEFINE_CALLBACK_MODULE_FUNCTIONS(Element_point_ranges_selection_change)

DEFINE_CALLBACK_FUNCTIONS(Element_point_ranges_selection_change, \
	struct Element_point_ranges_selection *, \
	struct Element_point_ranges_selection_changes *)

static int Element_point_ranges_selection_update(
	struct Element_point_ranges_selection *element_point_ranges_selection)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Tells the clients of the <element_point_ranges_selection> that objects have
been selected or unselected since the last update callback. Avoids sending
message if caching is on or if no such changes have occurred.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges_selection_changes changes;

	ENTER(Element_point_ranges_selection_update);
	if (element_point_ranges_selection)
	{
		if (!element_point_ranges_selection->cache)
		{
			/* only update if something changed */
			if ((0<NUMBER_IN_LIST(Element_point_ranges)(
				element_point_ranges_selection->
				newly_selected_element_point_ranges_list))||
				(0<NUMBER_IN_LIST(Element_point_ranges)(
					element_point_ranges_selection->
					newly_unselected_element_point_ranges_list)))
			{
				/* construct the call data */
				changes.newly_selected_element_point_ranges_list=
					element_point_ranges_selection->
					newly_selected_element_point_ranges_list;
				changes.newly_unselected_element_point_ranges_list=
					element_point_ranges_selection->
					newly_unselected_element_point_ranges_list;
				/* send the callbacks */
				CALLBACK_LIST_CALL(Element_point_ranges_selection_change)(
					element_point_ranges_selection->change_callback_list,
					element_point_ranges_selection,&changes);
				/* clear the newly selected and unselected lists */
				REMOVE_ALL_OBJECTS_FROM_LIST(Element_point_ranges)(
					element_point_ranges_selection->
					newly_selected_element_point_ranges_list);
				REMOVE_ALL_OBJECTS_FROM_LIST(Element_point_ranges)(
					element_point_ranges_selection->
					newly_unselected_element_point_ranges_list);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_update */

/*
Global functions
----------------
*/

struct Element_point_ranges_selection
	*CREATE(Element_point_ranges_selection)(void)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Creates the global store of selected element_point_ranges for group actions and
highlighting.
==============================================================================*/
{
	struct Element_point_ranges_selection *element_point_ranges_selection;

	ENTER(CREATE(Element_point_ranges_selection));
	if (ALLOCATE(element_point_ranges_selection,
		struct Element_point_ranges_selection,1))
	{
		element_point_ranges_selection->cache=0;
		element_point_ranges_selection->element_point_ranges_list=
			CREATE(LIST(Element_point_ranges))();
		element_point_ranges_selection->newly_selected_element_point_ranges_list=
			CREATE(LIST(Element_point_ranges))();
		element_point_ranges_selection->newly_unselected_element_point_ranges_list=
			CREATE(LIST(Element_point_ranges))();
		element_point_ranges_selection->change_callback_list=
			CREATE(LIST(CALLBACK_ITEM(Element_point_ranges_selection_change)))();
		if (!(element_point_ranges_selection->element_point_ranges_list&&
			element_point_ranges_selection->newly_selected_element_point_ranges_list&&
			element_point_ranges_selection->
			newly_unselected_element_point_ranges_list&&
			element_point_ranges_selection->change_callback_list))
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_point_ranges_selection).  Could not create lists");
			DESTROY(Element_point_ranges_selection)(
				&element_point_ranges_selection);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_point_ranges_selection).  Not enough memory");
	}
	LEAVE;

	return (element_point_ranges_selection);
} /* CREATE(Element_point_ranges_selection) */

int DESTROY(Element_point_ranges_selection)(
	struct Element_point_ranges_selection
	**element_point_ranges_selection_address)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Destroys the Element_point_ranges_selection.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges_selection *element_point_ranges_selection;

	ENTER(DESTROY(Element_point_ranges_selection));
	if (element_point_ranges_selection_address&&
		(element_point_ranges_selection= *element_point_ranges_selection_address))
	{
		DESTROY(LIST(Element_point_ranges))(
			&(element_point_ranges_selection->element_point_ranges_list));
		DESTROY(LIST(Element_point_ranges))(
			&(element_point_ranges_selection->
				newly_selected_element_point_ranges_list));
		DESTROY(LIST(Element_point_ranges))(
			&(element_point_ranges_selection->
				newly_unselected_element_point_ranges_list));
		DESTROY(LIST(CALLBACK_ITEM(Element_point_ranges_selection_change)))(
			&(element_point_ranges_selection->change_callback_list));
		DEALLOCATE(*element_point_ranges_selection_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_point_ranges_selection).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Element_point_ranges_selection) */

int Element_point_ranges_selection_add_callback(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	CALLBACK_FUNCTION(Element_point_ranges_selection_change) *function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Adds a callback to <element_point_ranges_selection> so that when it changes
<function> is called with <user_data>. <function> has 3 arguments, a
struct Element_point_ranges_selection, a
struct Element_point_ranges_selection_changes and the void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_selection_add_callback);
	if (element_point_ranges_selection&&function)
	{
		if (CALLBACK_LIST_ADD_CALLBACK(Element_point_ranges_selection_change)(
			element_point_ranges_selection->change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_ranges_selection_add_callback.  "
				"Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_add_callback.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_add_callback */

int Element_point_ranges_selection_remove_callback(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	CALLBACK_FUNCTION(Element_point_ranges_selection_change) *function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<element_point_ranges_selection>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_selection_remove_callback);
	if (element_point_ranges_selection&&function)
	{
		if (CALLBACK_LIST_REMOVE_CALLBACK(Element_point_ranges_selection_change)(
			element_point_ranges_selection->change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_ranges_selection_remove_callback.  "
				"Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_remove_callback.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_remove_callback */

int Element_point_ranges_selection_begin_cache(
	struct Element_point_ranges_selection *element_point_ranges_selection)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Starts caching for changes in the <element_point_ranges_selection>.
No update callbacks will occur until
Element_point_ranges_selection_end_cache is called.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_selection_begin_cache);
	if (element_point_ranges_selection)
	{
		if (element_point_ranges_selection->cache)
		{
			display_message(ERROR_MESSAGE,
				"Element_point_ranges_selection_begin_cache.  "
				"Cache already enabled");
			return_code=0;
		}
		else
		{
			element_point_ranges_selection->cache=1;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_begin_cache.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_begin_cache */

int Element_point_ranges_selection_end_cache(
	struct Element_point_ranges_selection *element_point_ranges_selection)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Ends caching for selected list changes in the <selection>. Calls
Element_point_ranges_selection_update to inform clients of the changes that
took place while caching was on.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_selection_end_cache);
	if (element_point_ranges_selection)
	{
		if (element_point_ranges_selection->cache)
		{
			element_point_ranges_selection->cache=0;
			Element_point_ranges_selection_update(element_point_ranges_selection);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_ranges_selection_end_cache.  Cache not enabled");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_end_cache.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_end_cache */

int Element_point_ranges_selection_clear(
	struct Element_point_ranges_selection *element_point_ranges_selection)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Clears all the selected element_point_ranges in the
<element_point_ranges_selection>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_selection_clear);
	if (element_point_ranges_selection)
	{
		FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
			Element_point_ranges_add_to_list,(void *)element_point_ranges_selection->
			newly_unselected_element_point_ranges_list,
			element_point_ranges_selection->element_point_ranges_list);
		REMOVE_ALL_OBJECTS_FROM_LIST(Element_point_ranges)(
			element_point_ranges_selection->element_point_ranges_list);
		REMOVE_ALL_OBJECTS_FROM_LIST(Element_point_ranges)(
			element_point_ranges_selection->newly_selected_element_point_ranges_list);
		Element_point_ranges_selection_update(element_point_ranges_selection);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_clear.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_clear */

int Element_point_ranges_selection_select_element_point_ranges(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges *element_point_ranges)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Ensures <element_point_ranges> is in <element_point_ranges_selection>. Even if
it is already selected, <element_point_ranges> is added to the
newly_selected_element_point_ranges_list and removed from the
newly_unselected_element_point_ranges_list if it is currently there.
Calls Element_point_ranges_selection_update.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_selection_select_element_point_ranges);
	if (element_point_ranges_selection&&element_point_ranges)
	{
		return_code=Element_point_ranges_add_to_list(element_point_ranges,
			(void *)element_point_ranges_selection->element_point_ranges_list);
		Element_point_ranges_add_to_list(element_point_ranges,(void *)
			element_point_ranges_selection->newly_selected_element_point_ranges_list);
		Element_point_ranges_remove_from_list(element_point_ranges,
			(void *)element_point_ranges_selection->
			newly_unselected_element_point_ranges_list);
		Element_point_ranges_selection_update(element_point_ranges_selection);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_select_element_point_ranges.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_select_element_point_ranges */

int FE_element_select_grid_field_ranges(struct FE_element *element,
	void *select_data_void)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Iterator function that gets an Element_point_ranges structure representing all
the grid_points in <element> with discretization of the single component
integer <grid_field>, for which the field value is in the given <ranges>.
Note that there may legitimately be none if <grid_field> is not grid-based in
<element> or the ranges do not intersect with the values in the field.
The structure is then added to the <element_point_ranges_selection>.
select_data_void should point to a
struct FE_element_select_grid_field_ranges_data.
Note only looks at top-level elements.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_select_grid_field_ranges_data *select_data;

	ENTER(FE_element_select_grid_field_ranges);
	if (element&&(select_data=
		(struct FE_element_select_grid_field_ranges_data *)select_data_void))
	{
		/* use only top-level elements */
		if ((CM_ELEMENT==element->cm.type)&&
			(element_point_ranges=Element_point_ranges_from_grid_field_ranges(
				element,select_data->grid_field,select_data->ranges)))
		{
			return_code=Element_point_ranges_selection_select_element_point_ranges(
				select_data->element_point_ranges_selection,element_point_ranges);
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
			"FE_element_select_grid_field_ranges.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_select_grid_field_ranges */

int Element_point_ranges_selection_unselect_element_point_ranges(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges *element_point_ranges)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
If <element_point_ranges> is in <element_point_ranges_selection> it is removed
and added to the newly_unselected_element_point_ranges_list. Note it is added to
this list if not currently selected. Also ensures it is removed from the
newly_selected_element_point_ranges_list if it is currently there. Calls
Element_point_ranges_selection_update.
==============================================================================*/
{
	int i,number_of_ranges,return_code,start,stop;
	struct Element_point_ranges *existing_element_point_ranges,
		*removed_element_point_ranges;
	struct Element_point_ranges_identifier identifier;
	struct Multi_range *intersecting_ranges;

	ENTER(Element_point_ranges_selection_unselect_element_point_ranges);
	if (element_point_ranges_selection&&element_point_ranges)
	{
		Element_point_ranges_get_identifier(element_point_ranges,&identifier);
		if (existing_element_point_ranges=FIND_BY_IDENTIFIER_IN_LIST(
			Element_point_ranges,identifier)(&identifier,
			element_point_ranges_selection->element_point_ranges_list))
		{
			/* add ranges actually removed to newly_unselected list = intersection of
				 element_point_ranges and existing_element_point_ranges */
			if (intersecting_ranges=CREATE(Multi_range)())
			{
				Multi_range_copy(intersecting_ranges,
					Element_point_ranges_get_ranges(element_point_ranges));
				Multi_range_intersect(intersecting_ranges,
					Element_point_ranges_get_ranges(existing_element_point_ranges));
				if (removed_element_point_ranges=
					CREATE(Element_point_ranges)(&identifier))
				{
					number_of_ranges=
						Multi_range_get_number_of_ranges(intersecting_ranges);
					for (i=0;i<number_of_ranges;i++)
					{
						if (Multi_range_get_range(intersecting_ranges,i,&start,&stop))
						{
							Element_point_ranges_add_range(removed_element_point_ranges,
								start,stop);
						}
					}
					Element_point_ranges_add_to_list(removed_element_point_ranges,
						(void *)element_point_ranges_selection->
						newly_unselected_element_point_ranges_list);
					DESTROY(Element_point_ranges)(&removed_element_point_ranges);
				}
				DESTROY(Multi_range)(&intersecting_ranges);
			}
			Element_point_ranges_remove_from_list(element_point_ranges,
				(void *)element_point_ranges_selection->
				newly_selected_element_point_ranges_list);
			return_code=Element_point_ranges_remove_from_list(element_point_ranges,
				(void *)element_point_ranges_selection->element_point_ranges_list);
			Element_point_ranges_selection_update(element_point_ranges_selection);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_unselect_element_point_ranges.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_unselect_element_point_ranges */

int FE_element_unselect_grid_field_ranges(struct FE_element *element,
	void *unselect_data_void)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Iterator function that gets an Element_point_ranges structure representing all
the grid_points in <element> with discretization of the single component
integer <grid_field>, for which the field value is in the given <ranges>.
Note that there may legitimately be none if <grid_field> is not grid-based in
<element> or the ranges do not intersect with the values in the field.
The structure is then removed from the <element_point_ranges_selection>.
select_data_void should point to a
struct FE_element_unselect_grid_field_ranges_data.
Note only looks at top-level elements.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_select_grid_field_ranges_data *unselect_data;

	ENTER(FE_element_unselect_grid_field_ranges);
	if (element&&(unselect_data=
		(struct FE_element_select_grid_field_ranges_data *)unselect_data_void))
	{
		/* use only top-level elements */
		if ((CM_ELEMENT==element->cm.type)&&
			(element_point_ranges=Element_point_ranges_from_grid_field_ranges(
				element,unselect_data->grid_field,unselect_data->ranges)))
		{
			return_code=Element_point_ranges_selection_unselect_element_point_ranges(
				unselect_data->element_point_ranges_selection,element_point_ranges);
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
			"FE_element_unselect_grid_field_ranges.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_unselect_grid_field_ranges */

struct LIST(Element_point_ranges)
	*Element_point_ranges_selection_get_element_point_ranges_list(
		struct Element_point_ranges_selection *element_point_ranges_selection)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Returns the list containing the element_point_ranges currently in
<element_point_ranges_selection>.
RETURNED LIST AND OBJECTS IN IT ARE NOT TO BE MODIFIED.
==============================================================================*/
{
	struct LIST(Element_point_ranges) *element_point_ranges_list;

	ENTER(Element_point_ranges_selection_get_element_point_ranges_list);
	if (element_point_ranges_selection)
	{
		element_point_ranges_list=
			element_point_ranges_selection->element_point_ranges_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_get_element_point_ranges_list.  "
			"Invalid argument(s)");
		element_point_ranges_list=(struct LIST(Element_point_ranges) *)NULL;
	}
	LEAVE;

	return (element_point_ranges_list);
} /* Element_point_ranges_selection_get_element_point_ranges_list */

int Element_point_ranges_selection_is_element_point_ranges_selected(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges *element_point_ranges)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Returns true if <element_point_ranges> overlaps any of the
selected_element_point_ranges in <element_point_ranges_selection>.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges *existing_element_point_ranges;
	struct Element_point_ranges_identifier identifier;

	ENTER(Element_point_ranges_selection_is_element_point_ranges_selected);
	if (element_point_ranges_selection&&element_point_ranges)
	{
		Element_point_ranges_get_identifier(element_point_ranges,&identifier);
		if (existing_element_point_ranges=
			FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(&identifier,
				element_point_ranges_selection->element_point_ranges_list))
		{
			return_code=Multi_ranges_overlap(
				Element_point_ranges_get_ranges(element_point_ranges),
				Element_point_ranges_get_ranges(existing_element_point_ranges));
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_selection_is_element_point_ranges_selected.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_selection_is_element_point_ranges_selected */
