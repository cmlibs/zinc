/**
 * FILE : elementtemplate.cpp
 *
 * Implementation of elementtemplate.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "computed_field/computed_field_finite_element.h"
#include "element/elementtemplate.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_shape.hpp"
#include "finite_element/finite_element_region_private.h"


namespace {

	inline int cmzn_element_shape_type_get_dimension(
		cmzn_element_shape_type shape_type)
	{
		switch (shape_type)
		{
		case CMZN_ELEMENT_SHAPE_TYPE_LINE:
			return 1;
			break;
		case CMZN_ELEMENT_SHAPE_TYPE_SQUARE:
		case CMZN_ELEMENT_SHAPE_TYPE_TRIANGLE:
			return 2;
			break;
		case CMZN_ELEMENT_SHAPE_TYPE_CUBE:
		case CMZN_ELEMENT_SHAPE_TYPE_TETRAHEDRON:
		case CMZN_ELEMENT_SHAPE_TYPE_WEDGE12:
		case CMZN_ELEMENT_SHAPE_TYPE_WEDGE13:
		case CMZN_ELEMENT_SHAPE_TYPE_WEDGE23:
			return 3;
			break;
		default:
			// do nothing
			break;
		}
		return 0;
	}

}

// stores map of EFT local node to legacy element nodes, per-component
class LegacyElementFieldData
{
public:
	class NodeMap
	{
		int nodeIndexesCount;
		int* nodeIndexes;

	public:
		NodeMap(int nodeIndexesCountIn, const int* nodeIndexesIn) :
			nodeIndexesCount(nodeIndexesCountIn),
			nodeIndexes(new int[nodeIndexesCountIn])
		{
			memcpy(this->nodeIndexes, nodeIndexesIn, nodeIndexesCountIn * sizeof(int));
		}

		~NodeMap()
		{
			delete[] this->nodeIndexes;
		}

		int getNodeIndexesCount() const
		{
			return this->nodeIndexesCount;
		}

		const int* getNodeIndexes() const
		{
			return this->nodeIndexes;
		}
	};

private:

	FE_field* fe_field;
	const int componentCount;
	NodeMap** componentNodeMaps;

	/** Handles sharing of same node map for multiple components.
	  * @param componentNumber  Starting at 0, or negative to clear all components. Not checked. */
	void clearComponentNodeMaps(int componentNumber)
	{
		if (componentNumber < 0)
		{
			for (int i = 0; i < this->componentCount; i++)
			{
				// handle sharing by multiple components
				for (int j = this->componentCount - 1; i < j; --j)
					if (this->componentNodeMaps[j] == this->componentNodeMaps[i])
						this->componentNodeMaps[j] = 0;
				delete this->componentNodeMaps[i];
				this->componentNodeMaps[i] = 0;
			}
		}
		else
		{
			bool unshared = true;
			for (int i = 0; i < this->componentCount; i++)
			{
				if ((i != componentNumber) && (this->componentNodeMaps[i] == this->componentNodeMaps[componentNumber]))
				{
					unshared = false;
					break;
				}
			}
			if (unshared)
				delete[] this->componentNodeMaps[componentNumber];
			this->componentNodeMaps[componentNumber] = 0;
		}
	}

public:

	LegacyElementFieldData(FE_field* fe_field) :
		fe_field(ACCESS(FE_field)(fe_field)),
		componentCount(get_FE_field_number_of_components(fe_field)),
		componentNodeMaps(new NodeMap* [componentCount])
	{
		for (int i = 0; i < this->componentCount; i++)
			componentNodeMaps[i] = 0;
	}

	~LegacyElementFieldData()
	{
		DEACCESS(FE_field)(&fe_field);
		this->clearComponentNodeMaps(/*componentNumber*/-1);
		delete[] this->componentNodeMaps;
	}

	FE_field* getField() const
	{
		return this->fe_field;
	}

	int getComponentCount() const
	{
		return this->componentCount;
	}

	/** @param componentNumber  Starting at 0, or negative to set for all components. Not checked.
	  * @param nodeIndexesCount  Size of nodeIndexes. Must equal number of nodes expected by EFT. */
	int setNodeMap(int componentNumber, int nodeIndexesCount, const int* nodeIndexes)
	{
		NodeMap* nodeMap = new NodeMap(nodeIndexesCount, nodeIndexes);
		if (!nodeMap)
			return CMZN_ERROR_MEMORY;
		clearComponentNodeMaps(componentNumber);
		if (componentNumber >= 0)
		{
			this->componentNodeMaps[componentNumber] = nodeMap;
		}
		else
		{
			for (int i = 0; i < this->componentCount; ++i)
				this->componentNodeMaps[i] = nodeMap;
		}
		return CMZN_OK;
	}

	/** @param componentNumber  From 0 to count - 1, not checked */
	const NodeMap* getComponentNodeMap(int componentNumber) const
	{
		return this->componentNodeMaps[componentNumber];
	}

};


