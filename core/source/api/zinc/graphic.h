/**
 * FILE : graphic.h
 *
 * The public interface to a zinc graphic which produces a 3-D graphics
 * object from an algorithm using fields in a region.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_GRAPHIC_H__
#define CMZN_GRAPHIC_H__

#include "types/elementid.h"
#include "types/fieldid.h"
#include "types/glyphid.h"
#include "types/graphicid.h"
#include "types/fontid.h"
#include "types/graphicsmaterialid.h"
#include "types/scenecoordinatesystem.h"
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
ZINC_API cmzn_graphic_id cmzn_graphic_access(cmzn_graphic_id graphic);

/**
 * Destroys the graphic and sets the pointer to NULL.
 *
 * @param graphic_address  The pointer to the handle of the graphic.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_destroy(cmzn_graphic_id *graphic_address);

/**
 * Gets the field supplying coordinates for the graphic.
 *
 * @param graphic  The graphic to be queried.
 * @return  Handle to coordinate field, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_get_coordinate_field(
	cmzn_graphic_id graphic);

/**
 * Sets the field supplying coordinates for the graphic.
 *
 * @param graphic  The graphic to modify.
 * @param coordinate_field  The field to use as the coordinate field. May have
 * from 1 to 3 components.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_coordinate_field(cmzn_graphic_id graphic,
	cmzn_field_id coordinate_field);

/**
 * Gets the data field used with the spectrum to colour the graphic.
 *
 * @param graphic  The graphic to be queried.
 * @return  Handle to data field, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_get_data_field(cmzn_graphic_id graphic);

/**
 * Sets the data field used with the spectrum to colour the graphic.
 *
 * @param graphic  The graphic to be modified.
 * @param data_field  The field to use as the data field.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_graphic_set_data_field(cmzn_graphic_id graphic,
	cmzn_field_id data_field);

/**
 * Gets the material giving the colour/shading of the graphic when unselected.
 *
 * @param graphic  The graphic to query.
 * @return  Handle to material, or 0 if error. Up to caller to destroy returned
 * handle.
 */
ZINC_API cmzn_graphics_material_id cmzn_graphic_get_material(
	cmzn_graphic_id graphic);

/**
 * Sets the material giving the colour/shading of the graphic when unselected.
 *
 * @param graphic  The graphic to modify.
 * @param material  The standard/unselected material.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_material(cmzn_graphic_id graphic,
	cmzn_graphics_material_id material);

/**
 * Get the width used for rendering line primitives in GL in renderer 'point
 * units', typically pixels for on-screen display.
 *
 * @param graphic  The graphic to query.
 * @return  The line render width in pixels, or 0.0 if bad argument.
 */
ZINC_API double cmzn_graphic_get_render_line_width(cmzn_graphic_id graphic);

/**
 * Set the width used for rendering line primitives in GL in renderer 'point
 * units', typically pixels for on-screen display but scalable to ensure lines
 * are of appropriate thickness on high resolution output.
 * Note there may be arbitrary limits on values depending on the GL
 * implementation, and for non-antialised rendering the nearest integer is used.
 * The default value is 1.0.
 *
 * @param graphic  The graphic to modify.
 * @param width  The line render width in pixels. Value > 0.0.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_render_line_width(cmzn_graphic_id graphic,
	double width);

/**
 * Get the size used for rendering point primitives in GL in renderer 'point
 * units', typically pixels for on-screen display.
 *
 * @param graphic  The graphic to query.
 * @return  The point render size in pixels, or 0.0 if bad argument.
 */
ZINC_API double cmzn_graphic_get_render_point_size(cmzn_graphic_id graphic);

/**
 * Set the size used for rendering point primitives in GL in renderer 'point
 * units', typically pixels for on-screen display but scalable to ensure points
 * are of appropriate size on high resolution output.
 * Note there may be arbitrary limits on values depending on the GL
 * implementation, and may be rounded to the nearest integer in use.
 * The default value is 1.0.
 *
 * @param graphic  The graphic to modify.
 * @param size  The point render size in pixels. Value > 0.0.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_render_point_size(cmzn_graphic_id graphic,
	double size);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name.
 * @return  The enumeration matching the string, or MODE_INVALID if not found.
 */
ZINC_API enum cmzn_graphic_render_polygon_mode
	cmzn_graphic_render_polygon_mode_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_graphic_render_polygon_mode_enum_to_string(
	enum cmzn_graphic_render_polygon_mode mode);

/**
 * Get mode controlling how polygons in the graphic are rendered in GL.
 *
 * @param graphic  The graphic to query.
 * @return  The render mode, or MODE_INVALID if bad arguments.
 */
ZINC_API enum cmzn_graphic_render_polygon_mode cmzn_graphic_get_render_polygon_mode(
	cmzn_graphic_id graphic);

/**
 * Set mode controlling how polygons in the graphic are rendered in GL:
 * shaded/filled, wireframe etc.
 * @see cmzn_graphic_render_polygon_mode
 *
 * @param graphic  The graphic to modify.
 * @param render_polygon_mode  Mode of polygon rendering: shaded, wireframe.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_render_polygon_mode(cmzn_graphic_id graphic,
	enum cmzn_graphic_render_polygon_mode render_polygon_mode);

/**
 * Gets the mode controlling how graphics are drawn depending on whether the
 * underlying object is selected, and also whether picking is enabled.
 *
 * @param graphic  The graphic to query.
 * @return  The select mode, or MODE_INVALID if bad argument.
 */
ZINC_API enum cmzn_graphic_select_mode cmzn_graphic_get_select_mode(
	cmzn_graphic_id graphic);

/**
 * Sets the mode controlling how graphics are drawn depending on whether the
 * underlying object is selected, and also whether picking is enabled.
 * An object is selected if it is in the current selection_group for the scene
 * which owns the graphic.
 * The default select mode for new graphics is CMZN_GRAPHIC_SELECT_ON.
 *
 * @param graphic  The graphic to modify.
 * @param select_mode  A valid select mode for the graphic.
 * @return  On success CMZN_OK, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_select_mode(cmzn_graphic_id graphic,
	enum cmzn_graphic_select_mode select_mode);

/**
 * Gets the material giving the colour/shading of the graphic when selected.
 *
 * @param graphic  The graphic to query.
 * @return  Handle to material, or 0 if error. Up to caller to destroy returned
 * handle.
 */
