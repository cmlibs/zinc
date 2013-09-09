/***************************************************************************//**
 * tessellation.h
 *
 * Public interface to tessellation objects. These describe how elements or
 * other continuous field domains are subdivided into graphics.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_TESSELLATION_H__
#define CMZN_TESSELLATION_H__

#include "types/tessellationid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
* Returns a new reference to the tessellation module with reference count
* incremented. Caller is responsible for destroying the new reference.
*
* @param tessellation_module  The tessellation module to obtain a new reference to.
* @return  Tessellation module with incremented reference count.
*/
ZINC_API cmzn_tessellation_module_id cmzn_tessellation_module_access(
	cmzn_tessellation_module_id tessellation_module);

/**
* Destroys this reference to the tessellation module (and sets it to NULL).
* Internally this just decrements the reference count.
*
* @param tessellation_module_address  Address of handle to tessellation module
*   to destroy.
* @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_tessellation_module_destroy(
	cmzn_tessellation_module_id *tessellation_module_address);

/**
 * Create and return a handle to a new tessellation.
 *
 * @param tessellation_module  The handle to the tessellation module the
 * tessellation will belong to.
 * @return  Handle to the newly created tessellation if successful, otherwise NULL.
 */
ZINC_API cmzn_tessellation_id cmzn_tessellation_module_create_tessellation(
	cmzn_tessellation_module_id tessellation_module);

/**
* Begin caching or increment cache level for this tessellation module. Call this
* function before making multiple changes to minimise number of change messages
* sent to clients. Must remember to end_change after completing changes.
* @see cmzn_tessellation_module_end_change
*
* @param tessellation_module  The tessellation_module to begin change cache on.
* @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_tessellation_module_begin_change(cmzn_tessellation_module_id tessellation_module);

/***************************************************************************//**
* Decrement cache level or end caching of changes for the tessellation module.
* Call cmzn_tessellation_module_begin_change before making multiple changes
* and call this afterwards. When change level is restored to zero,
* cached change messages are sent out to clients.
*
* @param tessellation_module  The glyph_module to end change cache on.
* @return  Status CMZN_OK on success, any other value on failure.
*/
ZINC_API int cmzn_tessellation_module_end_change(cmzn_tessellation_module_id tessellation_module);

/**
* Find the tessellation with the specified name, if any.
*
* @param tessellation_module  Tessellation module to search.
* @param name  The name of the tessellation.
* @return  Handle to the tessellation of that name, or 0 if not found.
* 	Up to caller to destroy returned handle.
*/
ZINC_API cmzn_tessellation_id cmzn_tessellation_module_find_tessellation_by_name(
	cmzn_tessellation_module_id tessellation_module, const char *name);

/**
 * Get the default tessellation to be used by new lines, surfaces and
 * isosurfaces graphics. If there is none, one is automatically created with
 * minimum divisions 1, refinement factors 4, and circle divisions 12,
 * and given the name "default".
 *
 * @param tessellation_module  Tessellation module to query.
 * @return  Handle to the default tessellation, or 0 on error.
 * Up to caller to destroy returned handle.
 */
ZINC_API cmzn_tessellation_id cmzn_tessellation_module_get_default_tessellation(
	cmzn_tessellation_module_id tessellation_module);

/**
 * Set the default tessellation to be used by new lines, surfaces and
 * isosurfaces graphics.
 *
 * @param tessellation_module  Tessellation module to modify.
 * @param tessellation  The tessellation to set as default.
 * @return  CMZN_OK on success otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_tessellation_module_set_default_tessellation(
	cmzn_tessellation_module_id tessellation_module,
	cmzn_tessellation_id tessellation);

/**
 * Get the default tessellation to be used by new points and streamlines
 * graphics. If there is none, one is automatically created with
 * minimum divisions 1, refinement factors 1, and circle divisions 12,
 * and given the name "default_points".
 *
 * @param tessellation_module  Tessellation module to query.
 * @return  Handle to the default points tessellation, or 0 on error.
 * Up to caller to destroy returned handle.
 */
ZINC_API cmzn_tessellation_id cmzn_tessellation_module_get_default_points_tessellation(
	cmzn_tessellation_module_id tessellation_module);

/**
 * Set the default tessellation to be used by new points and streamlines
 * graphics.
 *
 * @param tessellation_module  Tessellation module to modify.
 * @param tessellation  The tessellation to set as default for points.
 * @return  CMZN_OK on success otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_tessellation_module_set_default_points_tessellation(
	cmzn_tessellation_module_id tessellation_module,
	cmzn_tessellation_id tessellation);

/**
 * Returns a new reference to the tessellation with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param tessellation  The tessellation to obtain a new reference to.
 * @return  New tessellation reference with incremented reference count.
 */
ZINC_API cmzn_tessellation_id cmzn_tessellation_access(cmzn_tessellation_id tessellation);

/**
 * Destroys this reference to the tessellation (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param tessellation_address  The address to the handle of the tessellation
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_tessellation_destroy(cmzn_tessellation_id *tessellation_address);

/**
 * Gets the number of line segments used to approximate circles in graphics
 * produced with this tessellation. This applies to lines with a circle profile,
 * and to sphere, cylinder and other glyphs with circular features.
 *
 * @param tessellation  The tessellation to query.
 * @return  The number of circle divisions, or 0 on error.
 */
ZINC_API int cmzn_tessellation_get_circle_divisions(
	cmzn_tessellation_id tessellation);

