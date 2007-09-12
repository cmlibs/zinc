/*******************************************************************************
FILE : cmiss_region.c

LAST MODIFIED : 4 December 2003

DESCRIPTION :
Definition of Cmiss_region, used to create hierarchical data structures.
Object can be structured into directed-acyclic-graphs or DAGs, which contain a
list of child regions, each with their own name as seen by that parent.
Advanced hierarchical functionality is built up by attaching context objects
to regions, but hiding their details from the core Cmiss_region object.
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
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_private.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Cmiss_region_child
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Object in which a Cmiss_region keeps a reference to a child region.
Stores the name that this child is known by to this parent.
==============================================================================*/
{
  char *name;
  struct Cmiss_region *region;
};

FULL_DECLARE_CMISS_CALLBACK_TYPES(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *);

struct Cmiss_region
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Object responsible for building directed acyclic graph hierarchies in Cmiss.
Implements hierarchies by contains a list of Cmiss_region_child objects, each
with their own name as seen by this parent.
Implements advanced hierarchical functionality through context objects stored
within it. Type and role of context objects are not known to the Cmiss_region.
==============================================================================*/
{
	/* list of objects attached to region */
	struct LIST(Any_object) *any_object_list;
	/* ordered list of child regions 0..number_of_child_regions-1 */
	int number_of_child_regions;
	struct Cmiss_region_child **child;
	/* true if children added, removed or reordered in Cmiss_region */
	int children_changed;
	/* If a single child has been added then identify it here for efficiency */
	struct Cmiss_region *child_added;
	/* If a single child has been removed then identify it here for efficiency */
	struct Cmiss_region *child_removed;
	/* The name by which the parent used to name the child */
	char *child_removed_name;
	/* true if objects added to or removed from Cmiss_region */
	int objects_changed;
	/* change counter; if zero, change messages are sent with every change,
		 otherwise incremented/decremented for nested changes */
	int change;
	/* list of change callbacks */
	struct LIST(CMISS_CALLBACK_ITEM(Cmiss_region_change)) *change_callback_list;
	/* number of objects using this region */
	int access_count;
};

/*
Module functions
----------------
*/

