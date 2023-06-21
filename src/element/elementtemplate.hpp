/**
 * FILE : elementtemplate.hpp
 *
 * Interface to elementtemplate implementation.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "cmlibs/zinc/elementfieldtemplate.h"
#include "cmlibs/zinc/elementtemplate.h"
#include "finite_element/finite_element_field.hpp"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_region.h"
#include <vector>


class LegacyElementFieldData;


struct cmzn_elementtemplate
{
private:
	FE_element_template *fe_element_template; // internal implementation
	int legacyNodesCount;
	cmzn_node** legacyNodes;
	std::vector<LegacyElementFieldData*> legacyFieldDataList;
	int access_count;

	cmzn_elementtemplate(FE_mesh *feMeshIn);

	~cmzn_elementtemplate();

	inline void beginChange()
	{
		FE_region_begin_change(this->getFeMesh()->get_FE_region());
	}

	inline void endChange()
	{
		FE_region_end_change(this->getFeMesh()->get_FE_region());
	}

	void clearLegacyNodes();

	void clearLegacyElementFieldData(FE_field* fe_field);

	LegacyElementFieldData* getLegacyElementFieldData(FE_field* fe_field);

	LegacyElementFieldData* getOrCreateLegacyElementFieldData(FE_field* fe_field);

public:

	int setLegacyNodesInElement(cmzn_element* element);

	static cmzn_elementtemplate* create(FE_mesh* feMeshIn)
	{
		if (feMeshIn)
		{
			return new cmzn_elementtemplate(feMeshIn);
		}
		return nullptr;
	}

	cmzn_elementtemplate* access()
	{
		++access_count;
		return this;
	}

	static void deaccess(cmzn_elementtemplate*& elementtemplate)
	{
		if (elementtemplate)
		{
			--(elementtemplate->access_count);
			if (elementtemplate->access_count <= 0)
			{
				delete elementtemplate;
			}
			elementtemplate = nullptr;
		}
	}

	/** @return  Non-accessed shape */
	FE_element_shape *getElementShape() const
	{
		return this->fe_element_template->getElementShape();
	}

	int setElementShape(FE_element_shape *elementShape)
	{
		return this->fe_element_template->setElementShape(elementShape);
	}

	cmzn_element_shape_type getShapeType() const
	{
		return FE_element_shape_get_simple_type(this->fe_element_template->getElementShape());
	}

	int setElementShapeType(cmzn_element_shape_type shapeTypeIn);

	FE_mesh *getFeMesh() const
	{
		return this->fe_element_template->getMesh();
	}

	int getLegacyNumberOfNodes() const
	{
		return this->legacyNodesCount;
	}

	int setLegacyNumberOfNodes(int legacyNodesCountIn);

	int defineField(FE_field* field, int componentNumber, cmzn_elementfieldtemplate* eft);

	int defineField(cmzn_field* field, int componentNumber, cmzn_elementfieldtemplate* eft);

	/** Append legacy node indexes giving index in element template of node for each EFT local node.
	  * @param componentNumber  -1 for all components, or positive for single component. */
	int addLegacyNodeIndexes(FE_field* field, int componentNumber, int nodeIndexesCount,
		const int* nodeIndexes);

	bool validate() const
	{
		return this->fe_element_template->validate();
	}

	/** @param local_node_index  Index from 1 to legacy nodes count.
	  * @return  Non-accessed node, or 0 if invalid index or no node at index. */
	cmzn_node* getLegacyNode(int local_node_index);

	/** @param local_node_index  Index from 1 to legacy nodes count. */
	int setLegacyNode(int local_node_index, cmzn_node* node);

	int removeField(cmzn_field* field);

	int undefineField(cmzn_field* field);

	cmzn_element* createElement(int identifier);

	/** Variant for EX reader which assumes template has already been validated,
	  * does not set legacy nodes and does not cache changes as assumed on */
	cmzn_element* createElementEX(int identifier)
	{
		return this->getFeMesh()->create_FE_element(identifier, this->fe_element_template);
	}

	int mergeIntoElement(cmzn_element* element);

	/** Variant for EX reader which assumes template has already been validated,
	  * does not set legacy nodes and does not cache changes as assumed on */
	int mergeIntoElementEX(cmzn_element* element)
	{
		return this->getFeMesh()->merge_FE_element_template(element, this->fe_element_template);
	}

	FE_element_template* get_FE_element_template()
	{
		return this->fe_element_template;
	}

};