ZINC_API cmzn_graphics_material_id cmzn_graphic_get_selected_material(
	cmzn_graphic_id graphic);

/**
 * Set the material giving the colour/shading of the graphic when selected.
 *
 * @param graphic  The graphic to modify.
 * @param selected_material  The selected/highlight material.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_selected_material(cmzn_graphic_id graphic,
	cmzn_graphics_material_id selected_material);

/**
 * Gets the spectrum used with the data field to colour the graphic.
 *
 * @param graphic  The graphic to be queried.
 * @return  Handle to spectrum, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API cmzn_spectrum_id cmzn_graphic_get_spectrum(cmzn_graphic_id graphic);

/**
 * Sets the spectrum used with the data field to colour the graphic.
 *
 * @param graphic  The graphic to be modified.
 * @param spectrum  The spectrum to use to map colours.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_graphic_set_spectrum(cmzn_graphic_id graphic,
	cmzn_spectrum_id spectrum);

/**
 * Gets the field which returns true/non-zero for primitive to be created.
 *
 * @param graphic  The graphic to be queried.
 * @return  Handle to subgroup field, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_get_subgroup_field(cmzn_graphic_id graphic);

/**
 * Sets optional field which causes graphics to be generated only for parts of
 * the domain where its value is true/non-zero. Commonly a group, node_group or
 * element_group field which is efficiently iterated over. Note general fields
 * are evaluated at an arbitrary location in elements.
 *
 * @param graphic  The graphic to be modified.
 * @param subgroup_field  Scalar subgroup field.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_subgroup_field(cmzn_graphic_id graphic,
	cmzn_field_id subgroup_field);

/**
 * Returns the tessellation object of the graphic.
 * Caller must destroy reference.
 *
 * @param graphic  The graphic to query.
 * @return  Handle to tessellation, or 0 if none or error.
 */
ZINC_API cmzn_tessellation_id cmzn_graphic_get_tessellation(cmzn_graphic_id graphic);

/**
 * Sets the tessellation object of the graphics.
 *
 * @param graphic  The graphic to modify.
 * @param tessellation  The tessellation object to be set for graphic.
 *
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_tessellation(
	cmzn_graphic_id graphic, cmzn_tessellation_id tessellation);

/**
 * Gets the field controlling tessellation quality.
 *
 * @param graphic  The graphic to query.
 * @return  Handle to field, or 0 if error. Up to caller to destroy handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_get_tessellation_field(
	cmzn_graphic_id graphic);

/**
 * Sets a field which sets tessellation quality to match its native refinement
 * in any element (if any) and also apply refinement factors if the field
 * function is non-linear at its native refinement.
 * If no tessellation field is supplied, the coordinate field is checked for
 * non-linearity of function OR coordinate system, but not native refinement.
 *
 * @param graphic  The graphic to modify.
 * @param tessellation_field  The field which sets tessellation quality.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_tessellation_field(cmzn_graphic_id graphic,
	cmzn_field_id tessellation_field);

/**
 * Get the texture coordinate field of the graphic.
 * Caller must destroy reference.
 *
 * @param graphic  The graphic to query.
 * @return  Handle to field, or 0 if none or error.
 */
ZINC_API cmzn_field_id cmzn_graphic_get_texture_coordinate_field(
	cmzn_graphic_id graphic);

/**
 * Set the texture coordinate field of the graphic. Values of this field specify
 * mapping of the material's image field to coordinates in the graphic.
 * The texture coordinate field is currently unused by POINTS and STREAMLINES.
 *
 * @param graphic  The graphic to query.
 * @param texture_coordinate_field  The field to be set as the texture
 * coordinate field, or 0 for none. Field must have from 1 to 3 components.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_texture_coordinate_field(cmzn_graphic_id graphic,
	cmzn_field_id texture_coordinate_field);

/**
 * Return status of graphic visibility flag attribute.
 *
 * @param graphic  The graphic to query.
 * @return  Boolean value true if visibility flag is set, false if not.
 */
ZINC_API bool cmzn_graphic_get_visibility_flag(cmzn_graphic_id graphic);

/**
 * Sets status of graphic visibility flag attribute. Note this only affects
 * visibility of graphics when a scene filter is acting on it.
 *
 * @param graphic  The graphic to modify.
 * @param visibility_flag  Boolean true to set, false to clear.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_visibility_flag(cmzn_graphic_id graphic,
	bool visibility_flag);

/**
 * Gets flag to generate graphics for exterior faces or lines only.
 *
 * @param graphic  The graphic to query.
 * @return  Boolean true if the exterior flag is set, otherwise false.
 */
ZINC_API bool cmzn_graphic_is_exterior(cmzn_graphic_id graphic);

/**
 * Sets flag to generate graphics for exterior faces or lines only.
 *
 * @param graphic  The graphic to modify.
 * @param exterior  New boolean value: true to set, false to clear.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_exterior(cmzn_graphic_id graphic, bool exterior);

/**
 * Gets the face the graphic is limited to generate graphics for.
 *
 * @param graphic  The graphic to query.
 * @return  The face type of the graphic, or CMZN_ELEMENT_FACE_INVALID if the
 * graphic is invalid.
 */
ZINC_API enum cmzn_element_face_type cmzn_graphic_get_face(cmzn_graphic_id graphic);

/**
 * Sets the face the graphic is limited to generate graphics for.
 * e.g. CMZN_ELEMENT_FACE_XI1_0 generates graphics only on faces and lines
 * where the top-level element 'xi' coordinate equals 0.
 *
 * @param graphic  The graphic to modify.
 * @param face  A valid face type.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_face(cmzn_graphic_id graphic,
	enum cmzn_element_face_type face);

/**
 * Specifying the coordinate system in which to render the coordinates of graphics.
 *
 *  @param graphic  The graphic to modify.
 * @param coordinate_system  enumerator describing coordinate system to be set
 * 		for graphic.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_graphic_set_coordinate_system(cmzn_graphic_id graphic,
	enum cmzn_scene_coordinate_system coordinate_system);

/**
 * Get the coordinate system in which to render the coordinates of graphics.
 *
 * @param graphic  The graphic to modify.
 * @return  coordinate system used in graphic.
 */