/**
 * Sets the number of line segments used to approximate circles in graphics
 * produced with this tessellation. This applies to lines with a circle profile,
 * and to sphere, cylinder and other glyphs with circular features.
 *
 * @param tessellation  The tessellation to modify.
 * @param circleDivisions  The number of line segments used to approximate a
 * a circle, at least 3, but larger even numbers are recommended.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_tessellation_set_circle_divisions(
	cmzn_tessellation_id tessellation, int circleDivisions);

/**
 * Get managed status of tessellation in its owning tessellation_module.
 * @see cmzn_tessellation_set_managed
 *
 * @param tessellation  The tessellation to query.
 * @return  1 (true) if tessellation is managed, otherwise 0 (false).
 */
ZINC_API bool cmzn_tessellation_is_managed(cmzn_tessellation_id tessellation);

/**
 * Set managed status of tessellation in its owning tessellation module.
 * If set (managed) the tessellation will remain indefinitely in the tessellation
 * module even if no external references are held.
 * If not set (unmanaged) the tessellation will be automatically removed from the
 * module when no longer referenced externally, effectively marking it as
 * pending destruction.
 * All new objects are unmanaged unless stated otherwise.
 *
 * @param tessellation  The tessellation to modify.
 * @param value  The new value for the managed flag: true or false.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_tessellation_set_managed(cmzn_tessellation_id tessellation,
	bool value);

/**
 * Return an allocated string containing tessellation name.
 *
 * @param tessellation  handle to the zinc tessellation.
 * @return  allocated string containing tessellation name, or NULL on failure.
 * Up to caller to free using cmzn_deallocate().
 */
ZINC_API char *cmzn_tessellation_get_name(cmzn_tessellation_id tessellation);

/**
 * Set/change name for <tessellation>.
 *
 * @param tessellation  The handle to zinc tessellation.
 * @param name  name to be set to the tessellation
 * @return  status CMZN_OK if successfully set/change name for tessellation,
 * any other value on failure.
 */
ZINC_API int cmzn_tessellation_set_name(cmzn_tessellation_id tessellation, const char *name);

/**
 * Gets the minimum number of line segments used to approximate curves in each
 * element dimension for coarse tessellation.
 *
 * @see cmzn_tessellation_set_minimum_divisions
 * @param tessellation  The tessellation to query.
 * @param valuesCount  The size of the minimum_divisions array to fill. Values
 * for dimensions beyond the size set use the last divisions value.
 * @param valuesOut  Array to receive numbers of divisions.
 * @return  The actual number of minimum divisions values that have been
 * explicitly set using cmzn_tessellation_set_minimum_divisions. This can be
 * more than the number requested, so a second call may be needed with a
 * larger array. Returns 0 on error.
 */
ZINC_API int cmzn_tessellation_get_minimum_divisions(
	cmzn_tessellation_id tessellation, int valuesCount, int *valuesOut);

/**
 * Sets the minimum number of line segments used to approximate curves in each
 * element dimension of tessellation. Intended to be used where coarse
 * tessellation is acceptable, e.g. where fields vary linearly over elements.
 * The default minimum_divisions value for new tessellations is 1, size 1.
 * Note: The value set for the last dimension applies to all higher dimensions.
 *
 * @param tessellation  The tessellation to modify.
 * @param valuesCount  The size of the valuesIn array, >= 1.
 * @param valuesIn  Array of number of divisions (>=1) for each dimension, with
 * the last number in array applying to all higher dimensions.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_tessellation_set_minimum_divisions(
	cmzn_tessellation_id tessellation, int valuesCount, const int *valuesIn);

/**
 * Gets the refinements to be used in product with the minimum divisions
 * to approximate curves in each element dimension for fine tessellation.
 *
 * @see cmzn_tessellation_set_refinement_factors
 * @param tessellation  The tessellation to query.
 * @param valuesCount  The size of the refinement_factors array to fill. Values
 * for dimensions beyond the size set use the last refinement value.
 * @param valuesOut  Array to receive refinement factors.
 * @return  The actual number of refinement factor values that have been
 * explicitly set using cmzn_tessellation_set_refinement_factors. This can be
 * more than the number requested, so a second call may be needed with a
 * larger array. Returns 0 on error.
 */
ZINC_API int cmzn_tessellation_get_refinement_factors(
	cmzn_tessellation_id tessellation, int valuesCount, int *valuesOut);

/**
 * Sets the refinements to be used in product with the minimum divisions
 * to approximate curves in each element dimension for fine tessellation.
 * The refinement factors are applied whenever the basis functions of the
 * graphic's coordinate field (replaced by tessellation field if specified) are
 * non-linear anywhere. If there is no tessellation field or if it matches the
 * coordinate field, a non-linear coordinate system also triggers refinement.
 * The default refinement_factors value for new tessellations is 1, size 1.
 * Note: The value set for the last dimension applies to all higher dimensions.
 *
 * @see cmzn_tessellation_set_minimum_divisions
 * @param tessellation  The tessellation to modify.
 * @param valuesCount  The size of the refinement_factors array, >= 1.
 * @param valuesIn  Array of number of fine subdivisions (>=1) per
 * minimum_division for each dimension, with the last number in array
 * applying to all higher dimensions.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_tessellation_set_refinement_factors(
	cmzn_tessellation_id tessellation, int valuesCount, const int *valuesIn);

#ifdef __cplusplus
}
#endif

#endif
