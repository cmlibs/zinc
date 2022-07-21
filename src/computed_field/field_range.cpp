/**
 * FILE : field_range.cpp
 *
 * Stores range of a fields values over a chosen domain, the locations at
 * which each component minimum or maximum occurs and the field values there.
 */
 /* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/status.h"
#include "computed_field/field_range.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"
#include "finite_element/finite_element_discretization.h"
#include "general/message.h"
#include <cmath>

cmzn_fieldrange::cmzn_fieldrange(cmzn_fieldcache *fieldcacheIn) :
	fieldcache(fieldcacheIn->access()),
	field(nullptr),
	validRange(false),
	componentMinimumLocations(nullptr),
	componentMaximumLocations(nullptr),
	componentMinimumValues(nullptr),
	componentMaximumValues(nullptr),
	access_count(1)
{
	fieldcache->addFieldrange(this);
}

cmzn_fieldrange::~cmzn_fieldrange()
{
	this->clear();
	fieldcache->removeFieldrange(this);
	cmzn_fieldcache::deaccess(this->fieldcache);
}

void cmzn_fieldrange::clear()
{
	cmzn_field::deaccess(this->field);
	delete[] this->componentMinimumLocations;
	this->componentMinimumLocations = nullptr;
	delete[] this->componentMaximumLocations;
	this->componentMaximumLocations = nullptr;
	delete[] this->componentMinimumValues;
	this->componentMinimumValues = nullptr;
	delete[] this->componentMaximumValues;
	this->componentMaximumValues = nullptr;
}

void cmzn_fieldrange::setupForField(cmzn_field *field)
{
	this->clear();
	this->field = field->access();
	const int componentsCount = field->getNumberOfComponents();
	this->componentMinimumLocations = new Field_location_element_xi[componentsCount];
	this->componentMaximumLocations = new Field_location_element_xi[componentsCount];
	this->componentMinimumValues = new FE_value[componentsCount * componentsCount];
	this->componentMaximumValues = new FE_value[componentsCount * componentsCount];
}

cmzn_fieldrange *cmzn_fieldrange::create(cmzn_fieldcache *fieldcacheIn)
{
	if (fieldcacheIn)
	{
		return new cmzn_fieldrange(fieldcacheIn);
	}
	return nullptr;
}

void cmzn_fieldrange::deaccess(cmzn_fieldrange*& fieldrange)
{
	if (fieldrange)
	{
		--(fieldrange->access_count);
		if (fieldrange->access_count <= 0)
		{
			delete fieldrange;
		}
		fieldrange = nullptr;
	}
}

bool cmzn_fieldrange::findComponentLimit(cmzn_field *field, int componentIndex, cmzn_element *element,
	cmzn_fieldcache *fieldcache, FE_value *initialXi, bool findMinimum)
{
	FE_mesh *mesh = element->getMesh();
	const int meshDimension = mesh->getDimension();
	FieldDerivative *fieldDerivative1 = mesh->getFieldDerivative(/*order*/1);
	FieldDerivative *fieldDerivative2 = mesh->getFieldDerivative(/*order*/2);
	if (!((fieldDerivative1) && (fieldDerivative2)))
	{
		return false;
	}
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], lastXi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	FE_value xiDir[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	for (int i = 0; i < meshDimension; ++i)
	{
		lastXi[i] = xi[i] = initialXi[i];
	}
	int iter = 0;
	FE_value maxDeltaXi = 0.2;  // reduced after certain iterations
	const FE_value xiTolerance = 1e-06;
	FE_element_shape *elementShape = element->getElementShape();
	while (true)
	{
		fieldcache->setMeshLocation(element, xi);
		const DerivativeValueCache *derivative1Cache = field->evaluateDerivative(*fieldcache, *fieldDerivative1);
		if (!(derivative1Cache))
		{
			return false;
		}
		const FE_value *d1 = derivative1Cache->values + componentIndex * meshDimension;
		FE_value mag = 0.0;
		for (int i = 0; i < meshDimension; ++i)
		{
			mag += d1[i] * d1[i];
		}
		if (mag == 0.0)
		{
			break;  // no variation of field with location, use this location
		}
		const FE_value b = sqrt(mag);  // first derivative in direction of d1
		for (int i = 0; i < meshDimension; ++i)
		{
			xiDir[i] = d1[i] / b;
		}
		const DerivativeValueCache *derivative2Cache = field->evaluateDerivative(*fieldcache, *fieldDerivative2);
		if (!(derivative2Cache))
		{
			return false;
		}
		const FE_value *d2 = derivative2Cache->values + componentIndex * meshDimension * meshDimension;
		// get second derivative in direction of d1
		FE_value a = 0.0;
		for (int i = 0; i < meshDimension; ++i)
		{
			for (int j = 0; j < meshDimension; ++j)
			{
				a += d2[j * meshDimension + i] * xiDir[i] * xiDir[j];
			}
		}
		// have limits on delta xi to handle high field curvature
		if (iter == 5)
		{
			maxDeltaXi *= 0.5;
		}
		else if (iter == 20)
		{
			maxDeltaXi *= 0.25;
		}
		// step in direction of d1 (or reverse if findMinimum)
		FE_value x = findMinimum ? -maxDeltaXi : maxDeltaXi;
		if (((a > 0.0) && findMinimum) || ((a < 0.0) && (!findMinimum)))
		{
			// minimum/maximum is at point where quadratic equation ax^2 + bx has zero slope:
			x = -b / (2.0 * a);
			// limit xi increment
			if (x < -maxDeltaXi)
			{
				x = -maxDeltaXi;
			}
			else if (x > maxDeltaXi)
			{
				x = maxDeltaXi;
			}
		}
		for (int i = 0; i < meshDimension; ++i)
		{
			xi[i] += x * xiDir[i];
		}
		FE_element_shape_limit_xi_to_element(elementShape, xi, /*tolerance*/0.0);
		++iter;
		if (fabs(x) < xiTolerance)
		{
			display_message(INFORMATION_MESSAGE, "Field evaluateRange.  Field %s component %d find %s in element %d "
				"converged by increment within tolerance in %d iterations", field->name, componentIndex + 1,
				findMinimum ? "minimum" : "maximum", element->getIdentifier(), iter);
			break;
		}
		bool sameXi = true;
		for (int i = 0; i < meshDimension; ++i)
		{
			if (fabs(xi[i] - lastXi[i]) > xiTolerance)
			{
				sameXi = false;
				break;
			}
		}
		if (sameXi)
		{
			display_message(INFORMATION_MESSAGE, "Field evaluateRange.  Field %s component %d find %s in element %d "
				"converged by xi change within tolerance in %d iterations", field->name, componentIndex + 1,
				findMinimum ? "minimum" : "maximum", element->getIdentifier(), iter);
			break;
		}
		if (iter == 100)
		{
			display_message(WARNING_MESSAGE, "Field evaluateRange.  Field %s component %d find %s in element %d "
				"stopped without convergence in %d iterations", field->name, componentIndex + 1,
				findMinimum ? "minimum" : "maximum", element->getIdentifier(), iter);
			break;
		}
		for (int i = 0; i < meshDimension; ++i)
		{
			lastXi[i] = xi[i];
		}
	}
	// store mesh location and field values
	Field_location_element_xi *element_xi_location = (findMinimum ? this->componentMinimumLocations : this->componentMaximumLocations) + componentIndex;
	element_xi_location->set_element_xi(element, xi);
	fieldcache->setMeshLocation(element, xi);
	const int componentsCount = field->getNumberOfComponents();
	FE_value *values = (findMinimum ? this->componentMinimumValues : this->componentMaximumValues) + componentIndex * componentsCount;
	cmzn_field_evaluate_real(field, fieldcache, componentsCount, values);
	return true;
}

