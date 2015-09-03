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

FE_nodeset::FE_nodeset(FE_region *fe_region) :
	fe_region(fe_region),
	domainType(CMZN_FIELD_DOMAIN_TYPE_INVALID),
	nodeList(CREATE(LIST(FE_node)())),
	node_field_info_list(CREATE(LIST(FE_node_field_info))()),
	last_fe_node_field_info(0),
	fe_node_changes(0),
	next_fe_node_identifier_cache(0),
	access_count(1)
{
}

FE_nodeset::~FE_nodeset()
{
	DESTROY(CHANGE_LOG(FE_node))(&this->fe_node_changes);
	this->last_fe_node_field_info = 0;
	DESTROY(LIST(FE_node))(&(this->nodeList));
	FOR_EACH_OBJECT_IN_LIST(FE_node_field_info)(
		FE_node_field_info_clear_FE_nodeset, (void *)NULL,
		this->node_field_info_list);
	DESTROY(LIST(FE_node_field_info))(&(this->node_field_info_list));
}

void FE_nodeset::createChangeLog()
{
	if (this->fe_node_changes)
		DESTROY(CHANGE_LOG(FE_node))(&this->fe_node_changes);
	this->fe_node_changes = CREATE(CHANGE_LOG(FE_node))(this->nodeList, /*max_changes*/2000);
	this->last_fe_node_field_info = 0;
}

struct CHANGE_LOG(FE_node) *FE_nodeset::extractChangeLog()
{
	struct CHANGE_LOG(FE_node) *changes = this->fe_node_changes;
	this->fe_node_changes = 0;
	this->createChangeLog();
	return changes;
}

bool FE_nodeset::containsNode(FE_node *node)
{
	return (0 != IS_OBJECT_IN_LIST(FE_node)(node, this->nodeList));
}

struct LIST(FE_node) *FE_nodeset::createRelatedNodeList()
{
	return CREATE_RELATED_LIST(FE_node)(this->nodeList);
}

/**
 * Returns a struct FE_node_field_info for the supplied <fe_node_field_list>
 * with <number_of_values>. The fe_nodeset maintains an internal list of these
 * structures so they can be shared between nodes.
 * If <node_field_list> is omitted, an empty list is assumed.
 * @return  Accessed FE_node_field_info, or 0 on error.
 */
struct FE_node_field_info *FE_nodeset::get_FE_node_field_info(
	int number_of_values, struct LIST(FE_node_field) *fe_node_field_list)
{
	int existing_number_of_values;
	struct FE_node_field_info *fe_node_field_info = 0;
	struct FE_node_field_info *existing_fe_node_field_info;
	if (fe_node_field_list)
	{
		existing_fe_node_field_info =
			FIRST_OBJECT_IN_LIST_THAT(FE_node_field_info)(
				FE_node_field_info_has_matching_FE_node_field_list,
				(void *)fe_node_field_list, this->node_field_info_list);
	}
	else
	{
		existing_fe_node_field_info =
			FIRST_OBJECT_IN_LIST_THAT(FE_node_field_info)(
				FE_node_field_info_has_empty_FE_node_field_list, (void *)NULL,
				this->node_field_info_list);
	}
	if (existing_fe_node_field_info)
	{
		existing_number_of_values =
			FE_node_field_info_get_number_of_values(existing_fe_node_field_info);
		if (number_of_values == existing_number_of_values)
		{
			fe_node_field_info = ACCESS(FE_node_field_info)(existing_fe_node_field_info);
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::get_FE_node_field_info.  "
				"Existing node field information has %d values, not %d requested",
				existing_number_of_values, number_of_values);
		}
	}
	else
	{
		fe_node_field_info = CREATE(FE_node_field_info)(this,
			fe_node_field_list, number_of_values);
		if (fe_node_field_info)
		{
			if (!ADD_OBJECT_TO_LIST(FE_node_field_info)(fe_node_field_info,
				this->node_field_info_list))
			{
				display_message(ERROR_MESSAGE,
					"FE_nodeset::get_FE_node_field_info.  Could not add to FE_region");
				DEACCESS(FE_node_field_info)(&fe_node_field_info);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::get_FE_node_field_info.  "
				"Could not create node field information");
		}
	}
	return (fe_node_field_info);
}

/**
 * Updates the pointer to <node_field_info_address> to point to a node_field info
 * which appends to the fields in <node_field_info_address> one <new_node_field>.
 * The node_field_info returned in <node_field_info_address> will be for the
 * <new_number_of_values>.
 * The nodeset maintains an internal list of these structures so they can be
 * shared between nodes.  This function allows a fast path when adding a single
 * field.  If the node_field passed in is only referenced by one external object
 * then it is assumed that this function can modify it rather than copying.  If
 * there are more references then the object is copied and then modified.
 * This function handles the access and deaccess of the <node_field_info_address>
 * as if it is just updating then there is nothing to do.
 */
int FE_nodeset::get_FE_node_field_info_adding_new_field(
	struct FE_node_field_info **node_field_info_address,
	struct FE_node_field *new_node_field, int new_number_of_values)
{
	int return_code;
	struct FE_node_field_info *existing_node_field_info, *new_node_field_info;

