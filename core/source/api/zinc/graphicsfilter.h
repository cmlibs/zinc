/*******************************************************************************
 * cmiss_graphics_filter.h
 *
 * Public interface to Cmiss_graphics_filter objects for filtering graphics
 * displayed in a Cmiss_scene.
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

#ifndef __CMISS_GRAPHICS_FILTER_H__
#define __CMISS_GRAPHICS_FILTER_H__

#include "types/graphicid.h"
#include "types/graphicsfilterid.h"
#include "types/graphicsmoduleid.h"
#include "types/regionid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Returns a new reference to the graphics_filter module with reference count
* incremented. Caller is responsible for destroying the new reference.
*
* @param graphics_filter_module  The graphics_filter module to obtain a new reference to.
* @return  graphics_filter module with incremented reference count.
*/
ZINC_API Cmiss_graphics_filter_module_id Cmiss_graphics_filter_module_access(
	Cmiss_graphics_filter_module_id graphics_filter_module);

/**
* Destroys this reference to the graphics_filter module (and sets it to NULL).
* Internally this just decrements the reference count.
*
* @param graphics_filter_module_address  Address of handle to graphics_filter module
*   to destroy.
* @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_graphics_filter_module_destroy(
	Cmiss_graphics_filter_module_id *graphics_filter_module_address);

/**
 * Creates a Cmiss_graphics_filter which matches any graphic with visibility
 * flag set AND its owning region and all ancestor region renditions' visibility
 * flags set i.e. rendition visibility flags work hierarchically.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param scene  Scene to add filter to.
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_visibility_flags(
	Cmiss_graphics_filter_module_id graphics_filter_module);

/**
 * Creates a graphics filter which matches any graphic with given domain type.
 *
 * @param graphics_filter_module  The module to create the filter in.
 * @param domain_type  The domain type to be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure. Up to caller to
 * destroy the returned handle.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_domain_type(
	Cmiss_graphics_filter_module_id graphics_filter_module,
	enum Cmiss_field_domain_type domain_type);

/**
 * Creates a Cmiss_graphics_filter which matches any graphic with the supplied
 * name.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param match_name  The name of a graphic must be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_graphic_name(
	Cmiss_graphics_filter_module_id graphics_filter_module, const char *match_name);

/**
 * Creates a Cmiss_graphics_filter which matches any graphic with matching
 * type.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param graphic_type  The type of a graphic must be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_graphic_type(
	Cmiss_graphics_filter_module_id graphics_filter_module, enum Cmiss_graphic_type graphic_type);

/**
 * Creates a Cmiss_graphics_filter which matches any graphic in region or any
 * of its sub-regions.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param match_region  The region to be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_region(
	Cmiss_graphics_filter_module_id graphics_filter_module,Cmiss_region_id match_region);

/**
 * Creates a collective of Cmiss_graphics_filters which matches all supplied
 * filters.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_operator_and(
	Cmiss_graphics_filter_module_id graphics_filter_module);

/**
 * Creates a collective of Cmiss_graphics_filters which matches any of the supplied
 * filters.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_operator_or(
	Cmiss_graphics_filter_module_id graphics_filter_module);

/**
* Begin caching or increment cache level for this graphics_filter module. Call this
* function before making multiple changes to minimise number of change messages
* sent to clients. Must remember to end_change after completing changes.
* @see Cmiss_graphics_filter_module_end_change
*
* @param graphics_filter_module  The graphics_filter_module to begin change cache on.
* @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_graphics_filter_module_begin_change(
	Cmiss_graphics_filter_module_id graphics_filter_module);

/**
* Decrement cache level or end caching of changes for the graphics_filter module.
* Call Cmiss_graphics_filter_module_begin_change before making multiple changes
* and call this afterwards. When change level is restored to zero,
* cached change messages are sent out to clients.
*
* @param graphics_filter_module  The glyph_module to end change cache on.
* @return  Status CMISS_OK on success, any other value on failure.
*/
ZINC_API int Cmiss_graphics_filter_module_end_change(
	Cmiss_graphics_filter_module_id graphics_filter_module);

/**
* Find the graphics_filter with the specified name, if any.
*
* @param graphics_filter_module  graphics_filter module to search.
* @param name  The name of the graphics_filter.
* @return  Handle to the graphics_filter of that name, or 0 if not found.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_find_filter_by_name(
	Cmiss_graphics_filter_module_id graphics_filter_module, const char *name);

/**
* Get the default graphics_filter, if any.
*
* @param graphics_filter_module  graphics_filter module to query.
* @return  Handle to the default graphics_filter, or 0 if none.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_module_get_default_filter(
	Cmiss_graphics_filter_module_id graphics_filter_module);

/**
* Set the default graphics_filter.
*
* @param graphics_filter_module  graphics_filter module to modify
* @param graphics_filter  The graphics_filter to set as default.
* @return  CMISS_OK on success otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_graphics_filter_module_set_default_filter(
	Cmiss_graphics_filter_module_id graphics_filter_module,
	Cmiss_graphics_filter_id graphics_filter);

/***************************************************************************//**
 * Labels of graphics_filter attributes which may be set or obtained using generic
 * get/set_attribute functions.
 * Note: not all attributes can be set.
 */