int cmzn_fieldrange::evaluateElementRange(cmzn_field *field, cmzn_element *element, cmzn_fieldcache *fieldcache)
{
	if (!((field) && (field->isNumerical()) && (element) && (fieldcache) &&
		(fieldcache == this->fieldcache) &&
		(fieldcache->getRegion() == field->getRegion())))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	FE_element_shape *elementShape = element->getElementShape();
	int xiPointsCount;
	FE_value_triple *xiPoints;
	const int numberInXi[MAXIMUM_ELEMENT_XI_DIMENSIONS] = { 3, 3, 3 };
	int return_code = FE_element_shape_get_xi_points_cell_corners(elementShape,
		numberInXi, &xiPointsCount, &xiPoints);
	if ((!return_code) || (numberInXi <= 0))
	{
		display_message(ERROR_MESSAGE, "Field evaluateRange:  Unexpected failure sampling element xi coordinates");
		return CMZN_ERROR_GENERAL;
	}
	const int componentsCount = field->getNumberOfComponents();
	double *values = new double[xiPointsCount * componentsCount];
	int result = CMZN_OK;
	for (int i = 0; i < xiPointsCount; ++i)
	{
		fieldcache->setIndexedMeshLocation(i, element, xiPoints[i]);
		field->evaluate(*fieldcache);
		if (CMZN_OK != cmzn_field_evaluate_real(field, fieldcache, componentsCount, values + i * componentsCount))
		{
			result = CMZN_ERROR_NOT_FOUND;
			break;
		}
	}
	this->setupForField(field);
	// set up field range for field, reset.
	if (result == CMZN_OK)
	{
		for (int c = 0; c < componentsCount; ++c)
		{
			int iMin = 0;
			int iMax = 0;
			double vMin = values[c];
			double vMax = values[c];
			for (int i = 1; i < xiPointsCount; ++i)
			{
				const double v = values[i * componentsCount + c];
				if (v < vMin)
				{
					iMin = i;
					vMin = v;
				}
				else if (v > vMax)
				{
					iMax = i;
					vMax = v;
				}
			}
			if (!(this->findComponentLimit(field, c, element, fieldcache, xiPoints[iMin], /*findMinimum*/true) &&
				this->findComponentLimit(field, c, element, fieldcache, xiPoints[iMax], /*findMinimum*/false)))
			{
				display_message(ERROR_MESSAGE, "Field evaluateRange:  Unexpected failure to find element field limits");
				result = CMZN_ERROR_GENERAL;
				break;
			}
		}
		if (result == CMZN_OK)
		{
			this->validRange = true;
		}
	}
	DEALLOCATE(xiPoints);
	delete[] values;
	return result;
}

