/*******************************************************************************
FILE : element_selection.c

LAST MODIFIED : 3 April 2003

DESCRIPTION :
Global store of selected elements for group actions and highlighting.
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
#include "selection/element_selection.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

FULL_DECLARE_CMISS_CALLBACK_TYPES(FE_element_selection_change, \
	struct FE_element_selection *,struct FE_element_selection_changes *);

struct FE_element_selection
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Global store of selected elements for group actions and highlighting.
==============================================================================*/
{
	/* the FE_region the selection is from */
	struct FE_region *fe_region;
	/* flag indicating whether the cache is on */
	int cache;
	/* list of all elements currently selected */
	struct LIST(FE_element) *element_list;
	/* list of all elements selected since the last update; includes those already
		 selected; excludes those subsequently unselected */
	struct LIST(FE_element) *newly_selected_element_list;
	/* list of all elements unselected since the last update; excludes those not
		 already selected; excludes those subsequently selected */
	struct LIST(FE_element) *newly_unselected_element_list;
	/* list of callbacks requested by other objects when selction changes */
	struct LIST(CMISS_CALLBACK_ITEM(FE_element_selection_change)) *change_callback_list;
}; /* struct FE_element_selection */

/*
Module functions
----------------
*/

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(FE_element_selection_change, void)

DEFINE_CMISS_CALLBACK_FUNCTIONS(FE_element_selection_change, \
	struct FE_element_selection *,struct FE_element_selection_changes *)

static int FE_element_selection_update(
	struct FE_element_selection *element_selection)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Tells the clients of the <element_selection> that objects have been selected or
