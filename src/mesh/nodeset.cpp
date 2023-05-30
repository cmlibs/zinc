/**
 * FILE : nodeset.cpp
 *
 * Nodeset implementation.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "general/mystring.h"
#include "computed_field/computed_field.h"
#include "computed_field/field_cache.hpp"
#include "computed_field/field_module.hpp"
#include "finite_element/finite_element_region_private.h"
#include "mesh/nodeset.hpp"
#include "node/nodetemplate.hpp"
#include "region/cmiss_region.hpp"


void cmzn_nodeset::deaccess(cmzn_nodeset*& nodeset)
{
	if (nodeset)
	{
		--(nodeset->access_count);
		if (nodeset->access_count <= 0)
		{
			delete nodeset;
		}
		nodeset = nullptr;
	}
}

cmzn_node* cmzn_nodeset::createNode(int identifier, cmzn_nodetemplate* nodetemplate)
{
	// GRC check:
	if (nodetemplate->validate())
	{
		return this->feNodeset->create_FE_node(identifier, nodetemplate->get_FE_node_template());
	}
	else
	{
		display_message(ERROR_MESSAGE, "Nodeset createNode.  Node template is not valid");
	}
	return nullptr;
}

cmzn_nodetemplate* cmzn_nodeset::createNodetemplate() const
{
	return cmzn_nodetemplate::create(this->feNodeset);
}

int cmzn_nodeset::destroyNodesConditional(cmzn_field* conditional_field)
{
	if (!conditional_field)
	{
		return CMZN_ERROR_ARGUMENT;
	}

	DsLabelsGroup* tmpLabelsGroup = this->feNodeset->createLabelsGroup();
	if (!tmpLabelsGroup)
	{
		return CMZN_ERROR_GENERAL;
	}
	cmzn_region* region = FE_region_get_cmzn_region(this->feNodeset->get_FE_region());
	cmzn_fieldcache* fieldcache = cmzn_fieldcache::create(region);
	cmzn_nodeiterator* iterator = this->createNodeiterator();
	cmzn_node* node = nullptr;
	while ((node = cmzn_nodeiterator_next_non_access(iterator)))
	{
		cmzn_fieldcache_set_node(fieldcache, node);
		if (cmzn_field_evaluate_boolean(conditional_field, fieldcache))
		{
			tmpLabelsGroup->setIndex(node->getIndex(), true);
		}
	}
	cmzn::Deaccess(iterator);
	cmzn_fieldcache::deaccess(fieldcache);
	int return_code = this->feNodeset->destroyNodesInGroup(*tmpLabelsGroup);
	cmzn::Deaccess(tmpLabelsGroup);
	return return_code;
}

char* cmzn_nodeset::getName() const
{
	return duplicate_string(this->feNodeset->getName());
}

cmzn_nodeset* cmzn_nodeset::getMasterNodeset() const
{
	cmzn_region* region = FE_region_get_cmzn_region(this->feNodeset->get_FE_region()); // not accessed
	return region->findNodesetByFieldDomainType(this->feNodeset->getFieldDomainType());
}

bool cmzn_nodeset::hasMembershipChanges() const
{
	return this->feNodeset->hasMembershipChanges();
}

cmzn_region* cmzn_nodeset::getRegion() const
{
	// gracefully handle FE_nodeset being orphaned at cleanup time
	FE_region* feRegion = this->feNodeset->get_FE_region();
	if (feRegion)
	{
		return feRegion->getRegion();
	}
	return nullptr;

}

/*
Global functions
----------------
*/

cmzn_nodeset_id cmzn_fieldmodule_find_nodeset_by_field_domain_type(
	cmzn_fieldmodule_id fieldmodule, enum cmzn_field_domain_type domain_type)
{
	if (fieldmodule)
	{
		cmzn_nodeset* nodeset = cmzn_fieldmodule_get_region_internal(fieldmodule)->findNodesetByFieldDomainType(domain_type);
		if (nodeset)
		{
			return nodeset->access();
		}
	}
	return nullptr;
}

cmzn_nodeset_id cmzn_fieldmodule_find_nodeset_by_name(
	cmzn_fieldmodule_id fieldmodule, const char* nodeset_name)
{
	if ((fieldmodule) && (nodeset_name))
	{
		cmzn_nodeset* nodeset = cmzn_fieldmodule_get_region_internal(fieldmodule)->findNodesetByName(nodeset_name);
		if (nodeset)
		{
			return nodeset->access();
		}
	}
	return nullptr;
}

cmzn_nodeset_id cmzn_nodeset_access(cmzn_nodeset_id nodeset)
{
	if (nodeset)
	{
		return nodeset->access();
	}
	return nullptr;
}

int cmzn_nodeset_destroy(cmzn_nodeset_id* nodeset_address)
{
	if (nodeset_address)
	{
		cmzn_nodeset::deaccess(*nodeset_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_nodeset_contains_node(cmzn_nodeset_id nodeset, cmzn_node_id node)
{
	if (nodeset && node)
	{
		return nodeset->containsNode(node);
	}
	return false;
}

cmzn_nodetemplate_id cmzn_nodeset_create_nodetemplate(
	cmzn_nodeset_id nodeset)
{
	if (nodeset)
	{
		return nodeset->createNodetemplate();
	}
	return nullptr;
}

cmzn_node_id cmzn_nodeset_create_node(cmzn_nodeset_id nodeset,
	int identifier, cmzn_nodetemplate_id node_template)
{
	if (nodeset && node_template)
	{
		return nodeset->createNode(identifier, node_template);
	}
	return nullptr;
}

cmzn_nodeiterator_id cmzn_nodeset_create_nodeiterator(
	cmzn_nodeset_id nodeset)
{
	if (nodeset)
	{
		return nodeset->createNodeiterator();
	}
	return nullptr;
}

cmzn_node_id cmzn_nodeset_find_node_by_identifier(cmzn_nodeset_id nodeset,
	int identifier)
{
	if (nodeset)
	{
		cmzn_node* node = nodeset->findNodeByIdentifier(identifier);
		if (node)
		{
			return node->access();
		}
	}
	return nullptr;
}

char* cmzn_nodeset_get_name(cmzn_nodeset_id nodeset)
{
	if (nodeset)
	{
		return nodeset->getName();
	}
	return nullptr;
}

int cmzn_nodeset_get_size(cmzn_nodeset_id nodeset)
{
	if (nodeset)
	{
		return nodeset->getSize();
	}
	return 0;
}

int cmzn_nodeset_destroy_all_nodes(cmzn_nodeset_id nodeset)
{
	if (nodeset)
	{
		return nodeset->destroyAllNodes();
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_destroy_node(cmzn_nodeset_id nodeset, cmzn_node_id node)
{
	if (nodeset && node)
	{
		return nodeset->destroyNode(node);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_destroy_nodes_conditional(cmzn_nodeset_id nodeset,
	cmzn_field_id conditional_field)
{
	if (nodeset && conditional_field)
	{
		return nodeset->destroyNodesConditional(conditional_field);
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_fieldmodule_id cmzn_nodeset_get_fieldmodule(cmzn_nodeset_id nodeset)
{
	if (nodeset)
	{
		return cmzn_fieldmodule_create(nodeset->getRegion());
	}
	return nullptr;
}

cmzn_nodeset_id cmzn_nodeset_get_master_nodeset(cmzn_nodeset_id nodeset)
{
	if (nodeset)
	{
		return nodeset->getMasterNodeset()->access();
	}
	return nullptr;
}
