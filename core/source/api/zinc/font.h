/*******************************************************************************
 * cmiss_font.h
 *
 * Public interface to cmzn_graphics_filter objects for filtering graphics
 * displayed in a cmzn_scene.
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
 * Portions created by the Initial Developer are Copyright (C) 2012
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

#ifndef CMZN_FONT_H__
#define CMZN_FONT_H__

#include "types/fontid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Returns a new reference to the font module with reference count
* incremented. Caller is responsible for destroying the new reference.
*
* @param font_module  The font module to obtain a new reference to.
* @return  font module with incremented reference count.
*/
ZINC_API cmzn_font_module_id cmzn_font_module_access(
	cmzn_font_module_id font_module);

/**
* Destroys this reference to the font module (and sets it to NULL).
* Internally this just decrements the reference count.
*
* @param font_module_address  Address of handle to font module
*   to destroy.
* @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_font_module_destroy(
	cmzn_font_module_id *font_module_address);

/**
 * Create and return a handle to a new font.
 *
 * @param font_module  The handle to the font module the
 * font will belong to.
 * @return  Handle to the newly created font if successful, otherwise NULL.
 */
ZINC_API cmzn_font_id cmzn_font_module_create_font(
	cmzn_font_module_id font_module);

/**
* Begin caching or increment cache level for this font module. Call this
* function before making multiple changes to minimise number of change messages
* sent to clients. Must remember to end_change after completing changes.
* @see cmzn_font_module_end_change
*
* @param font_module  The font_module to begin change cache on.
* @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_font_module_begin_change(cmzn_font_module_id font_module);

/***************************************************************************//**
* Decrement cache level or end caching of changes for the font module.
* Call cmzn_font_module_begin_change before making multiple changes
* and call this afterwards. When change level is restored to zero,
* cached change messages are sent out to clients.
*
* @param font_module  The glyph_module to end change cache on.
* @return  Status CMZN_OK on success, any other value on failure.
*/
ZINC_API int cmzn_font_module_end_change(cmzn_font_module_id font_module);

/**
* Find the font with the specified name, if any.
*
* @param font_module  font module to search.
* @param name  The name of the font.
* @return  Handle to the font of that name, or 0 if not found.
* 	Up to caller to destroy returned handle.
*/
ZINC_API cmzn_font_id cmzn_font_module_find_font_by_name(
	cmzn_font_module_id font_module, const char *name);

/**
* Get the default font, if any.
*
* @param font_module  font module to query.
* @return  Handle to the default font, or 0 if none.
* 	Up to caller to destroy returned handle.
*/
ZINC_API cmzn_font_id cmzn_font_module_get_default_font(
	cmzn_font_module_id font_module);

/**
* Set the default font.
*
* @param font_module  font module to modify
* @param font  The font to set as default.
* @return  CMZN_OK on success otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_font_module_set_default_font(
	cmzn_font_module_id font_module,
	cmzn_font_id font);

/**
 * Access the font, increase the access count of the font by one.
 *
 * @param font  handle to the "to be access" cmiss font.
 * @return  handle to font if successfully access font.
 */
ZINC_API cmzn_font_id cmzn_font_access(cmzn_font_id font);

/**
 * Destroy the graphics font.
 *
 * @param font_address  address to the handle to the "to be destroyed"
 *   cmiss graphics font.
 * @return  status CMZN_OK if successfully destroy graphics font, any other value
 * on failure.
 */
ZINC_API int cmzn_font_destroy(cmzn_font_id *font_address);

/**
 * Return an allocated string containing font name.
 *
 * @param spectrum  handle to the cmiss graphics font.
 * @return  allocated string containing font name, otherwise NULL. Up to
 * caller to free using cmzn_deallocate().
 */
ZINC_API char *cmzn_font_get_name(cmzn_font_id font);

/**
 * Set/change name for <font>.
 *
 * @param font  The handle to cmiss graphics font.
 * @param name  name to be set to the font
 * @return  status CMZN_OK if successfully set/change name for font,
 * any other value on failure.
 */
ZINC_API int cmzn_font_set_name(cmzn_font_id font,
	const char *name);

/**
 * Get the true type font of the given font.
 *
 * @param font  The handle to cmiss graphics font.
 * @return The true type of font, otherwise returns INVALID_TYPE;
 */
ZINC_API cmzn_font_type cmzn_font_get_font_type(
	cmzn_font_id font);

/**
 * Set the true type font of the given font.
 *
 * @param font  The handle to cmiss graphics font.
 * @param font_type  the true type font to use for font.
 * @return  status CMZN_OK if successfully set the true type for font,
 * any other value on failure.
 */
ZINC_API int cmzn_font_set_font_type(cmzn_font_id font,
	cmzn_font_type font_type);

/**
 * Get the rendering type of the given font.
 *
 * @param font  The handle to cmiss graphics font.
 * @return The render type of font, otherwise returns INVALID_TYPE;
 */
ZINC_API cmzn_font_render_type cmzn_font_get_render_type(
	cmzn_font_id font);

/**
 * Set the rendering type of the given font.
 *
 * @param font  The handle to cmiss graphics font.
 * @param render_type  the render type to use for font.
 * @return  status CMZN_OK if successfully set the font type,
 * any other value on failure.
 */
ZINC_API int cmzn_font_set_render_type(cmzn_font_id font,
	cmzn_font_render_type render_type);

/**
 * Get whether bold text is enabled.
 *
 * @param font  The handle to cmiss graphics font.
 * @return  1 if bold text is enabled otherwise 0.
 */
ZINC_API int cmzn_font_get_bold(cmzn_font_id font);

/**
 * Set whether font should be bold or not.
 *
 * @param font  The handle to cmiss graphics font.
 * @param bold  1 to enable bold text or 0 to disable it.
 * @return  status CMZN_OK if successfully enable/disable bold text,
 * any other value on failure.
 */
ZINC_API int cmzn_font_set_bold(cmzn_font_id font, int bold);

/**
 * Get the depth for extrude font type.
 *
 * @param font  The handle to cmiss graphics font.
 * @return  depth of the font.
 */
ZINC_API double cmzn_font_get_depth(cmzn_font_id font);

/**
 * Set the depth for extrude font type.
 *
 * @param font  The handle to cmiss graphics font.
 * @param depth  depth of the font to be set.
 * @return  status CMZN_OK if successfully set depth for font,
 * any other value on failure.
 */
ZINC_API int cmzn_font_set_depth(cmzn_font_id font, double depth);

/**
 * Get whether italic text is enabled.
 *
 * @param font  The handle to cmiss graphics font.
 * @return  1 if italic text is enabled otherwise 0.
 */
ZINC_API int cmzn_font_get_italic(cmzn_font_id font);

/**
 * Set whether font should be italic or not.
 *
 * @param font  The handle to cmiss graphics font.
 * @param italic  1 to enable italic text or 0 to disable it.
 * @return  status CMZN_OK if successfully enable/disable italic text,
 * any other value on failure.
 */
ZINC_API int cmzn_font_set_italic(cmzn_font_id font, int italic);

/**
 * Get the size for extrude font type.
 *
 * @param font  The handle to cmiss graphics font.
 * @return  size of the font.
 */
ZINC_API int cmzn_font_get_size(cmzn_font_id font);

/**
 * Set the size for font type.
 *
 * @param font  The handle to cmiss graphics font.
 * @param size  size of the font to be set.
 * @return  status CMZN_OK if successfully set size for font,
 * any other value on failure.
 */
ZINC_API int cmzn_font_set_size(cmzn_font_id font, int size);


#ifdef __cplusplus
}
#endif

#endif
