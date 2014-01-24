/**
 * FILE : fieldcache.h
 *
 * The public interface to the zinc field evaluation and assignment cache.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDCACHE_H__
#define CMZN_FIELDCACHE_H__

#include "types/elementid.h"
#include "types/fieldcacheid.h"
#include "types/fieldid.h"
#include "types/nodeid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new reference to the field cache with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param cache  The field cache to obtain a new reference to.
 * @return  New field cache reference with incremented reference count.
 */
ZINC_API cmzn_fieldcache_id cmzn_fieldcache_access(cmzn_fieldcache_id cache);

/*******************************************************************************
 * Destroys this reference to the field cache, and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param cache_address  Address of handle to field cache to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldcache_destroy(cmzn_fieldcache_id *cache_address);

/**
 * Clears domain locations held in the field cache. Call this function before
 * evaluating fields in a different domain (e.g. nodes <-> elements <-> point)
 * to ensure false field values at the last domain location are not returned.
 * Note that all domain locations are cleared by this function including time
 * which is reset to 0.0, so these need to be set again if needed.
 *
 * @param cache  The field cache to clear the location in.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_fieldcache_clear_location(cmzn_fieldcache_id cache);

/**
 * Prescribes an element location without specifying chart coordinates. Suitable
 * only for evaluating fields that are constant across the element.
 * Note: replaces any other spatial location in cache (e.g. node.) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param element  The element to set. Must belong to same region as cache.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldcache_set_element(cmzn_fieldcache_id cache,
	cmzn_element_id element);

/**
 * Prescribes a location in an element for field evaluation or assignment with
 * the cache.
 * Note: replaces any other spatial location in cache (e.g. node.) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param element  The element the location is in. Must belong to same region
 * as cache.
 * @param number_of_chart_coordinates  The size of the chart_coordinates array,
 * checked to be not less than the element dimension.
 * @param chart_coordinates  Location in element chart. Value is not checked;
 * caller is responsible for supplying locations within the bounds of the
 * element shape.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldcache_set_mesh_location(cmzn_fieldcache_id cache,
	cmzn_element_id element, int number_of_chart_coordinates,
	const double *chart_coordinates);

/**
 * Prescribes a value of a field for subsequent evaluation and assignment with
 * the cache.
 * Note: currently treated as a spatial location, replacing any other spatial
 * location in cache (e.g. element, node) but time is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param reference_field  The field whose values are to be prescribed.
 * @param number_of_values  The size of the values array. Can be less than the
 * number of field components, and if so it is padded with zeroes.
 * @param values  The field values to set.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldcache_set_field_real(cmzn_fieldcache_id cache,
	cmzn_field_id reference_field, int number_of_values, const double *values);

/**
 * Prescribes a node location for field evaluation or assignment with the cache.
 * Note: replaces any other spatial location in cache (e.g. element) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param node  The node to set as spatial location. Must belong to same region
 * as cache.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldcache_set_node(cmzn_fieldcache_id cache, cmzn_node_id node);

/**
 * Prescribes the time for field evaluation or assignment with the cache.
 *
 * @param cache  The field cache to set the location in.
 * @param time  The time value to be set.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldcache_set_time(cmzn_fieldcache_id cache, double time);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_FIELDCACHE_H__ */
