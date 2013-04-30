/**
 * FILE : cmiss_graphic.h
 *
 * The public interface to a zinc graphic which produces a 3-D graphics
 * object from an algorithm using fields in a region.
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
 * Portions created by the Initial Developer are Copyright (C) 2005-2013
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
#include "types/graphicsfontid.h"
#include "types/graphicisosurfaceid.h"
#include "types/graphicsrendertype.h"
#include "types/graphicscoordinatesystem.h"
#include "types/graphicsmaterialid.h"
#include "types/spectrumid.h"
#include "types/tessellationid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new reference to the graphic with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param graphic  The graphic to obtain a new reference to.
 * @return  New graphic reference with incremented reference count.
 */
ZINC_API Cmiss_graphic_id Cmiss_graphic_access(Cmiss_graphic_id graphic);

/**
 * Destroys the graphic and sets the pointer to NULL.
 *
 * @param graphic  The pointer to the handle of the graphic
 * @return  Status CMISS_OK if successfully destroy graphic, otherwise any
 * other value.
 */
ZINC_API int Cmiss_graphic_destroy(Cmiss_graphic_id *graphic);

/**
 * Gets the field supplying coordinates for the graphic.
 *
 * @param graphic  The graphic to be queried.
 * @return  Handle to coordinate field, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_graphic_get_coordinate_field(
	Cmiss_graphic_id graphic);

/**
 * Sets the field supplying coordinates for the graphic.
 *
 * @param graphic  The graphic to be modified.
 * @param coordinate_field  The field to use as the coordinate field.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_coordinate_field(Cmiss_graphic_id graphic,
	Cmiss_field_id coordinate_field);

/**
 * Gets the data field used with the spectrum to colour the graphic.
 *
 * @param graphic  The graphic to be queried.
 * @return  Handle to data field, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_graphic_get_data_field(Cmiss_graphic_id graphic);

/**
 * Sets the data field used with the spectrum to colour the graphic.
 *
 * @param graphic  The graphic to be modified.
 * @param data_field  The field to use as the data field.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_data_field(Cmiss_graphic_id graphic,
	Cmiss_field_id data_field);

/**
 * Sets the material giving the colour/shading of the graphic.
 *
 * @param graphic  The graphic to be modified.
 * @param material  The standard/unselected material colour.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_material(Cmiss_graphic_id graphic, Cmiss_graphics_material_id material);

/**
 * Set the material giving the colour/shading of the graphic when selected.
 *
 * @param graphic  The graphic to be modified.
 * @param material  The selected/highlight material.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_selected_material(
	Cmiss_graphic_id graphic, Cmiss_graphics_material_id material);

/**
 * Gets the spectrum used with the data field to colour the graphic.
 *
 * @param graphic  The graphic to be queried.
 * @return  Handle to spectrum, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_spectrum_id Cmiss_graphic_get_spectrum(Cmiss_graphic_id graphic);

/**
 * Sets the spectrum used with the data field to colour the graphic.
 *
 * @param graphic  The graphic to be modified.
 * @param spectrum  The spectrum to use to map colours.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_spectrum(Cmiss_graphic_id graphic,
	Cmiss_spectrum_id spectrum);

/**
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

/**
 * Returns the tessellation object of the graphics or NULL if none.
 * Caller must destroy reference.
 *
 * @param graphic  The graphic to be edit
 *
 * @return  tessellation for graphic or NULL if none.
 */
ZINC_API Cmiss_tessellation_id Cmiss_graphic_get_tessellation(Cmiss_graphic_id graphic);

/**
 * Sets the tessellation object of the graphics.
 *
 * @param graphic  The graphic to be edit
 * @param tessellation  The tessellation object to be set for graphic
 *
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_tessellation(
		Cmiss_graphic_id graphic, Cmiss_tessellation_id tessellation);

/**
 * Get the render type of the graphic.
 *
 * @param graphic  The handle to the graphic to be edit
 * @return  The render type If successfully get render_type for graphic, otherwise
 * 		it returns INVALID_TYPE;
 */
ZINC_API enum Cmiss_graphics_render_type Cmiss_graphic_get_render_type(
	Cmiss_graphic_id graphic);

