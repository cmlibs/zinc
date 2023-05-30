/**
 * FILE : nodeset.hpp
 *
 * Interface to nodeset implementation.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "cmlibs/zinc/node.h"
#include "cmlibs/zinc/nodeset.h"
#include "cmlibs/zinc/nodetemplate.h"
#include "finite_element/finite_element_nodeset.hpp"


struct cmzn_nodeset
{
	friend cmzn_region;

protected:
	FE_nodeset* feNodeset;
	int access_count;

	cmzn_nodeset(FE_nodeset* feNodesetIn) :
		feNodeset(cmzn::Access(feNodesetIn)),
		access_count(1)
	{
	}

	virtual ~cmzn_nodeset()
	{
		cmzn::Deaccess(this->feNodeset);
	}

public:

	cmzn_nodeset* access()
	{
		++access_count;
		return this;
	}

	static void deaccess(cmzn_nodeset*& nodeset);

	/** @return  Number of references held to object */
	int getAccessCount() const
	{
		return this->access_count;
	}

	/** @return  Accessed new node or nullptr if failed */
	virtual cmzn_node* createNode(int identifier, cmzn_nodetemplate* nodetemplate);

	virtual bool containsNode(cmzn_node* node) const
	{
		return this->feNodeset->containsNode(node);
	}

	/** @return  Accessed element template, or nullptr if failed */
	cmzn_nodetemplate* createNodetemplate() const;

	virtual cmzn_nodeiterator* createNodeiterator() const
	{
		return this->feNodeset->createNodeiterator();
	}

	virtual int destroyAllNodes()
	{
		return this->feNodeset->destroyAllNodes();
	}

	int destroyNode(cmzn_node* node)
	{
		if (this->containsNode(node))
		{
			return this->feNodeset->destroyNode(node);
		}
		return CMZN_ERROR_ARGUMENT;
	}

	int destroyNodesConditional(cmzn_field* conditional_field);

	/** @return  Non-accessed node, or nullptr if not found */
	virtual cmzn_node* findNodeByIdentifier(int identifier) const
	{
		return this->feNodeset->findNodeByIdentifier(identifier);
	}

	cmzn_field_domain_type getFieldDomainType() const
	{
		return this->feNodeset->getFieldDomainType();
	}

	FE_nodeset* getFeNodeset() const
	{
		return this->feNodeset;
	}

	/** @return  Non-allocated name of master nodeset */
	const char* getMasterNodesetName() const
	{
		return this->feNodeset->getName();
	}

	/** @return  Allocated name, or nullptr if failed */
	virtual char* getName() const;

	/** @return  Non-accessed master nodeset */
	cmzn_nodeset* getMasterNodeset() const;

	/** @return Non-accessed owning region */
	cmzn_region* getRegion() const;

	virtual int getSize() const
	{
		return this->feNodeset->getSize();
	}

	/** @return  True if nodeset has recorded changes in membership */
	virtual bool hasMembershipChanges() const;

};
