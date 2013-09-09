/***************************************************************************//**
 * FILE : field_cache.cpp
 * 
 * Implements a cache for prescribed domain locations and field evaluations.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstdio>
#include "zinc/field.h"
#include "computed_field/computed_field_find_xi.h"
#include "finite_element/finite_element.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"

FieldValueCache::~FieldValueCache()
{
	if (extraCache)
		cmzn_field_cache::deaccess(extraCache);
}

void FieldValueCache::clear()
{
	resetEvaluationCounter();
}

RealFieldValueCache::~RealFieldValueCache()
{
	if (find_element_xi_cache)
	{
		DESTROY(Computed_field_find_element_xi_cache)(&find_element_xi_cache);
		find_element_xi_cache = 0;
	}
	delete[] values;
	delete[] derivatives;
}

void RealFieldValueCache::clear()
{
	if (find_element_xi_cache)
	{
		DESTROY(Computed_field_find_element_xi_cache)(&find_element_xi_cache);
		find_element_xi_cache = 0;
	}
	FieldValueCache::clear();
}

char *RealFieldValueCache::getAsString()
{
	char *valueAsString = 0;
	int error = 0;
	char tmp_string[50];
	for (int i = 0; i < componentCount; i++)
	{
		if (0 < i)
		{
			sprintf(tmp_string, ", %g", values[i]);
		}
		else
		{
			sprintf(tmp_string, "%g", values[i]);
		}
		append_string(&valueAsString, tmp_string, &error);
	}
	return valueAsString;
}

void StringFieldValueCache::setString(const char *string_in)
{
	if (stringValue)
		DEALLOCATE(stringValue);
	stringValue = duplicate_string(string_in);
}

char *StringFieldValueCache::getAsString()
{
	return duplicate_string(stringValue);
}

char *MeshLocationFieldValueCache::getAsString()
{
	if (!element)
		return 0;
	char *valueAsString = 0;
	int error = 0;
	char tmp_string[50];
	sprintf(tmp_string,"%d :", cmzn_element_get_identifier(element));
	append_string(&valueAsString, tmp_string, &error);
	int dimension = cmzn_element_get_dimension(element);
	for (int i = 0; i < dimension; i++)
	{
		sprintf(tmp_string, " %g", xi[i]);
		append_string(&valueAsString, tmp_string, &error);
	}
	return valueAsString;
}

cmzn_field_cache::~cmzn_field_cache()
{
	for (ValueCacheVector::iterator iter = valueCaches.begin(); iter < valueCaches.end(); ++iter)
	{
		delete (*iter);
		*iter = 0;
	}
	cmzn_region_remove_field_cache(region, this);
	delete location;
	cmzn_region_destroy(&region);
}

int cmzn_field_cache::setFieldReal(cmzn_field_id field, int numberOfValues, const double *values)
{
	// to support the xi field which has 3 components regardless of dimensions, do not
	// check (numberOfValues >= field->number_of_components), just pad with zeros
	if (!(field && field->isNumerical() && (numberOfValues > 0) && values))
		return 0;
	RealFieldValueCache *valueCache = RealFieldValueCache::cast(field->getValueCache(*this));
	for (int i = 0; i < field->number_of_components; i++)
	{
		valueCache->values[i] = (i < numberOfValues) ? values[i] : 0.0;
	}
	valueCache->derivatives_valid = 0;
	locationChanged();
	valueCache->evaluationCounter = locationCounter;
	FE_value time = location->get_time();
	// still need to create Field_coordinate_location because image processing fields dynamic cast to recognise
	delete location;
	location = new Field_coordinate_location(field, numberOfValues, values, time);
	return 1;
}

int cmzn_field_cache::setFieldRealWithDerivatives(cmzn_field_id field, int numberOfValues, const double *values,
	int numberOfDerivatives, const double *derivatives)
{
	// to support the xi field which has 3 components regardless of dimensions, do not
	// check (numberOfValues >= field->number_of_components), just pad with zeros
	if (!(field && field->isNumerical() && (numberOfValues > 0) && values &&
		(0 < numberOfDerivatives) && (numberOfDerivatives <= MAXIMUM_ELEMENT_XI_DIMENSIONS) && derivatives))
		return 0;
	RealFieldValueCache *valueCache = RealFieldValueCache::cast(field->getValueCache(*this));
	for (int i = 0; i < field->number_of_components; i++)
	{
		valueCache->values[i] = (i < numberOfValues) ? values[i] : 0.0;
	}
	const int size = field->number_of_components * numberOfDerivatives;
	const int suppliedSize = numberOfValues * numberOfDerivatives;
	for (int i = 0; i < size; i++)
	{
		valueCache->derivatives[i] = (i < suppliedSize) ? derivatives[i] : 0.0;
	}
	valueCache->derivatives_valid = 1;
	locationChanged();
	valueCache->evaluationCounter = locationCounter;
	FE_value time = location->get_time();
	delete location;
	location = new Field_coordinate_location(field, numberOfValues, values, time, numberOfDerivatives, derivatives);
	return 1;
}

/*
Global functions
----------------
*/