int cmzn_fieldrange::evaluateRange(cmzn_field *field, cmzn_fieldcache *fieldcache)
{
	if (!((field) && (fieldcache) &&
		(fieldcache == this->fieldcache) &&
		(fieldcache->getRegion() == field->getRegion())))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const Field_location_element_xi *elementXiLocation = fieldcache->get_location_element_xi();
	if (elementXiLocation)
	{
		return this->evaluateElementRange(field, elementXiLocation->get_element(), fieldcache);
	}
	display_message(ERROR_MESSAGE, "Field evaluateRange:  Only implemented for element/mesh locations");
	return CMZN_ERROR_NOT_IMPLEMENTED;
}

cmzn_element *cmzn_fieldrange::getComponentMinimumMeshLocation(int componentNumber,
	int coordinatesCount, FE_value *coordinatesOut)
{
	if ((this->field) && (this->validRange) && (0 <= componentNumber) &&
		(componentNumber < this->field->getNumberOfComponents()) &&
		(coordinatesOut))
	{
		Field_location_element_xi& element_xi_location = this->componentMinimumLocations[componentNumber];
		return element_xi_location.get_mesh_location_api(coordinatesCount, coordinatesOut);
	}
	return nullptr;
}

cmzn_element *cmzn_fieldrange::getComponentMaximumMeshLocation(int componentNumber,
	int coordinatesCount, FE_value *coordinatesOut)
{
	if ((this->field) && (this->validRange) && (0 <= componentNumber) &&
		(componentNumber < this->field->getNumberOfComponents()) &&
		(coordinatesOut))
	{
		Field_location_element_xi& element_xi_location = this->componentMaximumLocations[componentNumber];
		return element_xi_location.get_mesh_location_api(coordinatesCount, coordinatesOut);
	}
	return nullptr;
}

int cmzn_fieldrange::getComponentMinimumValues(int componentNumber,
	int valuesCount, FE_value *valuesOut)
{
	if (!((this->field) && (this->validRange)))
	{
		return CMZN_ERROR_NOT_FOUND;
	}
	const int componentsCount = this->field->getNumberOfComponents();
	if ((componentNumber < 0) || (componentNumber >= componentsCount)
		|| (valuesCount < componentsCount) || (!valuesOut))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int offset = componentNumber * componentsCount;
	for (int c = 0; c < componentsCount; ++c)
	{
		valuesOut[c] = this->componentMinimumValues[offset + c];
	}
	return CMZN_OK;
}

