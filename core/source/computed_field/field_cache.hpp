/***************************************************************************//**
 * FILE : field_cache.hpp
 * 
 * Implements a cache for prescribed domain locations and field evaluations.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (FIELD_CACHE_HPP)
#define FIELD_CACHE_HPP

#include "zinc/element.h"
#include "zinc/field.h"
#include "zinc/fieldmodule.h"
#include "zinc/region.h"
#include "general/debug.h"
#include "region/cmiss_region.h"
#include "computed_field/field_location.hpp"
#include <vector>

struct Computed_field_find_element_xi_cache;

// dynamic_cast may make cache value type crashes more predictable.
// Enable for spurious errors, but switching off for performance reasons, release and debug.
#if defined (TEST_FIELD_VALUE_CACHE_CAST)
#  if (defined (OPTIMISED)) || (defined (NDEBUG)) // macros not consistent across platforms
#    define FIELD_VALUE_CACHE_CAST static_cast
#  else
#    define FIELD_VALUE_CACHE_CAST dynamic_cast
#  endif
#else // defined (TEST_FIELD_VALUE_CACHE_CAST)
#  define FIELD_VALUE_CACHE_CAST static_cast
#endif // defined (TEST_FIELD_VALUE_CACHE_CAST)

class FieldValueCache
{
private:
	cmzn_field_cache *extraCache; // optional extra cache for working evaluations at different locations

	void operator=(const FieldValueCache&); // private to prohibit

public:
	int evaluationCounter; // set to cmzn_field_cache::locationCounter when field evaluated
	int derivatives_valid; // only relevant to real caches, but having here saves a virtual function call

	FieldValueCache() :
		extraCache(0),
		evaluationCounter(-1),
		derivatives_valid(0)
	{
	}

	virtual ~FieldValueCache();

	void resetEvaluationCounter()
	{
		evaluationCounter = -1;
	}

	/** override to clear type-specific buffer information & call this */
	virtual void clear();

	void createExtraCache(cmzn_field_cache& parentCache, cmzn_region *region);

	cmzn_field_cache *getExtraCache()
	{
		return extraCache;
	}

	cmzn_field_cache *getOrCreateExtraCache(cmzn_field_cache& parentCache);

	/** all derived classed must implement function to return values as string
	 * @return  allocated string.
	 */
	virtual char *getAsString() = 0;

	bool hasDerivatives()
	{
		return derivatives_valid == 1;
	}

};

typedef std::vector<FieldValueCache*> ValueCacheVector;

struct cmzn_field_cache
{
private:
	cmzn_region_id region;
	int locationCounter; // incremented whenever domain location changes
	Field_location *location;
	int requestedDerivatives;
	ValueCacheVector valueCaches;
	bool assignInCache;
	int access_count;

	/** call whenever location changes to increment location counter */
	void locationChanged()
	{
		++locationCounter;
		// Must reset location and evaluation counters otherwise fields will not be re-evaluated
		// Logic assumes counter will overflow back to a negative value to trigger reset
		if (locationCounter < 0)
		{
			locationCounter = 0;
			for (unsigned int i = 0; i < valueCaches.size(); ++i)
			{
				if (valueCaches[i])
					valueCaches[i]->resetEvaluationCounter();
			}
		}
	}

public:

	cmzn_field_cache(cmzn_region_id region) :
		region(cmzn_region_access(region)),
		locationCounter(0),
		location(new Field_time_location()),
		requestedDerivatives(0),
		valueCaches(cmzn_region_get_field_cache_size(region), (FieldValueCache*)0),
		assignInCache(false),
		access_count(1)
	{
		cmzn_region_add_field_cache(region, this);
	}

	~cmzn_field_cache();

