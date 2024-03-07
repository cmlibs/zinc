/***************************************************************************//**
 * FILE : cmiss_element_private.cpp
 *
 * Implementation of public interface to cmzn_element, finite element meshes.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cmlibs/zinc/node.h"
#include "cmlibs/zinc/status.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/field_module.hpp"
#include "element/elementtemplate.hpp"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "general/enumerator_conversion.hpp"
#include "general/mystring.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/mesh.hpp"


/*
Global functions
----------------
*/

cmzn_element_id cmzn_element_access(cmzn_element_id element)
{
	if (element)
		return element->access();
	return 0;
}

int cmzn_element_destroy(cmzn_element_id *element_address)
{
	if (element_address)
		return cmzn_element::deaccess(*element_address);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_element_set_identifier(cmzn_element_id element, int identifier)
{
	return element->setIdentifier(identifier);
}

int cmzn_element_get_dimension(cmzn_element_id element)
{
	if (element)
		return element->getDimension();
	return 0;
}

cmzn_elementfieldtemplate_id cmzn_element_get_elementfieldtemplate(
	cmzn_element_id element, cmzn_field_id field, int componentNumber)
{
	if (!(element && field && ((-1 == componentNumber) ||
		((1 <= componentNumber) && (componentNumber <= cmzn_field_get_number_of_components(field))))))
	{
		display_message(ERROR_MESSAGE, "Element getElementfieldtemplate.  Invalid argument(s)");
		return 0;
	}
	FE_field *fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	Value_type valueType = get_FE_field_value_type(fe_field);
	if ((!fe_field)
		|| (get_FE_field_FE_field_type(fe_field) != GENERAL_FE_FIELD)
		|| ((valueType != FE_VALUE_VALUE) && (valueType != INT_VALUE)))
	{
		display_message(ERROR_MESSAGE, "Element getElementfieldtemplate.  Can only query a finite element type field on elements");
		return 0;
	}
	FE_mesh_field_data *meshFieldData = fe_field->getMeshFieldData(element->getMesh());
	if (!meshFieldData)
		return 0;
	FE_element_field_template *eft = 0;
	const int firstComponent = (componentNumber > 0) ? componentNumber - 1 : 0;
	const int limitComponent = (componentNumber > 0) ? componentNumber : get_FE_field_number_of_components(fe_field);
	FE_mesh_field_template *lastMft = 0;
	for (int c = firstComponent; c < limitComponent; ++c)
	{
		FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
		if (!mft)
		{
			display_message(ERROR_MESSAGE, "Element getElementfieldtemplate.  No mesh field template found for component %d", c + 1);
			return 0;
		}
		if (mft != lastMft)
		{
			FE_element_field_template *eftTmp = mft->getElementfieldtemplate(element->getIndex());
			if (!eftTmp)
				return 0; // not defined
			if (eft)
			{
				if (eftTmp != eft)
					return 0; // not homogeneous;
			}
			else
			{
				eft = eftTmp;
			}
			lastMft = mft;
		}
	}
	return cmzn_elementfieldtemplate::create(eft);
}

int cmzn_element_get_number_of_faces(cmzn_element_id element)
{
	if (element)
	{
		FE_element_shape* elementShape = element->getElementShape();
		if (elementShape)
		{
			return FE_element_shape_get_number_of_faces(elementShape);
		}
	}
	return 0;
}

cmzn_element_id cmzn_element_get_face_element(cmzn_element_id element, int faceNumber)
{
	if ((element) && (element->getMesh()))
	{
		DsLabelIndex faceElementIndex = element->getMesh()->getElementFace(element->getIndex(), faceNumber - 1);
		if (faceElementIndex != DS_LABEL_INDEX_INVALID)
		{
			cmzn_element* faceElement = element->getMesh()->getFaceMesh()->getElement(faceElementIndex);
			if (faceElement)
			{
				return faceElement->access();
			}
		}
	}
	return nullptr;
}

int cmzn_element_get_identifier(cmzn_element_id element)
{
	if (element)
		return element->getIdentifier();
	return DS_LABEL_IDENTIFIER_INVALID;
}

cmzn_mesh_id cmzn_element_get_mesh(cmzn_element_id element)
{
	if (element)
	{
		// handle element being orphaned during clean-up
		FE_mesh* feMesh = element->getMesh();
		if (feMesh)
		{
			cmzn_region* region = feMesh->getRegion();
			if (region)
			{
				cmzn_mesh* mesh = region->findMeshByDimension(feMesh->getDimension());
				if (mesh)
				{
					return mesh->access();
				}
			}
		}
	}
	return nullptr;
}

cmzn_node_id cmzn_element_get_node(cmzn_element_id element,
	cmzn_elementfieldtemplate_id eft, int localNodeIndex)
{
	if (!((element) && (eft) && (0 < localNodeIndex) && (localNodeIndex <= eft->getNumberOfLocalNodes())))
	{
		display_message(ERROR_MESSAGE, "Element getNode.  Invalid argument(s)");
		return 0;
	}
	FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "Element getNode.  Invalid element");
		return 0;
	}
	FE_mesh_element_field_template_data *eftData = mesh->getElementfieldtemplateData(eft->get_FE_element_field_template());
	if (!eftData)
	{
		display_message(ERROR_MESSAGE, "Element getNode.  Element field template is not used by element's mesh");
		return 0;
	}
	const DsLabelIndex nodeIndex = eftData->getElementLocalNode(element->getIndex(), localNodeIndex - 1);
	if (nodeIndex == DS_LABEL_INDEX_INVALID)
		return 0;
	cmzn_node *node = mesh->getNodeset()->getNode(nodeIndex);
	cmzn_node_access(node);
	return node;
}

