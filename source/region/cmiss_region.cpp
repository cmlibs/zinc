/***************************************************************************//**
 * FILE : cmiss_region.cpp
 * 
 * Definition of Cmiss_region, container of fields for representing model data,
 * and child regions for building hierarchical models.
 */
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
#include "api/cmiss_field_module.h"
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

FULL_DECLARE_CMISS_CALLBACK_TYPES(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *);

enum Cmiss_region_attach_fields_variant
{
	CMISS_REGION_SHARE_BASES_SHAPES,
	CMISS_REGION_SHARE_FIELDS_GROUP
};

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
	void *field_manager_callback_id;
	struct FE_region *fe_region;
	int access_count;
};

/***************************************************************************//**
 * A region object which contains fields and child regions.
 * Other data can be attached using the any_object_list.
 */
struct Cmiss_region
{
	char *name;
	/* non-accessed pointer to parent region, or NULL if root */
	Cmiss_region *parent;
	/* accessed first child and next sibling for building region tree */
	Cmiss_region *first_child, *next_sibling;
	/* non-access pointer to previous sibling, if any */
	Cmiss_region *previous_sibling;
	
	/* fields owned by this region (or master) */
	Cmiss_region_fields *fields;

	/* list of objects attached to region */
	struct LIST(Any_object) *any_object_list;

	/* increment/decrement change_level to nest changes. Message sent when zero */
	int change_level;
	/* number of hierarchical changes in progress on this region tree. A region's
	 * change_level will have one increment per ancestor hierarchical_change_level.
	 * Must be tracked to safely transfer when re-parenting regions */
	int hierarchical_change_level;
	Cmiss_region_changes changes;
	/* list of change callbacks */
	struct LIST(CMISS_CALLBACK_ITEM(Cmiss_region_change)) *change_callback_list;

	/* number of objects using this region */
	int access_count;
};

/*
Module functions
----------------
*/

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Cmiss_region_change, void)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *)

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_region_fields);

/***************************************************************************//**
 * Computed field manager callback - for true regions only. Asks fields of
 * parent region to propagate field changes if hierarchical.
 * Initially supports Cmiss_field_group.
 *
 * @param message  The changes to the fields in the region's manager.
 * @param region_void  Void pointer to changed region (not the parent).
 */