ZINC_API enum cmzn_scene_coordinate_system cmzn_graphic_get_coordinate_system(
	cmzn_graphic_id graphic);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_graphic_type cmzn_graphic_type_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_graphic_type_enum_to_string(enum cmzn_graphic_type type);

/**
 * Return the name of the graphic. Graphic does not have a name until user has
 * set it.
 *
 * @param graphic  The graphic to query.
 * @return  On success: allocated string containing name, or NULL if none.
 * Up to caller to free using cmzn_deallocate().
 */
ZINC_API char *cmzn_graphic_get_name(cmzn_graphic_id graphic);

/**
 * Sets the name of the graphic. Unlike other containers, scene can contain
 * multiple graphics with the same name. New graphics default to having no name.
 * A common use of the name is to mark the graphic for filtering, like metadata.
 * @see cmzn_scenefiltermodule_create_scenefilter_graphic_name
 *
 * @param graphic  The graphic to modify.
 * @param name  The new name for the graphic, OR NULL to clear.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_name(cmzn_graphic_id graphic, const char *name);

/**
 * Get the field domain type graphics are created from with the graphic.
 *
 * @param graphic  Handle to the graphic to query.
 * @return  The domain type of the graphic or CMZN_FIELD_DOMAIN_TYPE_INVALID
 * on error.
 */
ZINC_API enum cmzn_field_domain_type cmzn_graphic_get_domain_type(
	cmzn_graphic_id graphic);

/**
 * Set the field domain type to create graphics from with the graphic.
 * Note that all domain types are applicable to all graphic types, for example
 * this attribute cannot be changed for LINES and SURFACES, which always use
 * 1D and 2D elements domains, respectively.
 *
 * @param graphic  Handle to the graphic to modify.
 * @param domain_type  Enumerated value of the field domain type to use.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_set_domain_type(cmzn_graphic_id graphic,
	enum cmzn_field_domain_type domain_type);

/**
 * If the graphic is of type contours then this function returns
 * the contours specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the new contours graphic reference.
 *
 * @param graphic  The graphic to be cast.
 * @return  Contours graphic specific representation if the input is the correct
 * graphic type, otherwise returns NULL.
 */
ZINC_API cmzn_graphic_contours_id cmzn_graphic_cast_contours(cmzn_graphic_id graphic);

/**
 * Cast contours graphic back to its base graphic and return the graphic.
 * IMPORTANT NOTE: Returned graphic does not have incremented reference count and
 * must not be destroyed. Use cmzn_graphic_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the contours graphic argument.
 *
 * @param contours_graphic  Handle to the contours graphic to cast.
 * @return  Non-accessed handle to the base graphic or NULL if failed.
 */
ZINC_C_INLINE cmzn_graphic_id cmzn_graphic_contours_base_cast(cmzn_graphic_contours_id contours_graphic)
{
	return (cmzn_graphic_id)(contours_graphic);
}

/**
 * Destroys this reference to the contours graphic (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param contours_address  Address of handle to the contours graphic.
 * @return  Status CMZN_OK if successfully destroyed the contours graphic handle,
 * any other value on failure.
 */
ZINC_API int cmzn_graphic_contours_destroy(cmzn_graphic_contours_id *contours_address);

/**
 * Gets the isoscalar field for the contours graphic.
 *
 * @param contours_graphic  The contours graphic to query.
 * @return  Handle to isoscalar field, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_contours_get_isoscalar_field(
	cmzn_graphic_contours_id contours_graphic);

/**
 * Set the isoscalar field for the contours graphic.
 *
 * @param contours_graphic  The Contours graphic to modify.
 * @param isoscalar_field  The isoscalar field to set, this field must have
 * only one component.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_contours_set_isoscalar_field(
	cmzn_graphic_contours_id contours_graphic,
	cmzn_field_id isoscalar_field);

/**
 * Get the iso values for the contours graphic when it has been set as an
 * explicit list.
 * @see cmzn_graphic_contours_set_list_isovalues.
 *
 * @param contours_graphic  The contours graphic to query.
 * @param number_of_isovalues  The size of the isovalues array.
 * @param isovalues  Array to receive number_of_isovalues iso values.
 * @return  The actual number of iso values that have been explicitly set using
 * cmzn_graphic_contours_set_list_isovalues. This can be more than the number
 * requested, so a second call may be needed with a larger array. A zero return
 * value can indicate isovalues were set as a range, not as an explicit list.
 */
ZINC_API int cmzn_graphic_contours_get_list_isovalues(
	cmzn_graphic_contours_id contours_graphic, int number_of_isovalues,
	double *isovalues);

/**
 * Set the iso values for the contours graphic as an explicit list.
 *
 * @param contours_graphic  The contours graphic to modify.
 * @param number_of_isovalues  The number of values in the isovalues array.
 * @param isovalues  The array of number_of_isovalues double values.
 * @return  Status CMZN_OK on success, otherwise any error code.
 */
ZINC_API int cmzn_graphic_contours_set_list_isovalues(
	cmzn_graphic_contours_id contours_graphic, int number_of_isovalues,
	const double *isovalues);

/**
 * Get the first isovalue for the contours graphic when set as a range.
 * @see cmzn_graphic_contours_set_range_isovalues
 *
 * @param contours_graphic  The contours graphic to query.
 * @return  First iso values in the range, or 0.0 if not set as a range or
 * other error.
 */
ZINC_API double cmzn_graphic_contours_get_range_first_isovalue(
	cmzn_graphic_contours_id contours_graphic);

/**
 * Get the last isovalue for the contours graphic when set as a range.
 * @see cmzn_graphic_contours_set_range_isovalues
 *
 * @param contours_graphic  The contours graphic to query.
 * @return  Last iso values in the range, or 0.0 if not set as a range or
 * other error.
 */
ZINC_API double cmzn_graphic_contours_get_range_last_isovalue(
	cmzn_graphic_contours_id contours_graphic);

/**
 * Get the number of isovalues for the contours graphic when set as a range.
 * @see cmzn_graphic_contours_set_range_isovalues
 *
 * @param contours_graphic  The contours graphic to query.
 * @return  Number of iso values in the range, or 0 if not set as a range or
 * other error.
 */
ZINC_API int cmzn_graphic_contours_get_range_number_of_isovalues(
	cmzn_graphic_contours_id contours_graphic);

