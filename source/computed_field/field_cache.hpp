/***************************************************************************//**
 * FILE : field_cache.hpp
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

#if !defined (FIELD_CACHE_HPP)
#define FIELD_CACHE_HPP

extern "C" {
#include "api/cmiss_element.h"
#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_region.h"
#include "general/debug.h"
#include "region/cmiss_region.h"
}
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
	Cmiss_field_cache *extraCache; // optional extra cache for working evaluations at different locations

	void operator=(const FieldValueCache&); // private to prohibit

public:
	int evaluationCounter; // set to Cmiss_field_cache::locationCounter when field evaluated
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

	void createExtraCache(Cmiss_field_cache& parentCache, Cmiss_region *region);

	Cmiss_field_cache *getExtraCache()
	{
		return extraCache;
	}

	Cmiss_field_cache *getOrCreateExtraCache(Cmiss_field_cache& parentCache);

	/** all derived classed must implement function to return values as string
	 * @return  allocated string.
	 */
	virtual char *getAsString() = 0;

	bool hasDerivatives()
	{
		return derivatives_valid;
	}

};

typedef std::vector<FieldValueCache*> ValueCacheVector;

struct Cmiss_field_cache
{
private:
	Cmiss_region_id region;
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

	Cmiss_field_cache(Cmiss_region_id region) :
		region(Cmiss_region_access(region)),
		locationCounter(0),
		location(new Field_time_location()),
		requestedDerivatives(0),
		valueCaches(Cmiss_region_get_field_cache_size(region), (FieldValueCache*)0),
		assignInCache(false),
		access_count(1)
	{
		Cmiss_region_add_field_cache(region, this);
	}

	~Cmiss_field_cache();

	Cmiss_field_cache_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_field_cache_id &cache)
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

	Cmiss_region_id getRegion()
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
	int setMeshLocation(Cmiss_element_id element, const double *chart_coordinates,
		Cmiss_element_id top_level_element = 0)
	{
		FE_value time = location->get_time();
		delete location;
		location = new Field_element_xi_location(element, chart_coordinates, time, top_level_element);
		locationChanged();
		return 1;
	}

	int setNode(Cmiss_node_id node)
	{
		FE_value time = location->get_time();
		delete location;
		location = new Field_node_location(node, time);
		locationChanged();
		return 1;
	}

	int setFieldReal(Cmiss_field_id field, int numberOfValues, const double *values);

	int setFieldRealWithDerivatives(Cmiss_field_id field, int numberOfValues, const double *values,
		int numberOfDerivatives, const double *derivatives);

	FieldValueCache* getValueCache(int cacheIndex)
	{
		return valueCaches[cacheIndex];
	}

	/** call if new field added to initialise value cache, and when cache created for field */
	// NOT THREAD SAFE
	void setValueCache(int cacheIndex, FieldValueCache* valueCache)
	{
		if (cacheIndex < (int)valueCaches.size())
		{
			delete valueCaches[cacheIndex];
		}
		else
		{
			for (int i = valueCaches.size(); i <= cacheIndex; ++i)
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
inline void FieldValueCache::createExtraCache(Cmiss_field_cache& /*parentCache*/, Cmiss_region *region)
{
	if (extraCache)
		Cmiss_field_cache::deaccess(extraCache);
	extraCache = new Cmiss_field_cache(region);
}

/** use this function for fields that may use an extraCache, e.g. derivatives propagating to top-level-element */
inline Cmiss_field_cache *FieldValueCache::getOrCreateExtraCache(Cmiss_field_cache& parentCache)
{
	if (!extraCache)
		extraCache = new Cmiss_field_cache(parentCache.getRegion());
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
	Cmiss_element_id element;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	MeshLocationFieldValueCache() :
		FieldValueCache(),
		element(0)
	{
	}

	virtual ~MeshLocationFieldValueCache()
	{
		Cmiss_element_destroy(&element);
	}

	virtual void clear()
	{
		Cmiss_element_destroy(&element);
	}

	static MeshLocationFieldValueCache* cast(FieldValueCache* valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<MeshLocationFieldValueCache*>(valueCache);
   }

	static MeshLocationFieldValueCache& cast(FieldValueCache& valueCache)
   {
		return FIELD_VALUE_CACHE_CAST<MeshLocationFieldValueCache&>(valueCache);
   }

	void setMeshLocation(Cmiss_element_id element_in, const FE_value *xi_in)
	{
		REACCESS(FE_element)(&element, element_in);
		int dimension = Cmiss_element_get_dimension(element_in);
		for (int i = 0; i < dimension; ++i)
		{
			xi[i] = xi_in[i];
		}
	}

	virtual char *getAsString();

};

#endif /* !defined (FIELD_CACHE_HPP) */
