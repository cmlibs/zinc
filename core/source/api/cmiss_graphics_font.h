/*******************************************************************************
 * cmiss_graphics_font.h
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

#ifndef __CMISS_GRAPHICS_FONT_H__
#define __CMISS_GRAPHICS_FONT_H__

#include "types/cmiss_c_inline.h"
#include "types/cmiss_graphics_font_id.h"

#include "cmiss_shared_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Access the font, increase the access count of the font by one.
 *
 * @param font  handle to the "to be access" cmiss font.
 * @return  handle to font if successfully access font.
 */
ZINC_API Cmiss_graphics_font_id Cmiss_graphics_font_access(Cmiss_graphics_font font);

/***************************************************************************//**
 * Destroy the graphics font.
 *
 * @param font_address  address to the handle to the "to be destroyed"
 *   cmiss graphics font.
 * @return  status CMISS_OK if successfully destroy graphics font, any other value
 * on failure.
 */
ZINC_API int Cmiss_graphics_font_destroy(Cmiss_graphics_font_id *font_address);

/***************************************************************************//**
 * Return an allocated string containing font name.
 *
 * @param spectrum  handle to the cmiss graphics font.
 * @return  allocated string containing font name, otherwise NULL. Up to
 * caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_graphics_font_get_name(Cmiss_graphics_font_id font);

/***************************************************************************//**
 * Set/change name for <graphics_font>.
 *
 * @param font  The handle to cmiss graphics font.
 * @param name  name to be set to the font
 * @return  status CMISS_OK if successfully set/change name for font,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphics_font_set_name(Cmiss_graphics_font_id font,
	const char *name);

/***************************************************************************//**
 * Get the true type font of the given font.
 *
 * @param font  The handle to cmiss graphics font.
 * @return The true type of font, otherwise returns INVALID_TYPE;
 */
ZINC_API Cmiss_graphics_font_true_type Cmiss_graphics_font_get_true_type(
	Cmiss_graphics_font_id font);

/***************************************************************************//**
 * Set the true type font of the given font.
 *
 * @param font  The handle to cmiss graphics font.
 * @param true_type  the true type font to use for font.
 * @return  status CMISS_OK if successfully set the true type for font,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphics_font_set_true_type(Cmiss_graphics_font_id font,
	Cmiss_graphics_font_true_type true_type);

/***************************************************************************//**
 * Get the rendering type of the given font.
 *
 * @param font  The handle to cmiss graphics font.
 * @return The type of font, otherwise returns INVALID_TYPE;
 */
ZINC_API Cmiss_graphics_font_type Cmiss_graphics_font_get_type(
	Cmiss_graphics_font_id font);

/***************************************************************************//**
 * Set the rendering type of the given font.
 *
 * @param font  The handle to cmiss graphics font.
 * @param font_type  the render type to use for font.
 * @return  status CMISS_OK if successfully set the font type,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphics_font_set_type(Cmiss_graphics_font_id font,
	Cmiss_graphics_font_type font_type);

/***************************************************************************//**
 * Get whether bold text is enabled.
 *
 * @param font  The handle to cmiss graphics font.
 * @return  1 if bold text is enabled otherwise 0.
 */
ZINC_API int Cmiss_graphics_font_get_bold(Cmiss_graphics_font_id font);

/***************************************************************************//**
 * Set whether font should be bold or not.
 *
 * @param font  The handle to cmiss graphics font.
 * @param bold  1 to enable bold text or 0 to disable it.
 * @return  status CMISS_OK if successfully enable/disable bold text,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphics_font_set_bold(Cmiss_graphics_font_id font, int bold);

/***************************************************************************//**
 * Get the depth for extrude font type.
 *
 * @param font  The handle to cmiss graphics font.
 * @return  depth of the font.
 */
ZINC_API double Cmiss_graphics_font_get_depth(Cmiss_graphics_font_id font);

/***************************************************************************//**
 * Set the depth for extrude font type.
 *
 * @param font  The handle to cmiss graphics font.
 * @param depth  depth of the font to be set.
 * @return  status CMISS_OK if successfully set depth for font,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphics_font_set_depth(Cmiss_graphics_font_id font, double depth);

/***************************************************************************//**
 * Get whether italic text is enabled.
 *
 * @param font  The handle to cmiss graphics font.
 * @return  1 if italic text is enabled otherwise 0.
 */
ZINC_API int Cmiss_graphics_font_get_italic(Cmiss_graphics_font_id font);

/***************************************************************************//**
 * Set whether font should be italic or not.
 *
 * @param font  The handle to cmiss graphics font.
 * @param italic  1 to enable italic text or 0 to disable it.
 * @return  status CMISS_OK if successfully enable/disable italic text,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphics_font_set_italic(Cmiss_graphics_font_id font, int italic);

/***************************************************************************//**
 * Get the size for extrude font type.
 *
 * @param font  The handle to cmiss graphics font.
 * @return  size of the font.
 */
ZINC_API int Cmiss_graphics_font_get_size(Cmiss_graphics_font_id font);

/***************************************************************************//**
 * Set the size for font type.
 *
 * @param font  The handle to cmiss graphics font.
 * @param size  size of the font to be set.
 * @return  status CMISS_OK if successfully set size for font,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphics_font_set_size(Cmiss_graphics_font_id font, int size);


#ifdef __cplusplus
}
#endif

#endif /*__CMISS_GRAPHICS_FONT_H__*/