int cmzn_element_set_node(cmzn_element_id element,
	cmzn_elementfieldtemplate_id eft, int localNodeIndex, cmzn_node_id node)
{
	if (!((element) && (eft) && (0 < localNodeIndex) && (localNodeIndex <= eft->getNumberOfLocalNodes())))
	{
		display_message(ERROR_MESSAGE, "Element setNode.  Invalid argument(s)");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "Element setNode.  Invalid element");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh_element_field_template_data *eftData = mesh->getElementfieldtemplateData(eft->get_FE_element_field_template());
	if (!eftData)
	{
		display_message(ERROR_MESSAGE, "Element setNode.  Element field template is not used by element's mesh");
		return CMZN_ERROR_ARGUMENT;
	}
	return eftData->setElementLocalNode(element->getIndex(), localNodeIndex - 1, (node) ? node->getIndex() : DS_LABEL_INDEX_INVALID);
}

int cmzn_element_set_nodes_by_identifier(cmzn_element_id element,
	cmzn_elementfieldtemplate_id eft, int identifiersCount,
	const int *identifiersIn)
{
	if (!((element) && (eft) && (identifiersCount == eft->getNumberOfLocalNodes()) && (identifiersIn)))
	{
		display_message(ERROR_MESSAGE, "Element setNodesByIdentifier.  Invalid argument(s)");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "Element setNodesByIdentifier.  Invalid element");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh_element_field_template_data *eftData = mesh->getElementfieldtemplateData(eft->get_FE_element_field_template());
	if (!eftData)
	{
		display_message(ERROR_MESSAGE, "Element setNodesByIdentifier.  Element field template is not used by element's mesh");
		return CMZN_ERROR_ARGUMENT;
	}
	const int result = eftData->setElementLocalNodesByIdentifier(element->getIndex(), identifiersIn);
	if (result != CMZN_OK)
	{
		display_message(ERROR_MESSAGE, "Element setNodesByIdentifier.  Failed to set nodes for %d-D element %d",
			mesh->getDimension(), element->getIdentifier());
	}
	return result;
}


int cmzn_element_get_number_of_parents(cmzn_element_id element)
{
	if ((element) && (element->getMesh()))
	{
		const DsLabelIndex* parents = 0;
		return element->getMesh()->getElementParents(element->getIndex(), parents);
	}
	return 0;
}

cmzn_element_id cmzn_element_get_parent_element(cmzn_element_id element, int parentNumber)
{
	if ((element) && (element->getMesh()) && (parentNumber > 0))
	{
		const DsLabelIndex* parents = 0;
		const int parentsCount = element->getMesh()->getElementParents(element->getIndex(), parents);
		if ((parentsCount > 0) && (parentNumber <= parentsCount))
		{
			const DsLabelIndex parentElementIndex = parents[parentNumber - 1];
			cmzn_element* parentElement = element->getMesh()->getParentMesh()->getElement(parentElementIndex);
			if (parentElement)
			{
				return parentElement->access();
			}
		}
	}
	return nullptr;
}

