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
#include "region/cmiss_region.h"


FieldDerivative::FieldDerivative(cmzn_region *regionIn, int orderIn, Type typeIn, FieldDerivative *lowerDerivative) :
	region(nullptr),  // set by cmzn_region_add_field_derivative
	lowerDerivative((orderIn > 1) ? lowerDerivative->access() : nullptr),
	cacheIndex(-1),
	order(orderIn),
	type(typeIn),
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

FieldDerivativeMesh::FieldDerivativeMesh(FE_mesh *meshIn, int orderIn, FieldDerivative *lowerDerivative) :
	FieldDerivative(FE_region_get_cmzn_region(meshIn->get_FE_region()), orderIn, TYPE_ELEMENT_XI, lowerDerivative),
	mesh(meshIn),
	elementDimension(mesh->getDimension())
{
}