static void Cmiss_region_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *region_void)
{
	Cmiss_region *region = (Cmiss_region *)region_void;
	if (message && region)
	{
		int change_summary =
			MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Computed_field)(message);
		if (change_summary & (MANAGER_CHANGE_RESULT(Computed_field) |
			MANAGER_CHANGE_ADD(Computed_field)))
		{
			Cmiss_region *parent = Cmiss_region_get_parent_internal(region);
			if (parent)
			{
				Computed_field_manager_propagate_hierarchical_field_changes(
					parent->fields->field_manager, message);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_Computed_field_change.  Invalid argument(s)");
	}
	LEAVE;
}

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
		region_fields->field_manager_callback_id = MANAGER_REGISTER(Computed_field)(
			Cmiss_region_Computed_field_change, (void *)owning_region,
			region_fields->field_manager);
		region_fields->fe_region = CREATE(FE_region)(
			/*master_fe_region*/(struct FE_region *)NULL,
			basis_manager, element_shape_list);
		ACCESS(FE_region)(region_fields->fe_region);
		region_fields->access_count = 1;
		if (!(region_fields->field_manager &&
			region_fields->fe_region &&
			FE_region_add_callback(region_fields->fe_region,
				Cmiss_region_FE_region_change, (void *)owning_region)))
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

/** call only when detaching Cmiss_region_fields from owning region */
static void Cmiss_region_fields_end_callbacks(Cmiss_region_fields *region_fields)
{
	MANAGER_DEREGISTER(Computed_field)(
		region_fields->field_manager_callback_id, region_fields->field_manager);
	region_fields->field_manager_callback_id = NULL;
	// remove link back from region_fields to this region
	FE_region_remove_callback(region_fields->fe_region,
		Cmiss_region_FE_region_change, (void *)region_fields->owning_region);
	region_fields->owning_region = NULL;
}

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
			if (region_fields->owning_region)
			{
				Cmiss_region_fields_end_callbacks(region_fields);
			}
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
		FE_region *fe_region = Cmiss_region_get_FE_region(region);
		FE_region_begin_change(fe_region);
		FE_region_begin_change(FE_region_get_data_FE_region(fe_region));
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
		FE_region *fe_region = Cmiss_region_get_FE_region(region);
		FE_region_end_change(fe_region);
		FE_region_end_change(FE_region_get_data_FE_region(fe_region));
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

struct Cmiss_region *Cmiss_region_fields_get_master_region(
	struct Cmiss_region_fields *region_fields)
{
	struct Cmiss_region *region;

	ENTER(Cmiss_region_fields_get_master_region);
	if (region_fields)
	{
		region = region_fields->owning_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_fields_get_master_region.  Missing region fields object");
		region = (struct Cmiss_region *)NULL;
	}
	LEAVE;

	return (region);
}

DECLARE_OBJECT_FUNCTIONS(Cmiss_region_fields)

namespace {

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
		if ((0 == region->change_level) && ((region->changes.children_changed) ||
			(region->changes.name_changed)))
		{
			if (0 != region->hierarchical_change_level)
			{
				display_message(WARNING_MESSAGE, "Cmiss_region_update.  Hierarchical change level mismatch");
			}
			changes = region->changes;
			/* must clear flags in the region before changes go out */
			region->changes.name_changed = 0;
			region->changes.children_changed = 0;
			region->changes.child_added = (struct Cmiss_region *)NULL;
			region->changes.child_removed = (struct Cmiss_region *)NULL;
			/* send the callbacks */
			CMISS_CALLBACK_LIST_CALL(Cmiss_region_change)(
				region->change_callback_list, region, &changes);
			// deaccess child pointers from message
			REACCESS(Cmiss_region)(&changes.child_added, NULL);
			REACCESS(Cmiss_region)(&changes.child_removed, NULL);
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

/***************************************************************************//**
 * Internal-only implementation of Cmiss_region_find_child_by_name which does
 * not ACCESS the returned reference.
 * @see Cmiss_region_find_child_by_name
 */
struct Cmiss_region *Cmiss_region_find_child_by_name_internal(
	struct Cmiss_region *region, const char *name)
{
	struct Cmiss_region *child = NULL;
	if (region && name)
	{
		child = region->first_child;
		while (child)
		{
			if (0 == strcmp(child->name, name))
			{
				break;
			}
			child = child->next_sibling;
		}
	}
	return (child);
}

/***************************************************************************//**
 * Internal-only implementation of Cmiss_region_find_subregion_at_path which
 * does not ACCESS the returned reference.
 * @see Cmiss_region_find_subregion_at_path
 */
struct Cmiss_region *Cmiss_region_find_subregion_at_path_internal(
	struct Cmiss_region *region, const char *path)
{
	struct Cmiss_region *subregion = NULL;
	if (region && path)
	{
		subregion = region;
		char *path_copy = duplicate_string(path);
		char *child_name = path_copy;
		/* skip leading separator */
		if (child_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		char *child_name_end;
		while (subregion && (child_name_end =
			strchr(child_name, CMISS_REGION_PATH_SEPARATOR_CHAR)))
		{
			*child_name_end = '\0';
			subregion = Cmiss_region_find_child_by_name_internal(subregion, child_name);
			child_name = child_name_end + 1;
		}
		/* already found the subregion if there was a single trailing separator */
		if (subregion && (child_name[0] != '\0'))
		{
			subregion = Cmiss_region_find_child_by_name_internal(subregion, child_name);
		}
		DEALLOCATE(path_copy);
	}

	return (subregion);
}

} // anonymous namespace

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
		region->name = NULL;
		region->parent = NULL;
		region->first_child = NULL;
		region->next_sibling = NULL;
		region->previous_sibling = NULL;
		region->fields = NULL;
		region->any_object_list = CREATE(LIST(Any_object))();
		region->change_level = 0;
		region->hierarchical_change_level = 0;
		region->changes.name_changed = 0;
		region->changes.children_changed = 0;
		region->changes.child_added = NULL;
		region->changes.child_removed = NULL;
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
				for (i = 0; i < region->change_level; i++)
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
	int return_code;
	struct Cmiss_region *region;

	ENTER(DESTROY(Cmiss_region));
	if (region_address && (NULL != (region = *region_address)))
	{
		if (0 == region->access_count)
		{
			DESTROY(LIST(Any_object))(&(region->any_object_list));

			// destroy child list
			Cmiss_region *next_child = region->first_child;
			region->first_child = NULL;
			Cmiss_region *child;
			while ((child = next_child))
			{
				next_child = child->next_sibling;
				child->parent = NULL;
				child->next_sibling = NULL;
				child->previous_sibling = NULL;
				DEACCESS(Cmiss_region)(&child);
			}

			REACCESS(Cmiss_region)(&region->changes.child_added, NULL);
			REACCESS(Cmiss_region)(&region->changes.child_removed, NULL);

			DESTROY(LIST(CMISS_CALLBACK_ITEM(Cmiss_region_change)))(
				&(region->change_callback_list));
			if (region->fields)
				DEACCESS(Cmiss_region_fields)(&region->fields);
			if (region->name)
			{
				DEALLOCATE(region->name);
			}
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
	ENTER(Cmiss_region_detach_fields_hierarchical);
	if (region) 
	{
		Cmiss_region *child = region->first_child;
		while (child)
		{
			Cmiss_region_detach_fields_hierarchical(child);
			child = child->next_sibling;
		}
		// remove link back from region_fields to owning region
		if (region->fields && (region->fields->owning_region == region))
		{
			Cmiss_region_fields_end_callbacks(region->fields);
		}
		DEACCESS(Cmiss_region_fields)(&(region->fields));
	}
	LEAVE;
} /* Cmiss_region_detach_fields */

struct Cmiss_region *Cmiss_region_create_internal(void)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
Creates a full Cmiss_region, able to have its own fields.
Region is created with an access_count of 1; DEACCESS to destroy.
==============================================================================*/
{
	struct Cmiss_region *region;

	ENTER(Cmiss_region_create_internal);
	region = CREATE(Cmiss_region)();
	if (!region || !Cmiss_region_attach_fields(region,/*master_region*/NULL,
		CMISS_REGION_SHARE_BASES_SHAPES))
	{
		DEACCESS(Cmiss_region)(&region);
	}
	LEAVE;

	return (region);
} /* Cmiss_region_create_internal */

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

struct Cmiss_region *Cmiss_region_create_region(struct Cmiss_region *base_region)
{
	struct Cmiss_region *region;

	ENTER(Cmiss_region_create_region);
	region = (struct Cmiss_region *)NULL;
	if (base_region)
	{
		region = CREATE(Cmiss_region)();
		if (!region || !Cmiss_region_attach_fields(region, base_region,
			CMISS_REGION_SHARE_BASES_SHAPES))
		{
			DEACCESS(Cmiss_region)(&region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_create_region.  Invalid argument(s)");
	}
	LEAVE;

	return (region);
} /* Cmiss_region_create_region */

struct Cmiss_region *Cmiss_region_create_child(struct Cmiss_region *parent_region, 
	const char *name)
{
	struct Cmiss_region *region = NULL;
	if (parent_region)
	{
		region = Cmiss_region_create_region(parent_region);
		if ((!Cmiss_region_set_name(region, name)) ||
			(!Cmiss_region_append_child(parent_region, region)))
		{
			Cmiss_region_destroy(&region);
		}
	}
	return region;
}

struct Cmiss_region *Cmiss_region_create_subregion(
	struct Cmiss_region *top_region, const char *path)
{
	// Fails if a subregion exists at that path already
	struct Cmiss_region *region = Cmiss_region_find_child_by_name(top_region, path);
	if (region)
	{
		Cmiss_region_destroy(&region);
		return NULL;
	}
	if (top_region && path)
	{
		region = ACCESS(Cmiss_region)(top_region);
		char *path_copy = duplicate_string(path);
		char *child_name = path_copy;
		if (child_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		while (region && child_name && (child_name[0] != '\0'))
		{
			char *child_name_end = strchr(child_name, CMISS_REGION_PATH_SEPARATOR_CHAR);
			if (child_name_end)
			{
				*child_name_end = '\0';
			}
			Cmiss_region *child_region = Cmiss_region_find_child_by_name(region, child_name);
			if (!child_region)
			{
				child_region = Cmiss_region_create_child(region, child_name);
			}
			REACCESS(Cmiss_region)(&region, child_region);
			DEACCESS(Cmiss_region)(&child_region);
			if (child_name_end)
			{
				child_name = child_name_end + 1;
			}
			else
			{
				child_name = (char *)NULL;
			}
		}
		DEALLOCATE(path_copy);
	}
	return (region);
}

int Cmiss_region_clear_finite_elements(struct Cmiss_region *region)
{
	return FE_region_clear(Cmiss_region_get_FE_region(region),
		/*destroy_in_master*/0);
}

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

/***************************************************************************//**
 * Option_table modifier function for selecting a region by relative path.
 * @see Option_table_add_set_Cmiss_region
 */
int set_Cmiss_region(struct Parse_state *state, void *region_address_void,
	void *root_region_void)
{
	const char *current_token;
	int return_code;
	struct Cmiss_region *region, **region_address, *root_region;

	ENTER(set_Cmiss_region);
	if (state && (root_region = static_cast<struct Cmiss_region *>(root_region_void)) &&
		(region_address = static_cast<struct Cmiss_region **>(region_address_void)))
	{
		if ((current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				region = Cmiss_region_find_subregion_at_path_internal(root_region, current_token);
				if (region)
				{
					REACCESS(Cmiss_region)(region_address, region);
					return_code = shift_Parse_state(state, 1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Cmiss_region:  Could not find subregion %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," PATH_TO_REGION");
				if (*region_address)
				{
					char *path = Cmiss_region_get_path(*region_address);
					display_message(INFORMATION_MESSAGE, "[%s]", path);
					DEALLOCATE(path);
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
		display_message(ERROR_MESSAGE, "set_Cmiss_region.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Option_table_add_set_Cmiss_region(struct Option_table *option_table,
	const char *token, struct Cmiss_region *root_region,
	struct Cmiss_region **region_address)
{
	int return_code;

	ENTER(Option_table_add_set_Cmiss_region);
	if (option_table && token && root_region && region_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)region_address, (void *)root_region, set_Cmiss_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_set_Cmiss_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int set_Cmiss_region_path(struct Parse_state *state, void *path_address_void,
	void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Modifier function for entering a path to a Cmiss_region, starting at
<root_region>.
==============================================================================*/
{
	const char *current_token;
	char **path_address;
	int return_code;
	struct Cmiss_region *region, *root_region;

	ENTER(set_Cmiss_region_path);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		if ((current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				region = Cmiss_region_find_subregion_at_path_internal(
					root_region, current_token);
				if (region)
				{
					if ((path_address = (char **)path_address_void))
					{
						if (*path_address)
						{
							DEALLOCATE(*path_address);
						}
						if ((*path_address = duplicate_string(current_token)))
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
	int return_code = 0;

	ENTER(Option_table_add_set_Cmiss_region_path);
	if (option_table && entry_string && root_region && path_address)
	{
		return_code = Option_table_add_entry(option_table, entry_string,
			(void *)path_address, (void *)root_region,
			set_Cmiss_region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_set_Cmiss_region_path.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_set_Cmiss_region_path */

int Cmiss_field_module_begin_change(Cmiss_field_module_id field_module)
{
	return Cmiss_region_fields_begin_change(Cmiss_field_module_get_region_internal(field_module));
}

int Cmiss_field_module_end_change(Cmiss_field_module_id field_module)
{
	return Cmiss_region_fields_end_change(Cmiss_field_module_get_region_internal(field_module));
}

int Cmiss_field_module_define_all_faces(Cmiss_field_module_id field_module)
{
	return FE_region_define_faces(Cmiss_region_get_FE_region(
		Cmiss_field_module_get_region_internal(field_module)));
}

int Cmiss_region_begin_change(struct Cmiss_region *region)
{
	int return_code;

	ENTER(Cmiss_region_begin_change);
	if (region)
	{
		region->change_level++;
		Cmiss_region_fields_begin_change(region);
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
}

int Cmiss_region_end_change(struct Cmiss_region *region)
{
	int return_code;

	ENTER(Cmiss_region_end_change);
	if (region)
	{
		if (0 < region->change_level)
		{
			Cmiss_region_fields_end_change(region);
			region->change_level--;
			if (0 == region->change_level)
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
}

/***************************************************************************//**
 * Returns the sum of the hierarchical_change_level members of region and all
 * its ancestors. Equals the number of begin_change calls needed for new
 * children of region.
 */
static int Cmiss_region_get_sum_hierarchical_change_level(struct Cmiss_region *region)
{
	int sum_hierarchical_change_level = 0;
	while (region)
	{
		sum_hierarchical_change_level += region->hierarchical_change_level;
		region = region->parent;
	}
	return sum_hierarchical_change_level;
}

/***************************************************************************//**
 * Adds delta_change_level to change_level of region and all its descendents.
 * Begins or ends change cache as many times as magnitude of delta_change_level.
 */
static void Cmiss_region_tree_change(struct Cmiss_region *region,
	int delta_change_level)
{
	if (region)
	{
		for (int i = 0; i < delta_change_level; i++)
		{
			Cmiss_region_begin_change(region);
		}
		Cmiss_region *child = region->first_child;
		while (child)
		{
			Cmiss_region_tree_change(child, delta_change_level);
			child = child->next_sibling;
		}
		for (int i = 0; i > delta_change_level; i--)
		{
			Cmiss_region_end_change(region);
		}
	}
}

int Cmiss_region_begin_hierarchical_change(struct Cmiss_region *region)
{
	if (region)
	{
		region->hierarchical_change_level++;
		Cmiss_region_tree_change(region, +1);
		return 1;
	}
	return 0;
}

int Cmiss_region_end_hierarchical_change(struct Cmiss_region *region)
{
	if (region)
	{
		region->hierarchical_change_level--;
		Cmiss_region_tree_change(region, -1);
		return 1;
	}
	return 0;
}

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

char *Cmiss_region_get_name(struct Cmiss_region *region)
{
	char *name = NULL;
	if (region && region->name)
	{
		name = duplicate_string(region->name); 
	}
	return (name);
}

int Cmiss_region_set_name(struct Cmiss_region *region, const char *name)
{
	int return_code = 0;
	if (region && name)
	{
		if (is_standard_object_name(name))
		{
			return_code = 1;
			if ((NULL == region->name) || strcmp(region->name, name))
			{
				if ((NULL == region->parent) ||
					(NULL == Cmiss_region_find_child_by_name_internal(region->parent, name)))
				{
					char *temp_name = duplicate_string(name);
					if (region->name)
					{
						DEALLOCATE(region->name);
					}
					region->name = temp_name;
					region->changes.name_changed = 1;
					Cmiss_region_update(region);
				}
				else
				{
					return_code = 0; // name is in use by sibling
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_set_name.  Invalid region name '%s'", name);
		}
	}
	return (return_code);
}

char *Cmiss_region_get_root_region_path(void)
{
	return duplicate_string(CMISS_REGION_PATH_SEPARATOR_STRING);
}

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
			if ((path = Cmiss_region_get_path(parent)))
			{
				append_string(&path, region->name, &error);
			}
			else
			{
				DEALLOCATE(path);
				error = 1;
			}
		}
		append_string(&path, CMISS_REGION_PATH_SEPARATOR_STRING, &error);
	}
	LEAVE;
	
	return (path);
}

char *Cmiss_region_get_relative_path(struct Cmiss_region *region,
	struct Cmiss_region *other_region)
{
	char *path = NULL;

	ENTER(Cmiss_region_get_relative_path);
	if (region && other_region)
	{
		int error = 0;
		if (region != other_region) 
		{
			Cmiss_region* parent = region->parent;
			if (parent)
			{
				if ((path = Cmiss_region_get_relative_path(parent, other_region)))
				{
					append_string(&path, region->name, &error);
				}
				else
				{
					error = 1;
				}
			}
			else
			{
				error = 1;
			}
		}
		append_string(&path, CMISS_REGION_PATH_SEPARATOR_STRING, &error);
	}
	LEAVE;
	
	return (path);
}

struct Cmiss_region *Cmiss_region_get_parent(struct Cmiss_region *region)
{
	return (region && region->parent) ?
		ACCESS(Cmiss_region)(region->parent) : NULL;
}

struct Cmiss_region *Cmiss_region_get_parent_internal(struct Cmiss_region *region)
{
	if (region)
		return region->parent;
	return 0;
}

struct Cmiss_region *Cmiss_region_get_first_child(struct Cmiss_region *region)
{
	return (region && region->first_child) ?
		ACCESS(Cmiss_region)(region->first_child) : NULL;
}

struct Cmiss_region *Cmiss_region_get_next_sibling(struct Cmiss_region *region)
{
	return (region && region->next_sibling) ?
		ACCESS(Cmiss_region)(region->next_sibling) : NULL;
}

struct Cmiss_region *Cmiss_region_get_previous_sibling(struct Cmiss_region *region)
{
	return (region && region->previous_sibling) ?
		ACCESS(Cmiss_region)(region->previous_sibling) : NULL;
}

void Cmiss_region_reaccess_next_sibling(struct Cmiss_region **region_address)
{
	if (region_address && (*region_address))
	{
		REACCESS(Cmiss_region)(region_address, (*region_address)->next_sibling);
	}
}

int Cmiss_region_append_child(struct Cmiss_region *region,
	struct Cmiss_region *new_child)
{
	return Cmiss_region_insert_child_before(region, new_child, NULL);
}

int Cmiss_region_insert_child_before(struct Cmiss_region *region,
	struct Cmiss_region *new_child, struct Cmiss_region *ref_child)
{
	int return_code = 1;
	if (!(region && new_child &&
		((NULL == ref_child) || (ref_child->parent == region)) &&
		(!Cmiss_region_contains_subregion(new_child, region)) &&
		((NULL != new_child->name) && ((new_child->parent == region) ||
		(NULL == Cmiss_region_find_child_by_name_internal(region,
			new_child->name))))))
	{
		return_code = 0;
	}
	if (region && Cmiss_region_is_group(region))
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_insert_child_before.  Can't add child to group region");
		return_code = 0;
	}
	else if (region && new_child && Cmiss_region_is_group(new_child))
	{
		FE_region *fe_region = Cmiss_region_get_FE_region(region);
		FE_region *child_fe_region = Cmiss_region_get_FE_region(new_child);
		FE_region *master_fe_region = NULL;
		if (fe_region &&
			FE_region_get_immediate_master_FE_region(child_fe_region, &master_fe_region) &&
			(master_fe_region && (master_fe_region != fe_region)))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_insert_child_before.  Group region must be a child of its master region");
			return_code = 0;
		}
	}
	if (return_code)
	{
		int delta_change_level = Cmiss_region_get_sum_hierarchical_change_level(region);
		Cmiss_region_begin_change(region);
		if (new_child->parent)
		{
			delta_change_level -= Cmiss_region_get_sum_hierarchical_change_level(new_child->parent);
			Cmiss_region_remove_child(new_child->parent, new_child);
		}
		new_child->parent = region;
		if (ref_child)
		{
			new_child->next_sibling = ref_child;
			new_child->previous_sibling = ref_child->previous_sibling;
			ref_child->previous_sibling = new_child;
			if (new_child->previous_sibling)
			{
				new_child->previous_sibling->next_sibling = ACCESS(Cmiss_region)(new_child);
			}
			else
			{
				region->first_child = ACCESS(Cmiss_region)(new_child);
			}
		}
		else
		{
			if (region->first_child)
			{
				Cmiss_region *last_child = region->first_child;
				while (last_child->next_sibling)
				{
					last_child = last_child->next_sibling;
				}
				last_child->next_sibling = ACCESS(Cmiss_region)(new_child);
				new_child->previous_sibling = last_child;
			}
			else
			{
				region->first_child = ACCESS(Cmiss_region)(new_child);
			}
		}
		if (!region->changes.children_changed)
		{
			region->changes.children_changed = 1;
			region->changes.child_added = ACCESS(Cmiss_region)(new_child);
		}
		else
		{
			/* multiple changes */
			REACCESS(Cmiss_region)(&region->changes.child_added, NULL);
			REACCESS(Cmiss_region)(&region->changes.child_removed, NULL);
		}
		if (delta_change_level != 0)
		{
			Cmiss_region_tree_change(new_child, delta_change_level);
		}
		Cmiss_region_end_change(region);
	}
	return (return_code);
}

int Cmiss_region_remove_child(struct Cmiss_region *region,
	struct Cmiss_region *old_child)
{
	int return_code = 0;
	if (region && old_child)
	{
		if (old_child->parent == region)
		{
			int delta_change_level = Cmiss_region_get_sum_hierarchical_change_level(region);
			if (old_child == region->first_child)
			{
				region->first_child = old_child->next_sibling;
			}
			else
			{
				old_child->previous_sibling->next_sibling = old_child->next_sibling;
			}
			if (old_child->next_sibling)
			{
				old_child->next_sibling->previous_sibling = old_child->previous_sibling;
				old_child->next_sibling = NULL;
			}
			old_child->previous_sibling = NULL;
			old_child->parent = NULL;
			if (!region->changes.children_changed)
			{
				region->changes.children_changed = 1;
				region->changes.child_removed = ACCESS(Cmiss_region)(old_child);
			}
			else
			{
				/* multiple changes */
				REACCESS(Cmiss_region)(&region->changes.child_added, NULL);
				REACCESS(Cmiss_region)(&region->changes.child_removed, NULL);
			}
			if (delta_change_level != 0)
			{
				Cmiss_region_tree_change(old_child, delta_change_level);
			}
			Cmiss_region_update(region);
			DEACCESS(Cmiss_region)(&old_child);
			return_code = 1;
		}
	}
	return (return_code);
}

int Cmiss_region_contains_subregion(struct Cmiss_region *region,
	struct Cmiss_region *subregion)
{
	int return_code = 0;
	if (region && subregion)
	{
		do
		{
			if (subregion == region)
			{
				return_code = 1;
				break;
			}
		} while (NULL != (subregion = subregion->parent));
	}
	return (return_code);
}

struct Cmiss_region *Cmiss_region_find_child_by_name(
	struct Cmiss_region *region, const char *name)
{
	Cmiss_region *child = Cmiss_region_find_child_by_name_internal(region, name);
	if (child)
	{
		ACCESS(Cmiss_region)(child);
	}
	return (child);
}

struct Cmiss_region *Cmiss_region_find_subregion_at_path(
	struct Cmiss_region *region, const char *path)
{
	Cmiss_region *subregion =
		Cmiss_region_find_subregion_at_path_internal(region, path);
	if (subregion)
	{
		ACCESS(Cmiss_region)(subregion);
	}
	return (subregion);
}

Cmiss_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region,
	const char *field_name)
{
	struct Cmiss_field *field = NULL;
	if (region && field_name)
	{
		field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			(char *)field_name, Cmiss_region_get_Computed_field_manager(region));
		if (field)
		{
			ACCESS(Computed_field)(field);
		}
	}
	return (field);
}

int Cmiss_region_get_region_from_path_deprecated(struct Cmiss_region *region,
	const char *path, struct Cmiss_region **subregion_address)
{
	int return_code = 0;
	if (subregion_address)
	{
		*subregion_address =
			Cmiss_region_find_subregion_at_path_internal(region, path);
		if (NULL != *subregion_address)
		{
			return_code = 1;
		}
	}
	return (return_code);
}

/***************************************************************************//**
 * Returns a reference to the root region of this region.
 *
 * @param region  The region.
 * @return  Accessed reference to root region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_get_root(struct Cmiss_region *region)
{
	if (!region)
		return NULL;
	Cmiss_region *root = region;
	while (root->parent)
	{
		root = root->parent;
	}
	return ACCESS(Cmiss_region)(root);
}

int Cmiss_region_get_partial_region_path(struct Cmiss_region *root_region,
	const char *path, struct Cmiss_region **region_address,
	char **region_path_address,	char **remainder_address)
{
	char *child_name, *path_copy, *child_name_end, *child_name_start;
	int length, return_code;
	struct Cmiss_region *next_region, *region;

	ENTER(Cmiss_region_get_partial_region_path);
	if (root_region && path && region_address && region_path_address &&
		remainder_address)
	{
		return_code = 1;
		region = root_region;
		path_copy = duplicate_string(path);
		child_name = path_copy;
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
			if ((next_region = Cmiss_region_find_child_by_name_internal(region, child_name)))
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
		DEALLOCATE(path_copy);
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
		if ((current_token = state->current_token))
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
	int return_code;

	ENTER(Cmiss_region_list);
	if (region && (0 <= indent) && (0 < indent_increment))
	{
		Cmiss_region *child = region->first_child;
		while (child)
		{
			display_message(INFORMATION_MESSAGE, "%*s%s%s\n", indent, " ", child->name,
				Cmiss_region_is_group(child) ? " (Group)" : "");
			Cmiss_region_list(child, indent + indent_increment, indent_increment);
			child = child->next_sibling;
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
		return_code =
			ADD_OBJECT_TO_LIST(Any_object)(any_object, region->any_object_list);
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
			return_code = REMOVE_OBJECT_FROM_LIST(Any_object)(any_object,
				region->any_object_list);
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

int Cmiss_region_is_group(struct Cmiss_region *region)
{
	int return_code = 0;

	ENTER(Cmiss_region_is_group);
	if (region)
	{
		FE_region *fe_region = Cmiss_region_get_FE_region(region);
		FE_region *master_fe_region = NULL;
		if (fe_region &&
			FE_region_get_immediate_master_FE_region(fe_region, &master_fe_region) &&
			master_fe_region)
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_is_group.  Missing region");
	}
	LEAVE;
	
	return (return_code);
}

/***************************************************************************//**
 * Returns field module container for this region's fields, which must be passed
 * to field factory create methods.
 *
 * @param region  The region from which to obtain the field module.
 * @return  Field module object.
 */
struct Cmiss_field_module *Cmiss_region_get_field_module(struct Cmiss_region *region)
{
	return Cmiss_field_module_create(region);
}

struct Cmiss_region *Cmiss_region_get_child_with_FE_region(
	struct Cmiss_region *region, struct FE_region *fe_region)
{
	if (region && fe_region)
	{
		bool is_data = FE_region_is_data_FE_region(fe_region);
		Cmiss_region *child = region->first_child;
		while (child)
		{
			FE_region *child_fe_region = Cmiss_region_get_FE_region(child);
			if ((child_fe_region == fe_region) || ((is_data) &&
				(fe_region == FE_region_get_data_FE_region(child_fe_region))))
			{
				return child;
			}
			child = child->next_sibling;
		}
	}
	return 0;
}

Cmiss_region_id Cmiss_region_access(Cmiss_region_id region)
{
	return (ACCESS(Cmiss_region)(region));
}

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