/**
 * Set the isovalues for the contours graphic as a number in a range from
 * first isovalue to last isovalue, in uniform increments.
 * For example, 5 values from 0.0 to 1.0 gives: 0.0, 0.25, 0.5, 0.75 and 1.0.
 *
 * @param contours_graphic  The contours graphic to modify.
 * @param number_of_isovalues  The number of contours from the first to the
 * last isovalue.
 * @param first_isovalue  The first iso value.
 * @param last_isovalue  The last iso value.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_contours_set_range_isovalues(
	cmzn_graphic_contours_id contours_graphic, int number_of_isovalues,
	double first_isovalue, double last_isovalue);

/**
 * If the graphic is of type lines then this function returns
 * the lines specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the new lines graphic reference.
 *
 * @param graphic  The graphic to be cast.
 * @return  Lines graphic specific representation if the input is the correct
 * graphic type, otherwise returns NULL.
 */
ZINC_API cmzn_graphic_lines_id cmzn_graphic_cast_lines(cmzn_graphic_id graphic);

/**
 * Cast lines graphic back to its base graphic and return the graphic.
 * IMPORTANT NOTE: Returned graphic does not have incremented reference count and
 * must not be destroyed. Use cmzn_graphic_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the lines graphic argument.
 *
 * @param lines_graphic  Handle to the lines graphic to cast.
 * @return  Non-accessed handle to the base graphic or NULL if failed.
 */
ZINC_C_INLINE cmzn_graphic_id cmzn_graphic_lines_base_cast(cmzn_graphic_lines_id lines_graphic)
{
	return (cmzn_graphic_id)(lines_graphic);
}

/**
 * Destroys this reference to the lines graphic (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param lines_address  Address of handle to the lines graphic.
 * @return  Status CMZN_OK if successfully destroyed the lines graphic handle,
 * any other value on failure.
 */
ZINC_API int cmzn_graphic_lines_destroy(cmzn_graphic_lines_id *lines_address);

/**
 * If the graphic is of type points then this function returns
 * the points specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the new points graphic reference.
 *
 * @param graphic  The graphic to be cast.
 * @return  Points graphic specific representation if the input is the correct
 * graphic type, otherwise returns NULL.
 */
ZINC_API cmzn_graphic_points_id cmzn_graphic_cast_points(cmzn_graphic_id graphic);

/**
 * Cast points graphic back to its base graphic and return the graphic.
 * IMPORTANT NOTE: Returned graphic does not have incremented reference count and
 * must not be destroyed. Use cmzn_graphic_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the points graphic argument.
 *
 * @param points_graphic  Handle to the points graphic to cast.
 * @return  Non-accessed handle to the base graphic or NULL if failed.
 */
ZINC_C_INLINE cmzn_graphic_id cmzn_graphic_points_base_cast(cmzn_graphic_points_id points_graphic)
{
	return (cmzn_graphic_id)(points_graphic);
}

/**
 * Destroys this reference to the points graphic (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param points_address  Address of handle to the points graphic.
 * @return  Status CMZN_OK if successfully destroyed the points graphic handle,
 * any other value on failure.
 */
ZINC_API int cmzn_graphic_points_destroy(cmzn_graphic_points_id *points_address);

/**
 * If the graphic is of type streamlines then this function returns
 * the streamlines specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the new streamlines graphic reference.
 *
 * @param graphic  The graphic to be cast.
 * @return  Streamlines graphic specific representation if the input is the correct
 * graphic type, otherwise returns NULL.
 */
ZINC_API cmzn_graphic_streamlines_id cmzn_graphic_cast_streamlines(cmzn_graphic_id graphic);

/**
 * Cast streamlines graphic back to its base graphic and return the graphic.
 * IMPORTANT NOTE: Returned graphic does not have incremented reference count and
 * must not be destroyed. Use cmzn_graphic_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the streamlines graphic argument.
 *
 * @param streamlines_graphic  Handle to the streamlines graphic to cast.
 * @return  Non-accessed handle to the base graphic or NULL if failed.
 */
ZINC_C_INLINE cmzn_graphic_id cmzn_graphic_streamlines_base_cast(cmzn_graphic_streamlines_id streamlines_graphic)
{
	return (cmzn_graphic_id)(streamlines_graphic);
}

/**
 * Destroys this reference to the streamlines graphic (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param streamlines_address  Address of handle to the streamlines graphic.
 * @return  Status CMZN_OK if successfully destroyed the streamlines graphic handle,
 * any other value on failure.
 */
ZINC_API int cmzn_graphic_streamlines_destroy(cmzn_graphic_streamlines_id *streamlines_address);

/**
 * Gets the vector field the streamline is tracking along.
 *
 * @param streamlines_graphic  The streamlines graphic to query.
 * @return  Handle to stream vector field, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_streamlines_get_stream_vector_field(
	cmzn_graphic_streamlines_id streamlines_graphic);

/**
 * Sets the vector field to track the streamline along.
 * For a 3-D domain with a 3-D coordinate field, can have 3, 6 or 9 components;
 * extra components set the lateral axes for extruded profiles.
 * For a 2-D domain the stream vector may have 2 components.
 *
 * @param streamlines_graphic  The streamlines graphic to modify.
 * @param stream_vector_field  The field to track the streamline along.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_streamlines_set_stream_vector_field(
	cmzn_graphic_streamlines_id streamlines_graphic,
	cmzn_field_id stream_vector_field);

/**
 * Gets the direction in which streamlines are tracked.
 *
 * @param streamlines_graphic  The streamlines graphic to query.
 * @return  The current tracking direction, or
 * CMZN_GRAPHIC_STREAMLINES_TRACK_DIRECTION_INVALID on error.
 */
ZINC_API enum cmzn_graphic_streamlines_track_direction
	cmzn_graphic_streamlines_get_track_direction(
		cmzn_graphic_streamlines_id streamlines_graphic);

/**
 * Sets the direction in which streamlines are tracked relative to the stream
 * vector field.
 * @see cmzn_graphic_streamlines_track_direction
 *
 * @param streamlines_graphic  The streamlines graphic to modify.
 * @param track_direction  The new tracking direction.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_streamlines_set_track_direction(
	cmzn_graphic_streamlines_id streamlines_graphic,
	enum cmzn_graphic_streamlines_track_direction track_direction);

/**
 * Gets the maximum length streamlines are tracked along.
 *
 * @param streamlines_graphic  The streamlines graphic to query.
 * @return  The track length, or 0.0 if invalid streamlines graphic.
 */