int cmzn_element_get_scale_factor(cmzn_element_id element,
	cmzn_elementfieldtemplate_id eft, int localScaleFactorIndex, double *valueOut)
{
	if (!((element) && (eft) && (0 < localScaleFactorIndex)
		&& (localScaleFactorIndex <= eft->getNumberOfLocalScaleFactors()) && (valueOut)))
	{
		display_message(ERROR_MESSAGE, "Element getScaleFactor.  Invalid argument(s)");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "Element getScaleFactor.  Invalid element");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh_element_field_template_data *eftData = mesh->getElementfieldtemplateData(eft->get_FE_element_field_template());
	if (!eftData)
	{
		display_message(ERROR_MESSAGE, "Element getScaleFactor.  Element field template is not used by element's mesh");
		return CMZN_ERROR_ARGUMENT;
	}
	return eftData->getElementScaleFactor(element->getIndex(), localScaleFactorIndex - 1, *valueOut);
}

int cmzn_element_set_scale_factor(cmzn_element_id element,
	cmzn_elementfieldtemplate_id eft, int localScaleFactorIndex, double value)
{
	if (!((element) && (eft) && (0 < localScaleFactorIndex)
		&& (localScaleFactorIndex <= eft->getNumberOfLocalScaleFactors())))
	{
		display_message(ERROR_MESSAGE, "Element setScaleFactor.  Invalid argument(s)");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "Element setScaleFactor.  Invalid element");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh_element_field_template_data *eftData = mesh->getElementfieldtemplateData(eft->get_FE_element_field_template());
	if (!eftData)
	{
		display_message(ERROR_MESSAGE, "Element setScaleFactor.  Element field template is not used by element's mesh");
		return CMZN_ERROR_ARGUMENT;
	}
	return eftData->setElementScaleFactor(element->getIndex(), localScaleFactorIndex - 1, value);
}

int cmzn_element_get_scale_factors(cmzn_element_id element,
	cmzn_elementfieldtemplate_id eft, int valuesCount, double *valuesOut)
{
	if (!((element) && (eft) && (valuesCount == eft->getNumberOfLocalScaleFactors()) && (valuesOut)))
	{
		display_message(ERROR_MESSAGE, "Element getScaleFactors.  Invalid argument(s)");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "Element getScaleFactors.  Invalid element");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh_element_field_template_data *eftData = mesh->getElementfieldtemplateData(eft->get_FE_element_field_template());
	if (!eftData)
	{
		display_message(ERROR_MESSAGE, "Element getScaleFactors.  Element field template is not used by element's mesh");
		return CMZN_ERROR_ARGUMENT;
	}
	return eftData->getOrCreateElementScaleFactors(element->getIndex(), valuesOut);
}
	
int cmzn_element_set_scale_factors(cmzn_element_id element,
	cmzn_elementfieldtemplate_id eft, int valuesCount, const double *valuesIn)
{
	if (!((element) && (eft) && (valuesCount == eft->getNumberOfLocalScaleFactors()) && (valuesIn)))
	{
		display_message(ERROR_MESSAGE, "Element setScaleFactors.  Invalid argument(s)");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "Element setScaleFactors.  Invalid element");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_mesh_element_field_template_data *eftData = mesh->getElementfieldtemplateData(eft->get_FE_element_field_template());
	if (!eftData)
	{
		display_message(ERROR_MESSAGE, "Element setScaleFactors.  Element field template is not used by element's mesh");
		return CMZN_ERROR_ARGUMENT;
	}
	return eftData->setElementScaleFactors(element->getIndex(), valuesIn);
}

enum cmzn_element_shape_type cmzn_element_get_shape_type(
	cmzn_element_id element)
{
	cmzn_element_shape_type shape_type = CMZN_ELEMENT_SHAPE_TYPE_INVALID;
	if (element)
	{
		FE_mesh *mesh = element->getMesh();
		if (!mesh)
		{
			display_message(ERROR_MESSAGE, "Element getShapeType.  Invalid element");
		}
		else
		{
			shape_type = mesh->getElementShapeType(get_FE_element_index(element));
		}
	}
	return shape_type;
}

