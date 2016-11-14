/**
 * FILE : finite_element_nodeset.cpp
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

FE_node_template::FE_node_template(FE_nodeset *nodeset_in, struct FE_node_field_info *node_field_info) :
	cmzn::RefCounted(),
	nodeset(nodeset_in->access()),
	template_node(create_template_FE_node(node_field_info))
{
}

FE_node_template::FE_node_template(FE_nodeset *nodeset_in, struct FE_node *node) :
	cmzn::RefCounted(),
	nodeset(nodeset_in->access()),
	template_node(create_FE_node_from_template(DS_LABEL_INDEX_INVALID, node))
{
}

FE_node_template::~FE_node_template()
{
	FE_nodeset::deaccess(this->nodeset);
	DEACCESS(FE_node)(&(this->template_node));
}

FE_nodeset::FE_nodeset(FE_region *fe_region) :
	fe_region(fe_region),
	domainType(CMZN_FIELD_DOMAIN_TYPE_INVALID),
	node_field_info_list(CREATE(LIST(FE_node_field_info))()),
	last_fe_node_field_info(0),
	changeLog(0),
	activeNodeIterators(0),
	access_count(1)
{
	this->createChangeLog();
}

FE_nodeset::~FE_nodeset()
{
	cmzn::Deaccess(this->changeLog);
	this->last_fe_node_field_info = 0;

	// remove pointers to this FE_nodeset as destroying
	cmzn_nodeiterator *nodeIterator = this->activeNodeIterators;
	while (nodeIterator)
	{
		nodeIterator->invalidate();
		nodeIterator = nodeIterator->nextIterator;
	}

	this->clear();

	FOR_EACH_OBJECT_IN_LIST(FE_node_field_info)(
		FE_node_field_info_clear_FE_nodeset, (void *)NULL,
		this->node_field_info_list);
	DESTROY(LIST(FE_node_field_info))(&(this->node_field_info_list));
}

/** Private: assumes current change log pointer is null or invalid */
void FE_nodeset::createChangeLog()
{
	cmzn::Deaccess(this->changeLog);
	this->changeLog = DsLabelsChangeLog::create(&this->labels);
	if (!this->changeLog)
		display_message(ERROR_MESSAGE, "FE_nodeset::createChangeLog.  Failed to create change log");
	this->last_fe_node_field_info = 0;
}

