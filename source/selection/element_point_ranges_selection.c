/*******************************************************************************
FILE : element_point_ranges_selection.c

LAST MODIFIED : 25 May 2000

DESCRIPTION :
Global store of selected element_point_ranges for group actions and
highlighting.
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
#include <stdio.h>
#include "general/callback_private.h"
#include "general/debug.h"
#include "selection/element_point_ranges_selection.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

FULL_DECLARE_CMISS_CALLBACK_TYPES(Element_point_ranges_selection_change, \
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
	struct LIST(CMISS_CALLBACK_ITEM(Element_point_ranges_selection_change))
		*change_callback_list;
}; /* struct Element_point_ranges_selection */

/*
Module functions
----------------
*/
DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Element_point_ranges_selection_change)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Element_point_ranges_selection_change, \
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
				CMISS_CALLBACK_LIST_CALL(Element_point_ranges_selection_change)(
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

struct Element_point_ranges_selection *CREATE(Element_point_ranges_selection)(
	void)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

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
			CREATE(LIST(CMISS_CALLBACK_ITEM(Element_point_ranges_selection_change)))();
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
		DESTROY(LIST(CMISS_CALLBACK_ITEM(Element_point_ranges_selection_change)))(
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
	CMISS_CALLBACK_FUNCTION(Element_point_ranges_selection_change) *function,
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
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Element_point_ranges_selection_change)(
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
	CMISS_CALLBACK_FUNCTION(Element_point_ranges_selection_change) *function,
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
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Element_point_ranges_selection_change)(
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
LAST MODIFIED : 25 May 2000

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
	if (element_point_ranges_selection&&element_point_ranges&&
		Element_point_ranges_has_ranges(element_point_ranges))
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

int Element_point_ranges_select(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_selection_void)
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Element_point_ranges iterator version of
Element_point_ranges_selection_select_element_point_ranges
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_select);
	return_code=Element_point_ranges_selection_select_element_point_ranges(
		(struct Element_point_ranges_selection *)
		element_point_ranges_selection_void,element_point_ranges);
	LEAVE;

	return (return_code);
} /* Element_point_ranges_select */

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

int Element_point_ranges_unselect(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_selection_void)
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Element_point_ranges iterator version of
Element_point_ranges_selection_unselect_element_point_ranges
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_ranges_unselect);
	return_code=Element_point_ranges_selection_unselect_element_point_ranges(
		(struct Element_point_ranges_selection *)
		element_point_ranges_selection_void,element_point_ranges);
	LEAVE;

	return (return_code);
} /* Element_point_ranges_unselect */

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