/**
 * Set the type for how the graphics will be rendered in GL.
 *
 * @param graphic  The handle to the graphic to be edit
 * @param render_type  type of rendering for this graphic, please see the
 *   render_type definition for more information.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_render_type(
	Cmiss_graphic_id graphic, enum Cmiss_graphics_render_type render_type);

/**
 * Return status of graphic visibility flag attribute.
 *
 * @param graphic  The graphic to query.
 * @return  1 if graphic visibility flag is set, 0 if not.
 */
ZINC_API int Cmiss_graphic_get_visibility_flag(Cmiss_graphic_id graphic);

/**
 * Sets status of graphic visibility flag attribute.
 *
 * @param graphic  The graphic to modify.
 * @param visibility_flag  1 to set, 0 to clear.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_visibility_flag(Cmiss_graphic_id graphic,
	int visibility_flag);

/**
 * Gets flag to generate graphics for exterior faces or lines only.
 *
 * @param graphic  The graphic to query.
 * @return  1 if exterior flag is set, otherwise 0.
 */
ZINC_API int Cmiss_graphic_get_exterior(Cmiss_graphic_id graphic);

/**
 * Sets flag to generate graphics for exterior faces or lines only.
 *
 * @param graphic  The graphic to modify.
 * @param exterior  1 to set, 0 to clear.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_set_exterior(Cmiss_graphic_id graphic, int exterior);

/**
 * Gets the face the graphic is limited to generate graphics for.
 *
 * @param graphic  The graphic to query.
 * @return  The face type of the graphic, or CMISS_GRAPHIC_FACE_INVALID if the
 * graphic is invalid.
 */
ZINC_API Cmiss_graphic_face_type Cmiss_graphic_get_face(Cmiss_graphic_id graphic);

/**
 * Sets the face the graphic is limited to generate graphics for.
 * e.g. CMISS_GRAPHIC_FACE_XI1_0 generates graphics only on faces and lines
 * where the top-level element 'xi' coordinate equals 0.
 *
 * @param graphic  The graphic to modify.
 * @param face  A valid face type.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_set_face(Cmiss_graphic_id graphic,
	Cmiss_graphic_face_type face);

/**
 * Specifying the coordinate system in which to render the coordinates of graphics.
 *
 *  @param graphic  The graphic to modify.
 * @param coordinate_system  enumerator describing coordinate system to be set
 * 		for graphic.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_coordinate_system(Cmiss_graphic_id graphic,
	enum Cmiss_graphics_coordinate_system coordinate_system);

/**
 * Get the coordinate system in which to render the coordinates of graphics.
 *
 * @param graphic  The graphic to modify.
 * @return  coordinate system used in graphic.
 */
ZINC_API enum Cmiss_graphics_coordinate_system Cmiss_graphic_get_coordinate_system(
	Cmiss_graphic_id graphic);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_graphic_type Cmiss_graphic_type_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_graphic_type_enum_to_string(enum Cmiss_graphic_type type);

/**
 * Return the name of the graphic. Graphic does not have a name until user has
 * set it.
 *
 * @param graphic  The graphic whose name is requested.
 * @return  On success: allocated string containing graphic name. Up to caller to
 * free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_graphic_get_name(Cmiss_graphic_id graphic);

/**
 * Set the name of the graphic. Unlike other containers, rendition can contains
 * multiple graphics with the same name.
 *
 * @param graphic  The graphic to be named.
 * @param name  The new name for the graphic.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_graphic_set_name(Cmiss_graphic_id graphic, const char *name);

/**
 * Get handle to point attributes object for specifying glyph, scaling fields
 * and scale factors used in visualising points produced by the graphic.
 * Only returned for graphic classes for viewing points.
 *
 * @param graphic  The graphic to request attributes for.
 * @return  Handle to point attributes object (up to caller to destroy), or 0
 * if none, not supported or error.
 */
ZINC_API Cmiss_graphic_point_attributes_id Cmiss_graphic_get_point_attributes(
	Cmiss_graphic_id graphic);

