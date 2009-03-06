/*******************************************************************************
FILE : cmiss_region.cpp

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
extern "C" {
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_finite_element.h"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_private.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"
}

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
  const char *name;
  struct Cmiss_region *region;
};

FULL_DECLARE_CMISS_CALLBACK_TYPES(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *);

struct Cmiss_region_fields
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Sharable object for holding fields owned by a region together with the
FE_region whose FE_fields are automatically wrapped.
'group' regions ACCESS the same Cmiss_region_fields as the full region.
==============================================================================*/
{
	/* non-accessed pointer to master region owning these fields */
	/* must be cleared to NULL on destruction of owning_region */
	struct Cmiss_region *owning_region;
	struct MANAGER(Computed_field) *field_manager;
	struct FE_region *fe_region;
	int access_count;
};

struct Cmiss_region
/*******************************************************************************
LAST MODIFIED : 19 May 2008

DESCRIPTION :
Object responsible for building directed acyclic graph hierarchies in Cmiss.
Implements hierarchies by contains a list of Cmiss_region_child objects, each
with their own name as seen by this parent.
Implements advanced hierarchical functionality through context objects stored
within it. Type and role of context objects are not known to the Cmiss_region.
==============================================================================*/
{
	/* non-accessed pointer to parent region, or NULL if root */
	struct Cmiss_region *parent;
	/* fields owned by this region (or master) */
	struct Cmiss_region_fields *fields;
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

static struct Cmiss_region_child *CREATE(Cmiss_region_child)(const char *name,
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

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_region_fields);

struct Cmiss_region_fields *CREATE(Cmiss_region_fields)(
	struct Cmiss_region *owning_region,
	struct MANAGER(FE_basis) *basis_manager,
	struct LIST(FE_element_shape) *element_shape_list)
/*******************************************************************************
LAST MODIFIED : 26 May 2008

DESCRIPTION :
Creates a field manager and FE_region for defining finite_element fields with.
This object is sharable between all regions using the same ultimate "master"
FE_region, i.e. the true region and subgroups of it. 
If <basis_manager> and <element_shape_list> are supplied they are reused,
otherwise new local manager & list are created.
Created with access_count of 1 so call DEACCESS to clean up.
@param owning_region  The master region owning these fields.
==============================================================================*/
{
	struct Cmiss_region_fields *region_fields;

	ENTER(CREATE(Cmiss_region_fields));
	ALLOCATE(region_fields, struct Cmiss_region_fields, 1);
	if (region_fields)
	{
		region_fields->owning_region = owning_region;
		region_fields->field_manager = CREATE(MANAGER(Computed_field))();
		Computed_field_manager_set_owner(region_fields->field_manager, region_fields);
		region_fields->fe_region = CREATE(FE_region)(
			/*master_fe_region*/(struct FE_region *)NULL,
			basis_manager, element_shape_list);
		ACCESS(FE_region)(region_fields->fe_region);
		region_fields->access_count = 1;
		if (!(region_fields->field_manager &&
			region_fields->fe_region &&
			Computed_field_manager_begin_autowrap_FE_fields(
				region_fields->field_manager, region_fields->fe_region)))
		{
			DEACCESS(Cmiss_region_fields)(&region_fields);
		}
	}
	if (!region_fields)
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_region_fields).  Failed to construct");
	}
	LEAVE;
	
	return region_fields;
} /* CREATE(Cmiss_region_fields) */

