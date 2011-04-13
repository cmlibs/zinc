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

struct Cmiss_graphics_filter;

#ifndef CMISS_GRAPHICS_FILTER_ID_DEFINED
   typedef struct Cmiss_graphics_filter *Cmiss_graphics_filter_id;
   #define CMISS_GRAPHICS_FILTER_ID_DEFINED
#endif /* CMISS_GRAPHICS_FILTER_ID_DEFINED */

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
	CMISS_GRAPHICS_FILTER_ATTRIBUTE_SHOW_MATCHING= 2
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
 * Query if the filter is active.
 * 
 * @param filter  The filter to query.
 * @return  1 if the filter is active, 0 if not.
 */
int Cmiss_graphics_filter_is_active(Cmiss_graphics_filter_id filter);

/*******************************************************************************
 * Sets whether the filter is active.
 * 
 * @param filter  The filter to modify.
 * @param active_flag  1 to make the filter active, 0 to make inactive.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_filter_set_active(Cmiss_graphics_filter_id filter,
	int active_flag);

/*******************************************************************************
 * Query if the filter is inverse.
 *
 * @param filter  The to to modify.
 * @return  1 if the filter is inverse, 0 if not.
 */
int Cmiss_graphics_filter_is_inverse_match(Cmiss_graphics_filter_id filter);

/*******************************************************************************
 * Sets whether the filter should invert the filter's match criterion or not.
 *
 * @param filter  The filter to modify.
 * @param inverse_flag  1 to make the filter inverse, 0 to make normal.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_filter_set_inverse_match(Cmiss_graphics_filter_id filter,
	int inverse_match_flag);

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

/*******************************************************************************
 * Add a filter into a collective Cmiss_graphics_filter.
 *
 * @param filter  The filter to modify.
 * @param filter_to_add  The filter to modify to be added into filter
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_filter_add(Cmiss_graphics_filter_id graphics_filter,
	Cmiss_graphics_filter_id filter_to_add);

/*******************************************************************************
 * Remove a filter from a collective Cmiss_graphics_filter.
 *
 * @param filter  The filter to modify.
 * @param filter_to_remove  The filter to modify to be added into filter
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_filter_remove(Cmiss_graphics_filter_id graphics_filter,
	Cmiss_graphics_filter_id filter_to_remove);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic with visibility flag
 * set AND its owning rendition visibility flag set.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param scene  Scene to add filter to.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_visibility_flags(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches all graphics.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param scene  Scene to add filter to.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_all(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic with the supplied
 * name. The new filter is placed last in the scene's filter priority list.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param match_name  The name a graphic must have to be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_graphic_name(
	Cmiss_graphics_module_id graphics_module, const char *match_name);

/***************************************************************************//**
 * Creates a Cmiss_graphics_filter which matches any graphic in region.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @param match_region  The region must have to be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_region(
	Cmiss_graphics_module_id graphics_module,Cmiss_region_id match_region);

/***************************************************************************//**
 * Creates a collective of Cmiss_graphics_filters which matches all supplied
 * filters.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_and(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Creates a collective of Cmiss_graphics_filters which matches any of the supplied
 * filters.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_graphics_filter_destroy to clean up the returned handle.
 *
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_or(
	Cmiss_graphics_module_id graphics_module);

/***************************************************************************//**
 * Set an integer or Boolean attribute of the graphics_filter object.
 *
 * @param graphics_filter  Handle to the cmiss graphics_filter.
 * @param attribute_id  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this graphics_filter object.
 */
int Cmiss_graphics_filter_set_attribute_integer(Cmiss_graphics_filter_id graphics_filter,
	enum Cmiss_graphics_filter_attribute_id attribute_id, int value);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the scene filter object.
 *
 * @param graphics_filter  Handle to the cmiss scene filter.
 * @param attribute_id  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
int Cmiss_graphics_filter_get_attribute_integer(Cmiss_graphics_filter_id graphics_filter,
	enum Cmiss_graphics_filter_attribute_id attribute_id);

#endif /*__CMISS_GRAPHICS_FILTER_H__*/