/**
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
 * Gets the iso scalar field for the iso surface graphic.
 *
 * @param iso_surface_graphic  The iso surface graphic to query.
 * @return  Handle to iso_scalar field, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_graphic_iso_surface_get_iso_scalar_field(
	Cmiss_graphic_iso_surface_id iso_surface_graphic);

/**
 * Set the iso scalar field for the iso surface graphic.
 *
 * @param iso_surface_graphic  The iso surface graphic to modify.
 * @param iso_scalar_field  The iso scalar field to set, this field must have
 * only one component.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_iso_surface_set_iso_scalar_field(
	Cmiss_graphic_iso_surface_id iso_surface_graphic,
	Cmiss_field_id iso_scalar_field);

/**
 * Get the iso values for the iso surface graphic when it has been set as an
 * explicit list.
 * @see Cmiss_graphic_iso_surface_set_iso_values.
 *
 * @param iso_surface_graphic  The iso surface graphic to query.
 * @param number_of_iso_values  The size of the iso_values array.
 * @param iso_values  Array to receive number_of_values iso values.
 * @return  The actual number of iso values that have been explicitly set using
 * Cmiss_graphic_iso_surface_set_iso_values. This can be more than the number
 * requested, so a second call may be needed with a larger array. A zero return
 * value can indicate iso_values were set as a range, not as an explicit list.
 */
ZINC_API int Cmiss_graphic_iso_surface_get_iso_values(
	Cmiss_graphic_iso_surface_id iso_surface_graphic, int number_of_iso_values,
	double *iso_values);

/**
 * Set the iso values for the iso surface graphic as an explicit list.
 *
 * @param iso_surface_graphic  The iso surface graphic to modify.
 * @param number_of_iso_values  The number of values in the iso_values array.
 * @param iso_values  The array of number_of_iso_values double values.
 * @return  Status CMISS_OK on success, otherwise any error code.
 */
ZINC_API int Cmiss_graphic_iso_surface_set_iso_values(
	Cmiss_graphic_iso_surface_id iso_surface_graphic, int number_of_iso_values,
	const double *iso_values);

/**
 * Get the iso values for the iso surface graphic as a number in a range from
 * first value to last value, in uniform increments.
 *
 * @param iso_surface_graphic  The iso surface graphic to query.
 * @param first_iso_value_address  The address to put the first iso value.
 * @param last_iso_value_address  The address to put the last iso value.
 * @return  Number of iso values in the range, or 0 if not set as a range or
 * other error.
 */
ZINC_API int Cmiss_graphic_iso_surface_get_iso_range(
	Cmiss_graphic_iso_surface_id iso_surface_graphic,
	double *first_iso_value_address, double *last_iso_value_address);

/**
 * Set the iso values for the iso surface graphic as a number in a range from
 * first value to last value, in uniform increments.
 * For example, 5 values from 0.0 to 1.0 gives: 0.0, 0.25, 0.5, 0.75 and 1.0.
 *
 * @param iso_surface_graphic  The iso surface graphic to modify.
 * @param number_of_iso_values  The number of values to have between the first
 * and last iso last values.
 * @param first_iso_value  The first iso value.
 * @param last_iso_value  The last iso value.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_iso_surface_set_iso_range(
	Cmiss_graphic_iso_surface_id iso_surface_graphic, int number_of_iso_values,
	double first_iso_value, double last_iso_value);

/**
 * If the graphic produces lines or extrusions then returns a handle to the
 * line attribute object for specifying section profile and scaling.
 *
 * @param graphic  The graphic to request line attributes from.
 * @return  Handle to line attributes object, or 0 if not supported for
 * graphic type or error. Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_graphic_line_attributes_id Cmiss_graphic_get_line_attributes(
	Cmiss_graphic_id graphic);

/**
 * Returns a new reference to the line attributes with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param line_attributes  The line_attributes to obtain a new reference to.
 * @return  New line attributes reference with incremented reference count.
 */
ZINC_API Cmiss_graphic_line_attributes_id Cmiss_graphic_line_attributes_access(
	Cmiss_graphic_line_attributes_id line_attributes);

/**
 * Destroys this reference to the line attributes, and sets it to 0.
 * Internally this just decrements the reference count.
 *
 * @param line_attributes_address  Address of handle to the line attributes.
 * @return  Status CMISS_OK if successfully destroyed the handle, any other
 * value on failure.
 */
