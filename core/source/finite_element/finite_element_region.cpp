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

// GRC temporary until CM_element_type removed from FE_element identifier
#define DIMENSION_TO_CM_ELEMENT_TYPE(dimension) \
	((dimension == 3) ? CM_ELEMENT : ((dimension == 2) ? CM_FACE : CM_LINE))
#define CM_ELEMENT_TYPE_TO_DIMENSION(cm_type) \
	((cm_type == CM_ELEMENT) ? 3 : ((cm_type == CM_FACE) ? 2 : 1))

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

struct FE_region
{
	/* pointer to the cmzn_region this FE_region is in: NOT ACCESSED */
	struct cmzn_region *cmiss_region;

	/* field information */
	struct FE_time_sequence_package *fe_time;
	struct LIST(FE_field) *fe_field_list;
	struct FE_field_info *fe_field_info;
	struct LIST(FE_element_field_info) *fe_element_field_info_list;
	struct MANAGER(FE_basis) *basis_manager;
	/* This flag indicates whether this region explictly created the basis manager
		and therefore should destroy it when destroyed.  Would prefer to have an
		access count on the MANAGER but these don't */
	bool ownsBasisManager;
	struct LIST(FE_element_shape) *element_shape_list;
	/* This flag indicates whether this region explictly created the element_shape_list
		and therefore should destroy it when destroyed.  Would prefer to have an
		access count on the LIST(FE_element_shape) but these don't */
	bool ownsElementShapeList;
	/* lists of nodes and elements in this region */
	FE_nodeset *nodesets[2];
	struct LIST(FE_element) *fe_element_list[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	/* change_level: if zero, change messages are sent with every change,
		 otherwise incremented/decremented for nested changes */
	int change_level;
	/* internal record of changes from which FE_field_changes is constructed */
	/* fields added, removed or otherwise changed are put in the following list.
		 Note this records changes to the FE_field structure itself, NOT changes to
		 values and representations of the field in nodes and elements -- these are
		 recorded separately as described below */
	struct CHANGE_LOG(FE_field) *fe_field_changes;
	/* elements added, removed or otherwise changed */
	struct CHANGE_LOG(FE_element) *fe_element_changes[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* keep pointer to last node or element field information so only need to
		 note changed fields when this changes */
	struct FE_element_field_info *last_fe_element_field_info;

	/* information for defining faces */
	/* existence of element_type_node_sequence_list can tell us whether faces
		 are being defined */
	struct LIST(FE_element_type_node_sequence) *element_type_node_sequence_list;
	int define_faces_of_dimension[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	/* Keep a record of where we got to searching for valid identifiers.
		We reset the cache if we delete any items */
	int next_fe_element_identifier_cache[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	/* initially false, these flags are set to true once wrapping computed field
	 * manager is informed that it should create cmiss_number and xi fields
	 * See: FE_region_need_add_cmiss_number_field, FE_region_need_add_xi_field */
	bool informed_make_cmiss_number_field;
	bool informed_make_xi_field;

	// scale factor sets in use. Used as identifier for finding scale factor
	// arrays stored with elements 
	std::vector<cmzn_mesh_scale_factor_set*> scale_factor_sets;

	/* number of objects using this region */
	int access_count;

	FE_region(struct MANAGER(FE_basis) *basis_manager,
		struct LIST(FE_element_shape) *element_shape_list);

	~FE_region();

	struct LIST(FE_element) *get_element_list(int dimension)
	{
		if ((1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
			return this->fe_element_list[dimension - 1];
		return 0;
	}

	std::vector<cmzn_mesh_scale_factor_set*>& get_scale_factor_sets()
	{
		return this->scale_factor_sets;
	}

	void createChangeLogs();

	void update();

	inline void FE_field_change(FE_field *fe_field, enum CHANGE_LOG_CHANGE(FE_field) change)
	{
		CHANGE_LOG_OBJECT_CHANGE(FE_field)(this->fe_field_changes, fe_field,  change);
		this->update();
	}

};

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
		this->fe_element_changes[dim] = fe_region->fe_element_changes[dim];
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

/* Using these macros to conditionally reset the cache based on the
	constant change type in the FE_REGION_FE_ELEMENT_CHANGE macro */
#define FE_ELEMENT_IDENTIFIER_CACHE_UPDATE_CHANGE_LOG_OBJECT_ADDED(type)

#define FE_ELEMENT_IDENTIFIER_CACHE_UPDATE_CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(type)

#define FE_ELEMENT_IDENTIFIER_CACHE_UPDATE_CHANGE_LOG_OBJECT_REMOVED(type)	\
	/* Don't want to add the overhead of working out which type at this point */ \
	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim) \
	{ \
		fe_region->next_fe_element_identifier_cache[dim] = 1; \
	} \

#define FE_REGION_FE_ELEMENT_CHANGE(fe_region, element, change, \
	field_info_element) \
/***************************************************************************** \
LAST MODIFIED : 31 May 2006 \
\
DESCRIPTION : \
If <fe_region> has clients for its change messages, records the <change> to \
<element>, and if the <field_info_element> has different element_field_info \
from the last, logs the FE_fields in it as changed and remembers it as the new \
last_fe_element_field_info. If the cache_level is zero, sends an update. \
Made into a macro for efficency/inlining. \
Must supply both <element> and <field_info_element>. \
When a element is added or removed, the same element is used for <element> and \
<field_info_element>. For changes to the contents of <element>,
<field_info_element> should contain the changed fields, consistent with \
merging it into <element>. \
============================================================================*/ \
{ \
	FE_ELEMENT_IDENTIFIER_CACHE_UPDATE_ ## change \
	int dimension = get_FE_element_dimension(element); \
	if (dimension) \
	{ \
		CHANGE_LOG_OBJECT_CHANGE(FE_element)( \
			fe_region->fe_element_changes[dimension - 1], element, change); \
		FE_element_log_FE_field_changes(field_info_element, \
			fe_region->fe_field_changes, &(fe_region->last_fe_element_field_info)); \
		fe_region->update(); \
	} \
}

#define FE_REGION_FE_ELEMENT_FE_FIELD_LIST_CHANGE(fe_region, element, \
	change, changed_fe_field_list) \
/***************************************************************************** \
LAST MODIFIED : 30 May 2003 \
\
DESCRIPTION : \
If <fe_region> has clients for its change messages, records the <change> to \
<element>, then logs each FE_field in <changed_fe_field_list> as having \
had a RELATED_OBJECT_CHANGED. If the cache_level is zero, sends an update. \
Made into a macro for consistency/efficency/inlining. \
============================================================================*/ \
{ \
	int dimension = get_FE_element_dimension(element); \
	if (dimension) \
	{ \
		CHANGE_LOG_OBJECT_CHANGE(FE_element)( \
			fe_region->fe_element_changes[dimension - 1], element, change); \
		FOR_EACH_OBJECT_IN_LIST(FE_field)(FE_field_log_FE_field_change, \
			(void *)fe_region->fe_field_changes, changed_fe_field_list); \
		fe_region->update(); \
	} \
}

#define FE_REGION_FE_ELEMENT_IDENTIFIER_CHANGE(fe_region, element) \
/***************************************************************************** \
LAST MODIFIED : 16 November 2002 \
\
DESCRIPTION : \
Use this macro instead of FE_REGION_FE_ELEMENT_CHANGE when only the identifier \
has changed. \
============================================================================*/ \
{ \
	/* Don't want to add the overhead of working out which type at this point */ \
	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim) \
	{ \
		fe_region->next_fe_element_identifier_cache[dim] = 1; \
	} \
	int dimension = get_FE_element_dimension(element); \
	if (dimension) \
	{ \
		CHANGE_LOG_OBJECT_CHANGE(FE_element)( \
			fe_region->fe_element_changes[dimension - 1], element, \
			CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_element)); \
		fe_region->update(); \
	} \
}

#define FE_REGION_FE_ELEMENT_FIELD_CHANGE(fe_region, element, fe_field) \
/***************************************************************************** \
LAST MODIFIED : 6 March 2003 \
\
DESCRIPTION : \
Use this macro instead of FE_REGION_FE_ELEMENT_CHANGE when exactly one field,
<fe_field> of <element> has changed. \
============================================================================*/ \
{ \
	int dimension = get_FE_element_dimension(element); \
	if (dimension) \
	{ \
		CHANGE_LOG_OBJECT_CHANGE(FE_element)( \
			fe_region->fe_element_changes[dimension - 1], element, \
			CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_element)); \
		CHANGE_LOG_OBJECT_CHANGE(FE_field)(fe_region->fe_field_changes, \
			fe_field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)); \
		fe_region->update(); \
	} \
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
	fe_element_field_info_list(CREATE(LIST(FE_element_field_info))()),
	basis_manager(basisManagerIn ? basisManagerIn : CREATE(MANAGER(FE_basis))()),
	ownsBasisManager(basisManagerIn ? false : true),
	element_shape_list(elementShapeListIn ? elementShapeListIn : CREATE(LIST(FE_element_shape))()),
	ownsElementShapeList(elementShapeListIn ? false : true),
	change_level(0),
	fe_field_changes(0),
	last_fe_element_field_info(0),
	element_type_node_sequence_list(0),
	informed_make_cmiss_number_field(false),
	informed_make_xi_field(false),
	access_count(1)
{
	for (int n = 0; n < 2; ++n)
		this->nodesets[n] = FE_nodeset::create(this);
	this->nodesets[0]->setFieldDomainType(CMZN_FIELD_DOMAIN_TYPE_NODES);
	this->nodesets[1]->setFieldDomainType(CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);

	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
	{
		this->fe_element_list[dim] = CREATE_LIST(FE_element)();
		this->fe_element_changes[dim] = (struct CHANGE_LOG(FE_element) *)NULL;
		this->next_fe_element_identifier_cache[dim] = 1;
	}
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
		DESTROY(LIST(FE_element))(&(this->fe_element_list[dim]));

	/* remove pointers to this fe_region because being destroyed */
	FOR_EACH_OBJECT_IN_LIST(FE_element_field_info)(
		FE_element_field_info_clear_FE_region, (void *)NULL,
		this->fe_element_field_info_list);
	DESTROY(LIST(FE_element_field_info))(&(this->fe_element_field_info_list));
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
	for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		DESTROY(CHANGE_LOG(FE_element))(&(this->fe_element_changes[dim]));

	const size_t size = this->scale_factor_sets.size();
	for (size_t i = 0; i < size; ++i)
	{
		cmzn_mesh_scale_factor_set *scale_factor_set = this->scale_factor_sets[i];
		cmzn_mesh_scale_factor_set::deaccess(scale_factor_set);
	}
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
	{
		this->fe_element_changes[dim] = CREATE(CHANGE_LOG(FE_element))(
			this->fe_element_list[dim], /*max_changes*/2000);
	}
	this->last_fe_element_field_info = (struct FE_element_field_info *)NULL;
}