	cmzn_field_cache_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_field_cache_id &cache)
	{
		if (!cache)
			return 0;
		--(cache->access_count);
		if (cache->access_count <= 0)
			delete cache;
		cache = 0;
		return 1;
	}

	/** caller is allowed to modify location with care */
	Field_location* getLocation()
	{
		return location;
	}

	Field_location *cloneLocation()
	{
		return location->clone();
	}

	// cache takes ownership of location object
	void setLocation(Field_location *newLocation)
	{
		// future optimisation: check if location has changed
		delete location;
		location = newLocation;
		locationChanged();
	}

	int getLocationCounter() const
	{
		return locationCounter;
	}

	cmzn_region_id getRegion()
	{
		return region;
	}

	FE_value getTime()
	{
		return location->get_time();
	}

	void setTime(FE_value time)
	{
		// avoid re-setting current time as cache may have valid values already
		if (time != location->get_time())
		{
			location->set_time(time);
			locationChanged();
		}
	}

	int getRequestedDerivatives()
	{
		return requestedDerivatives;
	}

	void setRequestedDerivatives(int requestedDerivativesIn)
	{
		if ((requestedDerivativesIn >= 0) && (requestedDerivativesIn <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
		{
			requestedDerivatives = requestedDerivativesIn;
		}
	}

	/** @param topLevelElement  Optional top-level element to inherit fields from */
	int setMeshLocation(cmzn_element_id element, const double *chart_coordinates,
		cmzn_element_id top_level_element = 0)
	{
		FE_value time = location->get_time();
		delete location;
		location = new Field_element_xi_location(element, chart_coordinates, time, top_level_element);
		locationChanged();
		return 1;
	}

	int setNode(cmzn_node_id node)
	{
		FE_value time = location->get_time();
		delete location;
		location = new Field_node_location(node, time);
		locationChanged();
		return 1;
	}

	int setFieldReal(cmzn_field_id field, int numberOfValues, const double *values);

	int setFieldRealWithDerivatives(cmzn_field_id field, int numberOfValues, const double *values,
		int numberOfDerivatives, const double *derivatives);

	FieldValueCache* getValueCache(int cacheIndex)
	{
		return valueCaches[cacheIndex];
	}

	/** call if new field added to initialise value cache, and when cache created for field */
	// NOT THREAD SAFE
	void setValueCache(int cacheIndex, FieldValueCache* valueCache)
	{
		if (cacheIndex < static_cast<int>(valueCaches.size()))
		{
			delete valueCaches[cacheIndex];
		}
		else
		{
			for (int i = static_cast<int>(valueCaches.size()); i <= cacheIndex; ++i)
				valueCaches.push_back(0);
		}
		valueCaches[cacheIndex] = valueCache;
	}

	bool assignInCacheOnly() const
	{
		return assignInCache;
	}

	/** @return  Previous value of flag */
	bool setAssignInCacheOnly(bool newAssignInCache)
	{
		bool oldAssignInCache = assignInCache;
		assignInCache = newAssignInCache;
		locationChanged();
		return oldAssignInCache;
	}
};

/** use this function with getExtraCache() when creating FieldValueCache for fields that must use an extraCache */
inline void FieldValueCache::createExtraCache(cmzn_field_cache& /*parentCache*/, cmzn_region *region)
{
	if (extraCache)
		cmzn_field_cache::deaccess(extraCache);
	extraCache = new cmzn_field_cache(region);
}

/** use this function for fields that may use an extraCache, e.g. derivatives propagating to top-level-element */
inline cmzn_field_cache *FieldValueCache::getOrCreateExtraCache(cmzn_field_cache& parentCache)
{
	if (!extraCache)
		extraCache = new cmzn_field_cache(parentCache.getRegion());
	return extraCache;
}

class RealFieldValueCache : public FieldValueCache
{
public:
	int componentCount;
	FE_value *values, *derivatives;
	Computed_field_find_element_xi_cache *find_element_xi_cache;

	RealFieldValueCache(int componentCount) :
		FieldValueCache(),
		componentCount(componentCount),
		values(new FE_value[componentCount]),
		derivatives(new FE_value[componentCount*MAXIMUM_ELEMENT_XI_DIMENSIONS]),
		find_element_xi_cache(0)
	{
	}

	virtual ~RealFieldValueCache();

	virtual void clear();

	static RealFieldValueCache* cast(FieldValueCache* valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<RealFieldValueCache*>(valueCache);
   }

	static RealFieldValueCache& cast(FieldValueCache& valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<RealFieldValueCache&>(valueCache);
   }

	virtual void copyValues(const RealFieldValueCache& source)
	{
		int i;
		for (i = 0; i < componentCount; ++i)
		{
			values[i] = source.values[i];
		}
		if (source.derivatives_valid)
		{
			// future optimisation: copy only number of derivatives evaluated
			int derivativeCount = componentCount*MAXIMUM_ELEMENT_XI_DIMENSIONS;
			for (i = 0; i < derivativeCount; ++i)
			{
				derivatives[i] = source.derivatives[i];
			}
			derivatives_valid = 1;
		}
		else
		{
			derivatives_valid = 0;
		}
	}

	virtual void copyValuesZeroDerivatives(const RealFieldValueCache& source)
	{
		int i;
		for (i = 0; i < componentCount; ++i)
		{
			values[i] = source.values[i];
		}
		int derivativeCount = componentCount*MAXIMUM_ELEMENT_XI_DIMENSIONS;
		for (i = 0; i < derivativeCount; ++i)
		{
			derivatives[i] = 0.0;
		}
		derivatives_valid = 1;
	}

	virtual char *getAsString();

	void setValues(const FE_value *values_in)
	{
		for (int i = 0; i < componentCount; ++i)
		{
			values[i] = values_in[i];
		}
		derivatives_valid = 0;
	}
};

class StringFieldValueCache : public FieldValueCache
{
public:
	char *stringValue;

	StringFieldValueCache() :
		FieldValueCache(),
		stringValue(0)
	{
	}

	virtual ~StringFieldValueCache()
	{
		DEALLOCATE(stringValue);
	}

	static StringFieldValueCache* cast(FieldValueCache* valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<StringFieldValueCache*>(valueCache);
   }

	static StringFieldValueCache& cast(FieldValueCache& valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<StringFieldValueCache&>(valueCache);
   }

	void setString(const char *string_in);

	virtual char *getAsString();

};

class MeshLocationFieldValueCache : public FieldValueCache
{
public:
	cmzn_element_id element;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	MeshLocationFieldValueCache() :
		FieldValueCache(),
		element(0)
	{
	}

	virtual ~MeshLocationFieldValueCache()
	{
		cmzn_element_destroy(&element);
	}

	virtual void clear()
	{
		cmzn_element_destroy(&element);
	}

	static MeshLocationFieldValueCache* cast(FieldValueCache* valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<MeshLocationFieldValueCache*>(valueCache);
   }

	static MeshLocationFieldValueCache& cast(FieldValueCache& valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<MeshLocationFieldValueCache&>(valueCache);
   }

	void setMeshLocation(cmzn_element_id element_in, const FE_value *xi_in)
	{
		REACCESS(FE_element)(&element, element_in);
		int dimension = cmzn_element_get_dimension(element_in);
		for (int i = 0; i < dimension; ++i)
		{
			xi[i] = xi_in[i];
		}
	}

	virtual char *getAsString();

};

#endif /* !defined (FIELD_CACHE_HPP) */
