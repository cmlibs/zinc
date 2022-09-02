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
#include "computed_field/field_range.hpp"
#include "finite_element/finite_element.h"
#include "general/message.h"
#include "general/mystring.h"
#include "region/cmiss_region.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"

FieldValueCache::~FieldValueCache()
{
	if (extraCache)
		cmzn_fieldcache::deaccess(extraCache);
}

void FieldValueCache::clear()
{
	this->resetEvaluationCounter();
}

RealFieldValueCache::~RealFieldValueCache()
{
	delete[] this->values;
	for (std::vector<DerivativeValueCache *>::iterator iter = this->derivatives.begin(); iter != this->derivatives.end(); ++iter)
		delete *iter;
}

void RealFieldValueCache::resetEvaluationCounter()
{
	for (std::vector<DerivativeValueCache *>::iterator iter = this->derivatives.begin(); iter != this->derivatives.end(); ++iter)
		if (*iter)
			(*iter)->resetEvaluationCounter();
	FieldValueCache::resetEvaluationCounter();
}

char *RealFieldValueCache::getAsString() const
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

char *StringFieldValueCache::getAsString() const
{
	return duplicate_string(this->stringValue);
}

void MeshLocationFieldValueCache::clear()
{
	cmzn_element::deaccess(this->element);
	FieldValueCache::clear();
}

char *MeshLocationFieldValueCache::getAsString() const
{
	if (!this->element)
		return nullptr;
	char *valueAsString = 0;
	int error = 0;
	char tmp_string[50];
	sprintf(tmp_string,"%d :", element->getIdentifier());
	append_string(&valueAsString, tmp_string, &error);
	const int dimension = element->getDimension();
	for (int i = 0; i < dimension; i++)
	{
		sprintf(tmp_string, " %g", xi[i]);
		append_string(&valueAsString, tmp_string, &error);
	}
	return valueAsString;
}

cmzn_fieldcache::cmzn_fieldcache(cmzn_region *regionIn, cmzn_fieldcache *parentCacheIn) :
	region(cmzn_region_access(regionIn)),
	locationCounter(0),
	modifyCounter(-1),
	indexed_location_element_xi(0),
	number_of_indexed_location_element_xi(0),
	location(&(this->location_time)),
	valueCaches(this->region->getFieldcacheSize(), (FieldValueCache*)0),
	assignInCache(false),
	parentCache(parentCacheIn),
	sharedWorkingCache(0),
	access_count(1)
{
	this->region->addFieldcache(this);
}

cmzn_fieldcache::~cmzn_fieldcache()
{
	if (this->sharedWorkingCache)
		this->sharedWorkingCache->parentCache = 0;  // detach first in case code tries to use it as this is being destroyed
	this->location = &this->location_time;
	delete[] indexed_location_element_xi;
	this->indexed_location_element_xi = 0;
	this->number_of_indexed_location_element_xi = 0;
	for (ValueCacheVector::iterator iter = this->valueCaches.begin(); iter != this->valueCaches.end(); ++iter)
	{
		delete (*iter);
		*iter = 0;
	}
	if (this->sharedWorkingCache)
	{
		cmzn_fieldcache::deaccess(this->sharedWorkingCache);  // should be the last reference as value caches destroyed above
	}
	for (RegionFieldcacheMap::iterator iter = this->sharedExternalWorkingCacheMap.begin(); iter != this->sharedExternalWorkingCacheMap.end(); ++iter)
	{
		cmzn_fieldcache::deaccess(iter->second);  // should be the last reference as value caches destroyed above
	}
	this->region->removeFieldcache(this);
	cmzn_region::deaccess(this->region);
}

cmzn_fieldcache *cmzn_fieldcache::create(cmzn_region *regionIn, cmzn_fieldcache *parentCacheIn)
{
	if (regionIn)
	{
		return new cmzn_fieldcache(regionIn, parentCacheIn);
	}
	return nullptr;
}

void cmzn_fieldcache::deaccess(cmzn_fieldcache*& fieldcache)
{
	if (fieldcache)
	{
		--(fieldcache->access_count);
		if (fieldcache->access_count <= 0)
		{
			delete fieldcache;
		}
		fieldcache = nullptr;
	}
}