	if (node_field_info_address &&
		(NULL != (existing_node_field_info = *node_field_info_address)))
	{
		return_code = 1;
		if (FE_node_field_info_used_only_once(existing_node_field_info))
		{
			FE_node_field_info_add_node_field(existing_node_field_info,
				new_node_field, new_number_of_values);
			new_node_field_info =
				FIRST_OBJECT_IN_LIST_THAT(FE_node_field_info)(
					FE_node_field_info_has_matching_FE_node_field_list,
					(void *)FE_node_field_info_get_node_field_list(existing_node_field_info),
					this->node_field_info_list);
			if (new_node_field_info)
			{
				REACCESS(FE_node_field_info)(node_field_info_address,
					new_node_field_info);
			}
		}
		else
		{
			/* Need to copy after all */
			struct LIST(FE_node_field) *node_field_list = CREATE_LIST(FE_node_field)();
			if (COPY_LIST(FE_node_field)(node_field_list,
					FE_node_field_info_get_node_field_list(existing_node_field_info)))
			{
				/* add the new node field */
				if (ADD_OBJECT_TO_LIST(FE_node_field)(new_node_field, node_field_list))
				{
					/* create the new node information */
					new_node_field_info = this->get_FE_node_field_info(new_number_of_values, node_field_list);
					if (new_node_field_info)
					{
						if (*node_field_info_address)
							DEACCESS(FE_node_field_info)(node_field_info_address);
						*node_field_info_address = new_node_field_info;
					}
				}
			}
			DESTROY(LIST(FE_node_field))(&node_field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::get_FE_node_field_info_adding_new_field.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/**
 * Returns a clone of <fe_node_field_info> that belongs to nodeset and
 * uses equivalent FE_fields and FE_time_sequences from <fe_region>.
 * Used to merge nodes from other FE_regions into this nodeset.
 * Returned object has an incremented access count that the caller takes over.
 * It is an error if an equivalent/same name FE_field is not found in nodeset.
 */
struct FE_node_field_info *FE_nodeset::clone_FE_node_field_info(
	struct FE_node_field_info *fe_node_field_info)
{
	struct FE_node_field_info *clone_fe_node_field_info = 0;
	if (fe_node_field_info)
	{
		struct LIST(FE_node_field) *fe_node_field_list = FE_node_field_list_clone_with_FE_field_list(
			FE_node_field_info_get_node_field_list(fe_node_field_info),
			fe_region->fe_field_list, fe_region->fe_time);
		if (fe_node_field_list)
		{
			clone_fe_node_field_info = this->get_FE_node_field_info(
				FE_node_field_info_get_number_of_values(fe_node_field_info),
				fe_node_field_list);
			DESTROY(LIST(FE_node_field))(&fe_node_field_list);
		}
		if (!clone_fe_node_field_info)
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset::clone_FE_node_field_info.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::clone_FE_node_field_info.  Invalid argument(s)");
	}
	return (clone_fe_node_field_info);
}

/** 
 * Provided EXCLUSIVELY for the use of DEACCESS and REACCESS functions.
 * Called when the access_count of <fe_node_field_info> drops to 1 so that
 * <fe_region> can destroy FE_node_field_info not in use.
 */
int FE_nodeset::remove_FE_node_field_info(struct FE_node_field_info *fe_node_field_info)
{
	return REMOVE_OBJECT_FROM_LIST(FE_node_field_info)(
		fe_node_field_info, this->node_field_info_list);
}

void FE_nodeset::detach_from_FE_region()
{
	FE_region_begin_change(this->fe_region);
	// clear embedded locations to avoid circular dependencies which prevent cleanup
	FE_nodeset_clear_embedded_locations(this, this->fe_region->fe_field_list);
	FE_region_end_change(this->fe_region);
	this->fe_region = 0;
}

/**
 * If <fe_region> has clients for its change messages, records the <change> to
 * <node>, and if the <field_info_node> has different node_field_info from the
 * last, logs the FE_fields in it as changed and remembers it as the new
 * last_fe_node_field_info. If the cache_level is zero, sends an update.
 * Must supply both <node> and <field_info_node>.
 * When a node is added or removed, the same node is used for <node> and
 * <field_info_node>. For changes to the contents of <node>, <field_info_node>
 * should contain the changed fields, consistent with merging it into <node>.
 */
void FE_nodeset::nodeChange(FE_node *node, CHANGE_LOG_CHANGE(FE_node) change, FE_node *field_info_node)
{
	if (this->fe_region)
	{
		CHANGE_LOG_OBJECT_CHANGE(FE_node)(this->fe_node_changes, node, change);
		struct FE_node_field_info *temp_fe_node_field_info =
			FE_node_get_FE_node_field_info(field_info_node);
		if (temp_fe_node_field_info != this->last_fe_node_field_info)
		{
			FE_node_field_info_log_FE_field_changes(temp_fe_node_field_info,
				fe_region->fe_field_changes);
			this->last_fe_node_field_info = temp_fe_node_field_info;
		}
		this->fe_region->update();
	}
}

/**
 * Use this instead of nodeChange when only the identifier has changed.
 */
void FE_nodeset::nodeIdentifierChange(FE_node *node)
{
	if (this->fe_region)
	{
		this->next_fe_node_identifier_cache = 0;
		CHANGE_LOG_OBJECT_CHANGE(FE_node)(this->fe_node_changes, node,
			CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_node));
		this->fe_region->update();
	}
}

/**
 * Use this macro instead of nodeChange when exactly one field, <fe_field> of
 * <node> has changed.
 */
void FE_nodeset::nodeFieldChange(FE_node *node, FE_field *fe_field)
{
	if (this->fe_region)
	{
		CHANGE_LOG_OBJECT_CHANGE(FE_node)(this->fe_node_changes, node,
			CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_node));
		CHANGE_LOG_OBJECT_CHANGE(FE_field)(this->fe_region->fe_field_changes,
			fe_field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
		this->fe_region->update();
	}
}

void FE_nodeset::nodeRemovedChange(FE_node *node)
{
	if (this->fe_region)
	{
		this->next_fe_node_identifier_cache = 0;
		CHANGE_LOG_OBJECT_CHANGE(FE_node)(this->fe_node_changes, node,
			CHANGE_LOG_OBJECT_REMOVED(FE_node));
		struct FE_node_field_info *temp_fe_node_field_info =
			FE_node_get_FE_node_field_info(node);
		if (temp_fe_node_field_info != this->last_fe_node_field_info)
		{
			FE_node_field_info_log_FE_field_changes(temp_fe_node_field_info,
				fe_region->fe_field_changes);
			this->last_fe_node_field_info = temp_fe_node_field_info;
		}
		this->fe_region->update();
	}
}

FE_region_changes::FE_region_changes(struct FE_region *fe_region) :
	access_count(1)
{
	this->fe_field_changes = fe_region->fe_field_changes;
	for (int n = 0; n < 2; ++n)
		this->fe_node_changes[n] = fe_region->nodesets[n]->extractChangeLog();
	// when extracting element change logs, propagate field change summary flags
	// from nodes and parent elements
	int nodeChanges;
	CHANGE_LOG_GET_CHANGE_SUMMARY(FE_node)(this->fe_node_changes[0], &nodeChanges);
	bool parentChange = 0 != (nodeChanges & CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_node));
	bool parentAllChange = CHANGE_LOG_IS_ALL_CHANGE(FE_node)(this->fe_node_changes[0]);
	for (int dim = MAXIMUM_ELEMENT_XI_DIMENSIONS - 1; 0 <= dim; --dim)
	{
		this->fe_element_changes[dim] = fe_region->meshes[dim]->extractChangeLog();
		if (parentChange)
		{
			if (parentAllChange)
				CHANGE_LOG_ALL_CHANGE(FE_element)(this->fe_element_changes[dim], CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_element));
			else
				CHANGE_LOG_PROPAGATE_PARENT_CHANGE_SUMMARY(FE_element)(this->fe_element_changes[dim]);
		}
		else
		{
			int elementChanges;
			CHANGE_LOG_GET_CHANGE_SUMMARY(FE_element)(this->fe_element_changes[dim], &elementChanges);
			parentChange = 0 != (elementChanges & CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_element));
		}
		if (parentChange && !parentAllChange)
			parentAllChange = CHANGE_LOG_IS_ALL_CHANGE(FE_element)(this->fe_element_changes[dim]);
	}
	fe_region->createChangeLogs();
}

