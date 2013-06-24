/***************************************************************************//**
 * cmiss_tessellation.h
 *
 * Public interface to tessellation objects. These describe how elements or
 * other continuous field domains are subdivided into graphics.
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
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#if !defined (CMISS_TESSELLATION_H)
#define CMISS_TESSELLATION_H

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
ZINC_API Cmiss_tessellation_module_id Cmiss_tessellation_module_access(
	Cmiss_tessellation_module_id tessellation_module);

/**
* Destroys this reference to the tessellation module (and sets it to NULL).
* Internally this just decrements the reference count.
*
* @param tessellation_module_address  Address of handle to tessellation module
*   to destroy.
* @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_tessellation_module_destroy(
	Cmiss_tessellation_module_id *tessellation_module_address);

/**
 * Create and return a handle to a new tessellation.
 *
 * @param tessellation_module  The handle to the tessellation module the
 * tessellation will belong to.
 * @return  Handle to the newly created tessellation if successful, otherwise NULL.
 */
ZINC_API Cmiss_tessellation_id Cmiss_tessellation_module_create_tessellation(
	Cmiss_tessellation_module_id tessellation_module);

/**
* Begin caching or increment cache level for this tessellation module. Call this
* function before making multiple changes to minimise number of change messages
* sent to clients. Must remember to end_change after completing changes.
* @see Cmiss_tessellation_module_end_change
*
* @param tessellation_module  The tessellation_module to begin change cache on.
* @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_tessellation_module_begin_change(Cmiss_tessellation_module_id tessellation_module);

/***************************************************************************//**
* Decrement cache level or end caching of changes for the tessellation module.
* Call Cmiss_tessellation_module_begin_change before making multiple changes
* and call this afterwards. When change level is restored to zero,
* cached change messages are sent out to clients.
*
* @param tessellation_module  The glyph_module to end change cache on.
* @return  Status CMISS_OK on success, any other value on failure.
*/
ZINC_API int Cmiss_tessellation_module_end_change(Cmiss_tessellation_module_id tessellation_module);

/**
* Find the tessellation with the specified name, if any.
*
* @param tessellation_module  Tessellation module to search.
* @param name  The name of the tessellation.
* @return  Handle to the tessellation of that name, or 0 if not found.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_tessellation_id Cmiss_tessellation_module_find_tessellation_by_name(
	Cmiss_tessellation_module_id tessellation_module, const char *name);

/**
* Get the default tessellation, if any.
*
* @param tessellation_module  Tessellation module to query.
* @return  Handle to the default tessellation, or 0 if none.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_tessellation_id Cmiss_tessellation_module_get_default_tessellation(
	Cmiss_tessellation_module_id tessellation_module);

/**
* Set the default tessellation.
*
* @param tessellation_module  Tessellation module to modify
* @param tessellation  The tessellation to set as default.
* @return  CMISS_OK on success otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_tessellation_module_set_default_tessellation(
	Cmiss_tessellation_module_id tessellation_module,
	Cmiss_tessellation_id tessellation);

/**
 * Labels of tessellation attributes which may be set or obtained using generic
 * get/set_attribute functions.
 * Note: not all attributes can be set.
 */
enum Cmiss_tessellation_attribute
{
	CMISS_TESSELLATION_ATTRIBUTE_INVALID = 0,
	CMISS_TESSELLATION_ATTRIBUTE_MINIMUM_DIVISIONS_SIZE = 1,
	/*!< positive integer size of minimum divisions array, defaults to 1 or size
	 * set in last call to Cmiss_tessellation_set_minimum_divisions.
	 * @see Cmiss_tessellation_set_minimum_divisions.
	 */
	CMISS_TESSELLATION_ATTRIBUTE_REFINEMENT_FACTORS_SIZE = 2
	/*!< positive integer size of refinement factors array, defaults to 1 or size
	 * set in last call to Cmiss_tessellation_set_refinement_factors.
	 * @see Cmiss_tessellation_set_refinement_factors.
	 */
};

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_tessellation_attribute
	Cmiss_tessellation_attribute_enum_from_string(const char  *string);
/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_tessellation_attribute_enum_to_string(
	enum Cmiss_tessellation_attribute attribute);

/**
 * Returns a new reference to the tessellation with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param tessellation  The tessellation to obtain a new reference to.
 * @return  New tessellation reference with incremented reference count.
 */