void cmzn_fieldcache::copyLocation(const cmzn_fieldcache &source)
{
	switch (source.location->get_type())
	{
	case Field_location::TYPE_ELEMENT_XI:
	{
		// source location may be location_element_xi or an indexed_location_element_xi
		// Future: transfer indexed locations
		Field_location_element_xi& source_location_element_xi = static_cast<Field_location_element_xi&>(*source.location);
		this->location_element_xi.set_element_xi(
			source_location_element_xi.get_element(),
			source_location_element_xi.get_xi(),
			source_location_element_xi.get_top_level_element());
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

int cmzn_fieldcache::setIndexedMeshLocation(unsigned int index,
	cmzn_element *element, const double *chart_coordinates,
	cmzn_element *top_level_element)
{
	if (!(element && chart_coordinates))
		return CMZN_ERROR_ARGUMENT;
	Field_location_element_xi *element_xi_location;
	const FE_value time = this->location->get_time();
	if (index < this->number_of_indexed_location_element_xi)
	{
		element_xi_location = &(this->indexed_location_element_xi[index]);
	}
	else if (index < 512)  // never store more than this number
	{
		// Grow from 0 --> 64 --> 512
		const unsigned int new_size = ((this->indexed_location_element_xi) || (index >= 64)) ? 512 : 64;
		Field_location_element_xi *new_indexed_location_element_xi = new Field_location_element_xi[new_size];
		if (new_indexed_location_element_xi)
		{
			delete[] this->indexed_location_element_xi;
			this->indexed_location_element_xi = new_indexed_location_element_xi;
			this->number_of_indexed_location_element_xi = new_size;
			element_xi_location = &(this->indexed_location_element_xi[index]);
			this->location = element_xi_location;  // ensure this->location is not pointing at freed memory
		}
		else
		{
			element_xi_location = &this->location_element_xi;
		}
	}
	else
	{
		element_xi_location = &this->location_element_xi;
	}
	element_xi_location->set_element_xi(element, chart_coordinates, top_level_element);
	element_xi_location->set_time(time);
	this->location = element_xi_location;
	this->locationChanged();
	return CMZN_OK;
}

int cmzn_fieldcache::setFieldReal(cmzn_field *field, int numberOfValues, const double *values)
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
		valueCache->values[i] = (i < numberOfValues) ? values[i] : 0.0;
	valueCache->evaluationCounter = locationCounter;
	return CMZN_OK;
}

cmzn_fieldcache *cmzn_fieldcache::getOrCreateSharedExternalWorkingCache(cmzn_region *regionIn)
{
	RegionFieldcacheMap::iterator iter = this->sharedExternalWorkingCacheMap.find(regionIn);
	if (iter != this->sharedExternalWorkingCacheMap.end())
	{
		return iter->second;
	}
	// as this is for a different region, do not set parentCache as not sharing finite element field caches
	cmzn_fieldcache *fieldcache = new cmzn_fieldcache(regionIn, /*parentCache*/nullptr);
	this->sharedExternalWorkingCacheMap[regionIn] = fieldcache;
	return fieldcache;
}

void cmzn_fieldcache::removeDerivativeCaches(int derivativeCacheIndex)
{
	for (ValueCacheVector::iterator iter = this->valueCaches.begin(); iter != this->valueCaches.end(); ++iter)
	{
		RealFieldValueCache *realValueCache = dynamic_cast<RealFieldValueCache *>(*iter);
		if (realValueCache)
			realValueCache->removeDerivativeValueCacheAtIndex(derivativeCacheIndex);
	}
}

/*
Global functions
----------------
*/

cmzn_fieldcache_id cmzn_fieldmodule_create_fieldcache(cmzn_fieldmodule_id fieldmodule)
{
	if (fieldmodule)
	{
		return cmzn_fieldcache::create(cmzn_fieldmodule_get_region_internal(fieldmodule));
	}
	return nullptr;
}

cmzn_fieldcache_id cmzn_fieldcache_access(cmzn_fieldcache_id cache)
{
	if (cache)
		return cache->access();
	return 0;
}

int cmzn_fieldcache_destroy(cmzn_fieldcache_id *fieldcache_address)
{
	if (fieldcache_address)
	{
		cmzn_fieldcache::deaccess(*fieldcache_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
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

cmzn_fieldrange_id cmzn_fieldcache_create_fieldrange(
	cmzn_fieldcache_id fieldcache)
{
	return cmzn_fieldrange::create(fieldcache);
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
	if ((cache) && (element) && (number_of_chart_coordinates >= element->getDimension()))
		return cache->setMeshLocation(element, chart_coordinates, top_level_element);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_fieldcache_set_mesh_location(cmzn_fieldcache_id cache,
	cmzn_element_id element, int number_of_chart_coordinates,
	const double *chart_coordinates)
{
	if ((cache) && (element) && (number_of_chart_coordinates >= element->getDimension()))
		return cache->setMeshLocation(element, chart_coordinates);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_fieldcache_set_node(cmzn_fieldcache_id cache, cmzn_node_id node)
{
	if ((cache) && (node))
		return cache->setNode(node);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_fieldcache_set_field_real(cmzn_fieldcache_id cache,
	cmzn_field_id reference_field, int number_of_values, const double *values)
{
	if (cache)
		return cache->setFieldReal(reference_field, number_of_values, values);
	return CMZN_ERROR_ARGUMENT;
}

// Internal function
int cmzn_fieldcache_set_assign_in_cache(cmzn_fieldcache_id cache, int assign_in_cache)
{
	if (!cache)
		return 0;
	cache->setAssignInCacheOnly(assign_in_cache != 0);
	return 1;
}