FE_region_changes::~FE_region_changes()
{
	for (int n = 0; n < 2; ++n)
		DESTROY(CHANGE_LOG(FE_node))(&(this->fe_node_changes[n]));
	DESTROY(CHANGE_LOG(FE_field))(&(this->fe_field_changes));
	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
	{
		DESTROY(CHANGE_LOG(FE_element))(&(this->fe_element_changes[dim]));
	}
}

bool FE_region_changes::elementOrParentChanged(FE_element *element)
{
	return (0 != FE_element_or_parent_changed(element, this->fe_element_changes, this->fe_node_changes[0]));
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

	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		FE_mesh::deaccess(this->meshes[dim]);

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

bool FE_nodeset::is_FE_field_in_use(struct FE_field *fe_field)
{
	if (FIRST_OBJECT_IN_LIST_THAT(FE_node_field_info)(
		FE_node_field_info_has_FE_field, (void *)fe_field,
		this->node_field_info_list))
	{
		/* since nodes may still exist in the change_log or slave FE_regions,
				must now check that no remaining nodes use fe_field */
		/*???RC For now, if there are nodes then fe_field is in use */
		if (0 < NUMBER_IN_LIST(FE_node)(this->nodeList))
			return true;
	}
	return false;
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

// NOTE! Only to be called by FE_region_clear
// Expects change cache to be on otherwise very inefficient
void FE_nodeset::clear()
{
	cmzn_nodeiterator *iter = CREATE_LIST_ITERATOR(FE_node)(this->nodeList);
	cmzn_node *node = 0;
	while ((0 != (node = cmzn_nodeiterator_next_non_access(iter))))
	{
		this->nodeRemovedChange(node);
	}
	cmzn_nodeiterator_destroy(&iter);
	REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)((LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,
		(void *)NULL, this->nodeList);
}

int FE_nodeset::change_FE_node_identifier(struct FE_node *node, int new_identifier)
{
	if (node && (new_identifier >= 0))
	{
		if (IS_OBJECT_IN_LIST(FE_node)(node, this->nodeList))
		{
			if (this->findNodeByIdentifier(new_identifier))
			{
				display_message(ERROR_MESSAGE,
					"FE_nodeset::change_FE_node_identifier.  "
					"Node with new identifier already exists");
				return CMZN_ERROR_ALREADY_EXISTS;
			}
			// this temporarily removes the object from all indexed lists
			if (LIST_BEGIN_IDENTIFIER_CHANGE(FE_node,cm_node_identifier)(
				this->nodeList, node))
			{
				int return_code = set_FE_node_identifier(node, new_identifier);
				LIST_END_IDENTIFIER_CHANGE(FE_node,cm_node_identifier)(
					this->nodeList);
				if (return_code)
				{
					this->nodeIdentifierChange(node);
					return CMZN_OK;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_nodeset::change_FE_node_identifier.  "
					"Could not safely change identifier in indexed lists");
			}
			return CMZN_ERROR_GENERAL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset::change_FE_node_identifier.  Node is not in this nodeset");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::change_FE_node_identifier.  Invalid argument(s)");
	}
	return CMZN_ERROR_ARGUMENT;
}

/**
 * Convenience function returning an existing node with <identifier> from
 * nodeset. If none is found, a new node with the given <identifier> is created.
 * If the returned node is not already in <fe_region> it is merged before return.
 * It is expected that the calling function has wrapped calls to this function
 * with FE_region_begin/end_change.
 */
FE_node *FE_nodeset::get_or_create_FE_node_with_identifier(int identifier)
{
	FE_node *node = FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(
		identifier, this->nodeList);
	if (node)
		return node;
	node = CREATE(FE_node)(identifier, this, /*template_node*/static_cast<FE_node *>(0));
	if (node && this->merge_FE_node(node))
		return node;
	DEACCESS(FE_node)(&node);
	return 0;
}

int FE_nodeset::get_next_FE_node_identifier(int start_identifier)
{
	int identifier = (start_identifier <= 0) ? 1 : start_identifier;
	if (this->next_fe_node_identifier_cache)
	{
		if (this->next_fe_node_identifier_cache > identifier)
		{
			identifier = this->next_fe_node_identifier_cache;
		}
	}
	while (FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(identifier,
		this->nodeList))
	{
		++identifier;
	}
	if (start_identifier < 2)
	{
		/* Don't cache the value if we didn't start at the beginning */
		this->next_fe_node_identifier_cache = identifier;
	}
	return identifier;
}

/**
 * Makes sure <fe_field> is not defined at any nodes in <fe_node_list> from
 * the nodeset, unless that field at the node is in interpolated over an element
 * from <fe_region>.
 * Should wrap call to this function between begin_change/end_change.
 * <fe_node_list> is unchanged by this function.
 * On return, the integer at <number_in_elements_address> will be set to the
 * number of nodes for which <fe_field> could not be undefined because they are
 * used by element fields of <fe_field>.
 */
int FE_nodeset::undefine_FE_field_in_FE_node_list(
	struct FE_field *fe_field, struct LIST(FE_node) *fe_node_list,
	int *number_in_elements_address)
{
	if (fe_field && fe_node_list && number_in_elements_address)
	{
		/* first make sure all the nodes are from this nodeset */
		if (FIRST_OBJECT_IN_LIST_THAT(FE_node)(FE_node_is_not_in_list,
			(void *)this->nodeList, fe_node_list))
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset::undefine_FE_field_in_FE_node_list.  "
				"Some nodes are not from this nodeset");
			return CMZN_ERROR_ARGUMENT;
		}
		int return_code = CMZN_OK;
		/* work with a copy of the node list */
		struct LIST(FE_node) *tmp_fe_node_list = CREATE(LIST(FE_node))();
		if (COPY_LIST(FE_node)(tmp_fe_node_list, fe_node_list))
		{
			FE_region_begin_change(this->fe_region);
			/* remove nodes for which fe_field is not defined */
			REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
				FE_node_field_is_not_defined, (void *)fe_field, tmp_fe_node_list);
			*number_in_elements_address = NUMBER_IN_LIST(FE_node)(tmp_fe_node_list);
			/* remove nodes used in element fields for fe_field */
			struct Node_list_field_data node_list_field_data;
			node_list_field_data.fe_field = fe_field;
			node_list_field_data.fe_node_list = tmp_fe_node_list;
			if (FE_region_for_each_FE_element(this->fe_region,
				FE_element_ensure_FE_field_nodes_are_not_in_list,
				(void *)&node_list_field_data))
			{
				*number_in_elements_address -= NUMBER_IN_LIST(FE_node)(tmp_fe_node_list);
				struct FE_node *node;
				while ((NULL != (node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
					(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL,
					tmp_fe_node_list))))
				{
					if (undefine_FE_field_at_node(node, fe_field))
					{
						REMOVE_OBJECT_FROM_LIST(FE_node)(node, tmp_fe_node_list);
						this->nodeFieldChange(node, fe_field);
					}
					else
					{
						return_code = CMZN_ERROR_GENERAL;
						break;
					}
				}
			}
			else
			{
				return_code = CMZN_ERROR_GENERAL;
			}
			FE_region_end_change(fe_region);
		}
		else
		{
			return_code = CMZN_ERROR_MEMORY;
		}
		DESTROY(LIST(FE_node))(&tmp_fe_node_list);
		return return_code;
	}
	return CMZN_ERROR_ARGUMENT;
}