static struct Cmiss_region_child *CREATE(Cmiss_region_child)(char *name,
	struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Creates a Cmiss_region_child for <region> with <name>.
==============================================================================*/
{
	struct Cmiss_region_child *cmiss_region_child;

	ENTER(CREATE(Cmiss_region_child));
	cmiss_region_child = (struct Cmiss_region_child *)NULL;
	if (name && region)
	{
		if (ALLOCATE(cmiss_region_child, struct Cmiss_region_child, 1) &&
			(cmiss_region_child->name = duplicate_string(name)))
		{ 
			cmiss_region_child->region = ACCESS(Cmiss_region)(region);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Cmiss_region_child).  Could not allocate memory");
			if (cmiss_region_child)
			{
				DEALLOCATE(cmiss_region_child);
				cmiss_region_child = (struct Cmiss_region_child *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_region_child).  Invalid argument(s)");
	}
	LEAVE;

	return (cmiss_region_child);
} /* CREATE(Cmiss_region_child) */

static int DESTROY(Cmiss_region_child)(
	struct Cmiss_region_child **cmiss_region_child_address)
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION :
Frees the memory for the Cmiss_region_child_child and sets <*cmiss_region_child_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Cmiss_region_child *cmiss_region_child;

	ENTER(DESTROY(Cmiss_region_child));
	if (cmiss_region_child_address &&
		(cmiss_region_child = *cmiss_region_child_address))
	{
		DEALLOCATE(cmiss_region_child->name);
		DEACCESS(Cmiss_region)(&(cmiss_region_child->region));
		DEALLOCATE(*cmiss_region_child_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_region_child).  Missing Cmiss_region_child");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_region_child) */

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Cmiss_region_change, void)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *)

static int Cmiss_region_update(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Tells the clients of the <region> about changes of children or objects in this
region. No messages sent if change count positive or no changes have occurred.
==============================================================================*/
{
	int return_code;
	struct Cmiss_region_changes changes;

	ENTER(Cmiss_region_update);
	if (region)
	{
		if ((!region->change) &&
			(region->children_changed || region->objects_changed))
		{
			/* construct the changes object */
			changes.children_changed = region->children_changed;
			changes.child_added = region->child_added;
			changes.child_removed = region->child_removed;
			/* This is an allocated string but we are transferring it and then
				DEALLOCATING at the end of this routine */
			changes.child_removed_name = region->child_removed_name;
			changes.objects_changed = region->objects_changed;
			/* must clear flags in the region before changes go out */
			region->children_changed = 0;
			region->child_added = (struct Cmiss_region *)NULL;
			region->child_removed = (struct Cmiss_region *)NULL;
			region->child_removed_name = (char *)NULL;
			region->objects_changed = 0;
			/* send the callbacks */
			CMISS_CALLBACK_LIST_CALL(Cmiss_region_change)(
				region->change_callback_list, region, &changes);
			if (changes.child_removed_name)
			{
				DEALLOCATE(changes.child_removed_name);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_update.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_update */

/*
Global functions
----------------
*/

DECLARE_OBJECT_FUNCTIONS(Cmiss_region)

struct Cmiss_region *CREATE(Cmiss_region)(void)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Creates an empty Cmiss_region.
==============================================================================*/
{
	struct Cmiss_region *region;

	ENTER(CREATE(Cmiss_region));
	if (ALLOCATE(region, struct Cmiss_region, 1))
	{
		region->any_object_list = CREATE(LIST(Any_object))();
		region->number_of_child_regions = 0;
		region->child = (struct Cmiss_region_child **)NULL;
		region->children_changed = 0;
		region->child_added = (struct Cmiss_region *)NULL;
		region->child_removed = (struct Cmiss_region *)NULL;
		region->child_removed_name = (char *)NULL;
		region->objects_changed = 0;
		region->change = 0;
		region->change_callback_list =
			CREATE(LIST(CMISS_CALLBACK_ITEM(Cmiss_region_change)))();
		region->access_count = 0;
		if (!(region->any_object_list && region->change_callback_list))
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Cmiss_region).  Could not build region");
			DESTROY(Cmiss_region)(&region);
			region = (struct Cmiss_region *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_region).  Could not allocate memory");
	}
	LEAVE;

	return (region);
} /* CREATE(Cmiss_region) */

int DESTROY(Cmiss_region)(struct Cmiss_region **region_address)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Frees the memory for the Cmiss_region and sets <*cmiss_region_address> to NULL.
==============================================================================*/
{
	int i, return_code;
	struct Cmiss_region *region;

	ENTER(DESTROY(Cmiss_region));
	if (region_address && (region = *region_address))
	{
		if (0 == region->access_count)
		{
			DESTROY(LIST(Any_object))(&(region->any_object_list));
			/* clean up child array. Note array may still be allocated even if
				 number_of_child_regions has returned to zero */
			if (0 < region->number_of_child_regions)
			{
				for (i = 0; i < region->number_of_child_regions; i++)
				{
					DESTROY(Cmiss_region_child)(&(region->child[i]));
				}
			}
			if (region->child)
			{
				DEALLOCATE(region->child);
			}
			if (region->child_removed_name)
			{
				DEALLOCATE(region->child_removed_name);
			}
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Cmiss_region_change)))(
				&(region->change_callback_list));
			DEALLOCATE(*region_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Cmiss_region).  Non-zero access count");
			return_code = 0;
		}
		*region_address = (struct Cmiss_region *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_region).  Missing Cmiss_region");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_region) */

int set_Cmiss_region_path(struct Parse_state *state, void *path_address_void,
	void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Modifier function for entering a path to a Cmiss_region, starting at
<root_region>.
==============================================================================*/
{
	char *current_token, **path_address;
	int return_code;
	struct Cmiss_region *region, *root_region;

	ENTER(set_Cmiss_region_path);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (Cmiss_region_get_region_from_path(root_region, current_token,
					&region) && region)
				{
					if (path_address = (char **)path_address_void)
					{
						if (*path_address)
						{
							DEALLOCATE(*path_address);
						}
						if (*path_address = duplicate_string(current_token))
						{
							return_code = shift_Parse_state(state, 1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Cmiss_region_path.  Could not allocate memory for path");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_Cmiss_region_path.  Missing path_address");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Invalid region path: %s",
						current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," PATH_TO_REGION");
				if ((path_address = (char **)path_address_void) && (*path_address))
				{
					display_message(INFORMATION_MESSAGE, "[%s]", *path_address);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing region path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_Cmiss_region_path.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Cmiss_region_path */

int Option_table_add_set_Cmiss_region_path(struct Option_table *option_table, 
	char *entry_string, struct Cmiss_region *root_region, char **path_address)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Adds an entry to the <option_table> with name <entry_name> that returns a 
region path in <path_address> relative to the <root_region>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_set_Cmiss_region_path);
	if (option_table && entry_string && root_region && path_address)
	{
		Option_table_add_entry(option_table, entry_string,
			(void *)path_address, (void *)root_region,
			set_Cmiss_region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_set_Cmiss_region_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_set_Cmiss_region_path */

int Cmiss_region_begin_change(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Increments the change counter in <region>. No update callbacks will occur until
change counter is restored to zero by calls to Cmiss_region_end_change.
???RC Make recursive/hierarchical?
???RC Difficult to make recursive/hierarchical since if new children are
added they would not be included!
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_begin_change);
	if (region)
	{
		region->change++;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_begin_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_begin_change */

int Cmiss_region_end_change(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 10 December 2002

DESCRIPTION :
Decrements the change counter in <region>. No update callbacks occur until the
change counter is restored to zero by this function.
???RC Make recursive/hierarchical?
???RC Difficult to make recursive/hierarchical since if new children are
added they would not be included!
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_end_change);
	if (region)
	{
		if (0 < region->change)
		{
			region->change--;
			if (0 == region->change)
			{
				Cmiss_region_update(region);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_end_change.  Change count is already zero");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_end_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_end_change */

int Cmiss_region_add_callback(struct Cmiss_region *region,
	CMISS_CALLBACK_FUNCTION(Cmiss_region_change) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds a callback to <region> so that when it changes <function> is called with
<user_data>. <function> has 3 arguments, a struct Cmiss_region *, a
struct Cmiss_region_changes * and the void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_add_callback);
	if (region && function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Cmiss_region_change)(
			region->change_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_add_callback.  Could not add callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_add_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_add_callback */

int Cmiss_region_remove_callback(struct Cmiss_region *region,
	CMISS_CALLBACK_FUNCTION(Cmiss_region_change) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <region>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_remove_callback);
	if (region && function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Cmiss_region_change)(
			region->change_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_remove_callback.  Could not remove callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_remove_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_remove_callback */

int Cmiss_region_add_child_region(struct Cmiss_region *region,
	struct Cmiss_region *child_region, char *child_name, int child_position)
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Adds <child_region> to <region> at <child_position> under the <child_name>.
<child_position> must be from -1 to the regions number_of_child_regions; if it
is -1 it is added at the end of the list.
Ensures:
- <child_name> passes is_standard_object_name;
- <child_name> is not already used in <region>;
- <child_region> is not already in <region>;
- <child_region> is not the same as or contains <region>;
==============================================================================*/
{
	int child_in_region, i, name_in_use, return_code;
	struct Cmiss_region_child *region_child, **temp_child;

	ENTER(Cmiss_region_add_child_region);
	if (region && child_name &&
		child_region && (-1 <= child_position) &&
		(child_position <= region->number_of_child_regions))
	{
		return_code = 1;
		/* check child name is valid */
		if (!is_standard_object_name(child_name))
		{
			display_message(ERROR_MESSAGE, "Cmiss_region_add_child_region.  "
				"'%s' is not a valid name for a child region", child_name);
			return_code = 0;
		}
		/* check not putting region in itself either directly or indirectly */ 
		if (Cmiss_region_contains_Cmiss_region(child_region, region))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_add_child_region.  Cannot add region to itself");
			return_code = 0;
		}
		/* check child_name and child_region not already in child list */
		name_in_use = 0;
		child_in_region = 0;
		for (i = 0; i < region->number_of_child_regions; i++)
		{
			if (0 == strcmp(child_name, region->child[i]->name))
			{
				name_in_use = 1;
			}
			if (child_region == region->child[i]->region)
			{
				child_in_region = 1;
			}
		}
		if (name_in_use)
		{
			display_message(ERROR_MESSAGE, "Cmiss_region_add_child_region.  "
				"Region already contains child named '%s'", child_name);
			return_code = 0;
		}
		if (child_in_region)
		{
			display_message(ERROR_MESSAGE, "Cmiss_region_add_child_region.  "
				"Region already contains this child region");
			return_code = 0;
		}
		if (return_code)
		{
			if (region_child = CREATE(Cmiss_region_child)(child_name, child_region))
			{
				if (REALLOCATE(temp_child, region->child, struct Cmiss_region_child *,
					region->number_of_child_regions + 1))
				{
					if (-1 == child_position)
					{
						temp_child[region->number_of_child_regions] = region_child;
					}
					else
					{
						for (i = region->number_of_child_regions; i > child_position; i--)
						{
							temp_child[i] = temp_child[i - 1];
						}
						temp_child[child_position] = region_child;
					}
					region->child = temp_child;
					region->number_of_child_regions++;
					/* note changes and inform clients */
					if (! region->children_changed)
					{
						region->children_changed = 1;
						region->child_added = child_region;
					}
					else
					{
						/* More than a simple change */
						region->child_added = (struct Cmiss_region *)NULL;
						region->child_removed = (struct Cmiss_region *)NULL;
						if (region->child_removed_name)
						{
							DEALLOCATE(region->child_removed_name);
						}
					}
					Cmiss_region_update(region);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Cmiss_region_add_child_region.  "
						"Could not add region child to list");
					return_code = 0;
					DESTROY(Cmiss_region_child)(&region_child);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_region_add_child_region.  Could not make region child");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_add_child_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_add_child_region */

int Cmiss_region_remove_child_region(struct Cmiss_region *region,
	struct Cmiss_region *child_region)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Removes <child_region> from <region>.
???RC what if this region contains the master FE_region for the FE_region
in the child?
==============================================================================*/
{
	char *child_region_name;
	int child_in_region, i, return_code;

	ENTER(Cmiss_region_remove_child_region);
	if (region && child_region)
	{
		child_in_region = 0;
		for (i = 0; i < region->number_of_child_regions; i++)
		{
			if (child_in_region)
			{
				region->child[i - 1] = region->child[i];
			}
			else if (child_region == region->child[i]->region)
			{
				if (! region->children_changed)
				{
					/* Copy out the name before we remove it */
					child_region_name = duplicate_string((region->child[i])->name);
				}
				DESTROY(Cmiss_region_child)(&(region->child[i]));
				child_in_region = 1;
			}
		}
		if (child_in_region)
		{
			(region->number_of_child_regions)--;
			/* note changes and inform clients */
			if (! region->children_changed)
			{
				region->children_changed = 1;
				region->child_removed = child_region;
				region->child_removed_name = child_region_name;
			}
			else
			{
				/* More than a simple change */
				region->child_added = (struct Cmiss_region *)NULL;
				region->child_removed = (struct Cmiss_region *)NULL;
				if (region->child_removed_name)
				{
					DEALLOCATE(region->child_removed_name);
				}
			}
			Cmiss_region_update(region);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE, "Cmiss_region_remove_child_region.  "
				"Region does not contain this child region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_remove_child_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_remove_child_region */

int Cmiss_region_contains_Cmiss_region(struct Cmiss_region *region,
	struct Cmiss_region *other_region)
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Returns true if <region> contains <other_region>, this is the case if
<region> equals <other_region> or any of child regions satisfy this function.
==============================================================================*/
{
	int i, return_code;

	ENTER(Cmiss_region_contains_Cmiss_region);
	return_code = 0;
	if (region && other_region)
	{
		if (region == other_region)
		{
			return_code = 1;
		}
		else
		{
			for (i = 0; (i < region->number_of_child_regions) && (!return_code); i++)
			{
				return_code = Cmiss_region_contains_Cmiss_region(
					region->child[i]->region, other_region);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_contains_Cmiss_region.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_contains_Cmiss_region */

struct Cmiss_region *Cmiss_region_get_child_region_from_name(
	struct Cmiss_region *region, const char *child_name)
/*******************************************************************************
LAST MODIFIED : 29 October 2002

DESCRIPTION :
Returns a pointer to the child of <region> known by <child_name>.
Returns NULL without error messages if no child of that name exists in <region>.
==============================================================================*/
{
	int i;
	struct Cmiss_region *child_region;

	ENTER(Cmiss_region_get_child_region_from_name);
	child_region = (struct Cmiss_region *)NULL;
	if (region && child_name)
	{
		for (i = 0; (i < region->number_of_child_regions) && (!child_region); i++)
		{
			if (0 == strcmp(child_name, region->child[i]->name))
			{
				child_region = (region->child[i])->region;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_child_region_from_name.  Invalid argument(s)");
	}
	LEAVE;

	return (child_region);
} /* Cmiss_region_get_child_region_from_name */

int Cmiss_region_get_child_region_number(struct Cmiss_region *region,
	struct Cmiss_region *child_region, int *child_number_address)
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Returns in <child_number_address> the position of <child_region> in <region>,
with numbering starting at 0.
Returns 0 without error messages if <child_region> is not in <region>.
==============================================================================*/
{
	int i, return_code;

	ENTER(Cmiss_region_get_child_region_number);
	if (region && child_region && child_number_address)
	{
		return_code = 0;
		for (i = 0; (i < region->number_of_child_regions) && (0 == return_code);
			i++)
		{
			if (child_region == region->child[i]->region)
			{
				*child_number_address = i;
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_child_region_number.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_get_child_region_number */

int Cmiss_region_set_child_region_number(struct Cmiss_region *region,
	struct Cmiss_region *child_region, int child_number)
/*******************************************************************************
LAST MODIFIED : 24 March 2003

DESCRIPTION :
Changes the position of <child_region> in the list of children of <region> to
<child_number>, with the first child being 0. A <child_number> of -1 puts the
child at the bottom of the list.
==============================================================================*/
{
	int current_child_number, i, return_code;
	struct Cmiss_region_child *temp_child;

	ENTER(Cmiss_region_set_child_region_number);
	if (region && child_region && (-1 <= child_number) &&
		(child_number < region->number_of_child_regions))
	{
		if (Cmiss_region_get_child_region_number(region, child_region,
			&current_child_number))
		{
			return_code = 1;
			if (-1 == child_number)
			{
				child_number = region->number_of_child_regions - 1;
			}
			if (child_number != current_child_number)
			{
				temp_child = region->child[current_child_number];
				if (current_child_number < child_number)
				{
					for (i = current_child_number; i < child_number; i++)
					{
						region->child[i] = region->child[i + 1];
					}
				}
				else
				{
					for (i = current_child_number; i > child_number; i--)
					{
						region->child[i] = region->child[i - 1];
					}
				}
				region->child[child_number] = temp_child;
				/* note changes and inform clients */
				region->children_changed = 1;
				Cmiss_region_update(region);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_set_child_region_number.  Region is not a child");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_set_child_region_number.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_set_child_region_number */

int Cmiss_region_get_region_from_path(struct Cmiss_region *root_region,
	const char *path, struct Cmiss_region **region_address)
/*******************************************************************************
LAST MODIFIED : 11 November 2002

DESCRIPTION :
Returns the region described relative to the <root_region> in <path>.
<path> is of the format CHILD_NAME/CHILD_NAME/...
where '/' is actually the CMISS_REGION_PATH_SEPARATOR_CHAR.
Single leading and trailing separator characters are ignored, hence:
- "/heart/coronaries/" = "heart/coronaries" -- the latter is preferred.
- both "/" and "" refer to the root_region itself.
If no region can be identified from path, returns successfully but with NULL
in <region_address> 
For safety, returns NULL in <region_address> on any error.
==============================================================================*/
{
	char *child_name, *child_name_allocate, *child_name_end;
	int return_code;
	struct Cmiss_region *region;

	ENTER(Cmiss_region_get_region_from_path);
	if (root_region && path && region_address)
	{
		region = root_region;
		child_name_allocate = duplicate_string(path);
		child_name = child_name_allocate;
		/* skip leading separator */
		if (child_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		while (region && (child_name_end =
			strchr(child_name, CMISS_REGION_PATH_SEPARATOR_CHAR)))
		{
			*child_name_end = '\0';
			region = Cmiss_region_get_child_region_from_name(region, child_name);
			*child_name_end = CMISS_REGION_PATH_SEPARATOR_CHAR;
			child_name = child_name_end + 1;
		}
		/* already found the region if there was a single trailing separator */
		if (region && (child_name[0] != '\0'))
		{
			region = Cmiss_region_get_child_region_from_name(region, child_name);
		}
		*region_address = region;
		DEALLOCATE(child_name_allocate);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_region_from_path.  Invalid argument(s)");
		if (region_address)
		{
			*region_address = (struct Cmiss_region *)NULL;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_get_region_from_path */

int Cmiss_region_get_child_region_from_path(struct Cmiss_region *root_region,
	char *path, struct Cmiss_region **child_region_address,
	char **remaining_path_address)
/*******************************************************************************
LAST MODIFIED : 28 March 2003

DESCRIPTION :
Returns in <child_region> the child region of <root_region> named by the first
part of <path> up to the CMISS_REGION_PATH_SEPARATOR_CHAR.
<path> is allowed to have zero or one leading or trailing separators, except
that "//" is illegal.
<remaining_path> is advanced to the next child region name in <path> or NULL
if none.
Special case: "/" and "" both return <root_region>.
Returns 0 with no error message if no child region found.
For safety, returns NULL in <child_region_address> and either NULL or a valid
pointer within path in <remaining_path_address> on any error.
==============================================================================*/
{
	char *child_name;
	int return_code;

	ENTER(Cmiss_region_get_child_region_from_path);
	if (root_region && path && child_region_address && remaining_path_address)
	{
		return_code = 1;
		child_name = path;
		/* skip leading separator */
		if (child_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		if (*remaining_path_address =
			strchr(child_name, CMISS_REGION_PATH_SEPARATOR_CHAR))
		{
			/* temporarily delimit the string to the first name */
			**remaining_path_address = '\0';
		}
		if (('\0' == child_name[0]) && ((char *)NULL == (*remaining_path_address)))
		{
			*child_region_address = root_region;
		}
		else
		{
			if (!(*child_region_address =
				Cmiss_region_get_child_region_from_name(root_region, child_name)))
			{
				return_code = 0;
			}
		}
		if (*remaining_path_address)
		{
			/* restore the string to how it was */
			**remaining_path_address = CMISS_REGION_PATH_SEPARATOR_CHAR;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_child_region_from_path.  Invalid argument(s)");
		if (child_region_address)
		{
			*child_region_address = (struct Cmiss_region *)NULL;
		}
		if (*remaining_path_address)
		{
			*remaining_path_address = (char *)NULL;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_get_child_region_from_path */

int Cmiss_region_get_number_of_child_regions(struct Cmiss_region *region,
	int *number_of_child_regions_address)
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Gets the number of child regions in <region>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_get_number_of_child_regions);
	if (region && number_of_child_regions_address)
	{
		*number_of_child_regions_address = region->number_of_child_regions;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_number_of_child_regions.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_get_number_of_child_regions */

int Cmiss_region_get_child_region(struct Cmiss_region *region,
	int child_number, struct Cmiss_region **child_region_address)
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Returns at <child_region_address> child region <child_number> of <region>.
<child_number> ranges from 0 to one less than number_of_child_regions.
For safety, returns NULL in <child_region_address> on any error.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_get_child_region);
	if (region && (0 <= child_number) &&
		(child_number < region->number_of_child_regions) && child_region_address)
	{
		*child_region_address = (region->child[child_number])->region;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_child_region.  Invalid argument(s)");
		return_code = 0;
		if (child_region_address)
		{
			*child_region_address = (struct Cmiss_region *)NULL;
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_get_child_region */

int Cmiss_region_get_child_region_name(struct Cmiss_region *region,
	int child_number, char **child_region_name_address)
/*******************************************************************************
LAST MODIFIED : 11 November 2002

DESCRIPTION :
Returns at <child_region_name_address> the name of child region <child_number>
as known by <region>.
<child_number> ranges from 0 to one less than number_of_child_regions.
For safety, returns NULL in <child_region_name_address> on any error.
Up to the calling function to deallocate the returned name.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_get_child_region_name);
	if (region && (0 <= child_number) &&
		(child_number < region->number_of_child_regions) &&
		child_region_name_address)
	{
		if (*child_region_name_address =
			duplicate_string((region->child[child_number])->name))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_get_child_region_name.  Could not duplicate name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_child_region_name.  Invalid argument(s)");
		return_code = 0;
		if (child_region_name_address)
		{
			*child_region_name_address = (char *)NULL;
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_get_child_region_name */

int Cmiss_region_get_root_region_path(char **path_address)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Allocates and returns the path to the root_region.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_get_root_region_path);
	if (path_address)
	{
		*path_address = duplicate_string(CMISS_REGION_PATH_SEPARATOR_STRING);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_root_region_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* Cmiss_region_get_root_region_path */

int Cmiss_region_list(struct Cmiss_region *region,
	int indent, int indent_increment)
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Lists the Cmiss_region hierarchy starting from <region>. Contents are listed
indented from the left margin by <indent> spaces; this is incremented by
<indent_increment> for each child level.
==============================================================================*/
{
	int i, return_code;

	ENTER(Cmiss_region_list);
	if (region && (0 <= indent) && (0 < indent_increment))
	{
		for (i = 0; i < region->number_of_child_regions; i++)
		{
			display_message(INFORMATION_MESSAGE, "%*s%s\n", indent, " ",
				(region->child[i])->name);
			Cmiss_region_list((region->child[i])->region,
				indent + indent_increment, indent_increment);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_list */

int Cmiss_region_private_attach_any_object(struct Cmiss_region *region,
	struct Any_object *any_object)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds <any_object> to the list of objects attached to <region>.
This function is only externally visible to context objects.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_private_attach_any_object);
	if (region && any_object)
	{
		if (ADD_OBJECT_TO_LIST(Any_object)(any_object, region->any_object_list))
		{
			/* note changes and inform clients */
			region->objects_changed = 1;
			Cmiss_region_update(region);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_private_attach_any_object.  Could not attach object");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_private_attach_any_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_private_attach_any_object */

int Cmiss_region_private_detach_any_object(struct Cmiss_region *region,
	struct Any_object *any_object)
/*******************************************************************************
LAST MODIFIED : 29 October 2002

DESCRIPTION :
Removes <any_object> from the list of objects attached to <region>.
Note that only in the case that <any_object> is the exact Any_object stored in
<region> may it be cleaned up. In any other case the <any_object> passed in
must be cleaned up by the calling function.
This function is only externally visible to context objects.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_private_detach_any_object);
	if (region && any_object)
	{
		if (IS_OBJECT_IN_LIST(Any_object)(any_object, region->any_object_list))
		{
			if (REMOVE_OBJECT_FROM_LIST(Any_object)(any_object,
				(void *)region->any_object_list))
			{
				/* note changes and inform clients */
				region->objects_changed = 1;
				Cmiss_region_update(region);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_region_private_detach_any_object.  "
					"Could not remove object from list");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_private_detach_any_object.  Object is not in list");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_private_detach_any_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_private_detach_any_object */

struct LIST(Any_object) *
Cmiss_region_private_get_any_object_list(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Returns the list of objects, abstractly stored as struct Any_object from
<region>. It is important that this list not be modified directly.
This function is only externally visible to context objects.
==============================================================================*/
{
	struct LIST(Any_object) *any_object_list;

	ENTER(Cmiss_region_private_get_any_object_list);
	if (region)
	{
		any_object_list = region->any_object_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_private_get_any_object_list.  Missing region");
		any_object_list = (struct LIST(Any_object) *)NULL;
	}
	LEAVE;

	return (any_object_list);
} /* Cmiss_region_private_get_any_object_list */
