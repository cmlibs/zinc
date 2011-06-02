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


#ifndef CMISS_GRAPHICS_FILTER_ID_DEFINED
	struct Cmiss_graphics_filter;
  typedef struct Cmiss_graphics_filter *Cmiss_graphics_filter_id;
  #define CMISS_GRAPHICS_FILTER_ID_DEFINED
#endif /* CMISS_GRAPHICS_FILTER_ID_DEFINED */

#ifndef CMISS_GRAPHICS_FILTER_OPERATOR_ID_DEFINED
	struct Cmiss_graphics_filter_operator;
	typedef struct Cmiss_graphics_filter_operator *Cmiss_graphics_filter_operator_id;
	#define CMISS_GRAPHICS_FILTER_OPERATOR_ID_DEFINED
#endif /* CMISS_GRAPHICS_FILTER_OPERATOR_ID_DEFINED */

#ifndef CMISS_GRAPHICS_MODULE_ID_DEFINED
struct Cmiss_graphics_module;
typedef struct Cmiss_graphics_module * Cmiss_graphics_module_id;
#define CMISS_GRAPHICS_MODULE_ID_DEFINED
#endif /* CMISS_GRAPHICS_MODULE_ID_DEFINED */

#ifndef CMISS_REGION_ID_DEFINED
struct Cmiss_region;
typedef struct Cmiss_region * Cmiss_region_id;
#define CMISS_REGION_ID_DEFINED
#endif

#ifndef CMISS_GRAPHIC_ID_DEFINED
struct Cmiss_graphic;
typedef struct Cmiss_graphic * Cmiss_graphic_id;
#define CMISS_GRAPHIC_ID_DEFINED
#endif /* CMISS_GRAPHIC_ID_DEFINED */

#ifndef CMISS_C_INLINE_DEFINED
#if defined (_MSC_VER)
	#define CMISS_C_INLINE __inline
#else
	#define CMISS_C_INLINE static inline
#endif
#define CMISS_C_INLINE_DEFINED
#endif

/***************************************************************************//**
 * Labels of graphics_filter attributes which may be set or obtained using generic
 * get/set_attribute functions.
 * Note: not all attributes can be set.
 */
enum Cmiss_graphics_filter_attribute_id
{
	CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_MANAGED = 1,
	/*!< Boolean as integer, when 0 (default) graphics_filter is destroyed when no
	 * longer in use, i.e. when number of external references to it drops to
	 * zero. Set to 1 to manage graphics_filter object indefinitely, or until this
	 * attribute is reset to zero, effectively marking it as pending destruction.
	 */
	CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE = 2
	/*!< Boolean as integer, when 0 (default) the filter returns positive when
	 * criterion/criteria is/are matched. Set to 1 to invert the result.
	 */
};

/*******************************************************************************
 * Returns a new reference to the filter with reference count incremented.
 * Caller is responsible for destroying the new reference.
 * 
 * @param filter  The filter to obtain a new reference to.
 * @return  New filter reference with incremented reference count.
 */
Cmiss_graphics_filter_id Cmiss_graphics_filter_access(Cmiss_graphics_filter_id filter);

/*******************************************************************************
 * Destroys this reference to the filter (and sets it to NULL).
 * Internally this just decrements the reference count.
 */
int Cmiss_graphics_filter_destroy(Cmiss_graphics_filter_id *filter_address);

/*******************************************************************************
 * Evaluate either a Cmiss_graphic is shown or hidden with a graphics_filter.
 *
 * @param filter  The filter to perform the check.
 * @param graphic  The graphic to query.
 * @return  Boolean value of filter for this graphic:
 *     1 == true == show or 0 == false == hide
 */