struct FE_node *FE_nodeset::create_FE_node_copy(int identifier, struct FE_node *source)
{
	FE_node *new_node = 0;
	if (source && (-1 <= identifier))
	{
		if (FE_node_get_FE_nodeset(source) == this)
		{
			int number = (identifier < 0) ? this->get_next_FE_node_identifier(0) : identifier;
			new_node = CREATE(FE_node)(number, (FE_nodeset *)0, source);
			if (ADD_OBJECT_TO_LIST(FE_node)(new_node, this->nodeList))
			{
				this->nodeChange(new_node, CHANGE_LOG_OBJECT_ADDED(FE_node), new_node); 
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_nodeset::create_FE_node_copy.  node identifier in use.");
				DESTROY(FE_node)(&new_node);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::create_FE_node_copy.  "
				"Source node is incompatible with region");
		}
	}
	return new_node;
}

/**
 * Checks <node> is compatible with this nodeset and any existing FE_node
 * using the same identifier, then merges it in.
 * If no FE_node of the same identifier exists in FE_region, <node> is added
 * to nodeset and returned by this function, otherwise changes are merged into
 * the existing FE_node and it is returned.
 * During the merge, any new fields from <node> are added to the existing node of
 * the same identifier. Where those fields are already defined on the existing
 * node, the existing structure is maintained, but the new values are added from
 * <node>. Fails if fields are not consistent in numbers of versions and
 * derivatives etc.
 * A NULL value is returned on any error.
 */