ZINC_API double cmzn_graphic_streamlines_get_track_length(
	cmzn_graphic_streamlines_id streamlines_graphic);

/**
 * Sets the maximum length to track streamlines along.
 * Default value is 1.0; generally need to adjust this for scale of model
 * as streamlines can be slow to evaluate.
 *
 * @param streamlines_graphic  The streamlines graphic to modify.
 * @param length  The maximum length to track streamlines along >= 0.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_streamlines_set_track_length(
	cmzn_graphic_streamlines_id streamlines_graphic, double length);

/**
 * If the graphic is of type surfaces then this function returns
 * the surfaces specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the new surfaces graphic reference.
 *
 * @param graphic  The graphic to be cast.
 * @return  Surfaces graphic specific representation if the input is the correct
 * graphic type, otherwise returns NULL.
 */
ZINC_API cmzn_graphic_surfaces_id cmzn_graphic_cast_surfaces(cmzn_graphic_id graphic);

/**
 * Cast surfaces graphic back to its base graphic and return the graphic.
 * IMPORTANT NOTE: Returned graphic does not have incremented reference count and
 * must not be destroyed. Use cmzn_graphic_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the surfaces graphic argument.
 *
 * @param surfaces_graphic  Handle to the surfaces graphic to cast.
 * @return  Non-accessed handle to the base graphic or NULL if failed.
 */
ZINC_C_INLINE cmzn_graphic_id cmzn_graphic_surfaces_base_cast(cmzn_graphic_surfaces_id surfaces_graphic)
{
	return (cmzn_graphic_id)(surfaces_graphic);
}

/**
 * Destroys this reference to the surfaces graphic (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param surfaces_address  Address of handle to the surfaces graphic.
 * @return  Status CMZN_OK if successfully destroyed the surfaces graphic handle,
 * any other value on failure.
 */
ZINC_API int cmzn_graphic_surfaces_destroy(cmzn_graphic_surfaces_id *surfaces_address);

/**
 * If the graphic produces lines or extrusions then returns a handle to the
 * line attribute object for specifying section profile and scaling.
 *
 * @param graphic  The graphic to request line attributes from.
 * @return  Handle to line attributes object, or 0 if not supported for
 * graphic type or error. Up to caller to destroy returned handle.
 */
ZINC_API cmzn_graphic_line_attributes_id cmzn_graphic_get_line_attributes(
	cmzn_graphic_id graphic);

/**
 * Returns a new reference to the line attributes with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param line_attributes  The line_attributes to obtain a new reference to.
 * @return  New line attributes reference with incremented reference count.
 */
ZINC_API cmzn_graphic_line_attributes_id cmzn_graphic_line_attributes_access(
	cmzn_graphic_line_attributes_id line_attributes);

/**
 * Destroys this reference to the line attributes, and sets it to 0.
 * Internally this just decrements the reference count.
 *
 * @param line_attributes_address  Address of handle to the line attributes.
 * @return  Status CMZN_OK if successfully destroyed the handle, any other
 * value on failure.
 */
ZINC_API int cmzn_graphic_line_attributes_destroy(
	cmzn_graphic_line_attributes_id *line_attributes_address);

/**
 * Gets the base size of the extrusion section, one value for each lateral axis.
 * @see cmzn_graphic_line_attributes_set_base_size.
 *
 * @param line_attributes  The line_attributes to query.
 * @param number  The number of base size values to request, starting with the
 * first lateral axis. If fewer values have been set it is padded with the last
 * base size value. 1 to 2 values can be obtained.
 * @param base_size  Array to receive base sizes. Must be big enough to contain
 * the specified number of values.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_line_attributes_get_base_size(
	cmzn_graphic_line_attributes_id line_attributes, int number,
	double *base_size);

/**
 * Sets the base size of the extrusion section, one value for each lateral axis.
 * 1 to 2 values can be set.
 * For a unit section profile, the final size in each lateral direction is:
 * base_size + scale_factor * field_scalar
 * where field_scalar is determined from the orientation_scale_field.
 * @see cmzn_graphic_line_attributes_set_orientation_scale_field.
 * The default base size is zero.
 * Note: for lines (circle_extrusion shape) only one value is currently used;
 * the second value is constrained to equal the first value.
 *
 * @param line_attributes  The line_attributes to modify.
 * @param number  The number of base size values to set, starting with the
 * first lateral axis. If fewer values are set than the number of axes,
 * the last value is assumed for subsequent axes. Hence a single value can
 * be used to set a diameter for a unit circle profile.
 * @param base_size  Array of base sizes with the number of values specified.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_line_attributes_set_base_size(
	cmzn_graphic_line_attributes_id line_attributes, int number,
	const double *base_size);

/**
 * Gets the orientation scale field from the graphic line attributes. This
 * controls lateral scaling and orientation of line extrusions.
 * @see cmzn_graphic_line_attributes_set_orientation_scale_field
 *
 * @param line_attributes  The line attributes to query.
 * @return Handle to orientation scale field, or 0 if none or error. Up to
 * caller to destroy the returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_line_attributes_get_orientation_scale_field(
	cmzn_graphic_line_attributes_id line_attributes);

/**
 * Sets the orientation scale field in the graphic line attributes. This
 * controls lateral scaling and orientation of line extrusions.
 * Note: Currently it is only used to supply a scalar field for scaling
 * cylinders and with default scale factor of 1 it gives its diameter. Soon
 * we will offer more options for controlling extrusions.
 *
 * @param line_attributes  The line attributes to modify.
 * @param orientation_scale_field  The orientation scale field to set.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_line_attributes_set_orientation_scale_field(
	cmzn_graphic_line_attributes_id line_attributes,
	cmzn_field_id orientation_scale_field);

/**
 * Gets the scale factors used in sizing the extrusion section, one value for
 * each lateral axis.
 * @see cmzn_graphic_line_attributes_set_scale_factors.
 *
 * @param line_attributes  The line_attributes to query.
 * @param number  The number of scale_factors to request, starting with the
 * first lateral axis. If fewer values have been set it is padded with the last
 * scale factor value. 1 to 2 values can be obtained.
 * @param scale_factors  Array to receive scale factors. Must be big enough to
 * contain the specified number of values.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_line_attributes_get_scale_factors(
	cmzn_graphic_line_attributes_id line_attributes, int number,
	double *scale_factors);

/**
 * Sets the scale factors used in sizing the extrusion section, one value for
 * each lateral axis. 1 to 2 values can be set.
 * For a unit section profile, the final size in each lateral direction is:
 * base_size + scale_factor * field_scalar
 * where field_scalar is determined from the orientation_scale_field.
 * @see cmzn_graphic_line_attributes_set_orientation_scale_field.
 * Scale factor values default to 1.
 * Note: for lines (circle_extrusion shape) only one value is currently used;
 * the second value is constrained to equal the first value.
 *
 * @param line_attributes  The line_attributes to modify.
 * @param number  The number of scale factor values to set, starting with the
 * first lateral axis. If fewer values are set than the number of axes,
 * the last value is assumed for subsequent axes. Hence a single value applies
 * to all axes.
 * @param scale_factors  Array of scale factors with the number of values
 * specified.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_line_attributes_set_scale_factors(
	cmzn_graphic_line_attributes_id line_attributes, int number,
	const double *scale_factors);

/**
 * Gets the shape or profile of graphics generated for lines.
 * @see cmzn_graphic_point_attributes_set_glyph_repeat_mode
 *
 * @param line_attributes  The line_attributes to query.
 * @return  The current shape.
 */