enum Cmiss_graphics_filter_attribute
{
	CMISS_GRAPHICS_FILTER_ATTRIBUTE_INVALID = 0,
	CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE = 1
	/*!< Boolean as integer, when 0 (default) the filter returns positive when
	 * criterion/criteria is/are matched. Set to 1 to invert the result.
	 */
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_graphics_filter_attribute
	Cmiss_graphics_filter_attribute_enum_from_string(const char  *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_graphics_filter_attribute_enum_to_string(
	enum Cmiss_graphics_filter_attribute attribute);

/*******************************************************************************
 * Returns a new reference to the filter with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param filter  The filter to obtain a new reference to.
 * @return  New filter reference with incremented reference count.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_access(Cmiss_graphics_filter_id filter);

/*******************************************************************************
 * Destroys this reference to the filter (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param filter_address  The address to the handle of the filter
 *    to be destroyed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_filter_destroy(Cmiss_graphics_filter_id *filter_address);

/**
 * Get managed status of graphics filter in its owning graphics filter module.
 * @see Cmiss_graphics_filter_set_managed
 *
 * @param graphics filter  The graphics filter to query.
 * @return  1 (true) if graphics filter is managed, otherwise 0 (false).
 */
ZINC_API int Cmiss_graphics_filter_is_managed(Cmiss_graphics_filter_id filter);

/**
 * Set managed status of graphics filter in its owning graphics filter module.
 * If set (managed) the graphics filter will remain indefinitely in the graphics filter module even
 * if no external references are held.
 * If not set (unmanaged) the graphics filter will be automatically removed from the
 * module when no longer referenced externally, effectively marking it as
 * pending destruction.
 * All new objects are unmanaged unless stated otherwise.
 *
 * @param graphics filter  The graphics filter to modify.
 * @param value  The new value for the managed flag: 0 or 1.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphics_filter_set_managed(Cmiss_graphics_filter_id filter,
	int value);

/*******************************************************************************
 * Evaluate either a Cmiss_graphic is shown or hidden with a graphics_filter.
 *
 * @param filter  The filter to perform the check.
 * @param graphic  The graphic to query.
 * @return  Boolean value of filter for this graphic:
 *     1 == true == show or 0 == false == hide
 */
ZINC_API int Cmiss_graphics_filter_evaluate_graphic(Cmiss_graphics_filter_id filter,
	Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Return an allocated string containing graphics_filter name.
 *
 * @param graphics_filter  handle to the cmiss scene filter.
 * @return  allocated string containing graphics_filter name, or NULL on failure.
 * Up to caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_graphics_filter_get_name(Cmiss_graphics_filter_id filter);

/*******************************************************************************
 * Set name of the scene filter
 *
 * @param filter  The filter to modify.
 * @param name  name to be set to the scene filter
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_filter_set_name(Cmiss_graphics_filter_id filter,
	const char *name);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic with visibility
 * flag set AND its owning region and all ancestor region renditions' visibility
 * flags set i.e. rendition visibility flags work hierarchically.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @deprecated see Cmiss_graphics_filter_module_create_filter_visibility_flags
 *
 * @param scene  Scene to add filter to.
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_visibility_flags(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic with the supplied
 * name.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @deprecated see Cmiss_graphics_filter_module_create_filter_graphic_name
 *
 * @param match_name  The name of a graphic must be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_graphic_name(
	Cmiss_graphics_module_id graphics_module, const char *match_name);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic with matching
 * type.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @deprecated see Cmiss_graphics_filter_module_create_filter_graphic_type
 *
 * @param graphic_type  The type of a graphic must be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_graphic_type(
	Cmiss_graphics_module_id graphics_module, enum Cmiss_graphic_type graphic_type);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic in region or any
 * of its sub-regions.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @deprecated see Cmiss_graphics_filter_module_create_filter_region
 *
 * @param match_region  The region to be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_region(
	Cmiss_graphics_module_id graphics_module,Cmiss_region_id match_region);

/***************************************************************************//**
 * Creates a collective of Cmiss_graphics_filters which matches all supplied
 * filters.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @deprecated see Cmiss_graphics_filter_module_create_filter_operator_and
 *
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_operator_and(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Creates a collective of Cmiss_graphics_filters which matches any of the supplied
 * filters.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @deprecated see Cmiss_graphics_filter_module_create_filter_operator_or
 *
 * @return  Handle to the new filter, or NULL on failure.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_operator_or(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the scene filter object.
 *
 * @param filter  Handle to the graphics filter.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
ZINC_API int Cmiss_graphics_filter_get_attribute_integer(Cmiss_graphics_filter_id filter,
	enum Cmiss_graphics_filter_attribute attribute);

/***************************************************************************//**
 * Set an integer or Boolean attribute of the graphics_filter object.
 *
 * @param filter  Handle to the cmiss graphics_filter.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or able to be set for this graphics_filter object.
 */
ZINC_API int Cmiss_graphics_filter_set_attribute_integer(Cmiss_graphics_filter_id filter,
	enum Cmiss_graphics_filter_attribute attribute, int value);

/***************************************************************************//**
 * If the filter is of operator and or or type, then this function returns the
 * operator representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived filter reference.
 *
 * @param filter  The generic filter to be cast.
 * @return  Operator specific representation if the input filter is of this type,
 * otherwise NULL.
 */
ZINC_API Cmiss_graphics_filter_operator_id Cmiss_graphics_filter_cast_operator(
	Cmiss_graphics_filter_id filter);

/***************************************************************************//**
 * Cast operator filter back to its base filter and return the filter.
 * IMPORTANT NOTE: Returned filter does not have incremented reference count and
 * must not be destroyed. Use Cmiss_graphics_filter_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the argument.
 * Use this function to call base-class API, e.g.:
 * Cmiss_graphics_filter_set_name(Cmiss_graphics_filter_operator_base_cast(
 * graphics_filter), "bob");
 *
 * @param operator_filter  Handle to the filter operator to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE Cmiss_graphics_filter_id Cmiss_graphics_filter_operator_base_cast(
	Cmiss_graphics_filter_operator_id operator_filter)
{
	return (Cmiss_graphics_filter_id)(operator_filter);
}

/***************************************************************************//**
 * Destroys this reference to the operator filter (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param operator_filter_address  Address of handle to the operator filter.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_filter_operator_destroy(
	Cmiss_graphics_filter_operator_id *operator_filter_address);

/*******************************************************************************
 * Adds operand to the end of the list of operands for the operator filter.
 * If operand is already in the list, it is moved to the end.
 * Fails if operand depends on the operator filter.
 *
 * @param operator_filter  The operator filter to be modified.
 * @param operand  The filter to be added
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_filter_operator_append_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand);

/*******************************************************************************
 * Get the first operand filter in the operator filter's list of operands.
 * @param operator_filter  The operator filter to be iterated over.
 * @return  Handle to the first operand, or NULL if none or invalid argument.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_operator_get_first_operand(
	Cmiss_graphics_filter_operator_id operator_filter);

/*******************************************************************************
 * Get the next filter after <ref_operand> in the operator filter's list of
 * operands.
 *
 * @param operator_filter  The operator filter to be iterated over.
 * @param ref_operand  The filter to be referenced
 * @return  Handle to the next operand, or NULL if none or invalid argument.
 */
ZINC_API Cmiss_graphics_filter_id Cmiss_graphics_filter_operator_get_next_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id ref_operand);

/*******************************************************************************
 * Check whether operand is active in the operator filter.
 *
 * @param operator_filter  The operator filter providing a list of filters.
 * @param operand  The filter to be checked.
 * @return  1 if operand is active, 0 if operand is not active or not an operand.
 */
ZINC_API int Cmiss_graphics_filter_operator_get_operand_is_active(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand);

/*******************************************************************************
 * Set an operand in the operator filter to be active or inactive.
 *
 * @param operator_filter  The operator filter providing a list of filters.
 * @param operand  The filter to be set.
 * @param is_active  Value to set: non-zero to mark as active, 0 for inactive.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_filter_operator_set_operand_is_active(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand, int is_active);

/*******************************************************************************
 * Insert a filter before ref_operand in the list of operands for the operator
 * filter. If the operand is already in the list of operands it is moved to the
 * new location. Fails if operand depends on the operator filter.
 *
 * @param operator_filter  The operator filter to be modified.
 * @param operand  The operand filter to be inserted.
 * @param ref_operand  The reference filter to insert before.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_filter_operator_insert_operand_before(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand, Cmiss_graphics_filter_id ref_operand);

/*******************************************************************************
 * Remove a filter from the list of operands in an operator filter.
 *
 * @param operator_filter  The operator filter to be modified.
 * @param operand  The filter to be removed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphics_filter_operator_remove_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand);

#ifdef __cplusplus
}
#endif

#endif /*__CMISS_GRAPHICS_FILTER_H__*/