struct FE_node *FE_nodeset::merge_FE_node(struct FE_node *node)
{
	struct FE_node *merged_node = 0;
	if (node)
	{
		if (FE_node_get_FE_nodeset(node) == this)
		{
			merged_node = FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(
				get_FE_node_identifier(node), this->nodeList);
			if (merged_node)
			{
				if (merged_node != node)
				{
					if (::merge_FE_node(merged_node, node))
					{
						this->nodeChange(merged_node, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_node), node); 
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"FE_nodeset::merge_FE_node.  Could not merge node %d", get_FE_node_identifier(merged_node));
						merged_node = (struct FE_node *)NULL;
					}
				}
			}
			else
			{
				if (ADD_OBJECT_TO_LIST(FE_node)(node, this->nodeList))
				{
					merged_node = node;
					this->nodeChange(merged_node, CHANGE_LOG_OBJECT_ADDED(FE_node), merged_node); 
				}
				else
				{
					display_message(ERROR_MESSAGE, "FE_nodeset::merge_FE_node.  Could not add node %d",
						get_FE_node_identifier(merged_node));
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::merge_FE_node.  "
				"Node %d is not of this nodeset", get_FE_node_identifier(merged_node));
		}
	}
	return merged_node;
}

int FE_nodeset::merge_FE_node_existing(struct FE_node *destination, struct FE_node *source)
{
	if (destination && source)
	{
		if ((FE_node_get_FE_nodeset(destination) == this) &&
			(FE_node_get_FE_nodeset(source) == this))
		{
			if (::merge_FE_node(destination, source))
			{
				this->nodeChange(destination, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_node), source); 
				return CMZN_OK;
			}
			return CMZN_ERROR_GENERAL;
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::merge_FE_node_existing.  "
				"Source and/or destination nodes are not from nodeset");
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

/**
 * Calls <iterator_function> with <user_data> for each FE_node in <region>.
 */
int FE_nodeset::for_each_FE_node(LIST_ITERATOR_FUNCTION(FE_node) iterator_function, void *user_data)
{
	return FOR_EACH_OBJECT_IN_LIST(FE_node)(iterator_function, user_data, this->nodeList);
}

cmzn_nodeiterator_id FE_nodeset::createNodeiterator()
{
	return CREATE_LIST_ITERATOR(FE_node)(this->nodeList);
}

/**
 * Removes <node> from the nodeset.
 * Nodes can only be removed if not in use by elements in <fe_region>.
 * Note it is more efficient to use FE_nodeset::remove_FE_node_list for more than
 * one node.
 */
int FE_nodeset::remove_FE_node(struct FE_node *node)
{
	int return_code = CMZN_ERROR_ARGUMENT;
	if (IS_OBJECT_IN_LIST(cmzn_node)(node, this->nodeList))
	{
		LIST(cmzn_node) *removeNodeList = CREATE_RELATED_LIST(cmzn_node)(this->nodeList);
		if (ADD_OBJECT_TO_LIST(cmzn_node)(node, removeNodeList))
		{
			return_code = this->remove_FE_node_list(removeNodeList);
			if (return_code != CMZN_OK)
			{
				display_message(ERROR_MESSAGE,
					"FE_nodeset::remove_FE_node.  Node is in use by elements in region");
			}
		}
		else
			return_code = CMZN_ERROR_GENERAL;
		DESTROY(LIST(FE_node))(&removeNodeList);
	}
	return return_code;
}

/**
 * Attempts to removes all the nodes in <node_list> from this nodeset.
 * Nodes can only be removed if not in use by elements in <fe_region>.
 * On return, <node_list> will contain all the nodes that are still in
 * <fe_region> after the call.
 * A successful return code is only obtained if all nodes from <node_list> are
 * removed.
 */
int FE_nodeset::remove_FE_node_list(struct LIST(FE_node) *node_list)
{
	int return_code = CMZN_ERROR_ARGUMENT;
	if (node_list)
	{
		return_code = CMZN_OK;
		LIST(FE_node) *exclusion_node_list = CREATE(LIST(FE_node))();
		// since we do not maintain pointers from nodes to elements using them,
		// must iterate over all elements to remove nodes referenced by them
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (0 < dimension); --dimension)
		{
			cmzn_elementiterator *iter = fe_region->meshes[dimension - 1]->createElementiterator();
			cmzn_element *element;
			while (0 != (element = cmzn_elementiterator_next_non_access(iter)) && (CMZN_OK == return_code))
				return_code = cmzn_element_add_stored_nodes_to_list(element, exclusion_node_list, /*onlyFrom*/node_list);
			cmzn_elementiterator_destroy(&iter);
		}
		if (CMZN_OK == return_code)
		{
			/* begin/end change to prevent multiple messages */
			FE_region_begin_change(fe_region);
			cmzn_nodeiterator *iter = CREATE_LIST_ITERATOR(cmzn_node)(node_list);
			cmzn_node *node = 0;
			while ((0 != (node = cmzn_nodeiterator_next_non_access(iter))))
			{
				if (IS_OBJECT_IN_LIST(cmzn_node)(node, exclusion_node_list))
					continue;
				if (REMOVE_OBJECT_FROM_LIST(cmzn_node)(node, this->nodeList))
					this->nodeRemovedChange(node);
				else
				{
					return_code = CMZN_ERROR_GENERAL;
					break;
				}
			}
			cmzn_nodeiterator_destroy(&iter);
			REMOVE_OBJECTS_FROM_LIST_THAT(cmzn_node)(FE_node_is_not_in_list,
				(void *)(this->nodeList), node_list);
			if (0 < NUMBER_IN_LIST(cmzn_node)(node_list))
				return_code = CMZN_ERROR_GENERAL;
			FE_region_end_change(fe_region);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset::remove_FE_node_list.  Could not exclude nodes in elements");
		}
		DESTROY(LIST(FE_node))(&exclusion_node_list);
	}
	return return_code;
}

int FE_nodeset::get_last_FE_node_identifier()
{
	cmzn_nodeiterator *iter = this->createNodeiterator();
	cmzn_node *node = 0;
	cmzn_node *last_node = 0;
	while ((0 != (node = cmzn_nodeiterator_next_non_access(iter))))
	{
		last_node = node;
	}
	cmzn_nodeiterator_destroy(&iter);
	if (last_node)
		return get_FE_node_identifier(node);
	return 0;
}