ZINC_API enum cmzn_graphic_line_attributes_shape
	cmzn_graphic_line_attributes_get_shape(
		cmzn_graphic_line_attributes_id line_attributes);

/**
 * Sets the shape or profile of graphics generated for lines:
 * LINE, RIBBON, CIRCLE_EXTRUSION or SQUARE_EXTRUSION.
 * Note: only LINE and CIRCLE_EXTRUSION are supported for graphic_lines type;
 * all shapes are supported for graphic_streamlines.
 * @see cmzn_graphic_line_attributes_shape
 *
 * @param line_attributes  The line_attributes to modify.
 * @param shape  The new shape.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_line_attributes_set_shape(
	cmzn_graphic_line_attributes_id line_attributes,
	enum cmzn_graphic_line_attributes_shape shape);

/**
 * If the graphic produces points then returns a handle to point attribute
 * object for specifying glyph, scaling fields, scale factors and labels.
 *
 * @param graphic  The graphic to request point attributes from.
 * @return  Handle to point attributes object, or 0 if not supported for
 * graphic type or error. Up to caller to destroy returned handle.
 */
ZINC_API cmzn_graphic_point_attributes_id cmzn_graphic_get_point_attributes(
	cmzn_graphic_id graphic);

/**
 * Returns a new reference to the point attributes with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param point_attributes  The point_attributes to obtain a new reference to.
 * @return  New point attributes reference with incremented reference count.
 */
ZINC_API cmzn_graphic_point_attributes_id cmzn_graphic_point_attributes_access(
	cmzn_graphic_point_attributes_id point_attributes);

/**
 * Destroys this reference to the point attributes, and sets it to 0.
 * Internally this just decrements the reference count.
 *
 * @param point_attributes_address  Address of handle to the point attributes.
 * @return  Status CMZN_OK if successfully destroyed the handle, any other
 * value on failure.
 */
ZINC_API int cmzn_graphic_point_attributes_destroy(
	cmzn_graphic_point_attributes_id *point_attributes_address);

/**
 * Gets the base size of the point glyph, up to 3 values, one for each axis.
 * @see cmzn_graphic_point_attributes_set_base_size.
 *
 * @param point_attributes  The point_attributes to query.
 * @param number  The number of base size values to request, starting with the
 * first axis. If fewer values have been set it is padded with the last
 * base size value. 1 to 3 values can be obtained.
 * @param base_size  Array to receive base sizes. Must be big enough to contain
 * the specified number of values.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_get_base_size(
	cmzn_graphic_point_attributes_id point_attributes, int number,
	double *base_size);

/**
 * Sets the base size of the point glyph, up to 3 values, one for each axis.
 * For a unit sized glyph, the final size in each direction is:
 * base_size + scale_factor * field_scalar
 * where field_scalar is determined from the orientation_scale_field.
 * @see cmzn_graphic_point_attributes_set_orientation_scale_field.
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
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_base_size(
	cmzn_graphic_point_attributes_id point_attributes, int number,
	const double *base_size);

/**
 * Gets the font in the graphic point attributes used to draw the label field.
 *
 * @param point_attributes  The point attributes to query.
 * @return  Handle to font, or 0 if none or error. Up to caller to destroy
 * returned handle.
 */
ZINC_API cmzn_font_id cmzn_graphic_point_attributes_get_font(
	cmzn_graphic_point_attributes_id point_attributes);

/**
 * Sets the font in the graphic point attributes used to draw the label field.
 *
 * @param point_attributes  The point attributes to modify.
 * @param font  The font to set.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_font(
	cmzn_graphic_point_attributes_id point_attributes,
	cmzn_font_id font);

/**
 * Gets the glyph in the graphic point attributes used to visualise points.
 *
 * @param point_attributes  The point attributes to query.
 * @return  Handle to glyph, or 0 if none or error. Up to caller to destroy
 * returned handle.
 */
ZINC_API cmzn_glyph_id cmzn_graphic_point_attributes_get_glyph(
	cmzn_graphic_point_attributes_id point_attributes);

/**
 * Sets the glyph in the graphic point attributes used to visualise points.
 *
 * @param point_attributes  The point attributes to modify.
 * @param glyph  The glyph to set.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_glyph(
	cmzn_graphic_point_attributes_id point_attributes, cmzn_glyph_id glyph);

/**
 * Gets the offset from the point coordinates to where the glyph origin is
 * drawn, in glyph units along the glyph local axes.
 * @see cmzn_graphic_point_attributes_set_glyph_offset.
 *
 * @param point_attributes  The point_attributes to query.
 * @param number  The number of offset values to request, up to 3.
 * @param offset  Array to receive offset values, starting with the first axis.
 * Must be big enough to contain the specified number of values.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_get_glyph_offset(
	cmzn_graphic_point_attributes_id point_attributes, int number,
	double *offset);

/**
 * Sets the offset from the point coordinates to where the glyph origin is
 * drawn, in glyph units along the glyph local axes.
 * @see cmzn_graphic_point_attributes_set_orientation_scale_field.
 * Tip: offset values can give the effect of moving the centre point / origin
 * of the glyph: just pass in negative coordinates for the new origin.
 *
 * @param point_attributes  The point_attributes to modify.
 * @param number  The number of offset values to set, up to 3.
 * @param offset  Array of offset values, starting with the first axis. If
 * fewer than 3 then zero is assumed for all other offset values.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_glyph_offset(
	cmzn_graphic_point_attributes_id point_attributes, int number,
	const double *offset);

/**
 * Return the current glyph repeat mode.
 * @see cmzn_graphic_point_attributes_set_glyph_repeat_mode
 *
 * @param point_attributes  The point attributes to query.
 * @return  The current glyph repeat mode.
 */