ZINC_API int Cmiss_graphic_line_attributes_destroy(
	Cmiss_graphic_line_attributes_id *line_attributes_address);

/**
 * Gets the base size of the extrusion section, one value for each lateral axis.
 * @see Cmiss_graphic_line_attributes_set_base_size.
 *
 * @param line_attributes  The line_attributes to query.
 * @param number  The number of base size values to request, starting with the
 * first lateral axis. If fewer values have been set it is padded with the last
 * base size value. 1 to 3 values can be obtained.
 * @param base_size  Array to receive base sizes. Must be big enough to contain
 * the specified number of values.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_line_attributes_get_base_size(
	Cmiss_graphic_line_attributes_id line_attributes, int number,
	double *base_size);

/**
 * Sets the base size of the extrusion section, one value for each lateral axis.
 * 1 to 3 values can be set.
 * For a unit section profile, the final size in each lateral direction is:
 * base_size + scale_factor * field_scalar
 * where field_scalar is determined from the orientation_scale_field.
 * @see Cmiss_graphic_line_attributes_set_orientation_scale_field.
 * The default base size is zero.
 * Note: only a single base size is used at present. It gives the base diameter
 * of cylinders and the width of streamlines.
 *
 * @param line_attributes  The line_attributes to modify.
 * @param number  The number of base size values to set, starting with the
 * first lateral axis. If fewer values are set than the number of axes,
 * the last value is assumed for subsequent axes. Hence a single value can
 * be used to set a diameter for a unit circle profile.
 * @param base_size  Array of base sizes with the number of values specified.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_line_attributes_set_base_size(
	Cmiss_graphic_line_attributes_id line_attributes, int number,
	const double *base_size);

/**
 * Gets the orientation scale field from the graphic line attributes. This
 * controls lateral scaling and orientation of line extrusions.
 * @see Cmiss_graphic_line_attributes_set_orientation_scale_field
 *
 * @param line_attributes  The line attributes to query.
 * @return Handle to orientation scale field, or 0 if none or error. Up to
 * caller to destroy the returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_graphic_line_attributes_get_orientation_scale_field(
	Cmiss_graphic_line_attributes_id line_attributes);

/**
 * Sets the orientation scale field in the graphic line attributes. This
 * controls lateral scaling and orientation of line extrusions.
 * Note: Currently it is only used to supply a scalar field for scaling
 * cylinders and with default scale factor of 1 it gives its diameter. Soon
 * we will offer more options for controlling extrusions.
 *
 * @param line_attributes  The line attributes to modify.
 * @param orientation_scale_field  The orientation scale field to set.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_line_attributes_set_orientation_scale_field(
	Cmiss_graphic_line_attributes_id line_attributes,
	Cmiss_field_id orientation_scale_field);

/**
 * Gets the scale factors used in sizing the extrusion section, one value for
 * each lateral axis.
 * @see Cmiss_graphic_line_attributes_set_scale_factors.
 *
 * @param line_attributes  The line_attributes to query.
 * @param number  The number of scale_factors to request, starting with the
 * first lateral axis. If fewer values have been set it is padded with the last
 * scale factor value. 1 to 3 values can be obtained.
 * @param scale_factors  Array to receive scale factors. Must be big enough to
 * contain the specified number of values.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_line_attributes_get_scale_factors(
	Cmiss_graphic_line_attributes_id line_attributes, int number,
	double *scale_factors);

/**
 * Sets the scale factors used in sizing the extrusion section, one value for
 * each lateral axis. 1 to 3 values can be set.
 * For a unit section profile, the final size in each lateral direction is:
 * base_size + scale_factor * field_scalar
 * where field_scalar is determined from the orientation_scale_field.
 * @see Cmiss_graphic_line_attributes_set_orientation_scale_field.
 * Scale factor values default to 1.
 * Note: only a single scale factor is used at present.
 *
 * @param line_attributes  The line_attributes to modify.
 * @param number  The number of scale factor values to set, starting with the
 * first lateral axis. If fewer values are set than the number of axes,
 * the last value is assumed for subsequent axes. Hence a single value applies
 * to all axes.
 * @param scale_factors  Array of scale factors with the number of values
 * specified.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_line_attributes_set_scale_factors(
	Cmiss_graphic_line_attributes_id line_attributes, int number,
	const double *scale_factors);

/**
 * If the graphic produces points then returns a handle to point attribute
 * object for specifying glyph, scaling fields, scale factors and labels.
 *
 * @param graphic  The graphic to request point attributes from.
 * @return  Handle to point attributes object, or 0 if not supported for
 * graphic type or error. Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_graphic_point_attributes_id Cmiss_graphic_get_point_attributes(
	Cmiss_graphic_id graphic);

/**
 * Returns a new reference to the point attributes with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param point_attributes  The point_attributes to obtain a new reference to.
 * @return  New point attributes reference with incremented reference count.
 */