/**
 * Returns the number of nodes in the nodeset
 */
int FE_nodeset::get_number_of_FE_nodes()
{
	return NUMBER_IN_LIST(FE_node)(this->nodeList);
}

bool FE_nodeset::FE_field_has_multiple_times(struct FE_field *fe_field)
{
	if (FIRST_OBJECT_IN_LIST_THAT(FE_node_field_info)(
		FE_node_field_info_has_FE_field_with_multiple_times,
		(void *)fe_field, this->node_field_info_list))
		return true;
	return false;
}

void FE_nodeset::list_btree_statistics()
{
	if (this->domainType == CMZN_FIELD_DOMAIN_TYPE_NODES)
		display_message(INFORMATION_MESSAGE, "Nodes:\n");
	else if (this->domainType == CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS)
		display_message(INFORMATION_MESSAGE, "Datapoints:\n");
	else
		display_message(INFORMATION_MESSAGE, "General nodeset:\n");
	FE_node_list_write_btree_statistics(this->nodeList);
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

struct FE_region_smooth_FE_element_data
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Data for FE_region_smooth_FE_element.
==============================================================================*/
{
	FE_value time;
	struct FE_field *fe_field;
	struct FE_field *element_count_fe_field;
	FE_mesh *fe_mesh;
	struct LIST(FE_node) *copy_node_list;
};

static int FE_region_smooth_FE_element(struct FE_element *element,
	void *smooth_element_data_void)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Creates a copy of element, calls FE_element_smooth_FE_field for it, then
merges it back into its FE_region.
Place between begin/end_change calls when used for a series of elements.
Must call FE_region_smooth_FE_node for copy_node_list afterwards.
<smooth_element_data_void> points at a struct FE_region_smooth_FE_element_data.
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm;
	struct FE_element *copy_element;
	struct FE_region_smooth_FE_element_data *smooth_element_data;

	ENTER(FE_region_smooth_FE_element);
	if (element && (smooth_element_data =
		(struct FE_region_smooth_FE_element_data *)smooth_element_data_void))
	{
		return_code = 1;
		/* skip elements without field defined appropriately */
		if (FE_element_field_is_standard_node_based(element,
			smooth_element_data->fe_field))
		{
			/*???RC Should make a copy of element with JUST fe_field - would result
				in a more precise change message and faster response */
			if (get_FE_element_identifier(element, &cm) &&
				(copy_element = CREATE(FE_element)(&cm,
					(struct FE_element_shape *)NULL, (FE_mesh *)NULL, element)))
			{
				ACCESS(FE_element)(copy_element);
				if (FE_element_smooth_FE_field(copy_element,
					smooth_element_data->fe_field,
					smooth_element_data->time,
					smooth_element_data->element_count_fe_field,
					smooth_element_data->copy_node_list))
				{
					if (!smooth_element_data->fe_mesh->merge_FE_element(copy_element))
					{
						return_code = 0;
					}
				}
				else
				{
					return_code = 0;
				}
				DEACCESS(FE_element)(&copy_element);
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_smooth_FE_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_smooth_FE_element */

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
				struct FE_region_smooth_FE_element_data smooth_element_data;
				smooth_element_data.time = time;
				smooth_element_data.fe_field = fe_field;
				/* create a field to store an integer value per component of fe_field */
				smooth_element_data.element_count_fe_field =
					CREATE(FE_field)("cmzn_smooth_element_count", fe_region);
				set_FE_field_number_of_components(
					smooth_element_data.element_count_fe_field,
					get_FE_field_number_of_components(fe_field));
				set_FE_field_value_type(smooth_element_data.element_count_fe_field,
					INT_VALUE);
				ACCESS(FE_field)(smooth_element_data.element_count_fe_field);
				smooth_element_data.fe_mesh = fe_region->meshes[dimension - 1];
				smooth_element_data.copy_node_list = CREATE(LIST(FE_node))();
				if (!smooth_element_data.fe_mesh->for_each_FE_element(FE_region_smooth_FE_element,
					(void *)&smooth_element_data))
				{
					display_message(ERROR_MESSAGE,
						"FE_region_smooth_FE_field.  Error smoothing elements");
					return_code = 0;
				}

				FE_nodeset *fe_nodeset = FE_region_find_FE_nodeset_by_field_domain_type(fe_region,
					CMZN_FIELD_DOMAIN_TYPE_NODES);
				cmzn_nodeiterator *iter = CREATE_LIST_ITERATOR(FE_node)(smooth_element_data.copy_node_list);
				cmzn_node *node = 0;
				while ((0 != (node = cmzn_nodeiterator_next_non_access(iter))))
				{
					FE_node_smooth_FE_field(node, fe_field,
						time, smooth_element_data.element_count_fe_field);
					fe_nodeset->merge_FE_node(node);
				}
				cmzn_nodeiterator_destroy(&iter);

				DESTROY(LIST(FE_node))(&smooth_element_data.copy_node_list);
				DEACCESS(FE_field)(&smooth_element_data.element_count_fe_field);

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

/***************************************************************************//**
 * When a region containing objects with embedded element_xi values is merged
 * into a global region, the elements -- often of unspecified shape -- need to
 * be converted into their global namesakes.
 * This structure contains global fe_regions from which those elements may
 * come.
 */
struct FE_regions_merge_embedding_data
{
	struct FE_region *target_root_fe_region;
	struct FE_region *target_fe_region;
	struct FE_region *source_current_fe_region;
};

/**
 * Returns the global element equivalent for <element> using the information in
 * the <embedding_data>.
 * Note: currently only supports embedding in local or root FE_region.
 * ???GRC: still determining how to use embedding in another region.
 */
static struct FE_element *FE_regions_merge_embedding_data_get_global_element(
	struct FE_regions_merge_embedding_data *embedding_data,
	struct FE_element *element)
{
	struct FE_element *global_element;
	if (embedding_data)
	{
		FE_region *source_fe_region = FE_element_get_FE_region(element);
		FE_region *global_fe_region = embedding_data->target_root_fe_region;
		if (source_fe_region == embedding_data->source_current_fe_region)
		{
			global_fe_region = embedding_data->target_fe_region;
		}
		int dimension = cmzn_element_get_dimension(element);
		FE_mesh *global_fe_mesh = global_fe_region->meshes[dimension - 1];
		int identifier = cmzn_element_get_identifier(element);
		struct FE_element_shape *element_shape = 0;
		get_FE_element_shape(element, &element_shape);
		global_element = global_fe_mesh->get_or_create_FE_element_with_identifier(
			identifier, element_shape);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_regions_merge_embedding_data_get_global_element.  "
			"Invalid argument(s)");
		global_element = (struct FE_element *)NULL;
	}
	return (global_element);
}

struct FE_node_merge_into_FE_nodeset_data
/*******************************************************************************
LAST MODIFIED : 29 November 2002

DESCRIPTION :
Data for passing to FE_node_merge_into_FE_nodeset.
==============================================================================*/
{
	struct FE_region *fe_region;
	FE_nodeset *fe_nodeset;
	/* use following array and number to build up matching pairs of old node
		 field info what they become in the global_fe_region.
		 Note these are ACCESSed */
	struct FE_node_field_info **matching_node_field_info;
	int number_of_matching_node_field_info;
	/* data for merging embedded fields */
	struct FE_regions_merge_embedding_data *embedding_data;
	/* array of element_xi fields that may be defined on node prior to its being
		 merged; these require substitution of global element pointers */
	int number_of_embedded_fields;
	struct FE_field **embedded_fields;
};

/***************************************************************************//**
 * If <field> is embedded, ie. returns element_xi, adds it to the embedded_field
 * array in the FE_node_merge_into_FE_nodeset_data.
 * @param merge_nodes_data_void  A struct FE_node_merge_into_FE_nodeset_data *.
 */
static int FE_field_add_embedded_field_to_array(struct FE_field *field,
	void *merge_nodes_data_void)
{
	int return_code;
	struct FE_field **embedded_fields;
	struct FE_node_merge_into_FE_nodeset_data *merge_nodes_data;

	ENTER(FE_field_add_embedded_field_to_array);
	if (field && (merge_nodes_data =
		(struct FE_node_merge_into_FE_nodeset_data *)merge_nodes_data_void))
	{
		return_code = 1;
		if (ELEMENT_XI_VALUE == get_FE_field_value_type(field))
		{
			if (REALLOCATE(embedded_fields, merge_nodes_data->embedded_fields,
				struct FE_field *, (merge_nodes_data->number_of_embedded_fields + 1)))
			{
				/* must be mapped to a global field */
				embedded_fields[merge_nodes_data->number_of_embedded_fields] =
					FE_region_merge_FE_field(merge_nodes_data->fe_region, field);
				merge_nodes_data->embedded_fields = embedded_fields;
				merge_nodes_data->number_of_embedded_fields++;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_field_add_embedded_field_to_array.  Invalid argument(s)");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_add_embedded_field_to_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_add_embedded_field_to_array */

static int FE_node_merge_into_FE_nodeset(struct FE_node *node,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 29 November 2002

DESCRIPTION :
FE_node iterator version of FE_region_merge_FE_node. Before merging, substitutes
into <node> an appropriate node field info from <fe_region>.
<data_void> points at a struct FE_node_merge_into_FE_nodeset_data.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_types;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int component_number, i, number_of_components, number_of_derivatives,
		number_of_versions, return_code, value_number, version_number;
	struct FE_element *element, *global_element;
	struct FE_field *field;
	struct FE_node_field_info *current_node_field_info;
	struct FE_node_merge_into_FE_nodeset_data *data;
	//struct FE_region *fe_region;
	FE_nodeset *fe_nodeset;

	ENTER(FE_node_merge_into_FE_nodeset);
	if (node &&
		(0 != (current_node_field_info = FE_node_get_FE_node_field_info(node))) &&
		(data = (struct FE_node_merge_into_FE_nodeset_data *)data_void) &&
		(fe_nodeset = data->fe_nodeset))
	{
		/* 1. Convert node to use a new FE_node_field_info from this FE_region */
		/* fast path: check if the node_field_info has already been assimilated */
		struct FE_node_field_info **matching_node_field_info = data->matching_node_field_info;
		struct FE_node_field_info *node_field_info = 0;
		for (i = 0; (i < data->number_of_matching_node_field_info); i++)
		{
			if (*matching_node_field_info == current_node_field_info)
			{
				node_field_info = ACCESS(FE_node_field_info)(*(matching_node_field_info + 1));
				break;
			}
			matching_node_field_info += 2;
		}
		if (!node_field_info)
		{
			node_field_info = fe_nodeset->clone_FE_node_field_info(current_node_field_info);
			if (node_field_info)
			{
				/* store combination of node field info in matching list */
				if (REALLOCATE(matching_node_field_info,
					data->matching_node_field_info, struct FE_node_field_info *,
					2*(data->number_of_matching_node_field_info + 1)))
				{
					matching_node_field_info[data->number_of_matching_node_field_info*2] =
						ACCESS(FE_node_field_info)(current_node_field_info);
					matching_node_field_info
						[data->number_of_matching_node_field_info*2 + 1] =
						ACCESS(FE_node_field_info)(node_field_info);
					data->matching_node_field_info = matching_node_field_info;
					(data->number_of_matching_node_field_info)++;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_merge_into_FE_nodeset.  "
					"Could not clone node_field_info");
			}
		}
		if (node_field_info)
		{
			/* substitute the new node field info */
			FE_node_set_FE_node_field_info(node, node_field_info);
			return_code = 1;
			/* substitute global elements etc. in embedded fields */
			for (i = 0; i < data->number_of_embedded_fields; i++)
			{
				field = data->embedded_fields[i];
				if (FE_field_is_defined_at_node(field, node))
				{
					number_of_components = get_FE_field_number_of_components(field);
					for (component_number = 0; component_number < number_of_components;
						component_number++)
					{
						number_of_versions = get_FE_node_field_component_number_of_versions(
							node,field, component_number);
						number_of_derivatives =
							get_FE_node_field_component_number_of_derivatives(node, field,
								component_number);
						nodal_value_types =	get_FE_node_field_component_nodal_value_types(node, field,
							component_number);
						if (nodal_value_types)
						{
							for (version_number = 0; version_number < number_of_versions;
								version_number++)
							{
								for (value_number = 0; value_number <= number_of_derivatives;
								 value_number++)
								{
									if (get_FE_nodal_element_xi_value(node, field,
										component_number, version_number,
										nodal_value_types[value_number], &element, xi))
									{
										if (element)
										{
											global_element =
												FE_regions_merge_embedding_data_get_global_element(
													data->embedding_data, element);
											if (global_element)
											{
												if (!set_FE_nodal_element_xi_value(node, field,
													component_number, version_number,
													nodal_value_types[value_number], global_element, xi))
												{
													return_code = 0;
												}
											}
											else
											{
												return_code = 0;
											}
										}
									}
									else
									{
										return_code = 0;
									}
								}
							}
							DEALLOCATE(nodal_value_types);
						}
						else
						{
							return_code = 0;
						}
					}
				}
			}
			/* now the actual merge! */
			FE_node *merged_node = fe_nodeset->merge_FE_node(node);
			if (merged_node)
			{
				if (merged_node != node)
				{
					/* restore the old node field info so marked as belonging to original FE_nodeset
					 * @see FE_nodeset_clear_embedded_locations */
					FE_node_set_FE_node_field_info(node, current_node_field_info);
				}
			}
			else
			{
				return_code = 0;
			}
			DEACCESS(FE_node_field_info)(&node_field_info);
		}
		else
		{
			return_code = 0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "FE_node_merge_into_FE_nodeset.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_merge_into_FE_nodeset.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

bool FE_nodeset::canMerge(FE_nodeset &source)
{
	bool result = true;
	/* check that definition of fields in nodes of source_fe_region match that
		of equivalent fields in equivalent nodes of global_fe_region */
	/* for efficiency, pairs of FE_node_field_info from the opposing nodes
		are stored if compatible to avoid checks later */
	struct FE_node_can_be_merged_data check_nodes_data;
	check_nodes_data.number_of_compatible_node_field_info = 0;
	/* store in pairs in the single array to reduce allocations */
	check_nodes_data.compatible_node_field_info = (struct FE_node_field_info **)NULL;
	check_nodes_data.node_list = this->nodeList;

	cmzn_nodeiterator *iter = source.createNodeiterator();
	cmzn_node *node;
	while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
	{
		if (!FE_node_can_be_merged(node, (void *)&check_nodes_data))
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset::canMerge.  Nodes are not compatible");
			result = false;
			break;
		}
	}
	cmzn_nodeiterator_destroy(&iter);

	DEALLOCATE(check_nodes_data.compatible_node_field_info);
	return result;
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

int FE_nodeset::merge(FE_nodeset &source, FE_region *target_root_fe_region)
{
	int return_code = 1;
	struct FE_node_merge_into_FE_nodeset_data merge_nodes_data;
	merge_nodes_data.fe_region = this->get_FE_region();
	merge_nodes_data.fe_nodeset = this;
	/* use following array and number to build up matching pairs of old node
		field info what they become in the target_fe_region */
	merge_nodes_data.matching_node_field_info =
		(struct FE_node_field_info **)NULL;
	merge_nodes_data.number_of_matching_node_field_info = 0;
	FE_regions_merge_embedding_data embedding_data =
	{
		target_root_fe_region,
		this->get_FE_region(),
		source.get_FE_region()
	};
	merge_nodes_data.embedding_data = &embedding_data;
	/* work out which fields in source_fe_region are embedded */
	merge_nodes_data.embedded_fields = (struct FE_field **)NULL;
	merge_nodes_data.number_of_embedded_fields = 0;
	if (!FOR_EACH_OBJECT_IN_LIST(FE_field)(
		FE_field_add_embedded_field_to_array, (void *)&merge_nodes_data,
		source.get_FE_region()->fe_field_list))
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::merge.  Could not get embedded fields");
		return_code = 0;
	}

	if (!source.for_each_FE_node(FE_node_merge_into_FE_nodeset, (void *)&merge_nodes_data))
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::merge.  Could not merge nodes");
		return_code = 0;
	}
	if (merge_nodes_data.matching_node_field_info)
	{
		for (int i = 2*merge_nodes_data.number_of_matching_node_field_info - 1; 0 <= i; --i)
			DEACCESS(FE_node_field_info)(&(merge_nodes_data.matching_node_field_info[i]));
		DEALLOCATE(merge_nodes_data.matching_node_field_info);
	}
	if (merge_nodes_data.embedded_fields)
		DEALLOCATE(merge_nodes_data.embedded_fields);
	return return_code;
}

int FE_region_merge(struct FE_region *target_fe_region,
	struct FE_region *source_fe_region, struct FE_region *target_root_fe_region)
{
	int return_code = 1;
	if (target_fe_region && source_fe_region && target_root_fe_region)
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
				if (!target_fe_region->nodesets[n]->merge(*(source_fe_region->nodesets[n]), target_root_fe_region))
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
