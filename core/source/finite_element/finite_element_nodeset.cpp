/**
 * FILE : finite_element_nodeset.hpp
 *
 * Class defining a domain consisting of a set of nodes.
 */
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
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "general/debug.h"
#include "general/message.h"
#include "general/mystring.h"
#include "general/object.h"

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
	if (fe_node_field_info == this->last_fe_node_field_info)
		this->last_fe_node_field_info = 0;
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

// NOTE! Only to be called by FE_region_clear
// Expects change cache to be on otherwise very inefficient
void FE_nodeset::clear()
{
	cmzn_nodeiterator *iter = this->createNodeiterator();
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
 * Attempts to removes all the nodes in remove_node_list from this nodeset.
 * Nodes can only be removed if not in use by elements in <fe_region>.
 * On return, <remove_node_list> will contain all the nodes that are still in
 * nodeset after the call.
 * A successful return code is only obtained if all nodes from remove_node_list are
 * removed.
 */
int FE_nodeset::remove_FE_node_list(struct LIST(FE_node) *remove_node_list)
{
	int return_code = CMZN_ERROR_ARGUMENT;
	if (remove_node_list)
	{
		return_code = CMZN_OK;
		LIST(FE_node) *exclusion_node_list = CREATE(LIST(FE_node))();
		// since we do not maintain pointers from nodes to elements using them,
		// must iterate over all elements to remove nodes referenced by them
		for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (0 < dimension); --dimension)
		{
			FE_mesh *fe_mesh = FE_region_find_FE_mesh_by_dimension(fe_region, dimension);
			if (!fe_mesh)
			{
				return_code = CMZN_ERROR_GENERAL;
				break;
			}
			cmzn_elementiterator *iter = fe_mesh->createElementiterator();
			cmzn_element *element;
			while (0 != (element = cmzn_elementiterator_next_non_access(iter)) && (CMZN_OK == return_code))
				return_code = cmzn_element_add_stored_nodes_to_list(element, exclusion_node_list, /*onlyFrom*/remove_node_list);
			cmzn_elementiterator_destroy(&iter);
		}
		if (CMZN_OK == return_code)
		{
			/* begin/end change to prevent multiple messages */
			FE_region_begin_change(fe_region);
			cmzn_nodeiterator *iter = CREATE_LIST_ITERATOR(cmzn_node)(remove_node_list);
			cmzn_node *node = 0;
			while ((0 != (node = cmzn_nodeiterator_next_non_access(iter))))
			{
				if (IS_OBJECT_IN_LIST(cmzn_node)(node, exclusion_node_list))
					continue;
				if (REMOVE_OBJECT_FROM_LIST(cmzn_node)(node, this->nodeList))
				{
					// must notify of change before invalidating node otherwise has no fields
					// OK since between begin/end change
					this->nodeRemovedChange(node);
					FE_node_invalidate(node);
				}
				else
				{
					return_code = CMZN_ERROR_GENERAL;
					break;
				}
			}
			cmzn_nodeiterator_destroy(&iter);
			REMOVE_OBJECTS_FROM_LIST_THAT(cmzn_node)(FE_node_is_not_in_list,
				(void *)(this->nodeList), remove_node_list);
			if (0 < NUMBER_IN_LIST(cmzn_node)(remove_node_list))
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

/**
 * Data for passing to FE_nodeset::merge_FE_element_external and
 * FE_field_add_embedded_field_to_array.
 */
struct FE_nodeset::Merge_FE_node_external_data
{
	FE_region *target_fe_region;
	/* use following array and number to build up matching pairs of old node
		 field info what they become in the global_fe_region.
		 Note these are ACCESSed */
	struct FE_node_field_info **matching_node_field_info;
	int number_of_matching_node_field_info;
	/* array of element_xi fields that may be defined on node prior to its being
		 merged; these require substitution of global element pointers */
	int number_of_embedded_fields;
	struct FE_field **embedded_fields;
};

/**
 * If <field> is embedded, ie. returns element_xi, adds it to the embedded_field
 * array in the FE_nodeset::Merge_FE_node_external_data.
 * @param merge_data_void  A struct FE_nodeset::Merge_FE_node_external_data *.
 */
static int FE_field_add_embedded_field_to_array(struct FE_field *field,
	void *merge_data_void)
{
	int return_code;
	FE_nodeset::Merge_FE_node_external_data *merge_data =
		static_cast<FE_nodeset::Merge_FE_node_external_data *>(merge_data_void);
	if (field && merge_data)
	{
		return_code = 1;
		if (ELEMENT_XI_VALUE == get_FE_field_value_type(field))
		{
			struct FE_field **embedded_fields;
			if (REALLOCATE(embedded_fields, merge_data->embedded_fields,
				struct FE_field *, (merge_data->number_of_embedded_fields + 1)))
			{
				/* must be mapped to a global field */
				embedded_fields[merge_data->number_of_embedded_fields] =
					FE_region_merge_FE_field(merge_data->target_fe_region, field);
				merge_data->embedded_fields = embedded_fields;
				merge_data->number_of_embedded_fields++;
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
	return (return_code);
}

/**
 * Specialised version of merge_FE_node for merging nodes from another nodeset,
 * used when reading models from files.
 * Before merging, substitutes into node an appropriate node field info
 * for this nodeset, and corresponding global elements for current elements in
 * embedded element:xi fields.
 */
int FE_nodeset::merge_FE_node_external(struct FE_node *node,
	Merge_FE_node_external_data &data)
{
	int return_code = 1;
	struct FE_node_field_info *current_node_field_info = FE_node_get_FE_node_field_info(node);
	if (current_node_field_info)
	{
		/* 1. Convert node to use a new FE_node_field_info from this FE_region */
		/* fast path: check if the node_field_info has already been assimilated */
		struct FE_node_field_info **matching_node_field_info = data.matching_node_field_info;
		struct FE_node_field_info *node_field_info = 0;
		for (int i = 0; (i < data.number_of_matching_node_field_info); i++)
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
			node_field_info = this->clone_FE_node_field_info(current_node_field_info);
			if (node_field_info)
			{
				/* store combination of node field info in matching list */
				if (REALLOCATE(matching_node_field_info,
					data.matching_node_field_info, struct FE_node_field_info *,
					2*(data.number_of_matching_node_field_info + 1)))
				{
					matching_node_field_info[data.number_of_matching_node_field_info*2] =
						ACCESS(FE_node_field_info)(current_node_field_info);
					matching_node_field_info
						[data.number_of_matching_node_field_info*2 + 1] =
						ACCESS(FE_node_field_info)(node_field_info);
					data.matching_node_field_info = matching_node_field_info;
					(data.number_of_matching_node_field_info)++;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_nodeset::merge_FE_node_external.  Could not clone node_field_info");
			}
		}
		if (node_field_info)
		{
			/* substitute the new node field info */
			FE_node_set_FE_node_field_info(node, node_field_info);
			return_code = 1;
			/* substitute global elements etc. in embedded fields */
			for (int i = 0; i < data.number_of_embedded_fields; i++)
			{
				FE_field *field = data.embedded_fields[i];
				if (FE_field_is_defined_at_node(field, node))
				{
					const int number_of_components = get_FE_field_number_of_components(field);
					for (int component_number = 0; component_number < number_of_components;
						component_number++)
					{
						const int number_of_versions =
							get_FE_node_field_component_number_of_versions(node,field, component_number);
						const int number_of_derivatives =
							get_FE_node_field_component_number_of_derivatives(node, field, component_number);
						FE_nodal_value_type *nodal_value_types =
							get_FE_node_field_component_nodal_value_types(node, field, component_number);
						if (nodal_value_types)
						{
							for (int version_number = 0; version_number < number_of_versions;
								version_number++)
							{
								for (int value_number = 0; value_number <= number_of_derivatives;
								 value_number++)
								{
									FE_element *element;
									FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
									if (get_FE_nodal_element_xi_value(node, field, component_number, version_number,
										nodal_value_types[value_number], &element, xi))
									{
										if (element)
										{
											// find or create equivalent element in mesh for this fe_region
											int dimension = cmzn_element_get_dimension(element);
											FE_mesh *fe_mesh = FE_region_find_FE_mesh_by_dimension(this->fe_region, dimension);
											if (fe_mesh)
											{
												struct FE_element_shape *element_shape = 0;
												get_FE_element_shape(element, &element_shape);
												FE_element *global_element = fe_mesh->get_or_create_FE_element_with_identifier(
													cmzn_element_get_identifier(element), element_shape);
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
			FE_node *merged_node = this->merge_FE_node(node);
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
			display_message(ERROR_MESSAGE, "FE_nodeset::merge_FE_node_external.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::merge_FE_node_external.  Invalid argument(s)");
		return_code = 0;
	}
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

int FE_nodeset::merge(FE_nodeset &source)
{
	int return_code = 1;
	Merge_FE_node_external_data merge_data;
	merge_data.target_fe_region = this->fe_region;
	/* use following array and number to build up matching pairs of old node
		field info what they become in the target_fe_region */
	merge_data.matching_node_field_info = (struct FE_node_field_info **)NULL;
	merge_data.number_of_matching_node_field_info = 0;
	/* work out which fields in source_fe_region are embedded */
	merge_data.embedded_fields = (struct FE_field **)NULL;
	merge_data.number_of_embedded_fields = 0;
	if (!FOR_EACH_OBJECT_IN_LIST(FE_field)(
		FE_field_add_embedded_field_to_array, (void *)&merge_data,
		source.get_FE_region()->fe_field_list))
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::merge.  Could not get embedded fields");
		return_code = 0;
	}

	cmzn_nodeiterator *iter = source.createNodeiterator();
	cmzn_node *node;
	while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
	{
		if (!this->merge_FE_node_external(node, merge_data))
		{
			display_message(ERROR_MESSAGE, "FE_ndoeset::merge.  Could not merge node");
			return_code = 0;
			break;
		}
	}
	cmzn_nodeiterator_destroy(&iter);

	if (merge_data.matching_node_field_info)
	{
		for (int i = 2*merge_data.number_of_matching_node_field_info - 1; 0 <= i; --i)
			DEACCESS(FE_node_field_info)(&(merge_data.matching_node_field_info[i]));
		DEALLOCATE(merge_data.matching_node_field_info);
	}
	if (merge_data.embedded_fields)
		DEALLOCATE(merge_data.embedded_fields);
	return return_code;
}
