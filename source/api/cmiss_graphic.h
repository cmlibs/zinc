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

#include "api/cmiss_field.h"
#include "api/cmiss_material.h"

struct Cmiss_graphic;

#ifndef CMISS_GRAPHIC_ID_DEFINED
/***************************************************************************//**
 * A handle to cmiss graphic. Cmiss graphic is individual graphic representation
 * such as lines, surfaces, node points and etc of a rendition. These graphics
 * can be customised through a numebr of set functions.
 */
typedef struct Cmiss_graphic * Cmiss_graphic_id;
#define CMISS_GRAPHIC_ID_DEFINED
#endif /* CMISS_GRAPHIC_ID_DEFINED */

#ifndef RENDER_TYPE_DEFINED
/***************************************************************************//**
 * An enum type to define the render type of a cmiss_graphic.
 */
enum Render_type
{
	RENDER_TYPE_SHADED, /*!< Draw colour filled vertexes. */
	RENDER_TYPE_WIREFRAME /*!< Draw boundary of each vertex. */
};
#define RENDER_TYPE_DEFINED
#endif /* RENDER_TYPE_DEFINED */

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
int Cmiss_graphic_set_material(Cmiss_graphic_id graphic, Cmiss_material_id material);

/***************************************************************************//** 
 * Set the selected material of the cmiss graphic
 *
 * @param graphic  The graphic to be edit
 * @param material  The material to be set to graphic as the selected material
 * @return  If successfully set material for graphic returns 1, otherwise 0
 */
int Cmiss_graphic_set_selected_material(
	Cmiss_graphic_id graphic, Cmiss_material_id material);

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
 * Destroys the graphic and sets the pointer to NULL.
 *
 * @param graphic  The pointer to the handle of the graphic
 * @return  If successfully destroy graphic returns 1, 
 *   otherwise 0
 */
int Cmiss_graphic_destroy(Cmiss_graphic_id *graphic);
	
#endif /*__CMISS_GRAPHIC_H__*/