int Cmiss_graphics_filter_evaluate_graphic(Cmiss_graphics_filter_id filter,
	Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Return an allocated string containing graphics_filter name.
 *
 * @param graphics_filter  handle to the cmiss scene filter.
 * @return  allocated string containing graphics_filter name, or NULL on failure.
 */
char *Cmiss_graphics_filter_get_name(Cmiss_graphics_filter_id filter);

/*******************************************************************************
 * Set name of the scene filter
 *
 * @param filter  The filter to modify.
 * @param name  name to be set to the scene filter
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_filter_set_name(Cmiss_graphics_filter_id filter,
	const char *name);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic with visibility
 * flag set AND its owning region and all ancestor region renditions' visibility
 * flags set i.e. rendition visibility flags work hierarchically.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param scene  Scene to add filter to.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_visibility_flags(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic with the supplied
 * name.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param match_name  The name a graphic must have to be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_graphic_name(
	Cmiss_graphics_module_id graphics_module, const char *match_name);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic in region or any
 * of its sub-regions.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param match_region  The region to be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_region(
	Cmiss_graphics_module_id graphics_module,Cmiss_region_id match_region);

/***************************************************************************//**
 * Creates a collective of Cmiss_graphics_filters which matches all supplied
 * filters.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_operator_and(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Creates a collective of Cmiss_graphics_filters which matches any of the supplied
 * filters.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_operator_or(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the scene filter object.
 *
 * @param filter  Handle to the graphics filter.
 * @param attribute_id  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
int Cmiss_graphics_filter_get_attribute_integer(Cmiss_graphics_filter_id filter,
	enum Cmiss_graphics_filter_attribute_id attribute_id);

/***************************************************************************//**
 * Set an integer or Boolean attribute of the graphics_filter object.
 *
 * @param filter  Handle to the cmiss graphics_filter.
 * @param attribute_id  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this graphics_filter object.
 */
int Cmiss_graphics_filter_set_attribute_integer(Cmiss_graphics_filter_id filter,
	enum Cmiss_graphics_filter_attribute_id attribute_id, int value);

/***************************************************************************//**
 * If the filter is of operator and or or type, then this function returns the
 * operator representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived filter reference.
 *
 * @param filter  The generic filter to be cast.
 * @return  Operator specific representation if the input filter is of this type,
 * otherwise NULL.
 */
Cmiss_graphics_filter_operator_id Cmiss_graphics_filter_cast_operator(
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
CMISS_C_INLINE Cmiss_graphics_filter_id Cmiss_graphics_filter_operator_base_cast(
	Cmiss_graphics_filter_operator_id operator_filter)
{
	return (Cmiss_graphics_filter_id)(operator_filter);
}

/***************************************************************************//**
 * Destroys this reference to the operator filter (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param operator_filter_address  Address of handle to the operator filter.
 * @return  1 if successfully destroyed the and filter, otherwise 0.
 */
int Cmiss_graphics_filter_operator_destroy(
	Cmiss_graphics_filter_operator_id *operator_filter_address);

/*******************************************************************************
 * Adds operand to the end of the list of operands for the operator filter.
 * If operand is already in the list, it is moved to the end.
 * Fails if operand depends on the operator filter.
 *
 * @param operator_filter  The operator filter to be modified.
 * @param operand  The filter to be added
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_filter_operator_append_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand);

/*******************************************************************************
 * Get the first operand filter in the operator filter's list of operands.
 * @param operator_filter  The operator filter to be iterated over.
 * @return  1 on success, 0 on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_filter_operator_get_first_operand(
	Cmiss_graphics_filter_operator_id operator_filter);

/*******************************************************************************
 * Get the next filter after <ref_operand> in the operator filter's list of
 * operands.
 *
 * @param operator_filter  The operator filter to be iterated over.
 * @param ref_operand  The filter to be referenced
 * @return  1 on success, 0 on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_filter_operator_get_next_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id ref_operand);

/*******************************************************************************
 * Check whether operand is active in the operator filter.
 *
 * @param operator_filter  The operator filter providing a list of filters.
 * @param operand  The filter to be checked.
 * @return  1 if operand is active, 0 if operand is not active or not an operand.
 */
int Cmiss_graphics_filter_operator_get_operand_is_active(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand);

/*******************************************************************************
 * Set an operand in the operator filter to be active or inactive.
 *
 * @param operator_filter  The operator filter providing a list of filters.
 * @param operand  The filter to be set.
 * @param is_active  Value to set: non-zero to mark as active, 0 for inactive.
 * @return  1 if successfully set, 0 otherwise.
 */
int Cmiss_graphics_filter_operator_set_operand_is_active(
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
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_filter_operator_insert_operand_before(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand, Cmiss_graphics_filter_id ref_operand);

/*******************************************************************************
 * Remove a filter from the list of operands in an operator filter.
 *
 * @param operator_filter  The operator filter to be modified.
 * @param operand  The filter to be removed.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_filter_operator_remove_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand);

#endif /*__CMISS_GRAPHICS_FILTER_H__*/