ZINC_API enum cmzn_glyph_repeat_mode
	cmzn_graphic_point_attributes_get_glyph_repeat_mode(
		cmzn_graphic_point_attributes_id point_attributes);

/**
 * Sets the glyph repeat mode which controls whether multiple glyphs are drawn
 * at each point and how.
 * @see cmzn_glyph_repeat_mode
 *
 * @param point_attributes  The point attributes to modify.
 * @param glyph_repeat_mode  The new repeat mode.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_glyph_repeat_mode(
	cmzn_graphic_point_attributes_id point_attributes,
	enum cmzn_glyph_repeat_mode glyph_repeat_mode);

/**
 * Gets the enumerated value identifying the current glyph used in the point
 * attributes.
 *
 * @param point_attributes  The point attributes to query.
 * @return  The glyph type identifier, or CMZN_GLYPH_TYPE_INVALID if
 * invalid arguments or glyph does not have a valid type.
 */
ZINC_API enum cmzn_glyph_type cmzn_graphic_point_attributes_get_glyph_type(
	cmzn_graphic_point_attributes_id point_attributes);

/**
 * Sets the glyph used for visualising a graphics point from a set of
 * enumerations. Requires standard glyphs to first be created.
 * @see cmzn_glyph_module_define_standard_glyphs
 * If no glyph exists with the type identifier, the glyph is unchanged.
 *
 * @param point_attributes  The point attributes to modify.
 * @param glyph_type  The glyph type identifier.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_glyph_type(
	cmzn_graphic_point_attributes_id point_attributes,
	enum cmzn_glyph_type glyph_type);

/**
 * Gets the label field from the graphic point attributes.
 *
 * @param point_attributes  The point attributes to query.
 * @return Handle to label field, or 0 if none or error. Up to caller to destroy
 * returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_point_attributes_get_label_field(
	cmzn_graphic_point_attributes_id point_attributes);

/**
 * Sets the label field in the graphic point attributes. A string representation
 * of this field's value (if defined) is drawn with the current font at the
 * glyph offset.
 *
 * @param point_attributes  The point attributes to modify.
 * @param label_field  The label field to set.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_label_field(
	cmzn_graphic_point_attributes_id point_attributes, cmzn_field_id label_field);

/**
 * Gets the label offset relative to the glyph axes from its origin.
 * @see cmzn_graphic_point_attributes_set_label_offset
 *
 * @param point_attributes  The point_attributes to query.
 * @param number  The number of offset values to request, up to 3.
 * @param label_offset  Array to receive offset values, starting with the first
 * axis. Must be big enough to contain the specified number of values.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_get_label_offset(
	cmzn_graphic_point_attributes_id point_attributes, int number,
	double *label_offset);

/**
 * Sets the label offset relative to the glyph axes from its origin set by the
 * glyph offset. A unit offset moves the label the length of the axis vector.
 * @see cmzn_graphic_point_attributes_set_orientation_scale_field.
 *
 * @param point_attributes  The point_attributes to modify.
 * @param number  The number of offset values to set, up to 3.
 * @param label_offset  Array of offset values, starting with the first axis. If
 * fewer than 3 then zero is assumed for all other offset values.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_label_offset(
	cmzn_graphic_point_attributes_id point_attributes, int number,
	const double *label_offset);

/**
 * Get static label text to be shown near each glyph.
 * @see cmzn_graphic_point_attributes_set_label_text
 *
 * @param point_attributes  The point_attributes to query.
 * @param label_number  The label_number to get, from 1 to 3.
 * @return  Allocated string containing label text, or NULL if none or error.
 * Up to caller to free using cmzn_deallocate().
 */
ZINC_API char *cmzn_graphic_point_attributes_get_label_text(
	cmzn_graphic_point_attributes_id point_attributes, int label_number);