cmzn_elementtemplate::cmzn_elementtemplate(FE_mesh *feMeshIn) :
	fe_element_template(feMeshIn->create_FE_element_template()),
	legacyNodesCount(0),
	legacyNodes(nullptr),
	access_count(1)
{
}

cmzn_elementtemplate::~cmzn_elementtemplate()
{
	const size_t legacyFieldDataCount = this->legacyFieldDataList.size();
	for (size_t i = 0; i < legacyFieldDataCount; ++i)
		delete this->legacyFieldDataList[i];
	this->clearLegacyNodes();
	cmzn::Deaccess(this->fe_element_template);
}

void cmzn_elementtemplate::clearLegacyNodes()
{
	if (this->legacyNodes)
	{
		for (int i = 0; i < this->legacyNodesCount; ++i)
			cmzn_node::deaccess(this->legacyNodes[i]);
		delete[] this->legacyNodes;
		this->legacyNodes = 0;
	}
}

void cmzn_elementtemplate::clearLegacyElementFieldData(FE_field* fe_field)
{
	for (std::vector<LegacyElementFieldData*>::iterator iter = this->legacyFieldDataList.begin();
		iter != this->legacyFieldDataList.end(); ++iter)
	{
		if ((*iter)->getField() == fe_field)
		{
			delete *iter;
			this->legacyFieldDataList.erase(iter);
			break;
		}
	}
}

LegacyElementFieldData* cmzn_elementtemplate::getLegacyElementFieldData(FE_field* fe_field)
{
	const size_t fieldDataCount = this->legacyFieldDataList.size();
	for (size_t i = 0; i < fieldDataCount; ++i)
		if (this->legacyFieldDataList[i]->getField() == fe_field)
			return this->legacyFieldDataList[i];
	return 0;
}

LegacyElementFieldData* cmzn_elementtemplate::getOrCreateLegacyElementFieldData(FE_field* fe_field)
{
	LegacyElementFieldData* legacyFieldData = this->getLegacyElementFieldData(fe_field);
	if (!legacyFieldData)
	{
		legacyFieldData = new LegacyElementFieldData(fe_field);
		this->legacyFieldDataList.push_back(legacyFieldData);
	}
	return legacyFieldData;
}

/** only call if have already checked has legacy node maps
  * Caller's responsibility to mark element as changed; this marks all fields as changed.
  * Expect to be called during FE_region change cache */