DsLabelsChangeLog *FE_nodeset::extractChangeLog()
{
	DsLabelsChangeLog *returnChangeLog = this->changeLog;
	this->createChangeLog();
	return returnChangeLog;
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

/** Assumes called by FE_region destructor, and change notification is disabled. */
void FE_nodeset::detach_from_FE_region()
{
	// clear embedded locations to avoid circular dependencies which prevent cleanup
	FE_nodeset_clear_embedded_locations(this, this->fe_region->fe_field_list);
	this->fe_region = 0;
}

/** Remove iterator from linked list in this nodeset */
void FE_nodeset::removeNodeiterator(cmzn_nodeiterator *iterator)
{
	if (iterator == this->activeNodeIterators)
		this->activeNodeIterators = iterator->nextIterator;
	else
	{
		cmzn_nodeiterator *prevIterator = this->activeNodeIterators;
		while (prevIterator && (prevIterator->nextIterator != iterator))
			prevIterator = prevIterator->nextIterator;
		if (prevIterator)
			prevIterator->nextIterator = iterator->nextIterator;
		else
			display_message(ERROR_MESSAGE, "FE_nodeset::removeNodeiterator.  Iterator not in linked list");
	}
	iterator->nextIterator = 0;
}

/**
* Create a node iterator object for iterating through the nodes of the
* nodeset. The iterator initially points at the position before the first node.
* @param labelsGroup  Optional group to iterate over.
* @return  Handle to node iterator at position before first, or NULL if error.
*/
cmzn_nodeiterator *FE_nodeset::createNodeiterator(DsLabelsGroup *labelsGroup)
{
	DsLabelIterator *labelIterator = labelsGroup ? labelsGroup->createLabelIterator() : this->labels.createLabelIterator();
	if (!labelIterator)
		return 0;
	cmzn_nodeiterator *iterator = new cmzn_nodeiterator(this, labelIterator);
	if (iterator)
	{
		iterator->nextIterator = this->activeNodeIterators;
		this->activeNodeIterators = iterator;
	}
	else
		cmzn::Deaccess(labelIterator);
	return iterator;
}

DsLabelsGroup *FE_nodeset::createLabelsGroup()
{
	return DsLabelsGroup::create(&this->labels); // GRC dodgy taking address here
}

/**
* Call this to mark node with the supplied change.
* Notifies change to clients of FE_region.
*/
void FE_nodeset::nodeChange(DsLabelIndex nodeIndex, int change)
{
	if (this->fe_region && this->changeLog)
	{
		this->changeLog->setIndexChange(nodeIndex, change);
		this->fe_region->update();
	}
}

/**
 * Call this to mark node with the supplied change, logging field changes
 * from the field_info_node in the fe_region.
 * Notifies change to clients of FE_region.
 * When a node is added or removed, the same node is used for <nodeIndex> and
 * <field_info_node>. For changes to the contents of <node>, <field_info_node>
 * should contain the changed fields, consistent with merging it into <node>.
*/
void FE_nodeset::nodeChange(DsLabelIndex nodeIndex, int change, FE_node *field_info_node)
{
	if (this->fe_region && this->changeLog && field_info_node)
	{
		this->changeLog->setIndexChange(nodeIndex, change);
		// for efficiency, the following marks field changes only if field info is different from last
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
 * Call this instead of nodeChange when exactly one field, <fe_field> of
 * <node> has changed.
 */
void FE_nodeset::nodeFieldChange(FE_node *node, FE_field *fe_field)
{
	if (this->fe_region && this->changeLog)
	{
		this->changeLog->setIndexChange(get_FE_node_index(node), DS_LABEL_CHANGE_TYPE_RELATED);
		CHANGE_LOG_OBJECT_CHANGE(FE_field)(this->fe_region->fe_field_changes,
			fe_field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
		this->fe_region->update();
	}
}

bool FE_nodeset::is_FE_field_in_use(struct FE_field *fe_field)
{
	if (FIRST_OBJECT_IN_LIST_THAT(FE_node_field_info)(
		FE_node_field_info_has_FE_field, (void *)fe_field,
		this->node_field_info_list))
	{
		/* since nodes may still exist in the change_log,
		   must now check that no remaining nodes use fe_field */
		/* for now, if there are nodes then fe_field is in use */
		if (0 < this->getSize())
			return true;
	}
	return false;
}

// @return  Number of element references to node at nodeIndex
int FE_nodeset::getElementUsageCount(DsLabelIndex nodeIndex)
{
	if (nodeIndex >= 0)
	{
		ElementUsageCountType *elementUsageCountAddress = this->elementUsageCount.getAddress(nodeIndex);
		if (elementUsageCountAddress)
			return static_cast<int>(*elementUsageCountAddress);
	}
	return 0;
}

// call when start using a node in an element, so we know it's in use
void FE_nodeset::incrementElementUsageCount(DsLabelIndex nodeIndex)
{
	if (nodeIndex >= 0)
	{
		ElementUsageCountType *elementUsageCountAddress = this->elementUsageCount.getAddress(nodeIndex);
		if (elementUsageCountAddress)
			++(*elementUsageCountAddress);
		else
			this->elementUsageCount.setValue(nodeIndex, 1);
	}
}

// call when end using a node in an element, to record no longer in use
void FE_nodeset::decrementElementUsageCount(DsLabelIndex nodeIndex)
{
	if (nodeIndex >= 0)
	{
		ElementUsageCountType *elementUsageCountAddress = this->elementUsageCount.getAddress(nodeIndex);
		if (elementUsageCountAddress)
		{
			if (*elementUsageCountAddress > 0)
				--(*elementUsageCountAddress);
			else
				display_message(ERROR_MESSAGE, "FE_nodeset::decrementElementUsageCount.  Count is already 0");
		}
		else
			display_message(ERROR_MESSAGE, "FE_nodeset::decrementElementUsageCount.  Missing count");
	}
}

// Only to be called by FE_region_clear, or when all nodeset already removed
// to reclaim memory in labels and mapped arrays
void FE_nodeset::clear()
{
	if (0 < this->labels.getSize())
	{
		const DsLabelIndex indexLimit = this->labels.getIndexSize();
		FE_node *node;
		for (DsLabelIndex index = 0; index < indexLimit; ++index)
		{
			if (this->fe_nodes.getValue(index, node) && (node))
			{
				// must invalidate nodes in case elements holding on to them
				// BUT! Don't invalidate nodes that have been merged into another region
				if (FE_node_get_FE_nodeset(node) == this)
					FE_node_invalidate(node);
				DEACCESS(FE_node)(&node);
			}
		}
	}
	this->fe_nodes.clear();
	this->labels.clear();
}

int FE_nodeset::change_FE_node_identifier(struct FE_node *node, int new_identifier)
{
	if ((FE_node_get_FE_nodeset(node) == this) && (new_identifier >= 0))
	{
		const DsLabelIndex nodeIndex = get_FE_node_index(node);
		const DsLabelIdentifier currentIdentifier = this->getNodeIdentifier(nodeIndex);
		if (currentIdentifier >= 0)
		{
			int return_code = this->labels.setIdentifier(nodeIndex, new_identifier);
			if (return_code == CMZN_OK)
				this->nodeIdentifierChange(node);
			else if (return_code == CMZN_ERROR_ALREADY_EXISTS)
				display_message(ERROR_MESSAGE, "FE_nodeset::change_FE_node_identifier.  Identifier %d is already used in nodeset",
					new_identifier);
			else
				display_message(ERROR_MESSAGE, "FE_nodeset::change_FE_node_identifier.  Failed to set label identifier");
			return return_code;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset::change_FE_node_identifier.  node is not in this nodeset");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::change_FE_node_identifier.  Invalid argument(s)");
	}
	return CMZN_ERROR_ARGUMENT;
}

/** Creates a template that is a copy of the existing node */
FE_node_template *FE_nodeset::create_FE_node_template(FE_node *node)
{
	if (FE_node_get_FE_nodeset(node) != this)
		return 0;
	FE_node_template *node_template = new FE_node_template(this, node);
	return node_template;
}

FE_node_template *FE_nodeset::create_FE_node_template()
{
	FE_node_template *node_template = new FE_node_template(this,
		this->get_FE_node_field_info(/*number_of_values*/0, (struct LIST(FE_node_field) *)NULL));
	return node_template;
}

/**
 * Convenience function returning an existing node with the identifier from
 * the nodeset, or if none found or if identifier is -1, a new node with
 * with the identifier (or the first available identifier if -1).
 * It is expected that the calling function has wrapped calls to this function
 * with FE_region_begin/end_change.
 * @return  Accessed node, or 0 on error.
 */
FE_node *FE_nodeset::get_or_create_FE_node_with_identifier(int identifier)
{
	struct FE_node *node = 0;
	if (-1 <= identifier)
	{
		if (identifier >= 0)
			node = this->findNodeByIdentifier(identifier);
		if (node)
		{
			ACCESS(FE_node)(node);
		}
		else
		{
			FE_node_template *node_template = this->create_FE_node_template();
			node = this->create_FE_node(identifier, node_template);
			cmzn::Deaccess(node_template);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset::get_or_create_FE_node_with_identifier.  Invalid argument(s)");
	}
	return (node);
}

/**
* Checks the node_template is compatible with nodeset & that there is no
* existing node of supplied identifier, then creates node of that
* identifier as a copy of node_template and adds it to the nodeset.
*
* @param identifier  Non-negative integer identifier of new node, or -1 to
* automatically generate (starting at 1). Fails if supplied identifier already
* used by an existing node.
* @return  Accessed node, or 0 on error.
*/
FE_node *FE_nodeset::create_FE_node(int identifier, FE_node_template *node_template)
{
	struct FE_node *new_node = 0;
	if ((-1 <= identifier) && node_template)
	{
		if (node_template->nodeset == this)
		{
			DsLabelIndex nodeIndex = (identifier < 0) ? this->labels.createLabel() : this->labels.createLabel(identifier);
			if (nodeIndex >= 0)
			{
				new_node = ::create_FE_node_from_template(nodeIndex, node_template->get_template_node());
				if ((new_node) && this->fe_nodes.setValue(nodeIndex, new_node))
				{
					ACCESS(FE_node)(new_node);
					this->nodeAddedChange(new_node);
				}
				else
				{
					display_message(ERROR_MESSAGE, "FE_nodeset::create_FE_node.  Failed to add node to list.");
					DEACCESS(FE_node)(&new_node);
					this->labels.removeLabel(nodeIndex);
				}
			}
			else
			{
				if (this->labels.findLabelByIdentifier(identifier) >= 0)
					display_message(ERROR_MESSAGE, "FE_nodeset::create_FE_node.  Identifier %d is already used in nodeset.",
						identifier);
				else
					display_message(ERROR_MESSAGE, "FE_nodeset::create_FE_node.  Could not create label");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset::create_FE_node.  node template is incompatible with nodeset");
		}
	}
	return (new_node);
}

/**
 * Merge fields and other data from source node into destination.
 * Both nodes must be of this nodeset.
 * @return  Status code.
 */
int FE_nodeset::merge_FE_node_existing(struct FE_node *destination, struct FE_node *source)
{
	if (destination && source)
	{
		if ((FE_node_get_FE_nodeset(destination) == this) &&
			(FE_node_get_FE_nodeset(source) == this))
		{
			if (::merge_FE_node(destination, source))
			{
				this->nodeChange(get_FE_node_index(destination), DS_LABEL_CHANGE_TYPE_RELATED, source);
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

int FE_nodeset::merge_FE_node_template(struct FE_node *destination, FE_node_template *fe_node_template)
{
	if (fe_node_template)
		return this->merge_FE_node_existing(destination, fe_node_template->get_template_node());
	return CMZN_ERROR_ARGUMENT;
}

/**
 *@return  Result code OK if undefined CMZN_NOT_FOUND if field undefined or
 *any other error if failed.
 */
int FE_nodeset::undefineFieldAtNode(struct FE_node *node, struct FE_field *fe_field)
{
	int result = ::undefine_FE_field_at_node(node, fe_field);
	if (result == CMZN_OK)
		this->nodeFieldChange(node, fe_field);
	return result;
}

/**
 * Calls <iterator_function> with <user_data> for each FE_node in <region>.
 */
int FE_nodeset::for_each_FE_node(LIST_ITERATOR_FUNCTION(FE_node) iterator_function, void *user_data_void)
{
	DsLabelIterator *iter = this->labels.createLabelIterator();
	if (!iter)
		return 0;
	int return_code = 1;
	DsLabelIndex nodeIndex;
	FE_node *node;
	while ((nodeIndex = iter->nextIndex()) != DS_LABEL_INDEX_INVALID)
	{
		node = this->getNode(nodeIndex);
		if (!node)
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::for_each_FE_node.  No node at index");
			return_code = 0;
			break;
		}
		if (!iterator_function(node, user_data_void))
		{
			return_code = 0;
			break;
		}
	}
	cmzn::Deaccess(iter);
	return return_code;
}

/**
 * Removes node from the nodeset if not in use by elements.
 * Private function which avoids checking whether node is in nodeset.
 * @return OK on success ERROR_IN_USE if in use by element
 */
int FE_nodeset::remove_FE_node_private(struct FE_node *node)
{
	const DsLabelIndex nodeIndex = get_FE_node_index(node);
	if (this->getElementUsageCount(nodeIndex) > 0)
		return CMZN_ERROR_IN_USE;
	// must notify of change before invalidating node otherwise has no fields
	// assumes within begin/end change
	this->nodeRemovedChange(node);
	// clear FE_node entry but deaccess at end of this function
	this->fe_nodes.setValue(nodeIndex, 0);
	FE_node_invalidate(node);
	this->labels.removeLabel(nodeIndex);
	DEACCESS(FE_node)(&node);
	return CMZN_OK;
}

/**
 * Destroys node if not in use by elements.
 * @return OK on success ERROR_IN_USE if in use by element,
 * or other error if more serious failure.
 */
int FE_nodeset::destroyNode(struct FE_node *node)
{
	if (this->containsNode(node))
	{
		FE_region_begin_change(this->fe_region);
		const int result = this->remove_FE_node_private(node);
		if (0 == this->labels.getSize())
			this->clear();
		FE_region_end_change(this->fe_region);
		return result;
	}
	display_message(ERROR_MESSAGE, "FE_nodeset::destroyNode.  Invalid argument(s)");
	return CMZN_ERROR_ARGUMENT;
}

/**
 * Destroy all nodes not in use by elements.
 * @return OK on success ERROR_IN_USE if any not removed because in use by
 * elements, or other error if more serious failure.
 */
int FE_nodeset::destroyAllNodes()
{
	int return_code = CMZN_OK;
	FE_region_begin_change(this->fe_region);
	// can't use an iterator as invalidated when node removed
	const DsLabelIndex indexLimit = this->labels.getIndexSize();
	FE_node *node;
	const bool contiguous = this->labels.isContiguous();
	for (DsLabelIndex index = 0; index < indexLimit; ++index)
	{
		// must handle holes left in identifier array by deleted nodes
		if (contiguous || (DS_LABEL_IDENTIFIER_INVALID != this->getNodeIdentifier(index)))
		{
			node = this->getNode(index);
			if (!node)
			{
				display_message(WARNING_MESSAGE, "FE_nodeset::destroyAllNodes.  No node at index");
				continue;
			}
			int result = this->remove_FE_node_private(node);
			if (result != CMZN_OK)
			{
				return_code = result;
				if (result != CMZN_ERROR_IN_USE)
					break;
			}
		}
	}
	if (0 == this->labels.getSize())
		this->clear();
	FE_region_end_change(this->fe_region);
	return (return_code);
}

/**
 * Destroy all the nodes in labelsGroup not in use by elements.
 * @return OK on success ERROR_IN_USE if any not removed because in use by
 * elements, or other error if more serious failure.
 */
int FE_nodeset::destroyNodesInGroup(DsLabelsGroup& labelsGroup)
{
	int return_code = CMZN_OK;
	FE_region_begin_change(this->fe_region);
	// can't use an iterator as invalidated when node removed
	DsLabelIndex index = -1; // DS_LABEL_INDEX_INVALID
	FE_node *node;
	while (labelsGroup.incrementIndex(index))
	{
		node = this->getNode(index);
		if (!node)
		{
			display_message(WARNING_MESSAGE, "FE_nodeset::destroyNodesInGroup.  No node at index");
			continue;
		}
		int result = this->remove_FE_node_private(node);
		if (result != CMZN_OK)
		{
			return_code = result;
			if (result != CMZN_ERROR_IN_USE)
				break;
		}
	}
	if (0 == this->labels.getSize())
		this->clear();
	FE_region_end_change(this->fe_region);
	return (return_code);
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
	if (this->labels.getSize() > 0)
	{
		if (this->domainType == CMZN_FIELD_DOMAIN_TYPE_NODES)
			display_message(INFORMATION_MESSAGE, "Nodes:\n");
		else if (this->domainType == CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS)
			display_message(INFORMATION_MESSAGE, "Datapoints:\n");
		else
			display_message(INFORMATION_MESSAGE, "General nodeset:\n");
		this->labels.list_storage_details();
	}
}

/**
 * Data for passing to FE_nodeset::merge_FE_node_external.
 */
struct FE_nodeset::Merge_FE_node_external_data
{
	FE_nodeset &source;
	FE_region *target_fe_region; // not accessed; needed to convert embedded locations
	/* use following array and number to build up matching pairs of old node
	   field info what they become in the global_fe_region.
	   Note these are ACCESSed */
	struct FE_node_field_info **matching_node_field_info;
	int number_of_matching_node_field_info;
	/* array of element_xi fields that may be defined on node prior to its being
	   merged; these require substitution of global element pointers */
	std::vector<FE_field *> embedded_fields;

	Merge_FE_node_external_data(FE_nodeset &sourceIn, FE_region *target_fe_region_in) :
		source(sourceIn),
		target_fe_region(target_fe_region_in),
		matching_node_field_info(0),
		number_of_matching_node_field_info(0)
	{
	}

	~Merge_FE_node_external_data()
	{
		if (this->matching_node_field_info)
		{
			for (int i = 2*this->number_of_matching_node_field_info - 1; 0 <= i; --i)
				DEACCESS(FE_node_field_info)(&(this->matching_node_field_info[i]));
			DEALLOCATE(this->matching_node_field_info);
		}
	}

	bool addEmbeddedField(struct FE_field *sourceField)
	{
		/* must be mapped to a global field */
		FE_field *targetField = FE_region_merge_FE_field(this->target_fe_region, sourceField);
		if (!targetField)
			return false;
		this->embedded_fields.push_back(targetField);
		return true;
	}

	/**
	 * If <field> is embedded, ie. returns element_xi, adds it to the embedded_field
	 * array in the FE_nodeset::Merge_FE_node_external_data.
	 * @param merge_data_void  A struct FE_nodeset::Merge_FE_node_external_data *.
	 */
	static int addEmbeddedFieldIterator(struct FE_field *field,
		void *merge_data_void)
	{
		FE_nodeset::Merge_FE_node_external_data *merge_data =
			static_cast<FE_nodeset::Merge_FE_node_external_data *>(merge_data_void);
		if (ELEMENT_XI_VALUE == get_FE_field_value_type(field))
		{
			if (!merge_data->addEmbeddedField(field))
				return 0;
		}
		return 1;
	}

	bool findEmbeddedFields()
	{
		if (FOR_EACH_OBJECT_IN_LIST(FE_field)(addEmbeddedFieldIterator,
			(void *)this, source.get_FE_region()->fe_field_list))
		{
			return true;
		}
		display_message(ERROR_MESSAGE,
			"FE_nodeset::Merge_FE_node_external_data::findEmbeddedFields.  Could not find embedded fields");
		return false;
	}

	/**
	 * @return  Target node field info if match, 0 if don't have a match for source.
	 */
	FE_node_field_info *get_matching_FE_node_field_info(FE_node_field_info *source_node_field_info)
	{
		FE_node_field_info **matching_node_field_info = this->matching_node_field_info;
		for (int i = 0; i < this->number_of_matching_node_field_info; ++i)
		{
			if (*matching_node_field_info == source_node_field_info)
				return *(matching_node_field_info + 1);
			matching_node_field_info += 2;
		}
		return 0;
	}

	/**
	 * Record match between source_node_field_info and target_node_field_info.
	 * @return  true on success.
	 */
	bool add_matching_FE_node_field_info(
		FE_node_field_info *source_node_field_info, FE_node_field_info *target_node_field_info)
	{
		FE_node_field_info **matching_node_field_info;
		if (REALLOCATE(matching_node_field_info,
			this->matching_node_field_info, struct FE_node_field_info *,
			2*(this->number_of_matching_node_field_info + 1)))
		{
			matching_node_field_info[this->number_of_matching_node_field_info*2] =
				ACCESS(FE_node_field_info)(source_node_field_info);
			matching_node_field_info[this->number_of_matching_node_field_info*2 + 1] =
				ACCESS(FE_node_field_info)(target_node_field_info);
			this->matching_node_field_info = matching_node_field_info;
			++(this->number_of_matching_node_field_info);
			return true;
		}
		display_message(ERROR_MESSAGE,
			"FE_nodeset::Merge_FE_node_external_data::add_matching_FE_node_field_info.  Failed");
		return false;
	}

};

/**
 * Merge node from another nodeset, used when reading models from files into
 * temporary regions.
 * Before merging, substitutes into node an appropriate node field info
 * for this nodeset, and corresponding global elements for current elements in
 * embedded element:xi fields.
 * Since this changes information in the node the caller is required to
 * destroy the source nodeset immediately after calling this function on any
 * nodes from it. Operations such as findNodeByIdentifier will no longer
 * work as the node is given a new index for this nodeset.
 */
int FE_nodeset::merge_FE_node_external(struct FE_node *node,
	Merge_FE_node_external_data &data)
{
	int return_code = 1;
	struct FE_node_field_info *old_node_field_info = FE_node_get_FE_node_field_info(node);
	if (old_node_field_info)
	{
		const DsLabelIndex sourceNodeIndex = get_FE_node_index(node);
		const DsLabelIdentifier identifier = get_FE_node_identifier(node);
		FE_node *global_node = this->findNodeByIdentifier(identifier);
		const DsLabelIndex newNodeIndex = (global_node) ? get_FE_node_index(global_node) :
			this->labels.createLabel(identifier);
		if (newNodeIndex < 0)
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::merge_FE_node_external.  Failed to get node label.");
			return 0;
		}

		// 1. Convert node to use a new FE_node_field_info from this nodeset
		FE_node_field_info *node_field_info = data.get_matching_FE_node_field_info(old_node_field_info);
		if (node_field_info)
			ACCESS(FE_node_field_info)(node_field_info);
		else
		{
			node_field_info = this->clone_FE_node_field_info(old_node_field_info);
			if (node_field_info)
			{
				if (!data.add_matching_FE_node_field_info(old_node_field_info, node_field_info))
				{
					DEACCESS(FE_node_field_info)(&node_field_info);
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
			for (int i = 0; i < data.embedded_fields.size(); i++)
			{
				FE_field *field = data.embedded_fields[i];
				if (FE_field_is_defined_at_node(field, node))
				{
					const int number_of_components = get_FE_field_number_of_components(field);
					for (int component_number = 0; component_number < number_of_components;
						component_number++)
					{
						const int number_of_versions =
							get_FE_node_field_component_number_of_versions(node, field, component_number);
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
												FE_element_shape *element_shape = get_FE_element_shape(element);
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
													DEACCESS(FE_element)(&global_element);
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
			if (return_code)
			{
				if (global_node)
					ACCESS(FE_node_field_info)(old_node_field_info);
				set_FE_node_index(node, newNodeIndex);
				if (global_node)
				{
					if (this->merge_FE_node_existing(global_node, node) != CMZN_OK)
						return_code = 0;
					// must restore the previous information for clean-up
					FE_node_set_FE_node_field_info(node, old_node_field_info);
					DEACCESS(FE_node_field_info)(&old_node_field_info);
					set_FE_node_index(node, sourceNodeIndex);
				}
				else
				{
					if (this->fe_nodes.setValue(newNodeIndex, node))
					{
						ACCESS(FE_node)(node);
						this->nodeAddedChange(node);
					}
					else
					{
						display_message(ERROR_MESSAGE, "FE_nodeset::merge_FE_node_external.  Failed to add node to list.");
						this->labels.removeLabel(newNodeIndex);
						return_code = 0;
					}
				}
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

/*
 * Check that definition of fields in nodes of source_fe_region match that
 * of equivalent fields in equivalent nodes of global_fe_region
 * ???GRC enforcing this is actually a limitation.
 */
bool FE_nodeset::canMerge(FE_nodeset &source)
{
	bool result = true;

	/* for efficiency, pairs of FE_node_field_info from the source and target nodes
	   are stored if compatible to avoid checks later */
	int number_of_compatible_node_field_info = 0;
	struct FE_node_field_info **compatible_node_field_info = 0; // stores pairs: source, target

	struct FE_node_field_info **infoAddress, *sourceInfo, *targetInfo;
	cmzn_nodeiterator *iter = source.createNodeiterator();
	cmzn_node *sourceNode;
	while (0 != (sourceNode = cmzn_nodeiterator_next_non_access(iter)))
	{
		const DsLabelIdentifier identifier = get_FE_node_identifier(sourceNode);
		cmzn_node *targetNode = this->findNodeByIdentifier(identifier);
		if (!targetNode)
			continue;
		// fast path: check if the node_field_info have already been proven compatible */
		sourceInfo = FE_node_get_FE_node_field_info(sourceNode);
		targetInfo = FE_node_get_FE_node_field_info(targetNode);
		infoAddress = compatible_node_field_info;
		bool compatible = false;
		for (int i = 0; i < number_of_compatible_node_field_info; ++i)
		{
			if ((*infoAddress == sourceInfo) && (*(infoAddress + 1) == targetInfo))	
			{
				compatible = true;
				break;
			}
			infoAddress += 2;
		}
		if (compatible)
			continue;
		// slow full check:
		if (!FE_node_can_merge(targetNode, sourceNode))
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::canMerge.  Nodes are not compatible");
			result = false;
			break;
		}
		/* store combination of source and target info for fast check in future */
		if (REALLOCATE(infoAddress, compatible_node_field_info,
			struct FE_node_field_info *,  2*(number_of_compatible_node_field_info + 1)))
		{
			compatible_node_field_info = infoAddress;
			compatible_node_field_info[number_of_compatible_node_field_info*2] = sourceInfo;
			compatible_node_field_info[number_of_compatible_node_field_info*2 + 1] = targetInfo;
			++number_of_compatible_node_field_info;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset::canMerge.  Could not reallocate compatible_node_field_info");
			result = false;
			break;
		}
	}
	cmzn_nodeiterator_destroy(&iter);
	if (compatible_node_field_info)
		DEALLOCATE(compatible_node_field_info);
	return result;
}

int FE_nodeset::merge(FE_nodeset &source)
{
	Merge_FE_node_external_data merge_data(source, this->fe_region);
	if (!merge_data.findEmbeddedFields())
		return 0;
	int return_code = 1;
	cmzn_nodeiterator *iter = source.createNodeiterator();
	cmzn_node *node;
	while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
	{
		if (!this->merge_FE_node_external(node, merge_data))
		{
			display_message(ERROR_MESSAGE, "FE_nodeset::merge.  Could not merge node");
			return_code = 0;
			break;
		}
	}
	cmzn_nodeiterator_destroy(&iter);
	return return_code;
}
