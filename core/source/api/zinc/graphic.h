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

#include "types/fieldid.h"
#include "types/graphicid.h"
#include "types/graphicisosurfaceid.h"
#include "types/graphicsrendertype.h"
#include "types/graphicscoordinatesystem.h"
#include "types/graphicsmaterialid.h"
#include "types/tessellationid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Returns a new reference to the graphic with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param graphic  The graphic to obtain a new reference to.
 * @return  New graphic reference with incremented reference count.
 */
ZINC_API Cmiss_graphic_id Cmiss_graphic_access(Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Destroys the graphic and sets the pointer to NULL.
 *
 * @param graphic  The pointer to the handle of the graphic
 * @return  Status CMISS_OK if successfully destroy graphic, otherwise any
 * other value.
 */
ZINC_API int Cmiss_graphic_destroy(Cmiss_graphic_id *graphic);

/***************************************************************************//**
 * Sets the field supplying coordinates for the graphic.
 *
 * @param graphic  The graphic to be edited.
 * @param coordinate_field  The cmiss_field to be use as the coordinate field.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_coordinate_field(Cmiss_graphic_id graphic,
	Cmiss_field_id coordinate_field);

/***************************************************************************//**
 * Set the material of the cmiss graphic
 *
 * @param graphic  The graphic to be edit
 * @param material  The material to be set to graphic as the default material
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_material(Cmiss_graphic_id graphic, Cmiss_graphics_material_id material);

/***************************************************************************//**
 * Set the selected material of the cmiss graphic
 *
 * @param graphic  The graphic to be edit
 * @param material  The material to be set to graphic as the selected material
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_selected_material(
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
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_texture_coordinate_field(Cmiss_graphic_id graphic,
	Cmiss_field_id texture_coordiante_field);

/***************************************************************************//**
 * Returns the tessellation object of the graphics or NULL if none.
 * Caller must destroy reference.
 *
 * @param graphic  The graphic to be edit
 *
 * @return  tessellation for graphic or NULL if none.
 */
ZINC_API Cmiss_tessellation_id Cmiss_graphic_get_tessellation(Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Sets the tessellation object of the graphics.
 *
 * @param graphic  The graphic to be edit
 * @param tessellation  The tessellation object to be set for graphic
 *
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_tessellation(
		Cmiss_graphic_id graphic, Cmiss_tessellation_id tessellation);

/***************************************************************************//**
 * Get the render type of the graphic.
 *
 * @param graphic  The handle to the graphic to be edit
 * @return  The render type If successfully get render_type for graphic, otherwise
 * 		it returns INVALID_TYPE;
 */
ZINC_API enum Cmiss_graphics_render_type Cmiss_graphic_get_render_type(
	Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Set the type for how the graphics will be rendered in GL.
 *
 * @param graphic  The handle to the graphic to be edit
 * @param render_type  type of rendering for this graphic, please see the
 *   render_type definition for more information.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_render_type(
	Cmiss_graphic_id graphic, enum Cmiss_graphics_render_type render_type);

/***************************************************************************//**
 * Return status of graphic visibility flag attribute.
 *
 * @param graphic  The graphic to query.
 * @return  1 if graphic visibility flag is set, 0 if not.
 */
ZINC_API int Cmiss_graphic_get_visibility_flag(Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Sets status of graphic visibility flag attribute.
 *
 * @param graphic  The graphic to modify.
 * @param visibility_flag  1 to set, 0 to clear.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_visibility_flag(Cmiss_graphic_id graphic,
	int visibility_flag);

ZINC_API int Cmiss_graphic_set_glyph_type(Cmiss_graphic_id graphic, enum Cmiss_graphic_glyph_type glyph_type);

/***************************************************************************//**
 * Specifying the coordinate system in which to render the coordinates of graphics.
 *
 * @param graphic  The graphic to modify.
 * @param coordinate_system  enumerator describing coordinate system to be set
 * 		for graphic.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_coordinate_system(Cmiss_graphic_id graphic,
	enum Cmiss_graphics_coordinate_system coordinate_system);

/***************************************************************************//**
 * Get the coordinate system in which to render the coordinates of graphics.
 *
 * @param graphic  The graphic to modify.
 * @return  coordinate system used in graphic.
 */
ZINC_API enum Cmiss_graphics_coordinate_system Cmiss_graphic_get_coordinate_system(
	Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_graphic_type Cmiss_graphic_type_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_graphic_type_enum_to_string(enum Cmiss_graphic_type type);

/***************************************************************************//**
 * Return the name of the graphic. Graphic does not have a name until user has
 * set it.
 *
 * @param graphic  The graphic whose name is requested.
 * @return  On success: allocated string containing graphic name. Up to caller to
 * free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_graphic_get_name(Cmiss_graphic_id graphic);

/***************************************************************************//**
 * Set the name of the graphic. Unlike other containers, rendition can contains
 * multiple graphics with the same name.
 *
 * @param graphic  The graphic to be named.
 * @param name  The new name for the graphic.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_name(Cmiss_graphic_id graphic, const char *name);

/***************************************************************************//**
 * It takes the same string of command as gfx modify g_element <region_name>
 * <graphic_type> does. User can use this to quickly modify graphics. Make sure
 * coordinates field is specified.
 *
 * NOTE: This function may be removed in the future once more API functions are
 * made available to the users.
 *
 * @param graphic  Handle to a cmiss_graphic object.
 * @param command  Command to be executed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_define(Cmiss_graphic_id graphic, const char *command_string);

/**
 * Set the element type to use for the graphic.  Note that this may not be applicable
 * to all graphic types.
 *
 * @param graphic Handle to a cmiss_graphic object.
 * @param use_type enum value of the element type to use.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_use_element_type(Cmiss_graphic_id graphic, enum Cmiss_graphic_use_element_type use_type);

/**
 * If the graphic is of type iso surface graphic then this function returns
 * the iso_surface specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the new iso surface graphic reference.
 *
 * @param graphic  The graphic to be cast.
 * @return  Iso surface graphic specific representation if the input is the correct
 * graphic type, otherwise returns NULL.
 */
ZINC_API Cmiss_graphic_iso_surface_id Cmiss_graphic_cast_iso_surface(Cmiss_graphic_id graphic);

/**
 * Cast iso surface graphic back to its base graphic and return the graphic.
 * IMPORTANT NOTE: Returned graphic does not have incremented reference count and
 * must not be destroyed. Use Cmiss_graphic_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the iso_surface_graphic argument.
 *
 * @param iso_surface_graphic  Handle to the iso surface graphic to cast.
 * @return  Non-accessed handle to the base graphic or NULL if failed.
 */
ZINC_C_INLINE Cmiss_graphic_id Cmiss_graphic_iso_surface_base_cast(Cmiss_graphic_iso_surface_id iso_surface_graphic)
{
	return (Cmiss_graphic_id)(iso_surface_graphic);
}

/**
 * Destroys this reference to the iso surface graphic (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param iso_surface_address  Address of handle to the iso surface graphic.
 * @return  Status CMISS_OK if successfully destroyed the iso surface graphic handle,
 * any other value on failure.
 */
ZINC_API int Cmiss_graphic_iso_surface_destroy(Cmiss_graphic_iso_surface_id *iso_surface_address);

/**
 * Set the iso scalar field for the iso surface graphic.
 *
 * @param iso_surface_graphic The iso surface graphic to set the field to.
 * @param iso_scalar_field The iso scalar field to set, this field must have only one component.
 * @return Status CMISS_OK if the field was successfully set, any other value on failure
 */
ZINC_API int Cmiss_graphic_iso_surface_set_iso_scalar_field(Cmiss_graphic_iso_surface_id iso_surface_graphic, Cmiss_field_id iso_scalar_field);

/**
 * Set the iso values for the iso surface graphic.
 *
 * @param iso_surface_graphic The iso surface graphic to set the field to.
 * @param number_of_values The number of values in the values array.
 * @param values The array of double values with length number_of_values.
 * @return Status CMISS_OK if the field was successfully set, any other value on failure
 */
ZINC_API int Cmiss_graphic_iso_surface_set_iso_values(Cmiss_graphic_iso_surface_id iso_surface_graphic, int number_of_values, double *values);

/**
 * Set the iso range for the iso surface graphic.
 *
 * @param iso_surface_graphic The iso surface graphic to set the field to.
 * @param number_of_values The number of values to have between the first and last values (inclusive).
 * @param first_value The first iso value.
 * @param last_value The last iso value.
 * @return Status CMISS_OK if the field was successfully set, any other value on failure
 */
ZINC_API int Cmiss_graphic_iso_surface_set_iso_range(Cmiss_graphic_iso_surface_id iso_surface_graphic, int number_of_values, double first_value, double last_value);

#ifdef __cplusplus
}
#endif

#endif /*__CMISS_GRAPHIC_H__*/
