/*******************************************************************************
FILE : finite_element_region.cpp

LAST MODIFIED : 8 August 2006

DESCRIPTION :
Object comprising a single finite element mesh including nodes, elements and
finite element fields defined on or interpolated over them.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstdlib>
#include <cstdio>
#include <vector>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "general/callback_private.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_private.h"
#include "general/message.h"

/*
Module types
------------
*/

FE_region_changes::FE_region_changes(struct FE_region *fe_region) :
	access_count(1)
{
	this->fe_field_changes = fe_region->fe_field_changes;
	for (int n = 0; n < 2; ++n)
		this->fe_node_changes[n] = fe_region->nodesets[n]->extractChangeLog();
	// when extracting element change logs, propagate field change summary flags
	// (RELATED flag) from nodes and parent elements
	int nodeChanges;
	CHANGE_LOG_GET_CHANGE_SUMMARY(FE_node)(this->fe_node_changes[0], &nodeChanges);
	bool parentChange = 0 != (nodeChanges & CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_node));
	bool parentAllChange = CHANGE_LOG_IS_ALL_CHANGE(FE_node)(this->fe_node_changes[0]);
	for (int dim = MAXIMUM_ELEMENT_XI_DIMENSIONS - 1; 0 <= dim; --dim)
	{
		if (0 != (this->elementChangeLogs[dim] = fe_region->meshes[dim]->extractChangeLog()))
		{
			if (parentChange)
			{
				if (parentAllChange)
					this->elementChangeLogs[dim]->setAllChange(DS_LABEL_CHANGE_TYPE_RELATED);
				else
					this->elementChangeLogs[dim]->setChange(DS_LABEL_CHANGE_TYPE_RELATED);
			}
			else
			{
				int elementChanges = this->elementChangeLogs[dim]->getChangeSummary();
				parentChange = 0 != (elementChanges & DS_LABEL_CHANGE_TYPE_RELATED);
			}
			if (parentChange && !parentAllChange)
				parentAllChange = this->elementChangeLogs[dim]->isAllChange();
		}
	}
	fe_region->createChangeLogs();
}

FE_region_changes::~FE_region_changes()
{
	for (int n = 0; n < 2; ++n)
		DESTROY(CHANGE_LOG(FE_node))(&(this->fe_node_changes[n]));
	DESTROY(CHANGE_LOG(FE_field))(&(this->fe_field_changes));
	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		cmzn::Deaccess(this->elementChangeLogs[dim]);
}

bool FE_region_changes::elementOrParentChanged(FE_element *element)
{
	return (0 != FE_element_or_parent_changed(element, this->elementChangeLogs, this->fe_node_changes[0]));
}

/**
 * Tells parent region about changes to fields, nodes and elements.
 * No messages sent if change counter is positive.
 * Only call this function if changes have actually been made.
 */
void FE_region::update()
{
	if (!this->change_level)
	{
		// note this only informs region of change; change logs are extracted
		// on demand when computed field manager change is sent to region
		if (this->cmiss_region)
			cmzn_region_FE_region_change(this->cmiss_region);
	}
}

/*
Module functions
----------------
*/

