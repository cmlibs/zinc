/*******************************************************************************
FILE : cmiss_graphic.h

LAST MODIFIED : 04 Nov 2009

DESCRIPTION :
The public interface to the Cmiss_rendition.
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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

#ifndef __CMISS_GRAPHIC_H__
#define __CMISS_GRAPHIC_H__

#include "api/types/zn_field_id.h"
#include "api/types/zn_graphic_id.h"
#include "api/types/zn_graphics_render_type.h"
#include "api/types/zn_graphics_coordinate_system.h"
#include "api/types/zn_graphics_material_id.h"
#include "api/types/zn_tessellation_id.h"

/*******************************************************************************
 * Returns a new reference to the graphic with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param graphic  The graphic to obtain a new reference to.
 * @return  New graphic reference with incremented reference count.
 */
Cmiss_graphic_id Cmiss_graphic_access(Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Destroys the graphic and sets the pointer to NULL.
 *
 * @param graphic  The pointer to the handle of the graphic
 * @return  If successfully destroy graphic returns 1,
 *   otherwise 0
 */
int Cmiss_graphic_destroy(Cmiss_graphic_id *graphic);

/***************************************************************************//**
 * Sets the field supplying coordinates for the graphic.
 *
 * @param graphic  The graphic to be edited.
 * @param coordinate_field  The cmiss_field to be use as the coordinate field.
 * @return  1 if coordinate field successfully set, otherwise 0.
 */
int Cmiss_graphic_set_coordinate_field(Cmiss_graphic_id graphic,
	Cmiss_field_id coordinate_field);

/***************************************************************************//** 
 * Set the material of the cmiss graphic
 *
 * @param graphic  The graphic to be edit
 * @param material  The material to be set to graphic as the default material
 * @return  If successfully set material for graphic returns 1, otherwise 0
 */
int Cmiss_graphic_set_material(Cmiss_graphic_id graphic, Cmiss_graphics_material_id material);

/***************************************************************************//** 
 * Set the selected material of the cmiss graphic
 *
 * @param graphic  The graphic to be edit
 * @param material  The material to be set to graphic as the selected material
 * @return  If successfully set material for graphic returns 1, otherwise 0
 */
int Cmiss_graphic_set_selected_material(
	Cmiss_graphic_id graphic, Cmiss_graphics_material_id material);

/***************************************************************************//** 
 * Set the texture coordinate field of the cmiss graphic.
 *
 * Texture coordinate field is use to set up and describe how a texture should
 * be mapped to a region.
 *
 * @param graphic  The graphic to be edit
 * @param texture_coordiante_field  The cmiss_field to be set as the texture
 *   texture coordinate field.
 * @return  If successfully set texture_coordinate_field for graphic returns 1, 
 *   otherwise 0
 */
int Cmiss_graphic_set_texture_coordinate_field(Cmiss_graphic_id graphic,
	Cmiss_field_id texture_coordiante_field);

/***************************************************************************//**
 * Returns the tessellation object of the graphics or NULL if none.
 * Caller must destroy reference.
 *
 * @param graphic  The graphic to be edit
 *
 * @return  tessellation for graphic or NULL if none.
 */
Cmiss_tessellation_id Cmiss_graphic_get_tessellation(Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Sets the tessellation object of the graphics.
 *
 * @param graphic  The graphic to be edit
 * @param tessellation  The tessellation object to be set for graphic
 *
 * @return  tessellation for graphic or NULL if none.
 */
int Cmiss_graphic_set_tessellation(
		Cmiss_graphic_id graphic, Cmiss_tessellation_id tessellation);

/***************************************************************************//**
 * Get the render type of the graphic.
 *
 * @param graphic  The handle to the graphic to be edit
 * @return  The render type If successfully get render_type for graphic, otherwise
 * 		it returns INVALID_TYPE;
 */
enum Cmiss_graphics_render_type Cmiss_graphic_get_render_type(
	Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Set the type for how the graphics will be rendered in GL.
 *
 * @param graphic  The handle to the graphic to be edit
 * @param render_type  type of rendering for this graphic, please see the 
 *   render_type definition for more information.
 * @return  If successfully set render_type for graphic returns 1, 
 *   otherwise 0
 */
int Cmiss_graphic_set_render_type(
	Cmiss_graphic_id graphic, enum Cmiss_graphics_render_type render_type);

/***************************************************************************//**
 * Return status of graphic visibility flag attribute.
 *
 * @param graphic  The graphic to query.
 * @return  1 if graphic visibility flag is set, 0 if not.
 */
int Cmiss_graphic_get_visibility_flag(Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Sets status of graphic visibility flag attribute.
 *
 * @param graphic  The graphic to modify.
 * @param visibility_flag  1 to set, 0 to clear.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphic_set_visibility_flag(Cmiss_graphic_id graphic,
	int visibility_flag);

/***************************************************************************//**
 * Specifying the coordinate system in which to render the coordinates of graphics.
 *
 * @param graphic  The graphic to modify.
 * @param coordinate_system  enumerator describing coordinate system to be set
 * 		for graphic.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphic_set_coordinate_system(Cmiss_graphic_id graphic,
	enum Cmiss_graphics_coordinate_system coordinate_system);

/***************************************************************************//**
 * Get the coordinate system in which to render the coordinates of graphics.
 *
 * @param graphic  The graphic to modify.
 * @return  coordinate system used in graphic.
 */
enum Cmiss_graphics_coordinate_system Cmiss_graphic_get_coordinate_system(
	Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_graphic_type Cmiss_graphic_type_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_graphic_type_enum_to_string(enum Cmiss_graphic_type type);

#endif /*__CMISS_GRAPHIC_H__*/