unselected since the last update callback. Avoids sending message if caching is
on or if no such changes have occurred.
==============================================================================*/
{
	int return_code;
	struct FE_element_selection_changes changes;

	ENTER(FE_element_selection_update);
	if (element_selection)
	{
		if (!element_selection->cache)
		{
			/* only update if something changed */
			if ((0<NUMBER_IN_LIST(FE_element)(
				element_selection->newly_selected_element_list))||
				(0<NUMBER_IN_LIST(FE_element)(
					element_selection->newly_unselected_element_list)))
			{
				/* construct the call data */
				changes.newly_selected_element_list=
					element_selection->newly_selected_element_list;
				changes.newly_unselected_element_list=
					element_selection->newly_unselected_element_list;
				/* send the callbacks */
				CMISS_CALLBACK_LIST_CALL(FE_element_selection_change)(
					element_selection->change_callback_list,element_selection,&changes);
				/* clear the newly selected and unselected lists */
				REMOVE_ALL_OBJECTS_FROM_LIST(FE_element)(
					element_selection->newly_selected_element_list);
				REMOVE_ALL_OBJECTS_FROM_LIST(FE_element)(
					element_selection->newly_unselected_element_list);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_update */

void FE_element_selection_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *element_selection_void)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Callback from <fe_region> with its <changes>.
Elements removed from the <fe_region> must be removed from the
<element_selection>.
==============================================================================*/
{
	enum CHANGE_LOG_CHANGE(FE_element) fe_element_change_summary;
	struct FE_element_selection *element_selection;

	ENTER(FE_element_selection_FE_region_change);
	if (fe_region && changes &&
		(element_selection = (struct FE_element_selection *)element_selection_void))
	{
		if (CHANGE_LOG_GET_CHANGE_SUMMARY(FE_element)(changes->fe_element_changes,
			&fe_element_change_summary) &&
			(fe_element_change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_element)))
		{
			REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(FE_element_is_not_in_FE_region,
				(void *)fe_region, element_selection->element_list);
			REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(FE_element_is_not_in_FE_region,
				(void *)fe_region, element_selection->newly_selected_element_list);
			REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(FE_element_is_not_in_FE_region,
				(void *)fe_region, element_selection->newly_unselected_element_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* FE_element_selection_FE_region_change */

/*
Global functions
----------------
*/

struct FE_element_selection *CREATE(FE_element_selection)(
	struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Creates the global store of selected elements for group actions and
highlighting.
==============================================================================*/
{
	struct FE_element_selection *element_selection;

	ENTER(CREATE(FE_element_selection));
	element_selection = (struct FE_element_selection *)NULL;
	if (fe_region)
	{
		if (ALLOCATE(element_selection,struct FE_element_selection,1))
		{
			FE_region_add_callback(fe_region,
				FE_element_selection_FE_region_change, (void *)element_selection);
			element_selection->fe_region = ACCESS(FE_region)(fe_region);
			element_selection->cache=0;
			element_selection->element_list=CREATE(LIST(FE_element))();
			element_selection->newly_selected_element_list=CREATE(LIST(FE_element))();
			element_selection->newly_unselected_element_list=CREATE(LIST(FE_element))();
			element_selection->change_callback_list=
				CREATE(LIST(CMISS_CALLBACK_ITEM(FE_element_selection_change)))();
			if (!(element_selection->element_list&&
				element_selection->newly_selected_element_list&&
				element_selection->newly_unselected_element_list&&
				element_selection->change_callback_list))
			{
				display_message(ERROR_MESSAGE,
					"CREATE(FE_element_selection).  Could not create lists");
				DESTROY(FE_element_selection)(&element_selection);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_element_selection).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_element_selection).  Invalid argument");
	}
	LEAVE;

	return (element_selection);
} /* CREATE(FE_element_selection) */

int DESTROY(FE_element_selection)(
	struct FE_element_selection **element_selection_address)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Destroys the FE_element_selection.
==============================================================================*/
{
	int return_code;
	struct FE_element_selection *element_selection;

	ENTER(DESTROY(FE_element_selection));
	if (element_selection_address&&
		(element_selection= *element_selection_address))
	{
		FE_region_remove_callback(element_selection->fe_region,
			FE_element_selection_FE_region_change, (void *)element_selection);
		DEACCESS(FE_region)(&(element_selection->fe_region));
		DESTROY(LIST(FE_element))(&(element_selection->element_list));
		DESTROY(LIST(FE_element))(
			&(element_selection->newly_selected_element_list));
		DESTROY(LIST(FE_element))(
			&(element_selection->newly_unselected_element_list));
		DESTROY(LIST(CMISS_CALLBACK_ITEM(FE_element_selection_change)))(
			&(element_selection->change_callback_list));
		DEALLOCATE(*element_selection_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_element_selection).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_element_selection) */

int FE_element_selection_add_callback(
	struct FE_element_selection *element_selection,
	CMISS_CALLBACK_FUNCTION(FE_element_selection_change) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Adds a callback to <element_selection> so that when it changes <function> is
called with <user_data>. <function> has 3 arguments, a
struct FE_element_selection, a struct FE_element_selection_changes and the
void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_selection_add_callback);
	if (element_selection&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(FE_element_selection_change)(
			element_selection->change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_selection_add_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_add_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_add_callback */

int FE_element_selection_remove_callback(
	struct FE_element_selection *element_selection,
	CMISS_CALLBACK_FUNCTION(FE_element_selection_change) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<element_selection>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_selection_remove_callback);
	if (element_selection&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(FE_element_selection_change)(
			element_selection->change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_selection_remove_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_remove_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_remove_callback */

int FE_element_selection_begin_cache(
	struct FE_element_selection *element_selection)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Starts caching for changes in the <element_selection>. No update callbacks will
occur until FE_element_selection_end_cache is called.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_selection_begin_cache);
	if (element_selection)
	{
		if (element_selection->cache)
		{
			display_message(ERROR_MESSAGE,
				"FE_element_selection_begin_cache.  Cache already enabled");
			return_code=0;
		}
		else
		{
			element_selection->cache=1;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_begin_cache.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_begin_cache */

int FE_element_selection_end_cache(
	struct FE_element_selection *element_selection)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Ends caching for selected list changes in the <selection>. Calls
FE_element_selection_update to inform clients of the changes that took place
while caching was on.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_selection_end_cache);
	if (element_selection)
	{
		if (element_selection->cache)
		{
			element_selection->cache=0;
			FE_element_selection_update(element_selection);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_selection_end_cache.  Cache not enabled");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_end_cache.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_end_cache */

int FE_element_selection_clear(struct FE_element_selection *element_selection)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Clears all the selected elements in the <element_selection>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_selection_clear);
	if (element_selection)
	{
		FOR_EACH_OBJECT_IN_LIST(FE_element)(ensure_FE_element_is_in_list,
			(void *)element_selection->newly_unselected_element_list,
			element_selection->element_list);
		REMOVE_ALL_OBJECTS_FROM_LIST(FE_element)(element_selection->element_list);
		REMOVE_ALL_OBJECTS_FROM_LIST(FE_element)(
			element_selection->newly_selected_element_list);
		FE_element_selection_update(element_selection);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_clear.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_clear */

int FE_element_selection_select_element(
	struct FE_element_selection *element_selection,struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Ensures <element> is in <element_selection>. Even if it is already selected,
<element> is added to the newly_selected_element_list and removed from the
newly_unselected_element_list if it is currently there.
Calls FE_element_selection_update.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_selection_select_element);
	if (element_selection&&element)
	{
		return_code=ensure_FE_element_is_in_list(element,
			(void *)element_selection->element_list);
		ensure_FE_element_is_in_list(element,
			(void *)element_selection->newly_selected_element_list);
		ensure_FE_element_is_not_in_list(element,
			(void *)element_selection->newly_unselected_element_list);
		FE_element_selection_update(element_selection);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_select_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_select_element */

int FE_element_select_in_FE_element_selection(struct FE_element *element,
	void *element_selection_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
FE_element iterator version of FE_element_selection_select_element.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_select_in_FE_element_selection);
	return_code=FE_element_selection_select_element(
		(struct FE_element_selection *)element_selection_void,element);
	LEAVE;

	return (return_code);
} /* FE_element_select_in_FE_element_selection */

int FE_element_selection_unselect_element(
	struct FE_element_selection *element_selection,struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
If <element> is in <element_selection> it is removed and added to the
newly_unselected_element_list. Note it is added to this list if not currently
selected. Also ensures it is removed from the newly_selected_element_list if it
is currently there. Calls FE_element_selection_update.
==============================================================================*/
{
	int return_code;
	struct CM_element_information identifier;

	ENTER(FE_element_selection_unselect_element);
	if (element_selection&&element&&
		get_FE_element_identifier(element, &identifier))
	{
		if (FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(
			&identifier,element_selection->element_list))
		{
			ADD_OBJECT_TO_LIST(FE_element)(element,
				element_selection->newly_unselected_element_list);
			ensure_FE_element_is_not_in_list(element,
				(void *)element_selection->newly_selected_element_list);
			return_code=REMOVE_OBJECT_FROM_LIST(FE_element)(element
				,element_selection->element_list);
			FE_element_selection_update(element_selection);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_unselect_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_unselect_element */

int FE_element_unselect_in_FE_element_selection(struct FE_element *element,
	void *element_selection_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
FE_element iterator version of FE_element_selection_unselect_element.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_unselect_in_FE_element_selection);
	return_code=FE_element_selection_unselect_element(
		(struct FE_element_selection *)element_selection_void,element);
	LEAVE;

	return (return_code);
} /* FE_element_unselect_in_FE_element_selection */

struct LIST(FE_element) *FE_element_selection_get_element_list(
	struct FE_element_selection *element_selection)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Returns the list containing the elements currently in <element_selection>.
RETURNED LIST IS NOT TO BE MODIFIED.
==============================================================================*/
{
	struct LIST(FE_element) *element_list;

	ENTER(FE_element_selection_get_element_list);
	if (element_selection)
	{
		element_list=element_selection->element_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_get_element_list.  Invalid argument(s)");
		element_list=(struct LIST(FE_element) *)NULL;
	}
	LEAVE;

	return (element_list);
} /* FE_element_selection_get_element_list */

int FE_element_selection_is_element_selected(
	struct FE_element_selection *element_selection,struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Returns true if the <element> is in the <element_selection>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_selection_is_element_selected);
	if (element_selection&&element)
	{
		if (IS_OBJECT_IN_LIST(FE_element)(element,element_selection->element_list))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_selection_is_element_selected.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_selection_is_element_selected */