ZINC_API Cmiss_graphic_point_attributes_id Cmiss_graphic_point_attributes_access(
	Cmiss_graphic_point_attributes_id point_attributes);

/**
 * Destroys this reference to the point attributes, and sets it to 0.
 * Internally this just decrements the reference count.
 *
 * @param point_attributes_address  Address of handle to the point attributes.
 * @return  Status CMISS_OK if successfully destroyed the handle, any other
 * value on failure.
 */
ZINC_API int Cmiss_graphic_point_attributes_destroy(
	Cmiss_graphic_point_attributes_id *point_attributes_address);

/**
 * Gets the base size of the point glyph, up to 3 values, one for each axis.
 * @see Cmiss_graphic_point_attributes_set_base_size.
 *
 * @param point_attributes  The point_attributes to query.
 * @param number  The number of base size values to request, starting with the
 * first axis. If fewer values have been set it is padded with the last
 * base size value. 1 to 3 values can be obtained.
 * @param base_size  Array to receive base sizes. Must be big enough to contain
 * the specified number of values.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_get_base_size(
	Cmiss_graphic_point_attributes_id point_attributes, int number,
	double *base_size);

/**
 * Sets the base size of the point glyph, up to 3 values, one for each axis.
 * For a unit sized glyph, the final size in each direction is:
 * base_size + scale_factor * field_scalar
 * where field_scalar is determined from the orientation_scale_field.
 * @see Cmiss_graphic_point_attributes_set_orientation_scale_field.
 * The default base size is zero.
 * Tip: to visualise a 2- or 3-component vector orientation_scale_field, use a
 * base size of 0,D,D with scale factors 1,0,0. This ensures the length equals
 * the magnitude and the glyph is fixed to size D on the orthogonal axes.
 *
 * @param point_attributes  The point_attributes to modify.
 * @param number  The number of base size values to set, starting with the
 * first axis. If fewer values are set than the number of axes, the last value
 * is assumed for subsequent axes. Hence a single value can be used to set the
 * diameter for a unit sphere glyph.
 * @param base_size  Array of base sizes with the number of values specified.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_set_base_size(
	Cmiss_graphic_point_attributes_id point_attributes, int number,
	const double *base_size);

/**
 * Gets the font in the graphic point attributes used to draw the label field.
 *
 * @param point_attributes  The point attributes to query.
 * @return Handle to font, or 0 if none or error. Up to caller to destroy
 * returned handle.
 */
ZINC_API Cmiss_graphics_font_id Cmiss_graphic_point_attributes_get_font(
	Cmiss_graphic_point_attributes_id point_attributes);