int cmzn_element_merge(cmzn_element_id element,
	cmzn_elementtemplate_id element_template)
{
	if (element && element_template)
		return element_template->mergeIntoElement(element);
	return CMZN_ERROR_ARGUMENT;
}

class cmzn_element_shape_type_conversion
{
public:
	static const char *to_string(enum cmzn_element_shape_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
			case CMZN_ELEMENT_SHAPE_TYPE_LINE:
				enum_string = "LINE";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_SQUARE:
				enum_string = "SQUARE";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_TRIANGLE:
				enum_string = "TRIANGLE";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_CUBE:
				enum_string = "CUBE";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_TETRAHEDRON:
				enum_string = "TETRAHEDRON";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_WEDGE12:
				enum_string = "WEDGE12";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_WEDGE13:
				enum_string = "WEDGE13";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_WEDGE23:
				enum_string = "WEDGE23";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_element_shape_type cmzn_element_shape_type_enum_from_string(
	const char *name)
{
	return string_to_enum<enum cmzn_element_shape_type,	cmzn_element_shape_type_conversion>(name);
}

char *cmzn_element_shape_type_enum_to_string(enum cmzn_element_shape_type type)
{
	const char *type_string = cmzn_element_shape_type_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}



//========================================

cmzn_meshchanges::cmzn_meshchanges(cmzn_fieldmoduleevent *eventIn, cmzn_mesh *meshIn) :
	event(eventIn->access()),
	changeLog(cmzn::Access(eventIn->getFeRegionChanges()->getElementChangeLog(meshIn->getDimension()))),
	access_count(1)
{
	// optimial time to do this:
	eventIn->getFeRegionChanges()->propagateToDimension(meshIn->getDimension());
}

cmzn_meshchanges::~cmzn_meshchanges()
{
	cmzn::Deaccess(this->changeLog);
	cmzn_fieldmoduleevent::deaccess(this->event);
}

cmzn_meshchanges *cmzn_meshchanges::create(cmzn_fieldmoduleevent *eventIn, cmzn_mesh *meshIn)
{
	if ((eventIn) && (eventIn->getFeRegionChanges()) && (meshIn) &&
		(eventIn->get_FE_region() == meshIn->getFeMesh()->get_FE_region()))
	{
		return new cmzn_meshchanges(eventIn, meshIn);
	}
	return nullptr;
}

int cmzn_meshchanges::deaccess(cmzn_meshchanges* &meshchanges)
{
	if (meshchanges)
	{
		--(meshchanges->access_count);
		if (meshchanges->access_count <= 0)
			delete meshchanges;
		meshchanges = 0;
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

// Note: internal and external flags have same numerical values
cmzn_element_change_flags cmzn_meshchanges::getElementChangeFlags(cmzn_element *element)
{
	// relies on FE_region_changes::propagateToDimension call in constructor
	if (element && this->changeLog->isIndexChange(element->getIndex()))
		return this->changeLog->getChangeSummary();
	return CMZN_ELEMENT_CHANGE_FLAG_NONE;
}

cmzn_meshchanges_id cmzn_meshchanges_access(
	cmzn_meshchanges_id meshchanges)
{
	if (meshchanges)
		return meshchanges->access();
	return 0;
}

int cmzn_meshchanges_destroy(cmzn_meshchanges_id *meshchanges_address)
{
	if (meshchanges_address)
		return cmzn_meshchanges::deaccess(*meshchanges_address);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_element_change_flags cmzn_meshchanges_get_element_change_flags(
	cmzn_meshchanges_id meshchanges, cmzn_element_id element)
{
	if (meshchanges)
		return meshchanges->getElementChangeFlags(element);
	return CMZN_ELEMENT_CHANGE_FLAG_NONE;
}

int cmzn_meshchanges_get_number_of_changes(
	cmzn_meshchanges_id meshchanges)
{
	if (meshchanges)
		return meshchanges->getNumberOfChanges();
	return 0;
}

cmzn_element_change_flags cmzn_meshchanges_get_summary_element_change_flags(
	cmzn_meshchanges_id meshchanges)
{
	if (meshchanges)
		return meshchanges->getSummaryElementChangeFlags();
	return CMZN_ELEMENT_CHANGE_FLAG_NONE;
}
