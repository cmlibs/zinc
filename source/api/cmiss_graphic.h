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

#include "api/types/cmiss_field_id.h"
#include "api/types/cmiss_graphic_id.h"
#include "api/types/cmiss_graphics_material_id.h"
#include "api/types/cmiss_tessellation_id.h"

#ifndef RENDER_TYPE_DEFINED
/***************************************************************************//**
 * An enum type to define the render type of a cmiss_graphic.
 */
enum Render_type
{
	RENDER_TYPE_INVALD = 0,
	RENDER_TYPE_SHADED = 1, /*!< Draw colour filled vertexes. */
	RENDER_TYPE_WIREFRAME = 2/*!< Draw boundary of each vertex. */
};
#define RENDER_TYPE_DEFINED
#endif /* RENDER_TYPE_DEFINED */

#ifndef CMISS_GRAPHIC_COORDINATE_SYSTEM_DEFINED
#define CMISS_GRAPHIC_COORDINATE_SYSTEM_DEFINED
/***************************************************************************//**
 * An enum type to define the coordinate system fore rendering a cmiss_graphic.
 */
enum Cmiss_graphic_coordinate_system
{
	CMISS_GRAPHIC_COORDINATE_SYSTEM_INVALID = 0,
	CMISS_GRAPHIC_COORDINATE_SYSTEM_LOCAL = 1, /*!< Render the graphic with the local
	coordinate system, relative to coordinate system specified by renditions */
	CMISS_GRAPHIC_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL = 2,
	/*!< Render the graphic with normalised window coordinate, this mode may distort
	 * the graphic. Normalised window coordinates system range from -1 to 1 from
	 * the left to the right and from the bottom to the top.*/
	CMISS_GRAPHIC_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE = 3,
	/*!< Render the graphic with normalised window coordinate, this mode will render
	 * the graphic with the largest square found in the middle of the display window
	 * thus preventing distortion of the graphic*/
	CMISS_GRAPHIC_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT = 4,
	/*!< Render the graphic with normalised window coordinate, this mode will render
	 * the graphic with the largest square found on the left of the display window
	 * thus preventing distortion of the graphic*/
	CMISS_GRAPHIC_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT = 5,
	/*!< Render the graphic with normalised window coordinate, this mode will render
	 * the graphic with the largest square found on the right of the display window
	 * thus preventing distortion of the graphic*/
	CMISS_GRAPHIC_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM = 6,
	/*!< Render the graphic with normalised window coordinate, this mode will render
	 * the graphic with the largest square found at the bottom of the display window
	 * thus preventing distortion of the graphic*/
	CMISS_GRAPHIC_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP = 7
	/*!< Render the graphic with normalised window coordinate, this mode will render
	 * the graphic with the largest square found on top of the display window
	 * thus preventing distortion of the graphic*/
}; /* enum Cmiss_graphics_type */
#endif /* CMISS_GRAPHIC_COORDINATE_SYSTEM_DEFINED */


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
 * Set the type for how the graphics will be rendered in GL.
 *
 * @param graphic  The handle to the graphic to be edit
 * @param render_type  type of rendering for this graphic, please see the 
 *   render_type definition for more information.
 * @return  If successfully set render_type for graphic returns 1, 
 *   otherwise 0
 */
int Cmiss_graphic_set_render_type(
	Cmiss_graphic_id graphic, enum Render_type render_type);

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
	enum Cmiss_graphic_coordinate_system coordinate_system);

/***************************************************************************//**
 * Get the coordinate system in which to render the coordinates of graphics.
 *
 * @param graphic  The graphic to modify.
 * @return  coordinate system used in graphic.
 */
enum Cmiss_graphic_coordinate_system Cmiss_graphic_get_coordinate_system(
	Cmiss_graphic_id graphic);
	
#endif /*__CMISS_GRAPHIC_H__*/