int cmzn_elementtemplate::setLegacyNodesInElement(cmzn_element* element)
{
	FE_mesh* feMesh = this->getFeMesh();
	std::vector<DsLabelIndex> workingNodeIndexes(this->legacyNodesCount, DS_LABEL_INDEX_INVALID);
	const size_t fieldCount = this->legacyFieldDataList.size();
	for (size_t f = 0; f < fieldCount; ++f)
	{
		LegacyElementFieldData* legacyData = this->legacyFieldDataList[f];
		FE_field* field = legacyData->getField();
		const int componentCount = legacyData->getComponentCount();
		const LegacyElementFieldData::NodeMap* lastComponentNodeMap = 0; // keep last one so efficient if used by multiple components
		for (int c = 0; c < componentCount; ++c)
		{
			const LegacyElementFieldData::NodeMap* componentNodeMap = legacyData->getComponentNodeMap(c);
			if (componentNodeMap && (componentNodeMap != lastComponentNodeMap))
			{
				lastComponentNodeMap = componentNodeMap;
				FE_element_field_template* eft = this->fe_element_template->getElementfieldtemplate(field, c);
				if (!eft)
				{
					display_message(ERROR_MESSAGE,
						"Elementtemplate  setLegacyNodesInElement.  Have legacy node map without field defined");
					return CMZN_ERROR_NOT_FOUND;
				}
				const int nodeIndexesCount = componentNodeMap->getNodeIndexesCount();
				const int* nodeIndexes = componentNodeMap->getNodeIndexes();
				if (eft->getNumberOfLocalNodes() != nodeIndexesCount)
				{
					display_message(ERROR_MESSAGE,
						"Elementtemplate  setLegacyNodesInElement.  Number of nodes does not match element field template");
					return CMZN_ERROR_GENERAL;
				}
				FE_mesh_element_field_template_data* eftData = feMesh->getElementfieldtemplateData(eft);
				if (!eftData)
				{
					display_message(ERROR_MESSAGE,
						"Elementtemplate  setLegacyNodesInElement.  Invalid element field template");
					return CMZN_ERROR_GENERAL;
				}
				if (workingNodeIndexes.capacity() < static_cast<size_t>(nodeIndexesCount))
					workingNodeIndexes.reserve(nodeIndexesCount);
				for (int n = 0; n < nodeIndexesCount; ++n)
				{
					cmzn_node* node = this->legacyNodes[nodeIndexes[n] - 1];
					workingNodeIndexes[n] = (node) ? node->getIndex() : DS_LABEL_INDEX_INVALID;
				}
				const int result = eftData->setElementLocalNodes(element->getIndex(), workingNodeIndexes.data());
				if (result != CMZN_OK)
				{
					display_message(ERROR_MESSAGE,
						"Elementtemplate  setLegacyNodesInElement.  Failed to set element local nodes");
					return result;
				}
			}
		}
	}
	// simplest to mark all fields as changed as they may share local nodes and scale factors
	// can optimise in future
	feMesh->get_FE_region()->FE_field_all_change(CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
	return CMZN_OK;
}

int cmzn_elementtemplate::setElementShapeType(cmzn_element_shape_type shapeTypeIn)
{
	FE_element_shape *elementShape = 0;
	if (CMZN_ELEMENT_SHAPE_TYPE_INVALID != shapeTypeIn)
	{
		const int shapeDimension = cmzn_element_shape_type_get_dimension(shapeTypeIn);
		if (shapeDimension != this->getFeMesh()->getDimension())
		{
			display_message(ERROR_MESSAGE,
				"Elementtemplate  setElementShapeType.  Shape dimension is different from template mesh");
			return CMZN_ERROR_ARGUMENT;
		}
		elementShape = FE_element_shape_create_simple_type(this->getFeMesh()->get_FE_region(), shapeTypeIn);
		if (!elementShape)
		{
			display_message(ERROR_MESSAGE,
				"Elementtemplate  setElementShapeType.  Failed to create element shape");
			return CMZN_ERROR_GENERAL;
		}
	}
	int return_code = this->fe_element_template->setElementShape(elementShape);
	if (elementShape)
	{
		DEACCESS(FE_element_shape)(&elementShape);
	}
	return return_code;
}

int cmzn_elementtemplate::setLegacyNumberOfNodes(int legacyNodesCountIn)
{
	if (legacyNodesCountIn < legacyNodesCount)
	{
		display_message(ERROR_MESSAGE,
			"Elementtemplate setLegacyNumberOfNodes.  Cannot reduce number of nodes");
		return CMZN_ERROR_ARGUMENT;
	}
	cmzn_node** newLegacyNodes = new cmzn_node * [legacyNodesCountIn];
	if (!newLegacyNodes)
		return CMZN_ERROR_MEMORY;
	this->clearLegacyNodes();
	for (int i = 0; i < legacyNodesCountIn; ++i)
		newLegacyNodes[i] = 0;
	this->legacyNodes = newLegacyNodes;
	this->legacyNodesCount = legacyNodesCountIn;
	return CMZN_OK;
}

int cmzn_elementtemplate::defineField(FE_field* field, int componentNumber, cmzn_elementfieldtemplate* eft)
{
	if (!((field) && ((-1 == componentNumber) || ((0 < componentNumber) && (componentNumber <= get_FE_field_number_of_components(field))))
		&& (eft)))
	{
		display_message(ERROR_MESSAGE, "Elementtemplate defineField.  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
    int return_code = this->fe_element_template->defineField(field, componentNumber - 1, eft);
	if (CMZN_OK != return_code)
		display_message(ERROR_MESSAGE, "Elementtemplate defineField.  Failed");
	return return_code;
}

int cmzn_elementtemplate::defineField(cmzn_field* field, int componentNumber, cmzn_elementfieldtemplate* eft)
{
	FE_field* fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	if (!fe_field)
	{
		display_message(ERROR_MESSAGE,
			"Elementtemplate defineField.  Can only define a finite element type field on elements");
		return CMZN_ERROR_ARGUMENT;
	}
	return this->defineField(fe_field, componentNumber, eft);
}

int cmzn_elementtemplate::addLegacyNodeIndexes(FE_field* field, int componentNumber, int nodeIndexesCount,
	const int* nodeIndexes)
{
	const int componentCount = get_FE_field_number_of_components(field);
	if (!((field) && ((-1 == componentNumber) || ((0 < componentNumber) && (componentNumber <= componentCount)))
		&& ((0 == nodeIndexesCount) || (nodeIndexes))))
	{
		display_message(ERROR_MESSAGE, "Elementtemplate addLegacyNodeIndexes.  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
	const FE_element_field_template* eft = this->fe_element_template->getElementfieldtemplate(field, (componentNumber > 0) ? componentNumber - 1 : 0);
	if (!eft)
	{
		display_message(ERROR_MESSAGE, "Elementtemplate addLegacyNodeIndexes.  Field %s component is not defined", get_FE_field_name(field));
		return CMZN_ERROR_ARGUMENT;
	}
	if (eft->getParameterMappingMode() != CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_NODE)
	{
		display_message(ERROR_MESSAGE, "Elementtemplate addLegacyNodeIndexes.  "
			"Field %s component is not using node-based parameter map", get_FE_field_name(field));
		return CMZN_ERROR_ARGUMENT;
	}
	if (componentNumber < 0)
	{
		// check homogeneous
		for (int c = 1; c < componentCount; ++c)
		{
			if (this->fe_element_template->getElementfieldtemplate(field, c) != eft)
			{
				display_message(ERROR_MESSAGE, "Elementtemplate addLegacyNodeIndexes.  "
					"Field %s must use same element template for all components to use component -1", get_FE_field_name(field));
				return CMZN_ERROR_ARGUMENT;
			}
		}
	}
	int highestNodeIndex = eft->getHighestLocalNodeIndex();
	if (highestNodeIndex > (nodeIndexesCount - 1))
	{
		display_message(ERROR_MESSAGE, "Elementtemplate addLegacyNodeIndexes.  "
			"Node index map does not cover all local nodes in component template for field %s", get_FE_field_name(field));
		return CMZN_ERROR_ARGUMENT;
	}
	for (int i = 0; i < nodeIndexesCount; i++)
	{
		if ((nodeIndexes[i] < 1) || (nodeIndexes[i] > this->legacyNodesCount))
		{
			display_message(ERROR_MESSAGE, "Elementtemplate addLegacyNodeIndexes.  Local node index out of range 1 to number in element (%d)",
				this->legacyNodesCount);
			return CMZN_ERROR_ARGUMENT;
		}
	}
	LegacyElementFieldData* legacyFieldData = this->getOrCreateLegacyElementFieldData(field);
	if (!legacyFieldData)
		return CMZN_ERROR_GENERAL;
	return legacyFieldData->setNodeMap(componentNumber - 1, nodeIndexesCount, nodeIndexes);
}

/** @param local_node_index  Index from 1 to legacy nodes count.
  * @return  Non-accessed node, or 0 if invalid index or no node at index. */
cmzn_node* cmzn_elementtemplate::getLegacyNode(int local_node_index)
{
	if ((0 < local_node_index) && (local_node_index <= this->legacyNodesCount))
		return this->legacyNodes[local_node_index - 1];
	return 0;
}

/** @param local_node_index  Index from 1 to legacy nodes count. */
int cmzn_elementtemplate::setLegacyNode(int local_node_index, cmzn_node* node)
{
	FE_nodeset* nodeset;
	if ((0 < local_node_index) && (local_node_index <= this->legacyNodesCount)
		&& ((!node) || ((0 != (nodeset = FE_node_get_FE_nodeset(node)))
			&& (nodeset->getFieldDomainType() == CMZN_FIELD_DOMAIN_TYPE_NODES)
			&& (this->fe_element_template->getMesh()->get_FE_region() == nodeset->get_FE_region()))))
	{
		cmzn_node::reaccess(this->legacyNodes[local_node_index - 1], node);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate::removeField(cmzn_field* field)
{
	if (!field)
		return CMZN_ERROR_ARGUMENT;
	FE_field* fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	if (!fe_field)
	{
		display_message(ERROR_MESSAGE, "Elementtemplate removeField.  Not a finite element field");
		return CMZN_ERROR_ARGUMENT;
	}
	int return_code = this->fe_element_template->removeField(fe_field);
	if (CMZN_OK == return_code)
		this->clearLegacyElementFieldData(fe_field);
	return return_code;
}

int cmzn_elementtemplate::undefineField(cmzn_field* field)
{
	if (!field)
		return CMZN_ERROR_ARGUMENT;
	FE_field* fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	if (!fe_field)
	{
		display_message(ERROR_MESSAGE, "Elementtemplate undefineField.  Not a finite element field");
		return CMZN_ERROR_ARGUMENT;
	}
	int return_code = this->fe_element_template->undefineField(fe_field);
	this->clearLegacyElementFieldData(fe_field);
	return return_code;
}

cmzn_element *cmzn_elementtemplate::createElement(int identifier)
{
	if (!this->validate())
	{
		display_message(ERROR_MESSAGE, "Mesh createElement.  Element template is not valid");
		return 0;
	}
	if (!this->fe_element_template->getElementShape())
	{
		display_message(ERROR_MESSAGE, "Mesh createElement.  Element template does not have a shape set");
		return 0;
	}
	this->beginChange();
	cmzn_element* element = this->getFeMesh()->create_FE_element(identifier, this->fe_element_template);
	if (element && (this->legacyNodes) && (this->legacyFieldDataList.size() > 0))
	{
		int return_code = this->setLegacyNodesInElement(element);
		if (CMZN_OK != return_code)
		{
			display_message(ERROR_MESSAGE, "Mesh createElement.  Failed to set legacy nodes (deprecated feature)");
			cmzn_element::deaccess(element);
		}
	}
	this->endChange();
	return element;
}

int cmzn_elementtemplate::mergeIntoElement(cmzn_element* element)
{
	if (this->validate())
	{
		this->beginChange();
		int return_code = this->getFeMesh()->merge_FE_element_template(element, this->fe_element_template);
		if ((CMZN_OK == return_code) && (this->legacyNodes) && (this->legacyFieldDataList.size() > 0))
		{
			return_code = this->setLegacyNodesInElement(element);
			if (CMZN_OK != return_code)
			{
				display_message(ERROR_MESSAGE, "Element merge.  Failed to set legacy nodes (deprecated feature)");
				cmzn_element::deaccess(element);
			}
		}
		this->endChange();
		return return_code;
	}
	display_message(ERROR_MESSAGE, "Element merge.  Element template is not valid");
	return CMZN_ERROR_ARGUMENT;
}

/*
Global functions
----------------
*/

cmzn_elementtemplate_id cmzn_elementtemplate_access(
	cmzn_elementtemplate_id element_template)
{
	if (element_template)
		return element_template->access();
	return 0;
}

int cmzn_elementtemplate_destroy(
	cmzn_elementtemplate_id *element_template_address)
{
	if (element_template_address)
	{
		cmzn_elementtemplate::deaccess(*element_template_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_element_shape_type cmzn_elementtemplate_get_element_shape_type(
	cmzn_elementtemplate_id element_template)
{
	if (element_template)
		return element_template->getShapeType();
	return CMZN_ELEMENT_SHAPE_TYPE_INVALID;
}

int cmzn_elementtemplate_set_element_shape_type(cmzn_elementtemplate_id element_template,
	enum cmzn_element_shape_type shape_type)
{
	if (element_template)
		return element_template->setElementShapeType(shape_type);
	return 0;
}

int cmzn_elementtemplate_define_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field,
	int component_number, cmzn_elementfieldtemplate_id eft)
{
	if (elementtemplate)
		return elementtemplate->defineField(field, component_number, eft);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate_remove_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field)
{
	if (elementtemplate)
		return elementtemplate->removeField(field);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate_undefine_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field)
{
	if (elementtemplate)
		return elementtemplate->undefineField(field);
	return CMZN_ERROR_ARGUMENT;
}