int cmzn_fieldrange::getComponentMaximumValues(int componentNumber,
	int valuesCount, FE_value *valuesOut)
{
	if (!((this->field) && (this->validRange)))
	{
		return CMZN_ERROR_NOT_FOUND;
	}
	const int componentsCount = this->field->getNumberOfComponents();
	if ((componentNumber < 0) || (componentNumber >= componentsCount)
		|| (valuesCount < componentsCount) || (!valuesOut))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int offset = componentNumber * componentsCount;
	for (int c = 0; c < componentsCount; ++c)
	{
		valuesOut[c] = this->componentMaximumValues[offset + c];
	}
	return CMZN_OK;
}

int cmzn_fieldrange::getRange(int valuesCount, FE_value *minimumValuesOut, FE_value *maximumValuesOut) const
{
	if (!((this->field) && (this->validRange)))
	{
		return CMZN_ERROR_NOT_FOUND;
	}
	const int componentsCount = this->field->getNumberOfComponents();
	if ((valuesCount < this->field->getNumberOfComponents())
		|| (!minimumValuesOut) || (!maximumValuesOut))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	for (int c = 0; c < componentsCount; ++c)
	{
		minimumValuesOut[c] = this->componentMinimumValues[c];
		maximumValuesOut[c] = this->componentMaximumValues[c];
	}
	for (int mc = 1; mc < componentsCount; ++mc)
	{
		const int offset = mc * componentsCount;
		for (int c = 0; c < componentsCount; ++c)
		{
			if (this->componentMinimumValues[offset + c] < minimumValuesOut[c])
			{
				minimumValuesOut[c] = this->componentMinimumValues[offset + c];
			}
			if (this->componentMaximumValues[offset + c] > maximumValuesOut[c])
			{
				maximumValuesOut[c] = this->componentMaximumValues[offset + c];
			}
		}
	}
	return CMZN_OK;
}

/*
Global functions
----------------
*/

cmzn_fieldrange_id cmzn_fieldrange_access(cmzn_fieldrange_id fieldrange)
{
	if (fieldrange)
	{
		return fieldrange->access();
	}
	return nullptr;
}

int cmzn_fieldrange_destroy(cmzn_fieldrange_id *fieldrange_address)
{
	if (fieldrange_address)
	{
		cmzn_fieldrange::deaccess(*fieldrange_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_element_id cmzn_fieldrange_get_component_minimum_mesh_location(
	cmzn_fieldrange_id fieldrange, int componentNumber,
	int coordinatesCount, double *coordinatesOut)
{
	if (fieldrange)
	{
		return fieldrange->getComponentMinimumMeshLocation(componentNumber - 1, coordinatesCount, coordinatesOut);
	}
	return nullptr;
}

cmzn_element_id cmzn_fieldrange_get_component_maximum_mesh_location(
	cmzn_fieldrange_id fieldrange, int componentNumber,
	int coordinatesCount, double *coordinatesOut)
{
	if (fieldrange)
	{
		return fieldrange->getComponentMaximumMeshLocation(componentNumber - 1, coordinatesCount, coordinatesOut);
	}
	return nullptr;
}

int cmzn_fieldrange_get_component_minimum_values_real(
	cmzn_fieldrange_id fieldrange, int componentNumber,
	int valuesCount, double *valuesOut)
{
	if (fieldrange)
	{
		return fieldrange->getComponentMinimumValues(componentNumber - 1, valuesCount, valuesOut);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_fieldrange_get_component_maximum_values_real(
	cmzn_fieldrange_id fieldrange, int componentNumber,
	int valuesCount, double *valuesOut)
{
	if (fieldrange)
	{
		return fieldrange->getComponentMaximumValues(componentNumber - 1, valuesCount, valuesOut);
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_field_id cmzn_fieldrange_get_field(cmzn_fieldrange_id fieldrange)
{
	if (fieldrange)
	{
		cmzn_field *field = fieldrange->getField();
		if (field)
		{
			return field->access();
		}
	}
	return nullptr;
}

int cmzn_fieldrange_get_range_real(cmzn_fieldrange_id fieldrange,
	int valuesCount, double *minimumValuesOut, double *maximumValuesOut)
{
	if (fieldrange)
	{
		return fieldrange->getRange(valuesCount, minimumValuesOut, maximumValuesOut);
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_fieldrange_has_valid_range(cmzn_fieldrange_id fieldrange)
{
	if (fieldrange)
	{
		return fieldrange->hasValidRange();
	}
	return false;
}