/**
 * Set static label text to be shown near each glyph. The number of labels
 * depends on the glyph repeat mode, one label per glyph for REPEAT_AXES,
 * one label for REPEAT_NONE and REPEAT_MIRROR. Note the label field value is
 * written for the first label only, prefixed by the first static label text.
 * @see cmzn_graphic_point_attributes_set_glyph_repeat_mode
 *
 * @param point_attributes  The point_attributes to modify.
 * @param label_number  The label_number to set, from 1 to 3.
 * @param label_text  The string to set as static label text.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_label_text(
	cmzn_graphic_point_attributes_id point_attributes, int label_number,
	const char *label_text);

/**
 * Gets the orientation scale field from the graphic point attributes. This
 * controls scaling and orientation of point glyphs.
 * @see cmzn_graphic_point_attributes_set_orientation_scale_field
 *
 * @param point_attributes  The point attributes to query.
 * @return Handle to orientation scale field, or 0 if none or error. Up to
 * caller to destroy the returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_point_attributes_get_orientation_scale_field(
	cmzn_graphic_point_attributes_id point_attributes);

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
 * @see cmzn_graphic_point_attributes_set_signed_scale_field
 *
 * @param point_attributes  The point attributes to modify.
 * @param orientation_scale_field  The orientation scale field to set.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_orientation_scale_field(
	cmzn_graphic_point_attributes_id point_attributes,
	cmzn_field_id orientation_scale_field);

/**
 * Sets the scale factors used in sizing the point glyph, up to 3 values,
 * one for each axis.
 * @see cmzn_graphic_point_attributes_set_scale_factors.
 *
 * @param point_attributes  The point_attributes to query.
 * @param number  The number of scale_factors to request, starting with the
 * first axis. If fewer values have been set it is padded with the last
 * scale factor value. 1 to 3 values can be obtained.
 * @param scale_factors  Array to receive scale factors. Must be big enough to
 * contain the specified number of values.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_get_scale_factors(
	cmzn_graphic_point_attributes_id point_attributes, int number,
	double *scale_factors);

/**
 * Sets the scale factors used in sizing the point glyph, up to 3 values,
 * one for each axis.
 * For a unit sized glyph, the final size in each direction is:
 * base_size + scale_factor * field_scalar
 * where field_scalar is determined from the orientation_scale_field.
 * @see cmzn_graphic_point_attributes_set_orientation_scale_field.
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
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_scale_factors(
	cmzn_graphic_point_attributes_id point_attributes, int number,
	const double *scale_factors);

/**
 * Gets the signed scale field from the graphic point attributes.
 * @see cmzn_graphic_point_attributes_set_signed_scale_field
 *
 * @param point_attributes  The point attributes to query.
 * @return Handle to signed scale field, or 0 if none or error. Up to
 * caller to destroy the returned handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_point_attributes_get_signed_scale_field(
	cmzn_graphic_point_attributes_id point_attributes);

/**
 * Sets the signed scale field in the graphic point attributes. Can have from 1
 * to 3 components which multiply the scaling from the orientation scale field
 * on the corresponding axis.
 * Note special behaviour applies when used with the GLYPH_REPEAT_MIRROR mode:
 * If value is negative on the first axis, the origin of the glyph is moved to
 * the end of the first axis and its direction is reversed. This is commonly
 * used to draw stress and strain with mirrored arrow-like glyphs pointing
 * inward for compression, and outward for tension. Do this by passing a single
 * eigenvalue (of stress or strain tensor) as the signed_scale and the
 * corresponding eigenvector as the orientation_scale field, with repeat mirror.
 * Use a separate points graphic for each eigenvalue.
 * @see cmzn_graphic_point_attributes_set_glyph_repeat_mode
 *
 * @param point_attributes  The point attributes to modify.
 * @param signed_scale_field  The signed scale field to set.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_point_attributes_set_signed_scale_field(
	cmzn_graphic_point_attributes_id point_attributes,
	cmzn_field_id signed_scale_field);

/**
 * If the graphic samples points from elements then returns a handle to sampling
 * attribute object for specifying sample mode, density field etc.
 *
 * @param graphic  The graphic to request sampling attributes from.
 * @return  Handle to sampling attributes object, or 0 if not supported for
 * graphic type or error. Up to caller to destroy returned handle.
 */
ZINC_API cmzn_graphic_sampling_attributes_id cmzn_graphic_get_sampling_attributes(
	cmzn_graphic_id graphic);

/**
 * Returns a new reference to the sampling attributes with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param sampling_attributes  The graphic sampling attributes to obtain a new
 * reference to.
 * @return  New sampling attributes reference with incremented reference count.
 */
ZINC_API cmzn_graphic_sampling_attributes_id cmzn_graphic_sampling_attributes_access(
	cmzn_graphic_sampling_attributes_id sampling_attributes);

/**
 * Destroys this reference to the sampling attributes, and sets it to 0.
 * Internally this just decrements the reference count.
 *
 * @param sampling_attributes_address  Address of handle to the graphic sampling
 * attributes.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_sampling_attributes_destroy(
	cmzn_graphic_sampling_attributes_id *sampling_attributes_address);

/**
 * Gets the scalar field specifying the density of points sampled from elements
 * when used with CELL_POISSON sample mode.
 *
 * @param sampling_attributes  The graphic sampling attributes to query.
 * @return  Handle to sample density field, or 0 if none. Up to caller to
 * destroy handle.
 */
ZINC_API cmzn_field_id cmzn_graphic_sampling_attributes_get_density_field(
	cmzn_graphic_sampling_attributes_id sampling_attributes);

/**
 * Sets the scalar field specifying the density of points sampled from elements
 * when used with CELL_POISSON sample mode. The density is applied per unit
 * volume/area/length, depending on dimension, evaluated at cell centres.
 * @see cmzn_graphic_sampling_attributes_set_mode
 *
 * @param sampling_attributes  The graphic sampling attributes to modify.
 * @param sample_density_field  Scalar density field to set.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_sampling_attributes_set_density_field(
	cmzn_graphic_sampling_attributes_id sampling_attributes,
	cmzn_field_id sample_density_field);

/**
 * Gets the location in the element chart where a point is sampled in mode
 * CMZN_ELEMENT_POINT_SAMPLE_SET_LOCATION. Up to 3 values can be returned.
 *
 * @param sampling_attributes  The graphic sampling attributes to query.
 * @param valuesCount  The size of the valuesOut array.
 * @param valuesOut  Array to receive location.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_sampling_attributes_get_location(
	cmzn_graphic_sampling_attributes_id sampling_attributes,
	int valuesCount, double *valuesOut);

/**
 * Sets the location in the element chart where a point is sampled in mode
 * CMZN_ELEMENT_POINT_SAMPLE_SET_LOCATION.
 * Up to 3 values can be set, with 0 assumed for additional chart coordinates.
 * @see cmzn_graphic_sampling_attributes_set_mode
 *
 * @param sampling_attributes  The graphic sampling attributes to modify.
 * @param valuesCount  The size of the valuesIn array.
 * @param valuesIn  Array containing element location to set.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_sampling_attributes_set_location(
	cmzn_graphic_sampling_attributes_id sampling_attributes,
	int valuesCount, const double *valuesIn);

/**
 * Get the mode for sampling points in elements.
 *
 * @param sampling_attributes  The graphic sampling attributes to query.
 * @return  The point sampling mode, or MODE_INVALID on error.
 */
ZINC_API enum cmzn_element_point_sample_mode cmzn_graphic_sampling_attributes_get_mode(
	cmzn_graphic_sampling_attributes_id sampling_attributes);

/**
 * Set the mode for sampling points in elements.
 * The default point sample mode is CELL_CENTRES.
 *
 * @param sampling_attributes  The graphic sampling attributes to modify.
 * @param sample_mode  The element point sample mode to set.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_graphic_sampling_attributes_set_mode(
	cmzn_graphic_sampling_attributes_id sampling_attributes,
	enum cmzn_element_point_sample_mode sample_mode);

#ifdef __cplusplus
}
#endif

#endif
