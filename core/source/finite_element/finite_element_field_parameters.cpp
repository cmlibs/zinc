/**
 * FILE : finite_element_field_parameters.cpp
 *
 * Records field parameter indexing.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/result.h"
#include "finite_element/finite_element_field.hpp"
#include "finite_element/finite_element_field_parameters.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region_private.h"
#include "general/message.h"
#include <vector>


FE_field_parameters::FE_field_parameters(FE_field *fieldIn) :
	field(fieldIn->access()),
	parametersCount(0),
	nodeParameterMap(/*blockLengthIn*/256, /*allocInitValueIn*/-1),
	parameterNodeMap(/*blockLengthIn*/256, /*allocInitValueIn*/-1),
	fieldModifyCounter(1),  // force maps to be rebuilt
	perturbationDelta(1.0E-5),
	access_count(1)
{
}

FE_field_parameters::~FE_field_parameters()
{
	this->field->clear_FE_field_parameters();
	FE_field::deaccess(&(this->field));
}

void FE_field_parameters::generateMaps()
{
	this->nodeParameterMap.clear();
	this->parameterNodeMap.clear();
	FE_region *feRegion = this->field->get_FE_region();
	FE_nodeset *feNodeset = FE_region_find_FE_nodeset_by_field_domain_type(feRegion, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_nodeiterator *nodeIter = feNodeset->createNodeiterator();
	FE_node_field_info *lastFieldInfo = nullptr;
	const FE_node_field *nodeField = nullptr;
	DsLabelIndex nodeValuesCount = 0;
	cmzn_node *node;
	DsLabelIndex parameterIndex = 0;
	// calculate value ranges to calculate perturbationDelta
	const int componentCount = this->field->getNumberOfComponents();
	std::vector<FE_value> minimumValues;
	std::vector<FE_value> maximumValues;
	std::vector<FE_value> values(componentCount);
	int nodeRangeCount = 0;
	while ((node = nodeIter->nextNode()) != nullptr)
	{
		if (node->fields != lastFieldInfo)
		{
			lastFieldInfo = node->fields;
			nodeField = node->getNodeField(this->field);
			if (!nodeField)
			{
				// not defined on node
				nodeValuesCount = 0;
				continue;
			}
			nodeValuesCount = nodeField->getTotalValuesCount();
		}
		else if (nodeValuesCount == 0)
		{
			continue;  // not defined on node
		}
		this->nodeParameterMap.setValue(node->getIndex(), parameterIndex);
		this->parameterNodeMap.setValues(parameterIndex, nodeValuesCount, node->getIndex());
		parameterIndex += nodeValuesCount;
		if (get_FE_nodal_FE_value_value(node, this->field, /*component*/-1, CMZN_NODE_VALUE_LABEL_VALUE, /*version*/0, /*time*/0.0, values.data()))
		{
			if (nodeRangeCount)
			{
				for (int c = 0; c < componentCount; ++c)
				{
					if (values[c] < minimumValues[c])
						minimumValues[c] = values[c];
					else if (values[c] > maximumValues[c])
						maximumValues[c] = values[c];
				}
			}
			else
			{
				minimumValues = values;
				maximumValues = values;
			}
			++nodeRangeCount;
		}
	}
	this->parametersCount = parameterIndex;
	this->fieldModifyCounter = 0;  // concurrency point
	// following could be improved knowing the number of elements
	// basically want a fraction of typical or minimum element span
	FE_value maxRange = 0.0;
	if (nodeRangeCount > 1)
		for (int c = 0; c < componentCount; ++c)
		{
			const FE_value range = maximumValues[c] - minimumValues[c];
			if (range > maxRange)
				maxRange = range;
		}
	if ((maxRange == 0.0) && (nodeRangeCount))
		for (int c = 0; c < componentCount; ++c)
		{
			const FE_value range = fabs(maximumValues[c]);
			if (range > maxRange)
				maxRange = range;
		}
	this->perturbationDelta = ((maxRange > 0.0) ? maxRange : 1.0)*1.0E-5;
}

FE_field_parameters *FE_field_parameters::create(FE_field *fieldIn)
{
	if ((fieldIn) && (fieldIn->get_FE_field_type() == GENERAL_FE_FIELD) && (fieldIn->getValueType() == FE_VALUE_VALUE))
		return new FE_field_parameters(fieldIn);
	display_message(ERROR_MESSAGE, "FE_field_parameters::create.  Missing or invalid field");
	return nullptr;
}

int FE_field_parameters::deaccess(FE_field_parameters* &fe_field_parameters)
{
	if (!fe_field_parameters)
		return CMZN_RESULT_ERROR_ARGUMENT;
	--(fe_field_parameters->access_count);
	if (fe_field_parameters->access_count <= 0)
		delete fe_field_parameters;
	fe_field_parameters = 0;
	return CMZN_RESULT_OK;
}

int FE_field_parameters::getNumberOfElementParameters(cmzn_element *element)
{
	const FE_mesh_field_data *meshFieldData = (element) ? this->field->getMeshFieldData(element->getMesh()) : nullptr;
	if (!meshFieldData)
	{
		display_message(ERROR_MESSAGE, "Fieldparameters getNumberOfElementParameters:  Element is invalid or not from this region");
		return -1;
	}
	int elementParameterCount = 0;
	const int componentCount = this->field->getNumberOfComponents();
	for (int c = 0; c < componentCount; ++c)
	{
		const FE_mesh_field_template *meshFieldTemplate = meshFieldData->getComponentMeshfieldtemplate(c);
		const FE_element_field_template *eft = meshFieldTemplate->getElementfieldtemplate(element->getIndex());
		if (!eft)
			return 0;  // not defined on this element
		if (eft->getParameterMappingMode() != CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_NODE)
		{
			display_message(ERROR_MESSAGE, "Fieldparameters getNumberOfElementParameters:  Not implemented for non-nodal parameters: ",
				"Element %d component %d", element->getIdentifier(), c + 1);
			continue;
		}
		elementParameterCount += eft->getParameterCount();
	}
	return elementParameterCount;
}

int FE_field_parameters::getNumberOfParameters()
{
	this->checkMaps();
	return this->parametersCount;
}
