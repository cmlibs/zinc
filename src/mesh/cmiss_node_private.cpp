/***************************************************************************//**
 * FILE : cmiss_node_private.cpp
 *
 * Implementation of public interface to cmzn_node.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdarg.h>
#include "cmlibs/zinc/fieldfiniteelement.h"
#include "cmlibs/zinc/fieldmodule.h"
#include "cmlibs/zinc/node.h"
#include "cmlibs/zinc/nodeset.h"
#include "cmlibs/zinc/status.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_module.hpp"
#include "general/debug.h"
#include "general/mystring.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region.h"
#include "general/message.h"
#include "general/enumerator_conversion.hpp"
#include "mesh/cmiss_node_private.hpp"
#include "mesh/nodeset.hpp"
#include "node/node_operations.h"
#include "node/nodetemplate.hpp"
#include <vector>

/*
Global types
------------
*/

cmzn_node_id cmzn_node_access(cmzn_node_id node)
{
	if (node)
		return node->access();
	return 0;
}

int cmzn_node_destroy(cmzn_node_id *node_address)
{
	if (node_address && *node_address)
	{
		cmzn_node::deaccess(*node_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_node_get_identifier(cmzn_node_id node)
{
	return get_FE_node_identifier(node);
}

cmzn_nodeset_id cmzn_node_get_nodeset(cmzn_node_id node)
{
	if (node)
	{
		// handle node being orphaned during clean-up
		FE_nodeset* feNodeset = node->getNodeset();
		if (feNodeset)
		{
			cmzn_region* region = feNodeset->getRegion();
			if (region)
			{
				cmzn_nodeset* nodeset = region->findNodesetByFieldDomainType(feNodeset->getFieldDomainType());
				if (nodeset)
				{
					return nodeset->access();
				}
			}
		}
	}
	return nullptr;
}

int cmzn_node_merge(cmzn_node_id node, cmzn_nodetemplate_id node_template)
{
	if ((node) && (node_template))
	{
		return node_template->mergeIntoNode(node);
	}
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_node_value_label cmzn_node_value_label_enum_from_string(
	const char *name)
{
	return string_to_enum<enum cmzn_node_value_label, cmzn_node_value_label_conversion>(name);
}

char *cmzn_node_value_label_enum_to_string(enum cmzn_node_value_label label)
{
	const char *label_string = cmzn_node_value_label_conversion::to_string(label);
	return (label_string ? duplicate_string(label_string) : 0);
}

cmzn_nodesetchanges::cmzn_nodesetchanges(cmzn_fieldmoduleevent *eventIn, cmzn_nodeset *nodesetIn) :
	event(eventIn->access()),
	changeLog(event->getFeRegionChanges()->getNodeChangeLog(
		nodesetIn->getFeNodeset()->getFieldDomainType())),
	access_count(1)
{
}

cmzn_nodesetchanges::~cmzn_nodesetchanges()
{
	cmzn_fieldmoduleevent::deaccess(this->event);
}

cmzn_nodesetchanges *cmzn_nodesetchanges::create(cmzn_fieldmoduleevent *eventIn, cmzn_nodeset *nodesetIn)
{
	if ((eventIn) && (eventIn->getFeRegionChanges()) && (nodesetIn) &&
		(eventIn->get_FE_region() == nodesetIn->getFeNodeset()->get_FE_region()))
	{
		return new cmzn_nodesetchanges(eventIn, nodesetIn);
	}
	return nullptr;
}

int cmzn_nodesetchanges::deaccess(cmzn_nodesetchanges* &nodesetchanges)
{
	if (nodesetchanges)
	{
		--(nodesetchanges->access_count);
		if (nodesetchanges->access_count <= 0)
			delete nodesetchanges;
		nodesetchanges = 0;
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_node_change_flags cmzn_nodesetchanges::getNodeChangeFlags(cmzn_node *node)
{
	cmzn_node_change_flags change = CMZN_NODE_CHANGE_FLAG_NONE;
	if (node)
	{
		if (this->changeLog->isIndexChange(node->getIndex()))
			change = this->changeLog->getChangeSummary();
	}
	return change;
}

cmzn_nodesetchanges_id cmzn_nodesetchanges_access(
	cmzn_nodesetchanges_id nodesetchanges)
{
	if (nodesetchanges)
		return nodesetchanges->access();
	return 0;
}

int cmzn_nodesetchanges_destroy(cmzn_nodesetchanges_id *nodesetchanges_address)
{
	if (nodesetchanges_address)
		return cmzn_nodesetchanges::deaccess(*nodesetchanges_address);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_node_change_flags cmzn_nodesetchanges_get_node_change_flags(
	cmzn_nodesetchanges_id nodesetchanges, cmzn_node_id node)
{
	if (nodesetchanges && node)
		return nodesetchanges->getNodeChangeFlags(node);
	return CMZN_NODE_CHANGE_FLAG_NONE;
}

int cmzn_nodesetchanges_get_number_of_changes(
	cmzn_nodesetchanges_id nodesetchanges)
{
	if (nodesetchanges)
		return nodesetchanges->getNumberOfChanges();
	return 0;
}

cmzn_node_change_flags cmzn_nodesetchanges_get_summary_node_change_flags(
	cmzn_nodesetchanges_id nodesetchanges)
{
	if (nodesetchanges)
		return nodesetchanges->getSummaryNodeChangeFlags();
	return CMZN_NODE_CHANGE_FLAG_NONE;
}
