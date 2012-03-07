/***************************************************************************//**
 * FILE : field_cache.cpp
 * 
 * Implements a cache for prescribed domain locations and field evaluations.
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <cstdio>
extern "C" {
#include "api/cmiss_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "finite_element/finite_element.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"

FieldValueCache::~FieldValueCache()
{
	if (extraCache)
		Cmiss_field_cache::deaccess(extraCache);
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
	sprintf(tmp_string,"%d :", Cmiss_element_get_identifier(element));
	append_string(&valueAsString, tmp_string, &error);
	int dimension = Cmiss_element_get_dimension(element);
	for (int i = 0; i < dimension; i++)
	{
		sprintf(tmp_string, " %g", xi[i]);
		append_string(&valueAsString, tmp_string, &error);
	}
	return valueAsString;
}

Cmiss_field_cache::~Cmiss_field_cache()
{
	for (ValueCacheVector::iterator iter = valueCaches.begin(); iter < valueCaches.end(); ++iter)
	{
		delete (*iter);
		*iter = 0;
	}
	Cmiss_region_remove_field_cache(region, this);
	delete location;
	Cmiss_region_destroy(&region);
}

int Cmiss_field_cache::setFieldReal(Cmiss_field_id field, int numberOfValues, const double *values)
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

int Cmiss_field_cache::setFieldRealWithDerivatives(Cmiss_field_id field, int numberOfValues, const double *values,
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

Cmiss_field_cache_id Cmiss_field_module_create_cache(Cmiss_field_module_id field_module)
{
	if (field_module)
		return new Cmiss_field_cache(Cmiss_field_module_get_region_internal(field_module));
	return 0;
}

Cmiss_field_cache_id Cmiss_field_cache_access(Cmiss_field_cache_id cache)
{
	return cache->access();
}

int Cmiss_field_cache_destroy(Cmiss_field_cache_id *cache_address)
{
	if (!cache_address)
		return 0;
	return Cmiss_field_cache::deaccess(*cache_address);
}

int Cmiss_field_cache_set_time(Cmiss_field_cache_id cache, double time)
{
	if (!cache)
		return 0;
	cache->setTime(time);
	return 1;
}

int Cmiss_field_cache_set_element(Cmiss_field_cache_id cache,
	Cmiss_element_id element)
{
	const double chart_coordinates[MAXIMUM_ELEMENT_XI_DIMENSIONS] = { 0.0, 0.0, 0.0 };
	return Cmiss_field_cache_set_mesh_location_with_parent(cache, element,
		MAXIMUM_ELEMENT_XI_DIMENSIONS, chart_coordinates, /*top_level_element*/0);
}

int Cmiss_field_cache_set_mesh_location_with_parent(
	Cmiss_field_cache_id cache, Cmiss_element_id element,
	int number_of_chart_coordinates, const double *chart_coordinates,
	Cmiss_element_id top_level_element)
{
	if (!(cache && element && (number_of_chart_coordinates == Cmiss_element_get_dimension(element))))
		return 0;
	cache->setMeshLocation(element, chart_coordinates, top_level_element);
	return 1;
}

int Cmiss_field_cache_set_mesh_location(Cmiss_field_cache_id cache,
	Cmiss_element_id element, int number_of_chart_coordinates,
	const double *chart_coordinates)
{
	return Cmiss_field_cache_set_mesh_location_with_parent(cache, element,
		number_of_chart_coordinates, chart_coordinates, /*top_level_element*/0);
}

int Cmiss_field_cache_set_node(Cmiss_field_cache_id cache, Cmiss_node_id node)
{
	if (!(cache && node))
		return 0;
	cache->setNode(node);
	return 1;
}

int Cmiss_field_cache_set_field_real(Cmiss_field_cache_id cache,
	Cmiss_field_id reference_field, int number_of_values, const double *values)
{
	if (!cache)
		return 0;
	return cache->setFieldReal(reference_field, number_of_values, values);
}

// Internal function
int Cmiss_field_cache_set_assign_in_cache(Cmiss_field_cache_id cache, int assign_in_cache)
{
	if (!cache)
		return 0;
	cache->setAssignInCacheOnly(assign_in_cache != 0);
	return 1;
}