/**
 * Sets the font in the graphic point attributes used to draw the label field.
 *
 * @param point_attributes  The point attributes to modify.
 * @param font  The font to set.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_set_font(
	Cmiss_graphic_point_attributes_id point_attributes,
	Cmiss_graphics_font_id font);

/**
 * Sets the glyph use for visualising a graphics point from a reduced set
 * of enumerations.
 *
 * @param point_attributes  The point attributes to modify.
 * @param glyph_type  The glyph type identifier.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_set_glyph_type(
	Cmiss_graphic_point_attributes_id point_attributes,
	enum Cmiss_graphics_glyph_type glyph_type);

/**
 * Gets the label field from the graphic point attributes.
 *
 * @param point_attributes  The point attributes to query.
 * @return Handle to label field, or 0 if none or error. Up to caller to destroy
 * returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_graphic_point_attributes_get_label_field(
	Cmiss_graphic_point_attributes_id point_attributes);

/**
 * Sets the label field in the graphic point attributes. A string representation
 * of this field's value (if defined) is drawn with the current font at the
 * glyph offset.
 *
 * @param point_attributes  The point attributes to modify.
 * @param label_field  The label field to set.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_set_label_field(
	Cmiss_graphic_point_attributes_id point_attributes, Cmiss_field_id label_field);

/**
 * Gets the offset from the point coordinates to where the glyph origin is
 * drawn, in glyph units along the axes, i.e. values are subsequently scaled.
 * @see Cmiss_graphic_point_attributes_set_offset.
 *
 * @param point_attributes  The point_attributes to query.
 * @param number  The number of offset values to request, up to 3.
 * @param offset  Array to receive offset values, starting with the first axis.
 * Must be big enough to contain the specified number of values.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_get_offset(
	Cmiss_graphic_point_attributes_id point_attributes, int number,
	double *offset);

/**
 * Sets the offset from the point coordinates to where the glyph origin is
 * drawn, in glyph units along the axes, i.e. values are subsequently scaled.
 * @see Cmiss_graphic_point_attributes_set_orientation_scale_field.
 * Tip: offset values can give the effect of moving the centre point / origin
 * of the glyph: just pass in negative coordinates for the new origin.
 *
 * @param point_attributes  The point_attributes to modify.
 * @param number  The number of offset values to set, up to 3.
 * @param offset  Array of offset values, starting with the first axis. If
 * fewer than 3 then zero is assumed for all other offset values.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_set_offset(
	Cmiss_graphic_point_attributes_id point_attributes, int number,
	const double *offset);

/**
 * Gets the orientation scale field from the graphic point attributes. This
 * controls scaling and orientation of point glyphs.
 * @see Cmiss_graphic_point_attributes_set_orientation_scale_field
 *
 * @param point_attributes  The point attributes to query.
 * @return Handle to orientation scale field, or 0 if none or error. Up to
 * caller to destroy the returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_graphic_point_attributes_get_orientation_scale_field(
	Cmiss_graphic_point_attributes_id point_attributes);

/**
 * Sets the orientation scale field in the graphic point attributes. This
 * controls scaling and orientation of point glyphs.
 * Only fields with the following numbers of components are allowed, with the
 * prescribed behaviour:
 * 1 : default orientation in line with local x, y and z axes, with scalar value
 *     equally applied in all 3 axes.
 * 2 : 2-D vector giving orientation of 1st axis. 2nd vector is 90 degrees
 *     anticlockwise in 2-D plane, and 3rd vector is v1 (x) v2. Scale in all
 *     directions is magnitude of vector; use a zero scale factor to not scale
 *     in any axis.
 * 3 : 3-D vector giving orientation of first axis. 2nd vector is arbitrary
 *     normal to this, and 3rd vector is v1 (x) v2. Scale in all directions is
 *     magnitude of vector; use a zero scale factor to not scale in any axis.
 * 4 : 2 2-D vectors each giving orientation and scale (from magnitude) of 1st
 *     and 2nd axes. 3rd axis is v1 (x) v2 including scale from its magnitude.
 * 6 : 2 3-D vectors each giving orientation and scale (from magnitude) of 1st
 *     and 2nd axes. 3rd axis is v1 (x) v2 including scale from its magnitude.
 * 9 : 3 3-D vectors giving orientation and scale (from magnitude) of all 3
 *     axes, in a right-handed sense.
 * Note the signed scale field provides additional scaling and orientation
 * reversal for negative values.
 * @see Cmiss_graphic_point_attributes_set_signed_scale_field
 *
 * @param point_attributes  The point attributes to modify.
 * @param orientation_scale_field  The orientation scale field to set.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_set_orientation_scale_field(
	Cmiss_graphic_point_attributes_id point_attributes,
	Cmiss_field_id orientation_scale_field);

/**
 * Sets the scale factors used in sizing the point glyph, up to 3 values,
 * one for each axis.
 * @see Cmiss_graphic_point_attributes_set_scale_factors.
 *
 * @param point_attributes  The point_attributes to query.
 * @param number  The number of scale_factors to request, starting with the
 * first axis. If fewer values have been set it is padded with the last
 * scale factor value. 1 to 3 values can be obtained.
 * @param scale_factors  Array to receive scale factors. Must be big enough to
 * contain the specified number of values.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_get_scale_factors(
	Cmiss_graphic_point_attributes_id point_attributes, int number,
	double *scale_factors);

/**
 * Sets the scale factors used in sizing the point glyph, up to 3 values,
 * one for each axis.
 * For a unit sized glyph, the final size in each direction is:
 * base_size + scale_factor * field_scalar
 * where field_scalar is determined from the orientation_scale_field.
 * @see Cmiss_graphic_point_attributes_set_orientation_scale_field.
 * Scale factor values default to 1.
 * Tip: to visualise a 2- or 3-component vector orientation_scale_field, use a
 * base size of 0,D,D with scale factors 1,0,0. This ensures the length equals
 * the magnitude and the glyph is fixed to size D on the orthogonal axes.
 *
 * @param point_attributes  The point_attributes to modify.
 * @param number  The number of scale factor values to set, starting with the
 * first axis. If fewer values are set than the number of axes, the last value
 * is assumed for subsequent axes. Hence a single value applies to all axes.
 * @param scale_factors  Array of scale factors with the number of values
 * specified.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_set_scale_factors(
	Cmiss_graphic_point_attributes_id point_attributes, int number,
	const double *scale_factors);

/**
 * Gets the signed scale field from the graphic point attributes.
 * @see Cmiss_graphic_point_attributes_set_signed_scale_field
 *
 * @param point_attributes  The point attributes to query.
 * @return Handle to signed scale field, or 0 if none or error. Up to
 * caller to destroy the returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_graphic_point_attributes_get_signed_scale_field(
	Cmiss_graphic_point_attributes_id point_attributes);

/**
 * Sets the signed scale field in the graphic point attributes. Can have from 1
 * to 3 components. Its absolute value in any axis multiplies the scaling from
 * the orientation scale field, and if negative adds a further offset to the end
 * of the scale axis and reverses the orientation. It is commonly used to draw
 * stress and strain with mirrored arrow glyphs to show them pointing inward for
 * compression, outward for tension.
 *
 * @param point_attributes  The point attributes to modify.
 * @param signed_scale_field  The signed scale field to set.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_graphic_point_attributes_set_signed_scale_field(
	Cmiss_graphic_point_attributes_id point_attributes,
	Cmiss_field_id signed_scale_field);

/**
 * If the graphic produces points then returns a handle to point attribute
 * object for specifying glyph, scaling fields, scale factors and labels.
 *
 * @param graphic  The graphic to request point attributes from.
 * @return  Handle to point attributes object, or 0 if not supported for
 * graphic type or error. Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_graphic_element_attributes_id Cmiss_graphic_get_element_attributes(
	Cmiss_graphic_id graphic);

/**
 * Returns a new reference to the point attributes with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param point_attributes  The point_attributes to obtain a new reference to.
 * @return  New point attributes reference with incremented reference count.
 */
ZINC_API Cmiss_graphic_element_attributes_id Cmiss_graphic_element_attributes_access(
	Cmiss_graphic_element_attributes_id element_attributes);

/**
 * Destroys this reference to the point attributes, and sets it to 0.
 * Internally this just decrements the reference count.
 *
 * @param point_attributes_address  Address of handle to the point attributes.
 * @return  Status CMISS_OK if successfully destroyed the handle, any other
 * value on failure.
 */
ZINC_API int Cmiss_graphic_element_attributes_destroy(
	Cmiss_graphic_element_attributes_id *element_attributes_address);

/**
 */
ZINC_API int Cmiss_graphic_element_attributes_set_discretization(
	Cmiss_graphic_element_attributes_id element_attributes, int number,
	const int *discretization);

/**
 */
ZINC_API int Cmiss_graphic_element_attributes_set_discretization_mode(
	Cmiss_graphic_element_attributes_id element_attributes, Cmiss_graphics_xi_discretization_mode mode);

#ifdef __cplusplus
}
#endif

#endif /*__CMISS_GRAPHIC_H__*/