int FE_region_merge_FE_element_iterator(struct FE_element *element,
	void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Forward declaration.
==============================================================================*/

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

struct FE_element_field_info *FE_region_get_FE_element_field_info(
	struct FE_region *fe_region,
	struct LIST(FE_element_field) *fe_element_field_list)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Returns a struct FE_element_field_info for the supplied <fe_element_field_list>.
The <fe_region> maintains an internal list of these structures so they can be
shared between elements.
If <element_field_list> is omitted, an empty list is assumed.
==============================================================================*/
{
	struct FE_element_field_info *fe_element_field_info = 0;
	if (fe_region)
	{
		struct FE_element_field_info *existing_fe_element_field_info;
		if (fe_element_field_list)
		{
			existing_fe_element_field_info =
				FIRST_OBJECT_IN_LIST_THAT(FE_element_field_info)(
					FE_element_field_info_has_matching_FE_element_field_list,
					(void *)fe_element_field_list,
					fe_region->fe_element_field_info_list);
		}
		else
		{
			existing_fe_element_field_info =
				FIRST_OBJECT_IN_LIST_THAT(FE_element_field_info)(
					FE_element_field_info_has_empty_FE_element_field_list, (void *)NULL,
					fe_region->fe_element_field_info_list);
		}
		if (existing_fe_element_field_info)
		{
			fe_element_field_info = existing_fe_element_field_info;
		}
		else
		{
			fe_element_field_info =
				CREATE(FE_element_field_info)(fe_region, fe_element_field_list);
			if (fe_element_field_info)
			{
				if (!ADD_OBJECT_TO_LIST(FE_element_field_info)(fe_element_field_info,
					fe_region->fe_element_field_info_list))
				{
					display_message(ERROR_MESSAGE,
						"FE_region_get_FE_element_field_info.  Could not add to FE_region");
					DESTROY(FE_element_field_info)(&fe_element_field_info);
					fe_element_field_info = (struct FE_element_field_info *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "FE_region_get_FE_element_field_info.  "
					"Could not create element field information");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_FE_element_field_info.  Invalid argument(s)");
	}
	return (fe_element_field_info);
}

/**
 * Returns a clone of <fe_element_field_info> that belongs to <fe_region> and
 * uses equivalent FE_fields, FE_time_sequences and scale factor sets from
 * <fe_region>.
 * Used to merge elements from other FE_regions into.
 * It is an error if an equivalent/same name FE_field is not found in <fe_region>.
 */
static struct FE_element_field_info *FE_region_clone_FE_element_field_info(
	struct FE_region *fe_region,
	struct FE_element_field_info *fe_element_field_info)
{
	FE_element_field_info *clone_fe_element_field_info = 0;
	if (fe_region && fe_element_field_info)
	{
		struct LIST(FE_element_field) *fe_element_field_list =
			FE_element_field_list_clone_for_FE_region(
				FE_element_field_info_get_element_field_list(fe_element_field_info),
				fe_region);
		if (fe_element_field_list)
		{
			clone_fe_element_field_info = FE_region_get_FE_element_field_info(
				fe_region, fe_element_field_list);
			DESTROY(LIST(FE_element_field))(&fe_element_field_list);
		}
		if (!clone_fe_element_field_info)
		{
			display_message(ERROR_MESSAGE,
				"FE_region_clone_FE_element_field_info.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_clone_FE_element_field_info.  Invalid argument(s)");
	}
	return (clone_fe_element_field_info);
}

int FE_region_remove_FE_element_field_info(struct FE_region *fe_region,
	struct FE_element_field_info *fe_element_field_info)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Provided EXCLUSIVELY for the use of DEACCESS and REACCESS functions.
Called when the access_count of <fe_element_field_info> drops to 1 so that
<fe_region> can destroy FE_element_field_info not in use.
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_remove_FE_element_field_info);
	if (fe_region && fe_element_field_info)
	{
		return_code = REMOVE_OBJECT_FROM_LIST(FE_element_field_info)(
			fe_element_field_info, fe_region->fe_element_field_info_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_remove_FE_element_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_remove_FE_element_field_info */

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

static int FE_region_remove_FE_element_private(struct FE_region *fe_region,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 7 April 2003

DESCRIPTION :
Removes <element> and all its faces that are not shared with other elements
from <fe_region>. Should enclose call between FE_region_begin_change and
FE_region_end_change to minimise messages.
This function is recursive.
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_remove_FE_element_private);
	int dimension = get_FE_element_dimension(element);
	if (fe_region && element && (1 <= dimension) &&
		(dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		return_code = 1;
		struct LIST(FE_element) *element_list = fe_region->get_element_list(dimension);
		if (IS_OBJECT_IN_LIST(FE_element)(element, element_list))
		{
			/* access element in case it is only accessed here */
			ACCESS(FE_element)(element);
			REMOVE_OBJECT_FROM_LIST(FE_element)(element, element_list);
			/* remove face elements from all their parents;
					mark parent elements as OBJECT_NOT_IDENTIFIER_CHANGED */
			int face_number;
			struct FE_element *face, *parent_element;
			while (return_code && (return_code =
				FE_element_get_first_parent(element, &parent_element, &face_number))
				&& parent_element)
			{
				return_code = set_FE_element_face(parent_element, face_number,
					(struct FE_element *)NULL);
				FE_REGION_FE_ELEMENT_CHANGE(fe_region, parent_element,
					CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_element),
					parent_element);
			}
			/* remove faces used by no elements in this fe_region */
			if (dimension > 1)
			{
				int number_of_faces = 0;
				get_FE_element_number_of_faces(element, &number_of_faces);
				for (int i = 0; (i < number_of_faces) && return_code; i++)
					if (get_FE_element_face(element, i, &face) && face)
					{
						if (!cmzn_element_has_parent_in_list(face, element_list))
							return_code = FE_region_remove_FE_element_private(fe_region, face);
					}
			}
			FE_REGION_FE_ELEMENT_CHANGE(fe_region, element,
				CHANGE_LOG_OBJECT_REMOVED(FE_element), element);
		}
		DEACCESS(FE_element)(&element);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_remove_FE_element_private.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
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
			struct FE_element *element;
			while ((element = FIRST_OBJECT_IN_LIST_THAT(FE_element)(
				(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL, (void *)NULL,
				fe_region->fe_element_list[dim])))
			{
				if (!FE_region_remove_FE_element_private(fe_region, element))
				{
					return_code = 0;
					break;
				}
			}
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
			if (FIRST_OBJECT_IN_LIST_THAT(FE_element_field_info)(
					FE_element_field_info_has_FE_field, (void *)fe_field,
					fe_region->fe_element_field_info_list))
			{
				/* since elements may still exist in the change_log or slave FE_regions,
					 must now check that no remaining elements use fe_field */
				/* for now, if there are elements then fe_field is in use */
				if (0 < FE_region_get_number_of_FE_elements_all_dimensions(fe_region))
					return true;
			}
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

struct CHANGE_LOG(FE_element) *FE_region_get_FE_element_changes(
	struct FE_region *fe_region, int dimension)
{
	if (fe_region && (0 < dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
		return fe_region->fe_element_changes[dimension - 1];
	return 0;
}

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
	if (node)
	{
		if (IS_OBJECT_IN_LIST(FE_node)(node, this->nodeList))
		{
			if (this->findNodeByIdentifier(new_identifier))
			{
				display_message(ERROR_MESSAGE,
					"FE_nodeset::change_FE_node_identifier.  "
					"Node with new identifier already exists");
				return CMZN_ERROR_GENERAL;
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
			cmzn_elementiterator *iter = CREATE_LIST_ITERATOR(cmzn_element)(fe_region->get_element_list(dimension));
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

int FE_region_change_FE_element_identifier(
	struct FE_region *fe_region, struct FE_element *element,
	int new_identifier)
{
	int return_code;

	ENTER(FE_region_change_FE_element_identifier);
	int dimension = get_FE_element_dimension(element);
	if (fe_region && element && new_identifier && (1 <= dimension) &&
		(dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		struct LIST(FE_element) *element_list = fe_region->get_element_list(dimension);
		if (IS_OBJECT_IN_LIST(FE_element)(element, element_list))
		{
			return_code = 1;
			if (FE_region_get_FE_element_from_identifier(fe_region,
				dimension, new_identifier))
			{
				display_message(ERROR_MESSAGE,
					"FE_region_change_FE_element_identifier.  "
					"Element with new identifier already exists");
				return_code = 0;
			}
			else
			{
				struct LIST(FE_element) *master_element_list = fe_region->get_element_list(dimension);
				if (LIST_BEGIN_IDENTIFIER_CHANGE(FE_element,identifier)(
					master_element_list, element))
				{
					CM_element_information cm;
					get_FE_element_identifier(element, &cm);
					cm.number = new_identifier;
					return_code = set_FE_element_identifier(element, &cm);
					LIST_END_IDENTIFIER_CHANGE(FE_element,identifier)(
						master_element_list);
					if (return_code)
					{
						FE_REGION_FE_ELEMENT_IDENTIFIER_CHANGE(fe_region, element);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_region_change_FE_element_identifier.  "
						"Could not safely change identifier in indexed lists");
					return_code = 0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_region_change_FE_element_identifier.  "
				"Element is not in this region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_change_FE_element_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_change_FE_element_identifier */

struct FE_element *FE_region_get_FE_element_from_identifier(
	struct FE_region *fe_region, int dimension, int identifier)
{
	struct FE_element *element;

	ENTER(FE_region_get_FE_element_from_identifier);
	if (fe_region && (1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		CM_element_information cm = { DIMENSION_TO_CM_ELEMENT_TYPE(dimension), identifier };
		element = FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(&cm,
			fe_region->get_element_list(dimension));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_FE_element_from_identifier.  Invalid argument(s)");
		element = (struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
}

struct FE_element *FE_region_get_FE_element_from_identifier_deprecated(
	struct FE_region *fe_region, struct CM_element_information *identifier)
{
	struct FE_element *element;

	ENTER(FE_region_get_FE_element_from_identifier_deprecated);
	if (fe_region && identifier)
	{
		int dimension = CM_ELEMENT_TYPE_TO_DIMENSION(identifier->type);
		element = FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(identifier,
			fe_region->get_element_list(dimension));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_FE_element_from_identifier_deprecated.  Invalid argument(s)");
		element = (struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
}

struct FE_element *FE_region_get_top_level_FE_element_from_identifier(
	struct FE_region *fe_region, int identifier)
{
	struct FE_element *element;

	ENTER(FE_region_get_top_level_FE_element_from_identifier);
	if (fe_region && identifier)
	{
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; 1 <= dimension; --dimension)
		{
			CM_element_information cm = { DIMENSION_TO_CM_ELEMENT_TYPE(dimension), identifier };
			element = FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(&cm,
				fe_region->get_element_list(dimension));
			if (element && FE_element_is_top_level(element, (void *)NULL))
			{
				break;
			}
			else
			{
				element = NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_top_level_FE_element_from_identifier.  Invalid argument(s)");
		element = (struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
}

struct FE_element *FE_region_get_or_create_FE_element_with_identifier(
	struct FE_region *fe_region, int dimension, int identifier,
	struct FE_element_shape *element_shape)
{
	struct FE_element *element = 0;
	int element_shape_dimension = 0;
	if (fe_region && (1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS) &&
		((!element_shape) || (get_FE_element_shape_dimension(element_shape, &element_shape_dimension) &&
		(element_shape_dimension == dimension))))
	{
		CM_element_information cm = { DIMENSION_TO_CM_ELEMENT_TYPE(dimension), identifier};
		element = FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(&cm,
			fe_region->get_element_list(dimension));
		if (!element)
		{
			struct FE_element_shape *temp_element_shape = 0;
			if (!element_shape)
			{
				element_shape = temp_element_shape = CREATE(FE_element_shape)(
					dimension, /*type*/(int *)NULL, fe_region);
				ACCESS(FE_element_shape)(temp_element_shape);
			}
			if (element_shape)
			{
				element = CREATE(FE_element)(&cm, element_shape,
					fe_region, /*template_element*/(struct FE_element *)NULL);
				if (temp_element_shape)
				{
					DEACCESS(FE_element_shape)(&temp_element_shape);
				}
			}
			if (!element)
			{
				display_message(ERROR_MESSAGE,
					"FE_region_get_or_create_FE_element_with_identifier.  "
					"Could not create element");
			}
			if (element)
			{
				if (!FE_region_merge_FE_element(fe_region, element))
				{
					display_message(ERROR_MESSAGE,
						"FE_region_get_or_create_FE_element_with_identifier.  "
						"Could not merge element");
					/* following cleans up element if newly created, and clears pointer */
					REACCESS(FE_element)(&element, (struct FE_element *)NULL);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_or_create_FE_element_with_identifier.  "
			"Invalid argument(s)");
	}
	return (element);
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

cmzn_mesh_scale_factor_set *FE_region_find_mesh_scale_factor_set_by_name(
	struct FE_region *fe_region, const char *name)
{
	if (fe_region && name)
	{
		std::vector<cmzn_mesh_scale_factor_set*>& scale_factor_sets = fe_region->get_scale_factor_sets();
		const size_t size = scale_factor_sets.size();
		for (size_t i = 0; i < size; ++i)
		{
			if (0 == strcmp(scale_factor_sets[i]->getName(), name))
			{
				return scale_factor_sets[i]->access();
			}
		}
	}
	return 0;
}

cmzn_mesh_scale_factor_set *FE_region_create_mesh_scale_factor_set(
	struct FE_region *fe_region)
{
	if (fe_region)
	{
		std::vector<cmzn_mesh_scale_factor_set*>& scale_factor_sets = fe_region->get_scale_factor_sets();
		char tempName[10];
		for (int i = static_cast<int>(scale_factor_sets.size()) + 1; ; ++i)
		{
			sprintf(tempName, "temp%d", i);
			cmzn_mesh_scale_factor_set *existingSet =
				FE_region_find_mesh_scale_factor_set_by_name(fe_region, tempName);
			if (existingSet)
			{
				cmzn_mesh_scale_factor_set::deaccess(existingSet);
			}
			else
			{
				cmzn_mesh_scale_factor_set *scale_factor_set = cmzn_mesh_scale_factor_set::create(fe_region, tempName);
				scale_factor_sets.push_back(scale_factor_set);
				return scale_factor_set->access();
			}
		}
	}
	return 0;
}

int FE_region_get_next_FE_element_identifier(struct FE_region *fe_region,
	int dimension, int start_identifier)
{
	int identifier = 0;

	ENTER(FE_region_get_next_FE_element_identifier);
	if (fe_region && (1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		int dim = dimension - 1;
		struct CM_element_information cm = { DIMENSION_TO_CM_ELEMENT_TYPE(dimension), 0 };
		if (start_identifier <= 0)
		{
			cm.number = fe_region->next_fe_element_identifier_cache[dim];
		}
		else
		{
			cm.number = start_identifier;
		}
		if (fe_region->next_fe_element_identifier_cache[dim])
		{
			if (fe_region->next_fe_element_identifier_cache[dim] > cm.number)
			{
				cm.number = fe_region->next_fe_element_identifier_cache[dim];
			}
		}
		while (FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(
			&cm, fe_region->fe_element_list[dim]))
		{
			cm.number++;
		}
		if (start_identifier <= 1)
		{
			/* Don't cache the value if we didn't start at the beginning */
			fe_region->next_fe_element_identifier_cache[dim] = cm.number;
		}
		identifier = cm.number;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_next_FE_element_identifier.  Missing fe_region");
	}
	LEAVE;

	return (identifier);
}

int FE_region_get_number_of_FE_elements_all_dimensions(struct FE_region *fe_region)
{
	int number_of_elements = 0;

	ENTER(FE_region_get_number_of_FE_elements_all_dimensions);
	if (fe_region)
	{
		for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		{
			number_of_elements += NUMBER_IN_LIST(FE_element)(fe_region->fe_element_list[dim]);
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
	int number_of_elements = 0;

	ENTER(FE_region_get_number_of_FE_elements_of_dimension);
	if (fe_region && (1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		number_of_elements =
			NUMBER_IN_LIST(FE_element)(fe_region->fe_element_list[dimension - 1]);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_number_of_FE_elements_of_dimension.  Invalid argument(s)");
	}
	LEAVE;

	return (number_of_elements);
}

int FE_region_get_highest_dimension(struct FE_region *fe_region)
{
	int highest_dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS;
	while (highest_dimension &&
		(0 == NUMBER_IN_LIST(FE_element)(fe_region->get_element_list(highest_dimension))))
	{
		--highest_dimension;
	}
	return highest_dimension;
}

int FE_region_contains_FE_element(struct FE_region *fe_region,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 9 December 2002

DESCRIPTION :
Returns true if <element> is in <fe_region>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_contains_FE_element);
	if (fe_region && element)
	{
		int dimension = get_FE_element_dimension(element);
		if (IS_OBJECT_IN_LIST(FE_element)(element, fe_region->get_element_list(dimension)))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_contains_FE_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_contains_FE_element */

int FE_region_contains_FE_element_conditional(struct FE_element *element,
	void *fe_region_void)
{
	return FE_region_contains_FE_element((struct FE_region *)fe_region_void, element);
}

int FE_element_is_not_in_FE_region(struct FE_element *element,
	void *fe_region_void)
{
	return !FE_region_contains_FE_element((struct FE_region *)fe_region_void, element);
}

int FE_region_define_FE_field_at_element(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *fe_field,
	struct FE_element_field_component **components)
{
	int return_code;

	ENTER(FE_region_define_FE_field_at_element);
	if (fe_region && element && fe_field && components)
	{
		if (FE_region_contains_FE_element(fe_region, element))
		{
			return_code = 1;
			if (!FE_field_is_defined_in_element(fe_field, element))
			{
				if (FE_field_get_FE_region(fe_field) != fe_region)
				{
					display_message(ERROR_MESSAGE,
						"FE_region_define_FE_field_at_element.  "
						"Field is not of this finite element region");
					return_code = 0;
				}
				if (return_code)
				{
					return_code =	define_FE_field_at_element(element, fe_field, components);
					if (return_code)
					{
						FE_REGION_FE_ELEMENT_FIELD_CHANGE(fe_region, element,
							fe_field);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_region_define_FE_field_at_element.  Element is not in region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_define_FE_field_at_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_define_FE_field_at_element */

struct FE_element *FE_region_create_FE_element_copy(struct FE_region *fe_region,
	int identifier, struct FE_element *source)
{
	struct FE_element *new_element;

	ENTER(FE_region_create_FE_element_copy);
	new_element = (struct FE_element *)NULL;
	if (fe_region && source && (-1 <= identifier))
	{
		int dimension = get_FE_element_dimension(source);
		struct LIST(FE_element) *element_list = fe_region->get_element_list(dimension);
		if (FE_element_get_FE_region(source) == fe_region)
		{
			struct CM_element_information cm =
				{ DIMENSION_TO_CM_ELEMENT_TYPE(dimension), identifier };
			if (cm.number < 0)
			{
				cm.number = FE_region_get_next_FE_element_identifier(
					fe_region, dimension, 0);
			}
			FE_element *existing_element = FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(&cm, element_list);
			if (existing_element)
			{
				display_message(ERROR_MESSAGE,
					"FE_region_create_FE_element_copy.  Element identifier in use.");
				return 0;
			}
			new_element = CREATE(FE_element)(&cm, (struct FE_element_shape *)NULL,
				(struct FE_region *)NULL, source);
			ADD_OBJECT_TO_LIST(FE_element)(new_element, element_list);
			FE_region_begin_change(fe_region);
			FE_REGION_FE_ELEMENT_CHANGE(fe_region, new_element,
				CHANGE_LOG_OBJECT_ADDED(FE_element), new_element);
			if (fe_region->define_faces_of_dimension[0])
				FE_region_merge_FE_element_and_faces_and_nodes(fe_region, new_element);
			FE_region_end_change(fe_region);
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_region_create_FE_element_copy.  "
				"Source element is incompatible with region");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_create_FE_element_copy.  Invalid argument(s)");
	}
	LEAVE;

	return (new_element);
}

struct FE_element *FE_region_merge_FE_element(struct FE_region *fe_region,
	struct FE_element *element)
{
	struct FE_element *merged_element = 0;
	if (fe_region && element)
	{
		int dimension = get_FE_element_dimension(element);
		struct LIST(FE_element) *element_list = fe_region->get_element_list(dimension);
		struct CM_element_information identifier;
		get_FE_element_identifier(element, &identifier);
		if (fe_region == FE_element_get_FE_region(element))
		{
			merged_element = FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(&identifier, element_list);
			if (merged_element)
			{
				// nothing to do if merging existing element; happens when adding faces
				if (merged_element != element)
				{
					struct LIST(FE_field) *changed_fe_field_list = CREATE(LIST(FE_field))();
					if (changed_fe_field_list)
					{
						/* merge fields from element into global merged_element */
						if (merge_FE_element(merged_element, element, changed_fe_field_list))
						{
#if defined (DEBUG_CODE)
							/*???debug*/printf("FE_region_merge_FE_element: %p OBJECT_NOT_IDENTIFIER_CHANGED element %p\n",fe_region,merged_element);
#endif /* defined (DEBUG_CODE) */
							FE_REGION_FE_ELEMENT_FE_FIELD_LIST_CHANGE(fe_region, merged_element,
								CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_element) | CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_element),
								changed_fe_field_list);
						}
						else
						{
							display_message(ERROR_MESSAGE, "FE_region_merge_FE_element.  Could not merge %s %d",
								CM_element_type_string(identifier.type), identifier.number);
							merged_element = (struct FE_element *)NULL;
						}
						DESTROY(LIST(FE_field))(&changed_fe_field_list);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"FE_region_merge_FE_element.  Could not create field list");
						merged_element = (struct FE_element *)NULL;
					}
				}
			}
			else
			{
				if (ADD_OBJECT_TO_LIST(FE_element)(element, element_list))
				{
					merged_element = element;
#if defined (DEBUG_CODE)
				/*???debug*/printf("FE_region_merge_FE_element: %p ADD element %p\n",fe_region,merged_element);
#endif /* defined (DEBUG_CODE) */
					FE_REGION_FE_ELEMENT_CHANGE(fe_region, merged_element,
						CHANGE_LOG_OBJECT_ADDED(FE_element), merged_element);
				}
				else
				{
					display_message(ERROR_MESSAGE, "FE_region_merge_FE_element.  Could not add %s %d",
						CM_element_type_string(identifier.type), identifier.number);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_region_merge_FE_element.  "
				"%s %d is not of this finite element region",
				CM_element_type_string(identifier.type), identifier.number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_merge_FE_element.  Invalid argument(s)");
	}
	return (merged_element);
}

int FE_region_merge_FE_element_existing(struct FE_region *fe_region,
	struct FE_element *destination, struct FE_element *source)
{
	int return_code;
	struct LIST(FE_field) *changed_fe_field_list;

	ENTER(FE_region_merge_FE_element_existing);
	return_code = 0;
	if (fe_region && destination && source)
	{
		int dimension = get_FE_element_dimension(destination);
		int source_dimension = get_FE_element_dimension(source);
		if (source_dimension == dimension)
		{
			if ((FE_element_get_FE_region(destination) == fe_region) &&
				(FE_element_get_FE_region(source) == fe_region))
			{
				changed_fe_field_list = CREATE(LIST(FE_field))();
				if (merge_FE_element(destination, source, changed_fe_field_list))
				{
					return_code = 1;
					FE_REGION_FE_ELEMENT_FE_FIELD_LIST_CHANGE(fe_region, destination,
						CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_element) | CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_element),
						changed_fe_field_list);
				}
				DESTROY(LIST(FE_field))(&changed_fe_field_list);
			}
			else
			{
				display_message(ERROR_MESSAGE, "FE_region_merge_FE_element_existing.  "
					"Source and/or destination elements are incompatible with region");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_region_merge_FE_element_existing.  "
				"Cannot merge a source element of different dimension");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_merge_FE_element_existing.  Invalid argument(s)");
	}
	LEAVE;

	return return_code;
}

int FE_region_merge_FE_element_iterator(struct FE_element *element,
	void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
Iterator version of <FE_region_merge_FE_element>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_merge_FE_element_iterator);
	if (FE_region_merge_FE_element((struct FE_region *)fe_region_void, element))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_merge_FE_element_iterator */

int FE_region_begin_define_faces(struct FE_region *fe_region, int face_dimension)
{
	int return_code;

	ENTER(FE_region_begin_define_faces);
	return_code = 0;
	if (fe_region &&
		(-1 <= face_dimension) && (face_dimension < MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		if (fe_region->element_type_node_sequence_list)
		{
			display_message(ERROR_MESSAGE,
				"FE_region_begin_define_faces.  Already defining faces");
		}
		else
		{
			fe_region->element_type_node_sequence_list =
				CREATE(LIST(FE_element_type_node_sequence))();
			if (fe_region->element_type_node_sequence_list)
			{
				for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; dim++)
				{
					fe_region->define_faces_of_dimension[dim] = 0;
				}
				if (face_dimension <= 0)
				{
					fe_region->define_faces_of_dimension[0] = 1;
				}
				int max_dimension = (face_dimension == -1) ? MAXIMUM_ELEMENT_XI_DIMENSIONS - 1 : face_dimension;
				int min_dimension = (face_dimension == -1) ? 1 : face_dimension;
				for (int dimension = max_dimension; min_dimension <= dimension; --dimension)
				{
					fe_region->define_faces_of_dimension[dimension] = 1;
					struct LIST(FE_element) *element_list = fe_region->get_element_list(dimension);
					if (0 < NUMBER_IN_LIST(FE_element)(element_list))
					{
						if (FOR_EACH_OBJECT_IN_LIST(FE_element)(
							FE_element_face_line_to_element_type_node_sequence_list,
							(void *)(fe_region->element_type_node_sequence_list),
							element_list))
						{
							return_code = 1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"FE_region_begin_define_faces.  "
								"May not be able to share faces properly - perhaps "
								"2 existing faces have same shape and node list?");
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_region_begin_define_faces.  Could not create node sequence list");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_begin_define_faces.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int FE_region_end_define_faces(struct FE_region *fe_region)
{
	int return_code;

	ENTER(FE_region_end_define_faces);
	return_code = 0;
	if (fe_region)
	{
		if (fe_region->element_type_node_sequence_list)
		{
			DESTROY(LIST(FE_element_type_node_sequence))(
				&fe_region->element_type_node_sequence_list);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_region_end_define_faces.  Already defining faces");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_end_define_faces.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

static int FE_region_merge_FE_element_nodes(struct FE_region *fe_region,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 19 May 2003

DESCRIPTION :
Merges the nodes used by fields of <element> into <fe_region>.
Currently only used by FE_region_merge_FE_element_and_faces_and_nodes.
Expects call to be wrapped in calls to FE_region_begin/end_changes.
If <element> has any parents, the nodes referenced by the default
coordinate field inherited from any single ultimate parent are merged.
???RC This "single ancestor field inheritance" is not sufficient in general;
one would have to inherit all fields from all top_level ancestor elements which
is quite expensive; deal with this once faces are actually passed to this
function -- they currently are not.
If the element has no parent, merges nodes directly referenced by <element>.
==============================================================================*/
{
	int i, number_of_element_field_nodes, number_of_nodes,
		return_code;
	struct FE_node **element_field_nodes_array, *node;

	ENTER(FE_region_merge_FE_element_nodes);
	if (fe_region && element &&
		get_FE_element_number_of_nodes(element, &number_of_nodes))
	{
		return_code = 1;
		FE_nodeset *fe_nodeset = fe_region->nodesets[0];
		const int number_of_parents = get_FE_element_number_of_parents(element);
		if (0 < number_of_parents)
		{
			if (calculate_FE_element_field_nodes(element, (struct FE_field *)NULL,
				&number_of_element_field_nodes, &element_field_nodes_array,
				/*top_level_element*/(struct FE_element *)NULL))
			{
				for (i = 0; i < number_of_element_field_nodes; i++)
				{
					node = element_field_nodes_array[i];
					if (node)
					{
						if (!fe_nodeset->merge_FE_node(node))
						{
							return_code = 0;
						}
						DEACCESS(FE_node)(&(element_field_nodes_array[i]));
					}
				}
				DEALLOCATE(element_field_nodes_array);
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			for (i = 0; (i < number_of_nodes) && return_code; i++)
			{
				if (get_FE_element_node(element, i, &node))
				{
					if (node)
					{
						if (!fe_nodeset->merge_FE_node(node))
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
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_region_merge_FE_element_nodes.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_merge_FE_element_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_merge_FE_element_nodes */

static int FE_region_merge_FE_element_and_faces_private(
	struct FE_region *fe_region, struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Private version of FE_region_merge_FE_element that merges not only <element>
into <fe_region> but any of its faces that are defined.
<element> and any of its faces may already be in <fe_region>.
This function is called by FE_region_merge_FE_element_and_faces to merge
<element> and its faces, but NOT nodes since node merge is performed only
with the initial element.

- MUST NOT iterate over the list of elements in a region to add or define faces
as the list will be modified in the process; copy the list and iterate over
the copy.

Expects call to be wrapped in calls to FE_region_begin/end_changes.

If calls to this function are placed between FE_region_begin/end_define_faces,
then any missing faces are created and also merged into <fe_region>.
Function ensures that elements share existing faces and lines in preference to
creating new ones if they have matching shape and nodes.

???RC Can only match faces correctly for coordinate fields with standard node
to element maps and no versions. A grid-based coordinate field would fail
utterly since it has no nodes. A possible future solution for all cases is to
match the geometry exactly either by using the FE_element_field_values
(coefficients of the monomial basis functions), although there is a problem with
xi-directions not matching up, or actual centre positions of the face being a
trivial rejection, narrowing down to a single face or list of faces to compare
against.
==============================================================================*/
{
	int face_number, number_of_faces, return_code;
	struct FE_element *face;
	struct FE_element_shape *element_shape, *face_shape;
	struct FE_element_type_node_sequence *element_type_node_sequence,
		*existing_element_type_node_sequence;

	ENTER(FE_region_merge_FE_element_and_faces_private);
	if (fe_region && element &&
		(FE_element_get_FE_region(element) == fe_region) &&
		get_FE_element_shape(element, &element_shape) &&
		get_FE_element_number_of_faces(element, &number_of_faces))
	{
		return_code = 1;
		int number_of_new_faces = 0;
		int face_dimension = get_FE_element_dimension(element) - 1;
		int define_new_faces = (0 != fe_region->element_type_node_sequence_list) &&
			fe_region->define_faces_of_dimension[face_dimension];
		for (face_number = 0; (face_number < number_of_faces) && return_code;
			face_number++)
		{
			face = (struct FE_element *)NULL;
			if ((return_code = get_FE_element_face(element, face_number, &face)) &&
				(!face) && define_new_faces)
			{
				face_shape = get_FE_element_shape_of_face(element_shape, face_number, fe_region);
				if (face_shape)
				{
					ACCESS(FE_element_shape)(face_shape);
					get_FE_element_shape_dimension(face_shape, &face_dimension);
					int new_face_number = FE_region_get_next_FE_element_identifier(
						fe_region, face_dimension, 0);
					struct CM_element_information face_identifier =
						{ DIMENSION_TO_CM_ELEMENT_TYPE(face_dimension), new_face_number };
					face = CREATE(FE_element)(&face_identifier, face_shape,	fe_region, (struct FE_element *)NULL);
					if (face)
					{
						/* must put the face in the element to inherit fields */
						set_FE_element_face(element, face_number, face);
						number_of_new_faces++;
						/* try to find an existing face in the FE_region with the same
							 shape and the same nodes as face */
						element_type_node_sequence = CREATE(FE_element_type_node_sequence)(face);
						if (element_type_node_sequence)
						{
							ACCESS(FE_element_type_node_sequence)(element_type_node_sequence);
							if (FE_element_type_node_sequence_is_collapsed(
								element_type_node_sequence))
							{
								/* clear the face */
								set_FE_element_face(element, face_number,
									(struct FE_element *)NULL);
								face = (struct FE_element *)NULL;
								number_of_new_faces--;
							}
							else
							{
								existing_element_type_node_sequence =
									FIND_BY_IDENTIFIER_IN_LIST(FE_element_type_node_sequence,
										identifier)(FE_element_type_node_sequence_get_identifier(
											element_type_node_sequence),
											fe_region->element_type_node_sequence_list);
								if (existing_element_type_node_sequence)
								{
									face = FE_element_type_node_sequence_get_FE_element(
										existing_element_type_node_sequence);
									set_FE_element_face(element, face_number, face);
								}
								else
								{
									/* remember this sequence */
									ADD_OBJECT_TO_LIST(FE_element_type_node_sequence)(
										element_type_node_sequence,
										fe_region->element_type_node_sequence_list);
								}
							}
							DEACCESS(FE_element_type_node_sequence)(
								&element_type_node_sequence);
						}
						else
						{
							/* clear the face */
							set_FE_element_face(element, face_number,
								(struct FE_element *)NULL);
							face = (struct FE_element *)NULL;
							number_of_new_faces--;
							return_code = 0;
						}
					}
					else
					{
						return_code = 0;
					}
					DEACCESS(FE_element_shape)(&face_shape);
				}
				else
				{
					/* could not get face_shape */
					return_code = 0;
				}
			}
			if (face)
			{
				/* ensure the face and its lines are in the fe_region */
				return_code =
					FE_region_merge_FE_element_and_faces_private(fe_region, face);
			}
		} /* loop over faces */
		if (return_code)
		{
			if (FE_region_contains_FE_element(fe_region, element))
			{
				if (number_of_new_faces)
				{
					FE_REGION_FE_ELEMENT_CHANGE(fe_region, element,
						CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_element), element);
				}
			}
			else
			{
				/* merge element into fe_region */
				if (!FE_region_merge_FE_element(fe_region, element))
				{
					return_code = 0;
				}
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_region_merge_FE_element_and_faces_private.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_merge_FE_element_and_faces_private.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_merge_FE_element_and_faces_private */

int FE_region_merge_FE_element_and_faces_and_nodes(struct FE_region *fe_region,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Version of FE_region_merge_FE_element that merges not only <element> into
<fe_region> but any of its faces that are defined.
<element> and any of its faces may already be in <fe_region>.
Also merges nodes referenced directly by <element> and its parents, if any.

- MUST NOT iterate over the list of elements in a region to add or define faces
as the list will be modified in the process; copy the list and iterate over
the copy.

FE_region_begin/end_change are called internally to reduce change messages to
one per call. User should place calls to the begin/end_change functions around
multiple calls to this function.

If calls to this function are placed between FE_region_begin/end_define_faces,
then any missing faces are created and also merged into <fe_region>.
Function ensures that elements share existing faces and lines in preference to
creating new ones if they have matching shape and nodes.

???RC Can only match faces correctly for coordinate fields with standard node
to element maps and no versions. A grid-based coordinate field would fail
utterly since it has no nodes. A possible future solution for all cases is to
match the geometry exactly either by using the FE_element_field_values
(coefficients of the monomial basis functions), although there is a problem with
xi-directions not matching up, or actual centre positions of the face being a
trivial rejection, narrowing down to a single face or list of faces to compare
against.
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_merge_FE_element_and_faces_and_nodes);
	if (fe_region && element && (FE_element_get_FE_region(element) == fe_region))
	{
		return_code = 1;
		FE_region_begin_change(fe_region);
		if (FE_region_merge_FE_element_nodes(fe_region, element))
		{
			if (!FE_region_merge_FE_element_and_faces_private(
				fe_region, element))
			{
				display_message(ERROR_MESSAGE,
					"FE_region_merge_FE_element_and_faces_and_nodes.  "
					"Could not merge element and faces");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_region_merge_FE_element_and_faces_and_nodes.  "
				"Could not merge element nodes");
			return_code = 0;
		}
		FE_region_end_change(fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_merge_FE_element_and_faces_and_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_merge_FE_element_and_faces_and_nodes */

int FE_region_define_faces(struct FE_region *fe_region)
{
	int return_code = 1;
	if (fe_region)
	{
		FE_region_begin_change(fe_region);
		FE_region_begin_define_faces(fe_region, /*all dimensions*/-1);
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (2 <= dimension) && return_code; --dimension)
		{
			LIST(FE_element) *element_list = fe_region->get_element_list(dimension);
			cmzn_elementiterator_id iter = CREATE_LIST_ITERATOR(FE_element)(element_list);
			cmzn_element_id element = 0;
			while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
			{
				if (!FE_region_merge_FE_element_and_faces_and_nodes(fe_region, element))
				{
					return_code = 0;
					break;
				}
			}
			cmzn_elementiterator_destroy(&iter);
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

static int FE_region_remove_FE_element_iterator(struct FE_element *element,
	void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Removes <element> and all its faces that are not shared with other elements
from <fe_region>. Should enclose call between FE_region_begin_change and
FE_region_end_change to minimise messages.
Can only remove faces and lines if there is no master_fe_region or no parents
in this fe_region.
This function is recursive.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(FE_region_remove_FE_element_iterator);
	if (element && (fe_region = (struct FE_region *)fe_region_void))
	{
		return_code = 1;
		int dimension = get_FE_element_dimension(element);
		if (IS_OBJECT_IN_LIST(FE_element)(element, fe_region->get_element_list(dimension)))
		{
			return_code = FE_region_remove_FE_element_private(fe_region, element);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_remove_FE_element_iterator.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_remove_FE_element_iterator */

int FE_region_remove_FE_element(struct FE_region *fe_region,
	struct FE_element *element)
{
	if (fe_region && element)
	{
		FE_region_begin_change(fe_region);
		int return_code = FE_region_remove_FE_element_private(fe_region, element);
		FE_region_end_change(fe_region);
		return return_code;
	}
	return 0;
}

int FE_region_remove_FE_element_list(struct FE_region *fe_region,
	struct LIST(FE_element) *element_list)
{
	int return_code;

	ENTER(FE_region_remove_FE_element_list);
	if (fe_region && element_list)
	{
		FE_region_begin_change(fe_region);
		return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
			FE_region_remove_FE_element_iterator, (void *)fe_region, element_list);
		REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(FE_element_is_not_in_FE_region,
			(void *)fe_region, element_list);
		if (0 < NUMBER_IN_LIST(FE_element)(element_list))
		{
			return_code = 0;
		}
		FE_region_end_change(fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_remove_FE_element_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_remove_FE_element_list */

int FE_region_element_or_parent_has_field(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Returns true if <element> is in <fe_region> and has <field> defined on it or
any parents also in <fe_region>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_element_or_parent_has_field);
	if (fe_region && element && field)
	{
		return_code = FE_element_or_parent_has_field(element, field,
			FE_region_contains_FE_element_conditional, (void *)fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_element_or_parent_has_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_element_or_parent_has_field */

struct FE_element *FE_region_get_first_FE_element_that(
	struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function,
	void *user_data_void)
{
	struct FE_element *element;

	ENTER(FE_region_get_first_FE_element_that);
	if (fe_region)
	{
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (1 <= dimension); --dimension)
		{
			element = FIRST_OBJECT_IN_LIST_THAT(FE_element)(conditional_function,
				user_data_void, fe_region->get_element_list(dimension));
			if (element)
				break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_first_FE_element_that.  Invalid argument(s)");
		element = (struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
}

struct FE_element *FE_region_get_first_FE_element_of_dimension_that(
	struct FE_region *fe_region, int dimension,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function,
	void *user_data_void)
{
	struct FE_element *element;

	ENTER(FE_region_get_first_FE_element_that);
	if (fe_region && (1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		element = FIRST_OBJECT_IN_LIST_THAT(FE_element)(conditional_function,
			user_data_void, fe_region->get_element_list(dimension));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_get_first_FE_element_that.  Invalid argument(s)");
		element = (struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
}

int FE_region_for_each_FE_element(struct FE_region *fe_region,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function, void *user_data)
{
	int return_code;

	ENTER(FE_region_for_each_FE_element);
	if (fe_region && iterator_function)
	{
		return_code = 1;
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (return_code) && (1 <= dimension); --dimension)
		{
			return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(iterator_function,
				user_data, fe_region->get_element_list(dimension));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_for_each_FE_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_for_each_FE_element */

int FE_region_for_each_FE_element_of_dimension(struct FE_region *fe_region,
	int dimension, LIST_ITERATOR_FUNCTION(FE_element) iterator_function,
	void *user_data)
{
	int return_code;

	ENTER(FE_region_for_each_FE_element_of_dimension);
	if (fe_region && iterator_function &&
		(1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(iterator_function,
			user_data, fe_region->get_element_list(dimension));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_for_each_FE_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int FE_region_for_each_FE_element_conditional(struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_element) conditional_function,
	void *conditional_user_data,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function,
	void *iterator_user_data)
{
	int return_code;
	struct FE_element_conditional_iterator_data data;

	ENTER(FE_region_for_each_FE_element_conditional);
	if (fe_region && conditional_function && iterator_function)
	{
		data.conditional_function = conditional_function;
		data.conditional_user_data = conditional_user_data;
		data.iterator_function = iterator_function;
		data.iterator_user_data = iterator_user_data;
		return_code = FE_region_for_each_FE_element(fe_region,
			FE_element_conditional_iterator, (void *)&data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_for_each_FE_element_conditional.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int FE_region_for_each_FE_element_of_dimension_conditional(
	struct FE_region *fe_region, int dimension,
	LIST_CONDITIONAL_FUNCTION(FE_element) conditional_function,
	void *conditional_user_data,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function,
	void *iterator_user_data)
{
	int return_code;
	struct FE_element_conditional_iterator_data data;

	ENTER(FE_region_for_each_FE_element_of_dimension_conditional);
	if (fe_region && conditional_function && iterator_function)
	{
		data.conditional_function = conditional_function;
		data.conditional_user_data = conditional_user_data;
		data.iterator_function = iterator_function;
		data.iterator_user_data = iterator_user_data;
		return_code = FE_region_for_each_FE_element_of_dimension(fe_region,
			dimension, FE_element_conditional_iterator, (void *)&data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_for_each_FE_element_of_dimension_conditional.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

cmzn_elementiterator_id FE_region_create_elementiterator(
	struct FE_region *fe_region, int dimension)
{
	if (fe_region)
		return CREATE_LIST_ITERATOR(FE_element)(fe_region->get_element_list(dimension));
	return 0;
}

struct FE_element *FE_region_element_string_to_FE_element(
	struct FE_region *fe_region, const char *name)
{
	cmzn_element_id element = 0;
	if (fe_region && name)
	{
		int highest_dimension = FE_region_get_highest_dimension(fe_region);
		if (highest_dimension)
		{
			element = FE_region_get_FE_element_from_identifier(fe_region, highest_dimension, atoi(name));
		}
	}
	return (element);
}

struct FE_element *FE_region_any_element_string_to_FE_element(
	struct FE_region *fe_region, const char *name)
{
	struct FE_element *element;

	ENTER(FE_region_any_element_string_to_FE_element);
	element = (struct FE_element *)NULL;
	if (fe_region && name)
	{
		const char *number_string, *type_string;
		struct CM_element_information cm;
		type_string = strpbrk(name,"eEfFlL");
		if (type_string)
		{
			if (('l' == *type_string)||('L' == *type_string))
			{
				cm.type = CM_LINE;
			}
			else if (('f' == *type_string)||('F' == *type_string))
			{
				cm.type = CM_FACE;
			}
			else
			{
				cm.type = CM_ELEMENT;
			}
		}
		else
		{
			cm.type = CM_ELEMENT;
		}
		number_string = strpbrk(name,"+-0123456789");
		if (number_string)
		{
			cm.number = atoi(number_string);
			int dimension = CM_ELEMENT_TYPE_TO_DIMENSION(cm.type);
			element = FE_region_get_FE_element_from_identifier(fe_region, dimension, cm.number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_any_element_string_to_FE_element.  Invalid argument(s)");
	}
	LEAVE;

	return (element);
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
	struct FE_region *fe_region;
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
					(struct FE_element_shape *)NULL, (struct FE_region *)NULL, element)))
			{
				ACCESS(FE_element)(copy_element);
				if (FE_element_smooth_FE_field(copy_element,
					smooth_element_data->fe_field,
					smooth_element_data->time,
					smooth_element_data->element_count_fe_field,
					smooth_element_data->copy_node_list))
				{
					if (!FE_region_merge_FE_element(smooth_element_data->fe_region,
						copy_element))
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
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Smooths node-based <fe_field> over its nodes and elements in <fe_region>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_smooth_FE_field);
	if (fe_region && fe_field)
	{
		if (IS_OBJECT_IN_LIST(FE_field)(fe_field, fe_region->fe_field_list))
		{
			return_code = 1;
			// use highest dimension non-empty element list
			int dimension = FE_region_get_highest_dimension(fe_region);
			if (dimension)
			{
				struct LIST(FE_element) *element_list = fe_region->get_element_list(dimension);
				FE_region_begin_change(fe_region);
				struct FE_region_smooth_FE_element_data smooth_element_data;
				smooth_element_data.time = time;
				smooth_element_data.fe_field = fe_field;
				/* create a field to store an integer value per component of fe_field */
				smooth_element_data.element_count_fe_field =
					CREATE(FE_field)("count", fe_region);
				set_FE_field_number_of_components(
					smooth_element_data.element_count_fe_field,
					get_FE_field_number_of_components(fe_field));
				set_FE_field_value_type(smooth_element_data.element_count_fe_field,
					INT_VALUE);
				ACCESS(FE_field)(smooth_element_data.element_count_fe_field);
				smooth_element_data.fe_region = fe_region;
				smooth_element_data.copy_node_list = CREATE(LIST(FE_node))();
				if (!FOR_EACH_OBJECT_IN_LIST(FE_element)(FE_region_smooth_FE_element,
					(void *)&smooth_element_data, element_list))
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
		return_code = 0;;
	}
	LEAVE;

	return (return_code);
} /* FE_region_smooth_FE_field */

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
		int identifier = cmzn_element_get_identifier(element);
		struct FE_element_shape *element_shape = 0;
		get_FE_element_shape(element, &element_shape);
		global_element = FE_region_get_or_create_FE_element_with_identifier(
			global_fe_region, dimension, identifier, element_shape);
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

struct FE_element_merge_into_FE_region_data
/*******************************************************************************
LAST MODIFIED : 26 March 2003

DESCRIPTION :
Data for passing to FE_element_merge_into_FE_region.
==============================================================================*/
{
	struct FE_region *fe_region;
	FE_nodeset *fe_nodeset;
	/* use following array and number to build up matching pairs of old element
		 field info what they become in the global_fe_region.
		 Note these are ACCESSed */
	struct FE_element_field_info **matching_element_field_info;
	int number_of_matching_element_field_info;
};

static int FE_element_merge_into_FE_region(struct FE_element *element,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
FE_element iterator version of FE_region_merge_FE_element. Before merging,
substitutes into <element> an appropriate element field info from <fe_region>,
plus nodes from <fe_region> of the same number as those currently used.
<data_void> points at a struct FE_element_merge_into_FE_region_data.
==============================================================================*/
{
	int element_dimension, i, number_of_faces, number_of_nodes, return_code;
	struct CM_element_information element_identifier;
	struct FE_element *old_face, *other_element, *new_face;
	struct FE_element_merge_into_FE_region_data *data;
	struct FE_element_field_info *current_element_field_info, *element_field_info,
		**matching_element_field_info;
	struct FE_element_shape *element_shape;
	struct FE_node *new_node, *old_node;
	struct FE_region *fe_region;

	ENTER(FE_element_merge_into_FE_region);
	if (element && (current_element_field_info =
		FE_element_get_FE_element_field_info(element)) &&
		get_FE_element_shape(element, &element_shape) &&
		(data = (struct FE_element_merge_into_FE_region_data *)data_void) &&
		(fe_region = data->fe_region))
	{
		element_dimension = get_FE_element_dimension(element);
		if (FE_element_shape_is_unspecified(element_shape))
		{
			/* must find an existing element of the same dimension */
			get_FE_element_identifier(element, &element_identifier);
			other_element = FE_region_get_FE_element_from_identifier(fe_region,
				element_dimension, element_identifier.number);
			if (other_element)
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_merge_into_FE_region.  No matching embedding element");
				return_code = 0;
			}
		}
		else
		{
			return_code = 1;
			/* 1. Convert element to use a new FE_element_field_info from this
				 FE_region */
			/* fast path: check if the element_field_info has already been matched */
			matching_element_field_info = data->matching_element_field_info;
			element_field_info = (struct FE_element_field_info *)NULL;
			for (i = 0; (i < data->number_of_matching_element_field_info) &&
				(!element_field_info); i++)
			{
				if (*matching_element_field_info == current_element_field_info)
				{
					element_field_info = *(matching_element_field_info + 1);
				}
				matching_element_field_info += 2;
			}
			if (!element_field_info)
			{
				element_field_info = FE_region_clone_FE_element_field_info(
					fe_region, current_element_field_info);
				if (element_field_info)
				{
					/* store combination of element field info in matching list */
					if (REALLOCATE(matching_element_field_info,
						data->matching_element_field_info, struct FE_element_field_info *,
						2*(data->number_of_matching_element_field_info + 1)))
					{
						matching_element_field_info
							[data->number_of_matching_element_field_info*2]
							= ACCESS(FE_element_field_info)(current_element_field_info);
						matching_element_field_info
							[data->number_of_matching_element_field_info*2 + 1] =
							ACCESS(FE_element_field_info)(element_field_info);
						data->matching_element_field_info = matching_element_field_info;
						(data->number_of_matching_element_field_info)++;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_element_merge_into_FE_region.  "
						"Could not clone element_field_info");
				}
			}
			if (element_field_info)
			{
				/* substitute the new element field info */
				FE_element_set_FE_element_field_info(element, element_field_info);
				/* substitute global nodes */
				if (get_FE_element_number_of_nodes(element, &number_of_nodes))
				{
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if (get_FE_element_node(element, i, &old_node))
						{
							if (old_node)
							{
								new_node = data->fe_nodeset->findNodeByIdentifier(
									get_FE_node_identifier(old_node));
								if (new_node)
								{
									if (!set_FE_element_node(element, i, new_node))
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
				else
				{
					return_code = 0;
				}
				/* substitute global scale factor set identifiers */
				int number_of_scale_factor_sets = 0;
				if (get_FE_element_number_of_scale_factor_sets(element, &number_of_scale_factor_sets))
				{
					for (i = 0; (i < number_of_scale_factor_sets) && return_code; i++)
					{
						cmzn_mesh_scale_factor_set *source_scale_factor_set =
							get_FE_element_scale_factor_set_identifier_at_index(element, i);
						cmzn_mesh_scale_factor_set *global_scale_factor_set =
							FE_region_find_mesh_scale_factor_set_by_name(fe_region, source_scale_factor_set->getName());
						if (!global_scale_factor_set)
						{
							global_scale_factor_set = FE_region_create_mesh_scale_factor_set(fe_region);
							global_scale_factor_set->setName(source_scale_factor_set->getName());
						}
						set_FE_element_scale_factor_set_identifier_at_index(element, i, global_scale_factor_set);
						cmzn_mesh_scale_factor_set::deaccess(global_scale_factor_set);
					}
				}
				else
				{
					return_code = 0;
				}
				/* substitute global faces */
				if (get_FE_element_number_of_faces(element, &number_of_faces))
				{
					int face_dimension = element_dimension - 1;
					for (i = 0; (i < number_of_faces) && return_code; i++)
					{
						if (get_FE_element_face(element, i, &old_face))
						{
							if (old_face)
							{
								if (get_FE_element_identifier(old_face, &element_identifier) &&
									(new_face = FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)(
										&element_identifier, fe_region->get_element_list(face_dimension))))
								{
									if (!set_FE_element_face(element, i, new_face))
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
				else
				{
					return_code = 0;
				}
				if (!FE_region_merge_FE_element_and_faces_private(data->fe_region, element))
				{
					return_code = 0;
				}
			}
			else
			{
				return_code = 0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_element_merge_into_FE_region.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_merge_into_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_merge_into_FE_region */

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

bool FE_region_can_merge(struct FE_region *target_fe_region,
	struct FE_region *source_fe_region)
{
	if (!target_fe_region || !source_fe_region)
		return false;

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

	// check elements match in shape and faces
	bool result = true;
	for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (0 < dimension) && result; --dimension)
	{
		cmzn_elementiterator *iter = CREATE_LIST_ITERATOR(cmzn_element)(source_fe_region->get_element_list(dimension));
		cmzn_element *element;
		while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
		{
			if (!FE_element_can_be_merged(element, target_fe_region))
			{
				display_message(ERROR_MESSAGE, "Cannot merge element(s) into region due to incompatible shape or faces");
				result = false;
				break;
			}
		}
		cmzn_elementiterator_destroy(&iter);
	}

	return result;
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
			display_message(ERROR_MESSAGE,
				"FE_region_merge.  Could not merge fields");
			return_code = 0;
		}

		// merge nodes (nodesets)
		if (return_code)
		{
			for (int n = 0; n < 2; ++n)
				target_fe_region->nodesets[n]->merge(*(source_fe_region->nodesets[n]), target_root_fe_region);
		}

		/* merge elements */
		if (return_code)
		{
			struct FE_element_merge_into_FE_region_data merge_elements_data;
			/* swap elements for global equivalents */
			merge_elements_data.fe_region = target_fe_region;
			merge_elements_data.fe_nodeset = FE_region_find_FE_nodeset_by_field_domain_type(
				target_fe_region, CMZN_FIELD_DOMAIN_TYPE_NODES);
			/* use following array and number to build up matching pairs of old
				 element field info what they become in the global_fe_region */
			merge_elements_data.matching_element_field_info =
				(struct FE_element_field_info **)NULL;
			merge_elements_data.number_of_matching_element_field_info = 0;
			/* merge elements from lowest to highest dimension so that faces are
				 merged before their parent elements */
			for (int dimension = 1; dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
			{
				if (!FOR_EACH_OBJECT_IN_LIST(FE_element)(
					FE_element_merge_into_FE_region, (void *)&merge_elements_data,
					source_fe_region->get_element_list(dimension)))
				{
					display_message(ERROR_MESSAGE, "FE_region_merge.  Could not merge elements");
					return_code = 0;
					break;
				}
			}
			if (merge_elements_data.matching_element_field_info)
			{
				for (int i = 2*merge_elements_data.number_of_matching_element_field_info - 1; 0 <= i; --i)
					DEACCESS(FE_element_field_info)(&(merge_elements_data.matching_element_field_info[i]));
				DEALLOCATE(merge_elements_data.matching_element_field_info);
			}
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

void FE_region_notify_FE_element_field_change(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *fe_field)
{
	if (fe_region && element && fe_field)
		FE_REGION_FE_ELEMENT_FIELD_CHANGE(fe_region, element, fe_field);
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

struct LIST(FE_element_field_info) *FE_region_get_FE_element_field_info_list_private(
	struct FE_region *fe_region)
{
	if (fe_region)
		return fe_region->fe_element_field_info_list;
	return NULL;
}

struct LIST(FE_element) *FE_region_create_related_element_list_for_dimension(
	struct FE_region *fe_region, int dimension)
{
	if (fe_region)
		return CREATE_RELATED_LIST(FE_element)(fe_region->get_element_list(dimension));
	return 0;
}

void FE_region_list_btree_statistics(struct FE_region *fe_region)
{
	if (fe_region)
	{
		for (int n = 0; n < 2; ++n)
			fe_region->nodesets[n]->list_btree_statistics();
		for (int dimension = 1; dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
		{
			LIST(FE_element) *element_list = fe_region->get_element_list(dimension);
			if (0 < NUMBER_IN_LIST(FE_element)(element_list))
			{
				display_message(INFORMATION_MESSAGE, "%d-D elements:\n",dimension);
				FE_element_list_write_btree_statistics(element_list);
			}
		}
	}
}