FE_region::FE_region(struct MANAGER(FE_basis) *basisManagerIn,
		struct LIST(FE_element_shape) *elementShapeListIn) :
	cmiss_region(0),
	fe_time(CREATE(FE_time_sequence_package)()),
	fe_field_list(CREATE(LIST(FE_field))()),
	fe_field_info(0),
	basis_manager(basisManagerIn ? basisManagerIn : CREATE(MANAGER(FE_basis))()),
	ownsBasisManager(basisManagerIn ? false : true),
	element_shape_list(elementShapeListIn ? elementShapeListIn : CREATE(LIST(FE_element_shape))()),
	ownsElementShapeList(elementShapeListIn ? false : true),
	change_level(0),
	fe_field_changes(0),
	informed_make_cmiss_number_field(false),
	informed_make_xi_field(false),
	access_count(1)
{
	for (int n = 0; n < 2; ++n)
		this->nodesets[n] = FE_nodeset::create(this);
	this->nodesets[0]->setFieldDomainType(CMZN_FIELD_DOMAIN_TYPE_NODES);
	this->nodesets[1]->setFieldDomainType(CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);

	for (int dimension = 1; dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
		this->meshes[dimension - 1] = FE_mesh::create(this, dimension);
	for (int dimension = 2; dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
		this->meshes[dimension - 1]->setFaceMesh(this->meshes[dimension - 2]);
	for (int dimension = 1; dimension < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
		this->meshes[dimension - 1]->setParentMesh(this->meshes[dimension]);
	this->createChangeLogs();
}

FE_region::~FE_region()
{
	if (0 != this->access_count)
		display_message(ERROR_MESSAGE, "~FE_region.  Non-zero access count");
	if (0 != this->change_level)
		display_message(WARNING_MESSAGE, "~FE_region.  Non-zero change_level %d", this->change_level);

	for (int n = 0; n < 2; ++n)
	{
		this->nodesets[n]->detach_from_FE_region();
		FE_nodeset::deaccess(this->nodesets[n]);
	}

	for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; 0 < dimension; --dimension)
		FE_mesh::deaccess(this->meshes[dimension - 1]);

	if (this->fe_field_info)
	{
		/* remove its pointer to this fe_region because being destroyed */
		FE_field_info_clear_FE_region(this->fe_field_info);
		DEACCESS(FE_field_info)(&(this->fe_field_info));
	}
	if (this->ownsBasisManager)
		DESTROY(MANAGER(FE_basis))(&this->basis_manager);
	if (this->ownsElementShapeList)
		DESTROY(LIST(FE_element_shape))(&this->element_shape_list);
	DESTROY(LIST(FE_field))(&(this->fe_field_list));
	DESTROY(FE_time_sequence_package)(&(this->fe_time));

	DESTROY(CHANGE_LOG(FE_field))(&(this->fe_field_changes));
}

/**
 * Creates and initializes the change logs in FE_region.
 * Centralised so they are created and recreated consistently.
 */
void FE_region::createChangeLogs()
{
	this->fe_field_changes = CREATE(CHANGE_LOG(FE_field))(
		this->fe_field_list, /*max_changes*/-1);

	for (int n = 0; n < 2; ++n)
		this->nodesets[n]->createChangeLog();

	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		this->meshes[dim]->createChangeLog();
}

/*
Private functions
-----------------
*/

struct FE_field_info *FE_region_get_FE_field_info(struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns a struct FE_field_info for <fe_region>.
This is an object private to FE_region that is common between all fields
owned by FE_region. FE_fields access this object, but this object maintains
a non-ACCESSed pointer to <fe_region> so fields can determine which FE_region
they belong to.
==============================================================================*/
{
	struct FE_field_info *fe_field_info = 0;
	if (fe_region)
	{
		if (!fe_region->fe_field_info)
		{
			fe_region->fe_field_info =
				ACCESS(FE_field_info)(CREATE(FE_field_info)(fe_region));
		}
		fe_field_info = fe_region->fe_field_info;
	}
	return (fe_field_info);
}

/*
Global functions
----------------
*/

struct FE_region *FE_region_create(struct MANAGER(FE_basis) *basis_manager,
	struct LIST(FE_element_shape) *element_shape_list)
{
	return new FE_region(basis_manager, element_shape_list);
}

/**
 * Frees the memory for the FE_region and sets <*fe_region_address> to NULL.
 */
static int DESTROY(FE_region)(struct FE_region **fe_region_address)
{
	if (fe_region_address)
	{
		delete *fe_region_address;
		fe_region_address = 0;
		return 1;
	}
	return 0;
}

DECLARE_OBJECT_FUNCTIONS(FE_region)

int FE_region_begin_change(struct FE_region *fe_region)
{
	if (fe_region)
	{
		++(fe_region->change_level);
		return 1;
	}
	return 0;
}

int FE_region_end_change(struct FE_region *fe_region)
{
	if (fe_region)
	{
		if (0 < fe_region->change_level)
		{
			--(fe_region->change_level);
			fe_region->update();
			return 1;
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_region_end_change.  Change not enabled");
		}
	}
	return 0;
}

int FE_region_end_change_no_notify(struct FE_region *fe_region)
{
	if (fe_region)
	{
		--(fe_region->change_level);
		return 1;
	}
	return 0;
}

bool FE_field_has_cached_changes(FE_field *fe_field)
{
	FE_region *fe_region;
	if (fe_field && (fe_region = FE_field_get_FE_region(fe_field)) && (fe_region->change_level))
	{
		int change = 0;
		CHANGE_LOG_QUERY(FE_field)(fe_region->fe_field_changes, fe_field, &change);
		if (change & (CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field) | 
			CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)))
			return true;
	}
	return false;
}

int FE_region_clear(struct FE_region *fe_region)
// This could be made faster.
{
	int return_code;
	struct FE_field *fe_field;

	ENTER(FE_region_clear);
	if (fe_region)
	{
		return_code = 1;
		FE_region_begin_change(fe_region);
		for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		{
			fe_region->meshes[dim]->clear();
		}
		for (int n = 0; n < 2; ++n)
		{
			fe_region->nodesets[n]->clear();
		}

		if (fe_region->fe_field_list)
		{
			while (return_code && (fe_field = FIRST_OBJECT_IN_LIST_THAT(FE_field)(
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL, (void *)NULL,
				fe_region->fe_field_list)))
			{
				return_code = FE_region_remove_FE_field(fe_region, fe_field);
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "FE_region_clear.  Failed");
			return_code = 0;
		}
		FE_region_end_change(fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "FE_region_clear.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct FE_field *FE_region_get_FE_field_with_general_properties(
	struct FE_region *fe_region, const char *name, enum Value_type value_type,
	int number_of_components)
{
	struct FE_field *fe_field;

	ENTER(FE_region_get_FE_field_with_general_properties);
	fe_field = (struct FE_field *)NULL;
	if (fe_region && name && (0 < number_of_components))
	{
		fe_field = FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(name,
			fe_region->fe_field_list);
		if (fe_field)
		{
			if ((get_FE_field_FE_field_type(fe_field) != GENERAL_FE_FIELD) ||
				(get_FE_field_value_type(fe_field) != value_type) ||
				(get_FE_field_number_of_components(fe_field) != number_of_components))
			{
				fe_field = (struct FE_field *)NULL;
			}
		}
		else
		{
			fe_field = CREATE(FE_field)(name, fe_region);
			if (!(set_FE_field_value_type(fe_field, value_type) &&
				set_FE_field_number_of_components(fe_field, number_of_components) &&
				set_FE_field_type_general(fe_field) &&
				FE_region_merge_FE_field(fe_region, fe_field)))
			{
				DESTROY(FE_field)(&fe_field);
				fe_field = (struct FE_field *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_FE_field_with_general_properties.  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field);
}

struct FE_field *FE_region_get_FE_field_with_properties(
	struct FE_region *fe_region, const char *name, enum FE_field_type fe_field_type,
	struct FE_field *indexer_field, int number_of_indexed_values,
	enum CM_field_type cm_field_type, struct Coordinate_system *coordinate_system,
	enum Value_type value_type, int number_of_components, char **component_names,
	int number_of_times, enum Value_type time_value_type,
	struct FE_field_external_information *external)
{
	char *component_name;
	int i;
	struct FE_field *fe_field;

	ENTER(FE_region_get_FE_field_with_properties);
	fe_field = (struct FE_field *)NULL;
	if (fe_region && name && coordinate_system && (0 < number_of_components))
	{
		/* search the FE_region for a field of that name */
		fe_field = FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(name,
			fe_region->fe_field_list);
		if (fe_field)
		{
			/* make sure the field matches in every way */
			if (!FE_field_matches_description(fe_field, name, fe_field_type,
				indexer_field, number_of_indexed_values, cm_field_type,
				coordinate_system, value_type, number_of_components, component_names,
				number_of_times, time_value_type, external))
			{
				display_message(ERROR_MESSAGE,
					"FE_region_get_FE_field_with_properties.  "
					"Inconsistent with field of same name in region");
				fe_field = (struct FE_field *)NULL;
			}
		}
		else
		{
			fe_field = CREATE(FE_field)(name, fe_region);
			if (fe_field &&
				set_FE_field_external_information(fe_field, external) &&
				set_FE_field_value_type(fe_field, value_type) &&
				set_FE_field_number_of_components(fe_field, number_of_components) &&
				((CONSTANT_FE_FIELD != fe_field_type) ||
					set_FE_field_type_constant(fe_field)) &&
				((GENERAL_FE_FIELD != fe_field_type) ||
					set_FE_field_type_general(fe_field)) &&
				((INDEXED_FE_FIELD != fe_field_type) ||
					set_FE_field_type_indexed(fe_field,
						indexer_field,number_of_indexed_values)) &&
				set_FE_field_CM_field_type(fe_field, cm_field_type) &&
				set_FE_field_coordinate_system(fe_field, coordinate_system) &&
				set_FE_field_time_value_type(fe_field, time_value_type) &&
				set_FE_field_number_of_times(fe_field, number_of_times))
			{
				if (component_names)
				{
					for (i = 0; (i < number_of_components) && fe_field; i++)
					{
						component_name = component_names[i];
						if (component_name)
						{
							if (!set_FE_field_component_name(fe_field, i, component_name))
							{
								DESTROY(FE_field)(&fe_field);
								fe_field = (struct FE_field *)NULL;
							}
						}
					}
				}
			}
			else
			{
				DESTROY(FE_field)(&fe_field);
				fe_field = (struct FE_field *)NULL;
			}
			if (fe_field)
			{
				if (!FE_region_merge_FE_field(fe_region, fe_field))
				{
					DESTROY(FE_field)(&fe_field);
					fe_field = (struct FE_field *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_region_get_FE_field_with_properties.  "
					"Could not create new field");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_FE_field_with_properties.  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field);
}

struct FE_field *FE_region_merge_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field)
{
	struct FE_field *merged_fe_field = 0;
	if (fe_region && fe_field)
	{
		if (FE_field_get_FE_region(fe_field) == fe_region)
		{
			merged_fe_field = FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(
				get_FE_field_name(fe_field), fe_region->fe_field_list);
			if (merged_fe_field)
			{
				/* no change needs to be noted if fields are exactly the same */
				if (!FE_fields_match_exact(merged_fe_field, fe_field))
				{
					/* can only change fundamentals -- number of components, value type
						 if merged_fe_field is not accessed by any other objects */
					if ((1 == FE_field_get_access_count(merged_fe_field)) ||
						FE_fields_match_fundamental(merged_fe_field, fe_field))
					{
						if (FE_field_copy_without_identifier(merged_fe_field, fe_field))
						{
#if defined (DEBUG_CODE)
					/*???debug*/printf("FE_region_merge_FE_field: %p OBJECT_NOT_IDENTIFIER_CHANGED field %p\n",fe_region,merged_fe_field);
#endif /* defined (DEBUG_CODE) */
							fe_region->FE_field_change(merged_fe_field, CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field));
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"FE_region_merge_FE_field.  Could not modify field");
							merged_fe_field = (struct FE_field *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "FE_region_merge_FE_field.  "
							"Existing field named %s is different",
							get_FE_field_name(merged_fe_field));
						merged_fe_field = (struct FE_field *)NULL;
					}
				}
			}
			else
			{
				if (ADD_OBJECT_TO_LIST(FE_field)(fe_field, fe_region->fe_field_list))
				{
					merged_fe_field = fe_field;
#if defined (DEBUG_CODE)
					/*???debug*/printf("FE_region_merge_FE_field: %p ADD field %p\n",fe_region,merged_fe_field);
#endif /* defined (DEBUG_CODE) */
					fe_region->FE_field_change(merged_fe_field, CHANGE_LOG_OBJECT_ADDED(FE_field));
				}
				else
				{
					display_message(ERROR_MESSAGE, "FE_region_merge_FE_field.  "
						"Could not add field %s", get_FE_field_name(fe_field));
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_region_merge_FE_field.  "
					"Field '%s' is not compatible with this finite element region",
				get_FE_field_name(fe_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_merge_FE_field.  Invalid argument(s)");
	}
	return (merged_fe_field);
}

bool FE_region_is_FE_field_in_use(struct FE_region *fe_region,
	struct FE_field *fe_field)
{
	if (fe_region && fe_field)
	{
		if (IS_OBJECT_IN_LIST(FE_field)(fe_field, fe_region->fe_field_list))
		{
			for (int n = 0; n < 2; ++n)
				if (fe_region->nodesets[n]->is_FE_field_in_use(fe_field))
					return true;
			for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
				if (fe_region->meshes[dim]->is_FE_field_in_use(fe_field))
					return true;
		}
		else
		{
			struct FE_region *referenced_fe_region = FE_field_get_FE_region(fe_field);
			if ((referenced_fe_region != NULL) && (referenced_fe_region != fe_region))
			{
				char *field_name;
				GET_NAME(FE_field)(fe_field, &field_name);
				display_message(ERROR_MESSAGE,
					"FE_region_is_FE_field_in_use.  Field %s is from another region",
					field_name);
				DEALLOCATE(field_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_is_FE_field_in_use.  Invalid argument(s)");
	}
	return false;
}

int FE_region_remove_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Removes <fe_field> from <fe_region>.
Fields can only be removed if not defined on any nodes and element in
<fe_region>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_remove_FE_field);
	return_code = 0;
	if (fe_region && fe_field)
	{
		if (IS_OBJECT_IN_LIST(FE_field)(fe_field, fe_region->fe_field_list))
		{
			if (FE_region_is_FE_field_in_use(fe_region, fe_field))
			{
				display_message(ERROR_MESSAGE,
					"FE_region_remove_FE_field.  Field is in use in region");
				/*???RC Could undefine it at nodes and elements */
			}
			else
			{
				/* access field in case it is only accessed here */
				ACCESS(FE_field)(fe_field);
				return_code = REMOVE_OBJECT_FROM_LIST(FE_field)(fe_field,
					fe_region->fe_field_list);
				if (return_code)
					fe_region->FE_field_change(fe_field, CHANGE_LOG_OBJECT_REMOVED(FE_field));
				DEACCESS(FE_field)(&fe_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_region_remove_FE_field.  Field %p is not in region %p",
				fe_field, fe_region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_remove_FE_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_region_remove_FE_field */

struct CHANGE_LOG(FE_field) *FE_region_get_FE_field_changes(struct FE_region *fe_region)
{
	if (fe_region)
		return fe_region->fe_field_changes;
	return 0;
}

struct FE_field *FE_region_get_FE_field_from_name(struct FE_region *fe_region,
	const char *field_name)
{
	if (fe_region && field_name)
		return FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(field_name, fe_region->fe_field_list);
	return 0;
}

int FE_region_set_FE_field_name(struct FE_region *fe_region,
	struct FE_field *field, const char *new_name)
{
	int return_code;

	ENTER(FE_region_set_FE_field_name);
	if (fe_region && field && new_name)
	{
		if (FE_region_contains_FE_field(fe_region, field))
		{
			return_code = 1;
			if (FE_region_get_FE_field_from_name(fe_region, new_name))
			{
				display_message(ERROR_MESSAGE, "FE_region_set_FE_field_name.  "
					"Field named \"%s\" already exists in this FE_region.", new_name);
				return_code = 0;
			}
			else
			{
				// this temporarily removes the object from all indexed lists
				if (LIST_BEGIN_IDENTIFIER_CHANGE(FE_field,name)(
					fe_region->fe_field_list, field))
				{
					return_code = set_FE_field_name(field, new_name);
					LIST_END_IDENTIFIER_CHANGE(FE_field,name)(fe_region->fe_field_list);
					if (return_code)
						fe_region->FE_field_change(field, CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field));
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_region_set_FE_field_name.  "
						"Could not safely change identifier in indexed lists");
					return_code = 0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_region_set_FE_field_name.  Field is not from this region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_set_FE_field_name.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

bool FE_region_contains_FE_field(struct FE_region *fe_region,
	struct FE_field *field)
{
	if (fe_region)
		return (0 != IS_OBJECT_IN_LIST(FE_field)(field, fe_region->fe_field_list));
	return false;
}

struct FE_field *FE_region_get_first_FE_field_that(struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_field) *conditional_function,
	void *user_data_void)
{
	if (fe_region)
		return FIRST_OBJECT_IN_LIST_THAT(FE_field)(conditional_function,
			user_data_void, fe_region->fe_field_list);
	return 0;
}

int FE_region_for_each_FE_field(struct FE_region *fe_region,
	LIST_ITERATOR_FUNCTION(FE_field) iterator_function, void *user_data)
{
	if (fe_region && iterator_function)
		return FOR_EACH_OBJECT_IN_LIST(FE_field)(iterator_function,
			user_data, fe_region->fe_field_list);
	return 0;
}

int FE_region_get_number_of_FE_fields(struct FE_region *fe_region)
{
	if (fe_region)
		return NUMBER_IN_LIST(FE_field)(fe_region->fe_field_list);
	return 0;
}

int FE_region_FE_field_has_multiple_times(struct FE_region *fe_region,
	struct FE_field *fe_field)
{
	if (fe_region && fe_field)
	{
		// currently only node fields can be time aware
		for (int n = 0; n < 2; ++n)
		{
			if (fe_region->nodesets[n]->FE_field_has_multiple_times(fe_field))
				return 1;
		}
	}
	return 0;
}

struct FE_field *FE_region_get_default_coordinate_FE_field(struct FE_region *fe_region)
{
	if (fe_region)
		return FE_region_get_first_FE_field_that(fe_region, FE_field_is_coordinate_field, (void *)NULL);
	return 0;
}

struct MANAGER(FE_basis) *FE_region_get_basis_manager(
	struct FE_region *fe_region)
{
	if (fe_region)
		return fe_region->basis_manager;
	return 0;
}

struct LIST(FE_field) *FE_region_get_FE_field_list(struct FE_region *fe_region)
{
	if (fe_region)
		return fe_region->fe_field_list;
	return 0;
}

struct LIST(FE_element_shape) *FE_region_get_FE_element_shape_list(
	struct FE_region *fe_region)
{
	if (fe_region)
		return fe_region->element_shape_list;
	return 0;
}

struct FE_element *FE_region_get_top_level_FE_element_from_identifier(
	struct FE_region *fe_region, int identifier)
{
	if (fe_region && (0 <= identifier))
	{
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; 1 <= dimension; --dimension)
		{
			FE_element *element = fe_region->meshes[dimension - 1]->findElementByIdentifier(identifier);
			if (element && FE_element_is_top_level(element, (void *)NULL))
				return element;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_top_level_FE_element_from_identifier.  Invalid argument(s)");
	}
	return 0;
}

FE_nodeset *FE_region_find_FE_nodeset_by_field_domain_type(
	struct FE_region *fe_region, enum cmzn_field_domain_type domain_type)
{
	if (fe_region)
	{
		if (domain_type == CMZN_FIELD_DOMAIN_TYPE_NODES)
			return fe_region->nodesets[0];
		else if (domain_type == CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS)
			return fe_region->nodesets[1];
	}
	return 0;
}

FE_mesh *FE_region_find_FE_mesh_by_dimension(
	struct FE_region *fe_region, int dimension)
{
	if (fe_region && (0 < dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
		return fe_region->meshes[dimension - 1];
	return 0;
}

int FE_region_get_number_of_FE_elements_all_dimensions(struct FE_region *fe_region)
{
	int number_of_elements = 0;

	ENTER(FE_region_get_number_of_FE_elements_all_dimensions);
	if (fe_region)
	{
		for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		{
			number_of_elements += fe_region->meshes[dim]->get_number_of_FE_elements();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_number_of_FE_elements_all_dimensions.  Invalid argument(s)");
	}
	LEAVE;

	return (number_of_elements);
}

int FE_region_get_number_of_FE_elements_of_dimension(
	struct FE_region *fe_region, int dimension)
{
	if (fe_region && (1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		return fe_region->meshes[dimension - 1]->get_number_of_FE_elements();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_number_of_FE_elements_of_dimension.  Invalid argument(s)");
	}
	return 0;
}

int FE_region_get_highest_dimension(struct FE_region *fe_region)
{
	int highest_dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS;
	while (highest_dimension &&
		(fe_region->meshes[highest_dimension - 1]->get_number_of_FE_elements() == 0))
	{
		--highest_dimension;
	}
	return highest_dimension;
}

int FE_region_begin_define_faces(struct FE_region *fe_region)
{
	if (fe_region)
	{
		for (int dimension = 1; dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
		{
			int result = fe_region->meshes[dimension - 1]->begin_define_faces();
			if (result != CMZN_OK)
			{
				if (result == CMZN_ERROR_ALREADY_EXISTS)
					break;
				FE_region_end_define_faces(fe_region);
				return 0;
			}
		}
		return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "FE_region_begin_define_faces.  Invalid argument(s)");
	}
	return 0;
}

int FE_region_end_define_faces(struct FE_region *fe_region)
{
	if (fe_region)
	{
		for (int dimension = 1; dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
			fe_region->meshes[dimension - 1]->end_define_faces();
		return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "FE_region_end_define_faces.  Invalid argument(s)");
	}
	return 0;
}

int FE_region_define_faces(struct FE_region *fe_region)
{
	int return_code = 1;
	if (fe_region)
	{
		FE_region_begin_change(fe_region);
		FE_region_begin_define_faces(fe_region);
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; 2 <= dimension; --dimension)
		{
			if (CMZN_OK != fe_region->meshes[dimension - 1]->define_faces())
			{
				return_code = 0;
				break;
			}
		}
		FE_region_end_define_faces(fe_region);
		FE_region_end_change(fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_define_faces.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

struct FE_element *FE_region_get_first_FE_element_that(
	struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function,
	void *user_data_void)
{
	if (fe_region)
	{
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (1 <= dimension); --dimension)
		{
			FE_element *element = fe_region->meshes[dimension - 1]->get_first_FE_element_that(conditional_function, user_data_void);
			if (element)
				return element;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_first_FE_element_that.  Invalid argument(s)");
	}
	return 0;
}

int FE_region_for_each_FE_element(struct FE_region *fe_region,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function, void *user_data)
{
	if (fe_region && iterator_function)
	{
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; 1 <= dimension; --dimension)
		{
			if (!fe_region->meshes[dimension - 1]->for_each_FE_element(iterator_function, user_data))
				return 0;
		}
		return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_for_each_FE_element.  Invalid argument(s)");
	}
	return 0;
}

int FE_region_smooth_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field, FE_value time)
{
	int return_code = 1;
	if (fe_region && fe_field)
	{
		if (IS_OBJECT_IN_LIST(FE_field)(fe_field, fe_region->fe_field_list))
		{
			// use highest dimension non-empty element list
			int dimension = FE_region_get_highest_dimension(fe_region);
			if (dimension)
			{
				FE_region_begin_change(fe_region);
				/* create a field to store an integer value per component of fe_field */
				FE_field *element_count_fe_field =
					CREATE(FE_field)("cmzn_smooth_element_count", fe_region);
				if (!(set_FE_field_number_of_components(element_count_fe_field,
							get_FE_field_number_of_components(fe_field)) &&
						set_FE_field_value_type(element_count_fe_field, INT_VALUE)))
					return_code = 0;
				ACCESS(FE_field)(element_count_fe_field);
				FE_mesh *fe_mesh = fe_region->meshes[dimension - 1];
				LIST(FE_node) *copy_node_list = CREATE(LIST(FE_node))();

				cmzn_elementiterator *elementIter = fe_mesh->createElementiterator();
				if (!((element_count_fe_field) && (copy_node_list) && (elementIter))) 
					return_code = 0;
				FE_element *element;
				while (0 != (element = cmzn_elementiterator_next_non_access(elementIter)))
				{
					/* skip elements without field defined appropriately */
					if (FE_element_field_is_standard_node_based(element, fe_field))
					{
						if (FE_element_smooth_FE_field(element, fe_field, time, element_count_fe_field, copy_node_list))
						{
							fe_mesh->elementFieldChange(element, fe_field);
						}
						else
						{
							return_code = 0;
							break;
						}
					}
				}
				cmzn_elementiterator_destroy(&elementIter);

				FE_nodeset *fe_nodeset = FE_region_find_FE_nodeset_by_field_domain_type(fe_region,
					CMZN_FIELD_DOMAIN_TYPE_NODES);
				cmzn_nodeiterator *nodeIter = CREATE_LIST_ITERATOR(FE_node)(copy_node_list);
				cmzn_node *node = 0;
				while ((0 != (node = cmzn_nodeiterator_next_non_access(nodeIter))))
				{
					FE_node_smooth_FE_field(node, fe_field, time, element_count_fe_field);
					fe_nodeset->merge_FE_node(node);
				}
				cmzn_nodeiterator_destroy(&nodeIter);

				DESTROY(LIST(FE_node))(&copy_node_list);
				DEACCESS(FE_field)(&element_count_fe_field);

				FE_region_end_change(fe_region);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_region_smooth_FE_field.  FE_field is not from this region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_smooth_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

struct FE_time_sequence *FE_region_get_FE_time_sequence_matching_series(
	struct FE_region *fe_region, int number_of_times, const FE_value *times)
{
	if (fe_region)
		return get_FE_time_sequence_matching_time_series(fe_region->fe_time, number_of_times, times);
	return 0;
}

struct FE_time_sequence *FE_region_get_FE_time_sequence_merging_two_time_series(
	struct FE_region *fe_region, struct FE_time_sequence *time_sequence_one,
	struct FE_time_sequence *time_sequence_two)
{
	if (fe_region)
		return get_FE_time_sequence_merging_two_time_series(fe_region->fe_time,
			time_sequence_one, time_sequence_two);
	return 0;
}

struct FE_basis *FE_region_get_FE_basis_matching_basis_type(
	struct FE_region *fe_region, int *basis_type)
{
	if (fe_region && basis_type)
		return make_FE_basis(basis_type, fe_region->basis_manager);
	return 0;
}

void FE_region_set_cmzn_region_private(struct FE_region *fe_region,
	struct cmzn_region *cmiss_region)
{
	if (fe_region)
		fe_region->cmiss_region = cmiss_region;
}

struct cmzn_region *FE_region_get_cmzn_region(struct FE_region *fe_region)
{
	if (fe_region)
		return fe_region->cmiss_region;
	return 0;
}

/**
 * FE_field iterator version of FE_region_merge_FE_field.
 * Assumed to be called from FE_region_merge since transfers ownership of new
 * field so source region is consumed / made unusable in the process. 
 */
static int FE_field_merge_into_FE_region(struct FE_field *fe_field,
	void *fe_region_void)
{
	int return_code = 1;
	FE_region *fe_region = reinterpret_cast<struct FE_region *>(fe_region_void);
	if (fe_field && fe_region)
	{
		FE_field *indexer_fe_field = 0;
		/* if the field is indexed, the indexer field needs to be merged first,
			 and the merged indexer field substituted */
		if (INDEXED_FE_FIELD == get_FE_field_FE_field_type(fe_field))
		{
			int number_of_indexed_values;
			if (get_FE_field_type_indexed(fe_field,
				&indexer_fe_field, &number_of_indexed_values))
			{
				char *indexer_fe_field_name;
				if (GET_NAME(FE_field)(indexer_fe_field, &indexer_fe_field_name))
				{
					if (FE_field_merge_into_FE_region(indexer_fe_field, fe_region_void))
					{
						/* get the merged indexer field */
						if (!(indexer_fe_field = FE_region_get_FE_field_from_name(fe_region,
							indexer_fe_field_name)))
						{
							return_code = 0;
						}
					}
					else
					{
						return_code = 0;
					}
					DEALLOCATE(indexer_fe_field_name);
				}
				else
				{
					return_code = 0;
				}
			}
			else
			{
				return_code = 0;
			}
		}
		if (return_code)
		{
			/* change fe_field to belong to <fe_region>;
				 substitute the indexer field if required */
			if (!(FE_field_set_FE_field_info(fe_field,
				FE_region_get_FE_field_info(fe_region)) &&
				((!indexer_fe_field) ||
					FE_field_set_indexer_field(fe_field, indexer_fe_field)) &&
				FE_region_merge_FE_field(fe_region, fe_field)))
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_merge_into_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/** @return  1 on success, 0 on failure. */
int FE_field_check_element_node_value_labels(struct FE_field *field,
	void *target_fe_region_void)
{
	FE_region *fe_region = FE_field_get_FE_region(field);
	FE_region *target_fe_region = static_cast<FE_region *>(target_fe_region_void);
	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
	{
		if (!fe_region->meshes[dim]->check_field_element_node_value_labels(field, target_fe_region))
			return 0;
	}
	return 1;
}

bool FE_region_can_merge(struct FE_region *target_fe_region,
	struct FE_region *source_fe_region)
{
	if (!source_fe_region)
		return false;

	if (target_fe_region)
	{
		// check fields of the same name have compatible definitions
		if (!FOR_EACH_OBJECT_IN_LIST(FE_field)(FE_field_can_be_merged_into_list,
			(void *)target_fe_region->fe_field_list, source_fe_region->fe_field_list))
		{
			display_message(ERROR_MESSAGE,
				"Cannot merge field(s) into region due to incompatible definition");
			return false;
		}

		// check nodes (nodesets)
		for (int n = 0; n < 2; ++n)
		{
			if (!target_fe_region->nodesets[n]->canMerge(*(source_fe_region->nodesets[n])))
				return false;
		}
	}

	// check/convert finite element field parameter mappings from indexes to derivatives & versions
	// conversion must happen with or without a target_fe_region
	if (!FOR_EACH_OBJECT_IN_LIST(FE_field)(FE_field_check_element_node_value_labels,
		(void *)target_fe_region, source_fe_region->fe_field_list))
	{
		display_message(ERROR_MESSAGE,
			"Cannot merge field(s) into region as cannot migrate element field mapping indexes to derivatives/versions");
		return false;
	}

	if (target_fe_region)
	{
		// check elements match in shape and faces
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (0 < dimension); --dimension)
		{
			FE_mesh *source_fe_mesh = source_fe_region->meshes[dimension - 1];
			FE_mesh *target_fe_mesh = target_fe_region->meshes[dimension - 1];
			if (!target_fe_mesh->canMerge(*source_fe_mesh))
				return false;
		}
	}

	return true;
}

int FE_region_merge(struct FE_region *target_fe_region,
	struct FE_region *source_fe_region)
{
	int return_code = 1;
	if (target_fe_region && source_fe_region)
	{
		FE_region_begin_change(target_fe_region);

		// merge fields
		if (!FOR_EACH_OBJECT_IN_LIST(FE_field)(
			FE_field_merge_into_FE_region, (void *)target_fe_region,
			source_fe_region->fe_field_list))
		{
			display_message(ERROR_MESSAGE, "FE_region_merge.  Could not merge fields");
			return_code = 0;
		}

		// merge nodes (nodesets)
		if (return_code)
		{
			for (int n = 0; n < 2; ++n)
				if (!target_fe_region->nodesets[n]->merge(*(source_fe_region->nodesets[n])))
					return_code = 0;
		}

		// merge elements (meshes)
		if (return_code)
		{
			// merge meshes from lowest to highest dimension so faces are merged before parent
			for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
				if (!target_fe_region->meshes[dim]->merge(*(source_fe_region->meshes[dim])))
					return_code = 0;
		}

		FE_region_end_change(target_fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_merge.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

bool FE_region_need_add_cmiss_number_field(struct FE_region *fe_region)
{
	if (fe_region && (!fe_region->informed_make_cmiss_number_field))
	{
		for (int n = 0; n < 2; ++n)
			if (fe_region->nodesets[n]->get_number_of_FE_nodes())
			{
				fe_region->informed_make_cmiss_number_field = true;
				return true;
			}
		if (FE_region_get_number_of_FE_elements_all_dimensions(fe_region))
		{
			fe_region->informed_make_cmiss_number_field = true;
			return true;
		}
	}
	return false;
}

bool FE_region_need_add_xi_field(struct FE_region *fe_region)
{
	if (fe_region && (!fe_region->informed_make_xi_field) &&
		FE_region_get_number_of_FE_elements_all_dimensions(fe_region))
	{
		fe_region->informed_make_xi_field = true;
		return true;
	}
	return false;
}

void FE_region_list_btree_statistics(struct FE_region *fe_region)
{
	if (fe_region)
	{
		for (int n = 0; n < 2; ++n)
			fe_region->nodesets[n]->list_btree_statistics();
		for (int dimension = 1; dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
			fe_region->meshes[dimension - 1]->list_btree_statistics();
	}
}