int DESTROY(Cmiss_region_fields)(struct Cmiss_region_fields **region_fields_address)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Frees the memory for the Cmiss_region_fields and sets
<*cmiss_region_field_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Cmiss_region_fields *region_fields;

	ENTER(DESTROY(Cmiss_region_fields));
	if (region_fields_address && (region_fields = *region_fields_address))
	{
		if (0 == region_fields->access_count)
		{
			Computed_field_manager_end_autowrap_FE_fields(
				region_fields->field_manager, region_fields->fe_region);
			DEACCESS(FE_region)(&region_fields->fe_region);
			DESTROY(MANAGER(Computed_field))(&(region_fields->field_manager));
			DEALLOCATE(*region_fields_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Cmiss_region_fields).  Non-zero access count");
			return_code = 0;
		}
		*region_fields_address = (struct Cmiss_region_fields *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_region_fields).  Missing Cmiss_region_fields");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_region_fields) */

int Cmiss_region_fields_begin_change(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 26 May 2008

DESCRIPTION :
Forwards begin change cache to region fields.
Note Cmiss_region is passed in rather than Cmiss_region_fields because currently
the non-master FE_region is attached in the region's any_object_list.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_fields_begin_change);
	if (region && region->fields)
	{
		FE_region_begin_change(Cmiss_region_get_FE_region(region));
		MANAGER_BEGIN_CACHE(Computed_field)(region->fields->field_manager);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_fields_begin_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_fields_begin_change */

int Cmiss_region_fields_end_change(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 26 May 2008

DESCRIPTION :
Forwards end change cache to region fields.
Note Cmiss_region is passed in rather than Cmiss_region_fields because currently
the non-master FE_region is attached in the region's any_object_list.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_fields_end_change);
	if (region && region->fields)
	{
		FE_region_end_change(Cmiss_region_get_FE_region(region));
		MANAGER_END_CACHE(Computed_field)(region->fields->field_manager);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_fields_end_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_fields_end_change */

DECLARE_OBJECT_FUNCTIONS(Cmiss_region_fields)
	
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

struct Cmiss_region *CREATE(Cmiss_region)(void)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Private, partial constructor. Creates an empty Cmiss_region WITHOUT fields.
The region is not fully constructed until Cmiss_region_attach_fields is called.
Region is created with an access_count of 1; DEACCESS to destroy.
==============================================================================*/
{
	struct Cmiss_region *region;

	ENTER(CREATE(Cmiss_region));
	if (ALLOCATE(region, struct Cmiss_region, 1))
	{
		region->parent = NULL;
		region->fields = NULL;
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
		region->access_count = 1;
		if (!(region->any_object_list && region->change_callback_list))
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Cmiss_region).  Could not build region");
			DEACCESS(Cmiss_region)(&region);
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

int Cmiss_region_attach_fields(struct Cmiss_region *region,
	struct Cmiss_region *master_region,
	enum Cmiss_region_attach_fields_variant variant)
/*******************************************************************************
LAST MODIFIED : 26 May 2008

DESCRIPTION :
Adds field-capability to <region> created with private constructor.
If <master_region> is not supplied, region is created with its own fields, nodes
and elements. 
If <master_region> is supplied; then behaviour depends on the <variant>; see
cases in code.
==============================================================================*/
{
	int i, return_code;
	struct FE_region *fe_region;

	ENTER(Cmiss_region_attach_fields);
	return_code = 0;
	if (region && (!master_region ||
		(master_region->fields && master_region->fields->fe_region))) 
	{
		if (region->fields || (NULL != Cmiss_region_get_FE_region(region)))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_attach_fields.  Region already has fields");
		}
		else
		{
			return_code = 1;
			fe_region = (struct FE_region *)NULL;
			if (master_region)
			{
				switch (variant)
				{
				case CMISS_REGION_SHARE_FIELDS_GROUP:
					{
						/* region shares master's fields but uses a subset of nodes and elements */
						region->fields = ACCESS(Cmiss_region_fields)(master_region->fields);
						fe_region = CREATE(FE_region)(Cmiss_region_get_FE_region(master_region),
							(struct MANAGER(FE_basis) *)NULL,
							(struct LIST(FE_element_shape) *)NULL);
					} break;
				case CMISS_REGION_SHARE_BASES_SHAPES:
				default:
					{
						/* creates a full region sharing the basis manager and shape list from
						 * the master region */
						region->fields = CREATE(Cmiss_region_fields)(region,
							FE_region_get_basis_manager(master_region->fields->fe_region),
							FE_region_get_FE_element_shape_list(master_region->fields->fe_region));
						if (region->fields)
						{
							fe_region = region->fields->fe_region;
						}
					} break;
				}
			}
			else
			{
				region->fields = CREATE(Cmiss_region_fields)(region,
					(struct MANAGER(FE_basis) *)NULL,
					(struct LIST(FE_element_shape) *)NULL);
				if (region->fields)
				{
					fe_region = region->fields->fe_region;
				}
			}
			ACCESS(FE_region)(fe_region);
			if (region->fields && Cmiss_region_attach_FE_region(region, fe_region))
			{
				/* must catch up to region change counter */
				for (i = 0; i < region->change; i++)
				{
					Cmiss_region_fields_begin_change(region);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_region_attach_fields.  Failed");
				return_code = 0;
			}
			DEACCESS(FE_region)(&fe_region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_attach_fields.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_attach_fields */

static int DESTROY(Cmiss_region)(struct Cmiss_region **region_address)
/*******************************************************************************
LAST MODIFIED : 29 May 2008

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
			// remove link back from region_fields to this region
			if (region->fields->owning_region == *region_address)
			{
				region->fields->owning_region = NULL;
			}
			DEACCESS(Cmiss_region_fields)(&region->fields);
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

DECLARE_OBJECT_FUNCTIONS(Cmiss_region)

void Cmiss_region_detach_fields_hierarchical(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 29 May 2008

DESCRIPTION :
Deaccesses fields from region and all child regions recursively.
Temporary until circular references sorted out - certain fields access regions.
Call ONLY before deaccessing root_region in command_data.
==============================================================================*/
{
	int i;

	ENTER(Cmiss_region_detach_fields_hierarchical);
	if (region) 
	{
		for (i = 0; i < region->number_of_child_regions; i++)
		{
			Cmiss_region_detach_fields_hierarchical(region->child[i]->region);
		}
		DEACCESS(Cmiss_region_fields)(&(region->fields));
	}
	LEAVE;
} /* Cmiss_region_detach_fields */

struct Cmiss_region *Cmiss_region_create(void)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Creates a full Cmiss_region, able to have its own fields.
Region is created with an access_count of 1; DEACCESS to destroy.
==============================================================================*/
{
	struct Cmiss_region *region;

	ENTER(Cmiss_region_create);
	region = CREATE(Cmiss_region)();
	if (!region || !Cmiss_region_attach_fields(region,/*master_region*/NULL,
		CMISS_REGION_SHARE_BASES_SHAPES))
	{
		DEACCESS(Cmiss_region)(&region);
	}
	LEAVE;

	return (region);
} /* Cmiss_region_create */

struct Cmiss_region *Cmiss_region_create_group(
	struct Cmiss_region *master_region)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Creates a Cmiss_region that shares the fields with master_region but
uses a subset of its nodes and element (i.e. a group).
Region is created with an access_count of 1; DEACCESS to destroy.
Consider as private: NOT approved for exposing in API.
==============================================================================*/
{
	struct Cmiss_region *region;

	ENTER(Cmiss_region_create_group);
	region = (struct Cmiss_region *)NULL;
	if (master_region)
	{
		region = CREATE(Cmiss_region)();
		if (!region || !Cmiss_region_attach_fields(region, master_region,
			CMISS_REGION_SHARE_FIELDS_GROUP))
		{
			DEACCESS(Cmiss_region)(&region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (region);
} /* Cmiss_region_create_group */

struct Cmiss_region *Cmiss_region_create_share_globals(
		struct Cmiss_region *source_region)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Equivalent to Cmiss_region_create(), except the new region uses
global basis_manager and shape_list from <source_region>.
Region is created with an access_count of 1; DEACCESS to destroy.
Consider as private: NOT approved for exposing in API.
==============================================================================*/
{
	struct Cmiss_region *region;

	ENTER(Cmiss_region_create_share_globals);
	region = (struct Cmiss_region *)NULL;
	if (source_region)
	{
		region = CREATE(Cmiss_region)();
		if (!region || !Cmiss_region_attach_fields(region, source_region,
			CMISS_REGION_SHARE_BASES_SHAPES))
		{
			DEACCESS(Cmiss_region)(&region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_create_share_globals.  Invalid argument(s)");
	}
	LEAVE;

	return (region);
} /* Cmiss_region_create_share_globals */

struct MANAGER(Computed_field) *Cmiss_region_get_Computed_field_manager(
	struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 20 May 2008

DESCRIPTION :
Returns the field manager containing all the fields for this region.
==============================================================================*/
{
	struct MANAGER(Computed_field) *field_manager;

	ENTER(Cmiss_region_get_Computed_field_manager);
	if (cmiss_region && cmiss_region->fields)
	{
		field_manager = cmiss_region->fields->field_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_Computed_field_manager.  Invalid argument(s)");
		field_manager = (struct MANAGER(Computed_field) *)NULL;
	}
	LEAVE;

	return (field_manager);
} /* Cmiss_region_get_Computed_field_manager */

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
	const char *entry_string, struct Cmiss_region *root_region, char **path_address)
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
LAST MODIFIED : 26 May 2008

DESCRIPTION :
Increments the change counter in <region>. No update callbacks will occur until
change counter is restored to zero by calls to Cmiss_region_end_change.
Also begins caching of changes to fields in <region>, which also begins
automatically when fields are added during a change. 
Note this is NOT Hierarchical: child region change caching is not affected. 
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_begin_change);
	if (region)
	{
		region->change++;
		if (region->fields)
		{
			Cmiss_region_fields_begin_change(region);
		}
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
LAST MODIFIED : 26 May 2008

DESCRIPTION :
Decrements the change counter in <region>. No update callbacks occur until the
change counter is restored to zero by this function.
Also ends caching of changes to fields in <region>, which also ends
automatically when fields are removed during a change. 
Note this is NOT Hierarchical: child region change caching is not affected. 
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_end_change);
	if (region)
	{
		if (0 < region->change)
		{
			if (region->fields)
			{
				Cmiss_region_fields_end_change(region);
			}
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
	struct Cmiss_region *child_region, const char *child_name, int child_position)
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
		if (child_region->parent)
		{
			display_message(ERROR_MESSAGE, "Cmiss_region_add_child_region.  "
				"Child region already has a parent");
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
					child_region->parent = region;
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
				region->child[i]->region->parent = NULL;
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

int Cmiss_region_get_partial_region_path(struct Cmiss_region *root_region,
	const char *path, struct Cmiss_region **region_address,
	char **region_path_address,	char **remainder_address)
{
	char *child_name, *child_name_allocate, *child_name_end, *child_name_start;
	int length, return_code;
	struct Cmiss_region *next_region, *region;

	ENTER(Cmiss_region_get_partial_region_path);
	if (root_region && path && region_address && region_path_address &&
		remainder_address)
	{
		return_code = 1;
		region = root_region;
		child_name_allocate = duplicate_string(path);
		child_name = child_name_allocate;
		/* skip leading separator */
		if (child_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		child_name_start = child_name;
		next_region = region;

		while (next_region && (*child_name != '\0'))
		{
			child_name_end = strchr(child_name, CMISS_REGION_PATH_SEPARATOR_CHAR);
			if (child_name_end)
			{
				*child_name_end = '\0';
			}
			if (next_region =
				Cmiss_region_get_child_region_from_name(region, child_name))
			{
				region = next_region;
				if (child_name_end)
				{
					child_name = child_name_end + 1;
				}
				else
				{
					child_name = child_name + strlen(child_name);
				}
			}
			if (child_name_end)
			{
				*child_name_end = CMISS_REGION_PATH_SEPARATOR_CHAR;
			}
		}

		length = child_name - child_name_start;
		if ((length > 0) &&
			(*(child_name - 1) == CMISS_REGION_PATH_SEPARATOR_CHAR))
		{
			length--;
		}
		if (ALLOCATE(*region_path_address, char, length + 1))
		{
			strncpy(*region_path_address, child_name_start, length);
			(*region_path_address)[length] = '\0';
		}
		else
		{
			return_code = 0;
		}

		length = strlen(child_name);
		if (length == 0)
		{
			*remainder_address = NULL;
		}
		else
		{
			/* remove trailing '/' */
			if (child_name[length-1] == CMISS_REGION_PATH_SEPARATOR_CHAR)
			{
				length--;
			}
			if (ALLOCATE(*remainder_address, char, length + 1))
			{
				strncpy(*remainder_address, child_name, length);
				(*remainder_address)[length] = '\0';
			}
			else
			{
				return_code = 0;
			}
		}

		*region_address = region;
		DEALLOCATE(child_name_allocate);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_partial_region_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_get_partial_region_path */

/***************************************************************************//**
 * Modifier function to set the region, region path and field name.
 * Fields must not be the same name as a child region.
 *
 * Examples:
 *   /heart/coordinates = region path and field name
 *   heart              = region path only
 *   coordinates        = field name only 
 * @param region_path_and_name a struct Cmiss_region_path_and_name which if
 *   set contains an ACCESSed region and allocated path and name which caller
 *   is required to clean up. Name may be NULL if path is fully resolved.
 */
static int set_region_path_and_or_field_name(struct Parse_state *state,
	void *region_path_and_name_void, void *root_region_void)
{
	const char *current_token;
	int return_code;
	struct Cmiss_region_path_and_name *name_data;
	struct Cmiss_region *root_region;

	ENTER(set_region_path_and_or_field_name);
	if (state && (name_data = (struct Cmiss_region_path_and_name *)region_path_and_name_void) &&
		(root_region = (struct Cmiss_region *)root_region_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (Cmiss_region_get_partial_region_path(root_region, current_token,
					&name_data->region, &name_data->region_path, &name_data->name))
				{
					ACCESS(Cmiss_region)(name_data->region);
					if (!name_data->name || (NULL == strchr(name_data->name, CMISS_REGION_PATH_SEPARATOR_CHAR)))
					{
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						display_message(ERROR_MESSAGE, "Bad region path and/or field name: %s",current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_region_path_and_or_field_name.  Failed to get path and name");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " REGION_PATH|REGION_PATH/FIELD_NAME|FIELD_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing region path and/or field name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_region_path_and_or_field_name.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_region_path_and_or_field_name */

int Option_table_add_region_path_and_or_field_name_entry(
	struct Option_table *option_table, char *token,
	struct Cmiss_region_path_and_name *region_path_and_name,
	struct Cmiss_region *root_region)
{
	int return_code;

	ENTER(Option_table_add_region_path_and_or_field_name_entry);
	if (option_table && region_path_and_name && root_region)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)region_path_and_name, (void *)root_region,
			set_region_path_and_or_field_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_region_path_and_or_field_name_entry.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_region_path_and_or_field_name_entry */

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

struct Cmiss_region *Cmiss_region_get_child_region(struct Cmiss_region *region,
	int child_number)
/*******************************************************************************
LAST MODIFIED : 26 May 2008

DESCRIPTION :
Returns the child <child_number> of <region> if within the range from 0 to
one less than number_of_child_regions, otherwise NULL.
==============================================================================*/
{
	struct Cmiss_region *child_region;

	ENTER(Cmiss_region_get_child_region);
	child_region = (struct Cmiss_region *)NULL;
	if (region)
	{
		if ((0 <= child_number) && (child_number < region->number_of_child_regions))
		{
			child_region = (region->child[child_number])->region;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_child_region.  Invalid argument(s)");
	}
	LEAVE;

	return (child_region);
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
				region->any_object_list))
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

Cmiss_field_id Cmiss_region_add_field(Cmiss_region_id region,
	Cmiss_field_id field)
{
	struct MANAGER(Computed_field) *manager;

	ENTER(Cmiss_region_add_field);
	if (region && 
		(manager = Cmiss_region_get_Computed_field_manager(region))
		&& field)
	{
		if (Computed_field_check_manager(field, &manager))
		{
			if (!Computed_field_add_to_manager_recursive(field, manager))
			{
				field = (struct Cmiss_field *)NULL;
				display_message(ERROR_MESSAGE,
					"Cmiss_region_add_field.  Field cannot be added to region.");
			}
		}
		else
		{
			field = (struct Cmiss_field *)NULL;
			display_message(ERROR_MESSAGE,
				"Cmiss_region_add_field.  Field or source field is already in another region.");
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

char *Cmiss_region_get_path(struct Cmiss_region *region)
{
	char *path = NULL;

	ENTER(Cmiss_region_get_path);
	if (region)
	{
		int error = 0;
		Cmiss_region* parent = region->parent;
		if (parent)
		{
			if (path = Cmiss_region_get_path(parent))
			{
				const char *child_name = NULL;
				for (int i = 0; i < parent->number_of_child_regions; i++)
				{
					if (region == parent->child[i]->region)
					{
						child_name = parent->child[i]->name;
						break;
					}
				}
				append_string(&path, child_name, &error);
			}
			else
			{
				error = 1;
			}
		}
		append_string(&path, CMISS_REGION_PATH_SEPARATOR_STRING, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_get_path.  Missing region");
	}
	LEAVE;
	
	return (path);
}