cmzn_field_cache_id cmzn_field_module_create_cache(cmzn_field_module_id field_module)
{
	if (field_module)
		return new cmzn_field_cache(cmzn_field_module_get_region_internal(field_module));
	return 0;
}

cmzn_field_cache_id cmzn_field_cache_access(cmzn_field_cache_id cache)
{
	return cache->access();
}

int cmzn_field_cache_destroy(cmzn_field_cache_id *cache_address)
{
	if (!cache_address)
		return 0;
	return cmzn_field_cache::deaccess(*cache_address);
}

int cmzn_field_cache_set_time(cmzn_field_cache_id cache, double time)
{
	if (!cache)
		return 0;
	cache->setTime(time);
	return 1;
}

int cmzn_field_cache_set_element(cmzn_field_cache_id cache,
	cmzn_element_id element)
{
	const double chart_coordinates[MAXIMUM_ELEMENT_XI_DIMENSIONS] = { 0.0, 0.0, 0.0 };
	return cmzn_field_cache_set_mesh_location_with_parent(cache, element,
		MAXIMUM_ELEMENT_XI_DIMENSIONS, chart_coordinates, /*top_level_element*/0);
}

int cmzn_field_cache_set_mesh_location_with_parent(
	cmzn_field_cache_id cache, cmzn_element_id element,
	int number_of_chart_coordinates, const double *chart_coordinates,
	cmzn_element_id top_level_element)
{
	if (!(cache && element && (number_of_chart_coordinates >= cmzn_element_get_dimension(element))))
		return 0;
	cache->setMeshLocation(element, chart_coordinates, top_level_element);
	return 1;
}

int cmzn_field_cache_set_mesh_location(cmzn_field_cache_id cache,
	cmzn_element_id element, int number_of_chart_coordinates,
	const double *chart_coordinates)
{
	return cmzn_field_cache_set_mesh_location_with_parent(cache, element,
		number_of_chart_coordinates, chart_coordinates, /*top_level_element*/0);
}

int cmzn_field_cache_set_node(cmzn_field_cache_id cache, cmzn_node_id node)
{
	if (!(cache && node))
		return 0;
	cache->setNode(node);
	return 1;
}

int cmzn_field_cache_set_field_real(cmzn_field_cache_id cache,
	cmzn_field_id reference_field, int number_of_values, const double *values)
{
	if (!cache)
		return 0;
	return cache->setFieldReal(reference_field, number_of_values, values);
}

// Internal function
int cmzn_field_cache_set_assign_in_cache(cmzn_field_cache_id cache, int assign_in_cache)
{
	if (!cache)
		return 0;
	cache->setAssignInCacheOnly(assign_in_cache != 0);
	return 1;
}