ZINC_API Cmiss_tessellation_id Cmiss_tessellation_access(Cmiss_tessellation_id tessellation);

/**
 * Destroys this reference to the tessellation (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param tessellation_address  The address to the handle of the tessellation
 *    to be destroyed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_tessellation_destroy(Cmiss_tessellation_id *tessellation_address);

/**
 * Get managed status of tessellation in its owning tessellation_module.
 * @see Cmiss_tessellation_set_managed
 *
 * @param tessellation  The tessellation to query.
 * @return  1 (true) if tessellation is managed, otherwise 0 (false).
 */
ZINC_API bool Cmiss_tessellation_is_managed(Cmiss_tessellation_id tessellation);

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
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_tessellation_set_managed(Cmiss_tessellation_id tessellation,
	bool value);

/**
 * Get an integer or Boolean attribute of the tessellation object.
 *
 * @param tessellation  Handle to the cmiss tessellation.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
ZINC_API int Cmiss_tessellation_get_attribute_integer(Cmiss_tessellation_id tessellation,
	enum Cmiss_tessellation_attribute attribute);

/**
 * Return an allocated string containing tessellation name.
 *
 * @param tessellation  handle to the cmiss tessellation.
 * @return  allocated string containing tessellation name, or NULL on failure.
 * Up to caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_tessellation_get_name(Cmiss_tessellation_id tessellation);

/**
 * Set/change name for <tessellation>.
 *
 * @param tessellation  The handle to cmiss tessellation.
 * @param name  name to be set to the tessellation
 * @return  status CMISS_OK if successfully set/change name for tessellation,
 * any other value on failure.
 */
ZINC_API int Cmiss_tessellation_set_name(Cmiss_tessellation_id tessellation, const char *name);

/**
 * Gets the minimum number of line segments used to approximate curves in each
 * dimension for coarse tessellation.
 *
 * @see Cmiss_tessellation_set_minimum_divisions
 * @param tessellation  The tessellation object to query.
 * @param size  The size of the minimum_divisions array to fill. Values for
 * dimensions beyond the size set use the last divisions value.
 * @param minimum_divisions  Array to receive numbers of divisions.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_tessellation_get_minimum_divisions(Cmiss_tessellation_id tessellation,
	int size, int *minimum_divisions);

/**
 * Sets the minimum number of line segments used to approximate curves in each
 * dimension of tessellation. Intended to be used where coarse tessellation is
 * acceptable, e.g. where fields vary linearly over elements.
 * The default minimum_divisions value for new tessellations is 1, size 1.
 * Note: The value set for the last dimension applies to all higher dimensions.
 *
 * @param tessellation  The tessellation object to modify.
 * @param size  The size of the minimum_divisions array, >= 1.
 * @param minimum_divisions  Array of number of divisions (>=1) for each
 * dimension, with the last number in array applying to all higher dimensions.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_tessellation_set_minimum_divisions(Cmiss_tessellation_id tessellation,
	int size, const int *minimum_divisions);

/**
 * Gets the refinements to be used in product with the minimum divisions
 * to approximate curves in each dimension for fine tessellation.
 *
 * @see Cmiss_tessellation_set_refinement_factors
 * @param tessellation  The tessellation object to query.
 * @param size  The size of the refinement_factors array to fill. Values for
 * dimensions beyond the size set use the last refinement value.
 * @param refinement_factors  Array to receive refinement factors.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_tessellation_get_refinement_factors(Cmiss_tessellation_id tessellation,
	int size, int *refinement_factors);

/**
 * Sets the refinements to be used in product with the minimum divisions
 * to approximate curves in each dimension for fine tessellation. To be used
 * e.g. where fields vary non-linearly over elements, or greater data sampling
 * is needed for rendering with a spectrum.
 * The default refinement_factors value for new tessellations is 1, size 1.
 * Note: The value set for the last dimension applies to all higher dimensions.
 *
 * @see Cmiss_tessellation_set_minimum_divisions
 * @param tessellation  The tessellation object to modify.
 * @param size  The size of the refinement_factors array, >= 1.
 * @param refinement_factors  Array of number of fine subdivisions (>=1) per
 * minimum_division for each dimension, with the last number in array
 * applying to all higher dimensions.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_tessellation_set_refinement_factors(Cmiss_tessellation_id tessellation,
	int size, const int *refinement_factors);

#ifdef __cplusplus
}
#endif

#endif
