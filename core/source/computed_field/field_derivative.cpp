/**
 * FILE : field_derivative.cpp
 *
 * Field derivative defining order and type of derivative operator to apply
 * to any given field. Each FieldDerivative has a unique index within its
 * owning region for efficient look up in field cache. Object describes how to
 * evaluate derivative, including links to next lower FieldDerivative so
 * can evaluate downstream derivatives using rules.
 */
 /* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/status.h"
#include "computed_field/field_derivative.hpp"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_region.h"
#include "general/message.h"
#include "region/cmiss_region.hpp"


FieldDerivative::FieldDerivative(cmzn_region *regionIn, FE_mesh *meshIn, int meshOrderIn, FieldDerivative *lowerDerivativeIn) :
	region(nullptr),  // set by cmzn_region_add_field_derivative
	lowerDerivative((lowerDerivativeIn) ? lowerDerivativeIn->access() : nullptr),
	cacheIndex(-1),
	mesh(meshIn),
	meshDimension((meshIn) ? meshIn->getDimension() : 0),
	meshOrder(meshOrderIn),
	access_count(1)
{
	regionIn->addFieldDerivative(this);
}

FieldDerivative::~FieldDerivative()
{
	if (this->region)
		this->region->removeFieldDerivative(this);
	if (this->lowerDerivative)
		FieldDerivative::deaccess(this->lowerDerivative);
}

FieldDerivative *FieldDerivative::createMeshDerivative(FE_mesh *mesh, FieldDerivative *lowerDerivative)
{
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "FieldDerivative::createMeshDerivative.  Missing mesh");
		return nullptr;
	}
	int meshOrder = 1;
	if (lowerDerivative)
	{
		if ((lowerDerivative->mesh) && (lowerDerivative->mesh != mesh))
		{
			display_message(ERROR_MESSAGE, "FieldDerivative::createMeshDerivative.  Cannot create derivative w.r.t. multiple meshes");
			return nullptr;
		}
		meshOrder += lowerDerivative->meshOrder;
	}
	cmzn_region *region = FE_region_get_cmzn_region(mesh->get_FE_region());
	return new FieldDerivative(region, mesh, meshOrder, lowerDerivative);
}

int FieldDerivative::deaccess(FieldDerivative* &field_derivative)
{
	if (!field_derivative)
		return CMZN_ERROR_ARGUMENT;
	--(field_derivative->access_count);
	if (field_derivative->access_count <= 0)
		delete field_derivative;
	field_derivative = 0;
	return CMZN_OK;
}

int FieldDerivative::getMeshTermCount() const
{
	int termCount = 1;
	for (int d = 0; d < this->meshOrder; ++d)
		termCount *= this->meshDimension;
	return termCount;
}

