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


FE_field_parameters::FE_field_parameters(FE_field *fieldIn) :
	field(fieldIn->access()),
	parametersCount(0),
	nodeParameterMap(/*blockLengthIn*/256, /*allocInitValueIn*/-1),
	parameterNodeMap(/*blockLengthIn*/256, /*allocInitValueIn*/-1),
	fieldModifyCounter(1),  // force maps to be rebuilt
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
	while ((node = nodeIter->nextNode()) != nullptr)
	{
		if (node->fields != lastFieldInfo)
		{
			lastFieldInfo = node->fields;
			nodeField = node->getNodeField(this->field);
			nodeValuesCount = nodeField->getTotalValuesCount();
		}
		this->nodeParameterMap.setValue(node->getIndex(), parameterIndex);
		this->parameterNodeMap.setValues(parameterIndex, nodeValuesCount, node->getIndex());
		parameterIndex += nodeValuesCount;
	}
	this->parametersCount = parameterIndex;
	this->fieldModifyCounter = 0;  // concurrency point
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
	this->checkMaps();
	display_message(ERROR_MESSAGE, "Fieldparameters getNumberOfElementParameters:  Not implemented");
	return -1;
}

int FE_field_parameters::getNumberOfParameters()
{
	this->checkMaps();
	return this->parametersCount;
}
