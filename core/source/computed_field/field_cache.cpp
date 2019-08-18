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
#include "opencmiss/zinc/field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/field_module.hpp"
#include "finite_element/finite_element.h"
#include "general/message.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"

FieldValueCache::~FieldValueCache()
{
	if (extraCache)
		cmzn_fieldcache::deaccess(extraCache);
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

void MeshLocationFieldValueCache::clear()
{
	cmzn_element_destroy(&element);
	FieldValueCache::clear();
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

cmzn_fieldcache::~cmzn_fieldcache()
{
	for (ValueCacheVector::iterator iter = valueCaches.begin(); iter < valueCaches.end(); ++iter)
	{
		delete (*iter);
		*iter = 0;
	}
	cmzn_region_remove_field_cache(region, this);
	cmzn_region_destroy(&region);
}

cmzn_fieldcache *cmzn_fieldcache::create(cmzn_region_id regionIn)
{
	if (regionIn)
		return new cmzn_fieldcache(regionIn);
	return 0;
}

void cmzn_fieldcache::copyLocation(const cmzn_fieldcache &source)
{
	switch (source.location->get_type())
	{
	case Field_location::TYPE_ELEMENT_XI:
	{
		this->location_element_xi.set_element_xi(
			source.location_element_xi.get_element(),
			source.location_element_xi.get_xi(),
			source.location_element_xi.get_top_level_element());
		this->location = &this->location_element_xi;
	} break;
	case Field_location::TYPE_FIELD_VALUES:
	{
		this->location_field_values.set_field_values(
			source.location_field_values.get_field(),
			source.location_field_values.get_number_of_values(),
			source.location_field_values.get_values());
		this->location = &this->location_field_values;
	} break;
	case Field_location::TYPE_NODE:
	{
		this->location_node.set_node(
			source.location_node.get_node());
		this->location = &this->location_node;
	} break;
	case Field_location::TYPE_TIME:
	{
		this->location = &this->location_time;
	} break;
	case Field_location::TYPE_INVALID:
	{
		display_message(ERROR_MESSAGE, "cmzn_fieldcache::copyLocation.  Invalid location type");
		this->location = &this->location_time;
	} break;
	}
	this->location->set_time(source.location->get_time());
	this->locationChanged();
}

int cmzn_fieldcache::setFieldReal(cmzn_field_id field, int numberOfValues, const double *values)
{
	// to support the xi field which has 3 components regardless of dimensions, do not
	// check (numberOfValues >= field->number_of_components), just pad with zeros
	if (!(field && field->isNumerical() && (numberOfValues > 0) && values))
		return CMZN_ERROR_ARGUMENT;
	// still need to set location_field_values because image processing uses it
	this->location_field_values.set_field_values(field, numberOfValues, values);
	if (this->location != &this->location_field_values)
	{
		this->location_field_values.set_time(this->location->get_time());
		this->location = &this->location_field_values;
	}
	locationChanged();  // must do first to ensure cache is valid below
	// now put the values in the cache. Note does not support derivatives!
	RealFieldValueCache *valueCache = RealFieldValueCache::cast(field->getValueCache(*this));
	for (int i = 0; i < field->number_of_components; i++)
	{
		valueCache->values[i] = (i < numberOfValues) ? values[i] : 0.0;
	}
	valueCache->derivatives_valid = 0;
	valueCache->evaluationCounter = locationCounter;
	return CMZN_OK;
}

/*
Global functions
----------------
*/

cmzn_fieldcache_id cmzn_fieldmodule_create_fieldcache(cmzn_fieldmodule_id field_module)
{
	if (field_module)
		return new cmzn_fieldcache(cmzn_fieldmodule_get_region_internal(field_module));
	return 0;
}

cmzn_fieldcache_id cmzn_fieldcache_access(cmzn_fieldcache_id cache)
{
	if (cache)
		return cache->access();
	return 0;
}

int cmzn_fieldcache_destroy(cmzn_fieldcache_id *cache_address)
{
	if (!cache_address)
		return CMZN_ERROR_ARGUMENT;
	return cmzn_fieldcache::deaccess(*cache_address);
}

int cmzn_fieldcache_clear_location(cmzn_fieldcache_id cache)
{
	if (cache)
	{
		cache->clearLocation();
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_fieldcache_set_time(cmzn_fieldcache_id cache, double time)
{
	if (!cache)
		return CMZN_ERROR_ARGUMENT;
	cache->setTime(time);
	return CMZN_OK;
}

int cmzn_fieldcache_set_element(cmzn_fieldcache_id cache,
	cmzn_element_id element)
{
	if (cache)
		return cache->setElement(element);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_fieldcache_set_mesh_location_with_parent(
	cmzn_fieldcache_id cache, cmzn_element_id element,
	int number_of_chart_coordinates, const double *chart_coordinates,
	cmzn_element_id top_level_element)
{
	if (!(cache && element && (number_of_chart_coordinates >= cmzn_element_get_dimension(element))))
		return CMZN_ERROR_ARGUMENT;
	cache->setMeshLocation(element, chart_coordinates, top_level_element);
	return CMZN_OK;
}

int cmzn_fieldcache_set_mesh_location(cmzn_fieldcache_id cache,
	cmzn_element_id element, int number_of_chart_coordinates,
	const double *chart_coordinates)
{
	return cmzn_fieldcache_set_mesh_location_with_parent(cache, element,
		number_of_chart_coordinates, chart_coordinates, /*top_level_element*/0);
}

int cmzn_fieldcache_set_node(cmzn_fieldcache_id cache, cmzn_node_id node)
{
	if (!(cache && node))
		return CMZN_ERROR_ARGUMENT;
	return cache->setNode(node);
}

int cmzn_fieldcache_set_field_real(cmzn_fieldcache_id cache,
	cmzn_field_id reference_field, int number_of_values, const double *values)
{
	if (!cache)
		return CMZN_ERROR_ARGUMENT;
	return cache->setFieldReal(reference_field, number_of_values, values);
}

// Internal function
int cmzn_fieldcache_set_assign_in_cache(cmzn_fieldcache_id cache, int assign_in_cache)
{
	if (!cache)
		return 0;
	cache->setAssignInCacheOnly(assign_in_cache != 0);
	return 1;
}

