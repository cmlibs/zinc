/**
 * glyph.h
 *
 * Public interface to Cmiss_glyph static graphics objects used to visualise
 * points in the scene.
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
 * Portions created by the Initial Developer are Copyright (C) 2013
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

#ifndef ZINC_GLYPH_H
#define ZINC_GLYPH_H

#include "types/glyphid.h"
#include "types/graphicsmaterialid.h"
#include "types/spectrumid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new reference to the glyph module with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param glyph_module  The glyph module to obtain a new reference to.
 * @return  Glyph module with incremented reference count.
 */
ZINC_API Cmiss_glyph_module_id Cmiss_glyph_module_access(
	Cmiss_glyph_module_id glyph_module);

/**
 * Destroys this reference to the glyph module (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param glyph_module_address  Address of handle to glyph module to destroy.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_module_destroy(
	Cmiss_glyph_module_id *glyph_module_address);

/**
 * Begin caching or increment cache level for this glyph module. Call this
 * function before making multiple changes to minimise number of change messages
 * sent to clients. Must remember to end_change after completing changes.
 * @see Cmiss_glyph_module_end_change
 *
 * @param glyph_module  The glyph_module to begin change cache on.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_module_begin_change(Cmiss_glyph_module_id glyph_module);

/***************************************************************************//**
 * Decrement cache level or end caching of changes for the glyph module.
 * Call Cmiss_glyph_module_begin_change before making multiple changes
 * and call this afterwards. When change level is restored to zero,
 * cached change messages are sent out to clients.
 *
 * @param glyph_module  The glyph_module to end change cache on.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_glyph_module_end_change(Cmiss_glyph_module_id glyph_module);

/**
 * Defines a selection of standard glyphs for visualising points, vectors etc.
 * Graphics for all standard glyphs fit in a unit cube which:
 * 1. for orientable glyphs e.g. line, arrow, cylinder: span [0,1] on axis 1,
 *    and [-0.5,0.5] on axes 2 and 3 (except line which has no width).
 * 2. are otherwise centred at 0,0,0 for all other glyphs.
 * These consist of:
 * "arrow", "arrow_solid" = line and solid arrows from (0,0,0) to (1,0,0) with
 *     arrowhead 1/3 of length and unit width.
 * "axis", "axis_solid" = variants of arrows with arrowhead 1/10 of length.
 * "cone", "cone_solid" = cone descending from unit diameter circle in 2-3 plane
 *      at (0,0,0) to a point at (1,0,0). Solid variant has base closed.
 * "cross" = lines from -0.5 to +0.5 on each axis through (0,0,0).
 * "cube_solid", "cube_wireframe" = solid and wireframe (line) unit cubes
 *     aligned with primary axes and centred at (0,0,0).
 * "cylinder", "cylinder_solid" = a unit diameter cylinder, centre line from
 *     (0,0,0) to (1,0,0). Solid variant has ends closed.
 * "line" = a line from (0,0,0) to (1,0,0).
 * "point" = a single pixel at (0,0,0).
 * "sheet" = a unit square surface in 1-2 plane, centred at (0,0,0).
 * "sphere" = a unit diameter sphere centred at (0,0,0).
 * Note if any glyphs of the predefined name already exist prior to calling this
 * function, the standard glyph is not created.
 * All glyphs created by this function have IS_MANAGED set to 1.
 * If not already set, the default glyph is set to "point" by this function.
 *
 * @param glyph_module  The glyph module to create the glyph in.
 * @return  CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_module_define_standard_glyphs(
	Cmiss_glyph_module_id glyph_module);

/**
 * Find the glyph with the specified name, if any.
 *
 * @param glyph_module  Glyph module to search.
 * @param name  The name of the glyph.
 * @return  Handle to the glyph of that name, or 0 if not found.
 * Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_glyph_id Cmiss_glyph_module_find_glyph_by_name(
	Cmiss_glyph_module_id glyph_module, const char *name);

/**
 * Get the default glyph used for new point graphics, if any.
 *
 * @param glyph_module  Glyph module to query.
 * @return  Handle to the default point glyph, or 0 if none.
 * Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_glyph_id Cmiss_glyph_module_get_default_point_glyph(
	Cmiss_glyph_module_id glyph_module);

/**
 * Set the default glyph used for new point graphics.
 *
 * @param glyph_module  Glyph module to modify.
 * @param glyph  The glyph to set as default.
 * @return  CMISS_OK on success otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_module_set_default_point_glyph(
	Cmiss_glyph_module_id glyph_module, Cmiss_glyph_id glyph);

/**
 * Returns a new reference to the glyph with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param glyph  The glyph to obtain a new reference to.
 * @return  New glyph reference with incremented reference count.
 */
ZINC_API Cmiss_glyph_id Cmiss_glyph_access(Cmiss_glyph_id glyph);

/**
 * Destroys this reference to the glyph (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param glyph_address  The address to the handle of the glyph to be destroyed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_glyph_destroy(Cmiss_glyph_id *glyph_address);

/**
 * Get managed status of glyph in its owning glyph_module.
 * @see Cmiss_glyph_set_managed
 *
 * @param glyph  The glyph to query.
 * @return  true if glyph is managed, otherwise 0 false.
 */
ZINC_API bool Cmiss_glyph_is_managed(Cmiss_glyph_id glyph);

/**
 * Set managed status of glyph in its owning glyph module.
 * If set (managed) the glyph will remain indefinitely in the glyph module even
 * if no external references are held.
 * If not set (unmanaged) the glyph will be automatically removed from the
 * module when no longer referenced externally, effectively marking it as
 * pending destruction.
 * All new objects are unmanaged unless stated otherwise.
 *
 * @param glyph  The glyph to modify.
 * @param value  The new value for the managed flag: true or false.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_set_managed(Cmiss_glyph_id glyph, bool value);

/**
 * Return an allocated string containing glyph name.
 *
 * @param glyph  The glyph to query.
 * @return  Allocated string containing glyph name, or NULL on failure.
 * Up to caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_glyph_get_name(Cmiss_glyph_id glyph);

/**
 * Set name of the glyph.
 *
 * @param glyph  The glyph to modify.
 * @param name  Name to be set for the glyph.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_glyph_set_name(Cmiss_glyph_id glyph, const char *name);

/**
 * Create a glyph which draws a colour bar for the spectrum with ticks and
 * value labels. The glyph dynamically updates to match the current range and
 * definition of the spectrum. Note it only shows a single component.
 *
 * @param glyph_module  The glyph_module to create the glyph in.
 * @return  Handle to new glyph or 0 on error. Up to caller to destroy.
 */
ZINC_API Cmiss_glyph_colour_bar_id Cmiss_glyph_module_create_colour_bar(
	Cmiss_glyph_module_id glyph_module, Cmiss_spectrum_id spectrum);

/**
 * If the glyph is type colour bar, returns the type-specific handle.
 *
 * @param glyph  The glyph to be cast.
 * @return  Colour bar glyph specific representation if the input is the correct
 * glyph type, otherwise returns NULL.
 */
ZINC_API Cmiss_glyph_colour_bar_id Cmiss_glyph_cast_colour_bar(Cmiss_glyph_id glyph);

/**
 * Cast colour bar glyph back to the base glyph type and return it.
 * IMPORTANT NOTE: Returned glyph does not have incremented reference count and
 * must not be destroyed. Use Cmiss_glyph_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the colour_bar glyph argument.
 *
 * @param colour_bar_glyph  Handle to the colour_bar glyph to cast.
 * @return  Non-accessed handle to the base glyph or NULL if failed.
 */
ZINC_C_INLINE Cmiss_glyph_id Cmiss_glyph_colour_bar_base_cast(Cmiss_glyph_colour_bar_id colour_bar)
{
	return (Cmiss_glyph_id)(colour_bar);
}

/**
 * Destroys this reference to the colour_bar glyph (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param colour_bar_address  Address of handle to the colour_bar glyph.
 * @return  Status CMISS_OK if successfully destroyed the colour_bar glyph handle,
 * any other value on failure.
 */
ZINC_API int Cmiss_glyph_colour_bar_destroy(Cmiss_glyph_colour_bar_id *colour_bar_address);

/**
 * Gets the vector defining the main axis of the colour bar.
 *
 * @param colour_bar  The colour bar glyph to query.
 * @param valuesCount  The size of valuesOut array. Gets maximum of 3 values.
 * @param valuesOut  Array to receive axis vector.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_get_axis(
	Cmiss_glyph_colour_bar_id colour_bar, int valuesCount, double *valuesOut);

/**
 * Sets the vector defining the main axis of the colour bar. The magnitude of
 * this vector gives the length of the bar without the extend length.
 * The default axis is (0,1,0) for vertical orientation in window coordinates.
 *
 * @param colour_bar  The colour bar glyph to modify.
 * @param valuesCount  The size of valuesIn array. Sets maximum of 3 values.
 * @param valuesIn  Array containing axis vector. If fewer than 3 values then
 * assumes zero for remaining components.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_set_axis(
	Cmiss_glyph_colour_bar_id colour_bar, int valuesCount, const double *valuesIn);

/**
 * Gets the centre position of the colour bar.
 *
 * @param colour_bar  The colour bar glyph to query.
 * @param valuesCount  The size of valuesOut array. Gets maximum of 3 values.
 * @param valuesOut  Array to receive centre position.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_get_centre(
	Cmiss_glyph_colour_bar_id colour_bar, int valuesCount, double *valuesOut);

/**
 * Sets the centre position of the colour bar.
 * The default centre is (0,0,0). It is recommended that this not be changed
 * and instead use the graphic point attributes offset.
 * @see Cmiss_graphic_point_attributes_set_offset
 *
 * @param colour_bar  The colour bar glyph to modify.
 * @param valuesCount  The size of valuesIn array. Sets maximum of 3 values.
 * @param valuesIn  Array containing centre position. If fewer than 3 values
 * then assumes zero for remaining components.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_set_centre(
	Cmiss_glyph_colour_bar_id colour_bar, int valuesCount, const double *valuesIn);

/**
 * Gets the extend length used at each end of the colour bar to show values
 * outside the spectrum range.
 *
 * @param colour_bar  The colour bar glyph to query.
 * @return  The extend length, or 0.0 if error.
 */
ZINC_API double Cmiss_glyph_colour_bar_get_extend_length(
	Cmiss_glyph_colour_bar_id colour_bar);

/**
 * Sets the extend length used at each end of the colour bar to show values
 * outside the spectrum range.
 * The default extend length is 0.05.
 *
 * @param colour_bar  The colour bar glyph to modify.
 * @param extendLength  The new extend length. Must be non-negative.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_set_extend_length(
	Cmiss_glyph_colour_bar_id colour_bar, double extendLength);

/**
 * Gets the number of divisions between labels.
 *
 * @param colour_bar  The colour bar glyph to query.
 * @return  The number of label divisions, or 0 if error.
 */
ZINC_API int Cmiss_glyph_colour_bar_get_label_divisions(
	Cmiss_glyph_colour_bar_id colour_bar);

/**
 * Sets the number of divisions between labels. This is one less than the
 * number of labels/ticks.
 * The default label divisions is 10.
 *
 * @param colour_bar  The colour bar glyph to modify.
 * @param labelDivisions  The new number of divisions, at least 1.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_set_label_divisions(
	Cmiss_glyph_colour_bar_id colour_bar, int labelDivisions);

/**
 * Gets the material used for colour bar labels and ticks. Can be NULL.
 *
 * @param colour_bar  The colour bar glyph to query.
 * @return  Handle to label material, or 0 if none or error.
 * Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_graphics_material_id Cmiss_glyph_colour_bar_get_label_material(
	Cmiss_glyph_colour_bar_id colour_bar);

/**
 * Sets the material used for colour bar labels and ticks. Can be NULL.
 * Default is none i.e. use the same material as for the colour bar
 * itself, which is supplied by the graphic.
 *
 * @param colour_bar  The colour bar glyph to modify.
 * @param material  The new label material; can be NULL to clear.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_set_label_material(
	Cmiss_glyph_colour_bar_id colour_bar, Cmiss_graphics_material_id material);

/**
 * Get the number format used to write value labels on the colour bar.
 *
 * @param colour_bar  The colour bar glyph to query.
 * @return  Allocated string containing number format, or NULL on failure.
 * Up to caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_glyph_colour_bar_get_number_format(
	Cmiss_glyph_colour_bar_id colour_bar);

/**
 * Set the number format used to write value labels on the colour bar.
 * This is a C printf format string with a single numerical format in the form:
 * %[+/-/0][length][.precision](e/E/f/F/g/G)
 * Other characters can be added before or after the number format.
 * Note a literal % is entered by repeating it i.e. %%.
 * The default format is "%+.4e".
 *
 * @param colour_bar  The colour bar glyph to modify.
 * @param numberFormat  The printf number format used for value labels.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_set_number_format(
	Cmiss_glyph_colour_bar_id colour_bar, const char *numberFormat);

/**
 * Gets the vector defining the side/tick axis of the colour bar.
 *
 * @param colour_bar  The colour bar glyph to query.
 * @param valuesCount  The size of valuesOut array. Gets maximum of 3 values.
 * @param valuesOut  Array to receive side axis vector.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_get_side_axis(
	Cmiss_glyph_colour_bar_id colour_bar, int valuesCount, double *valuesOut);

/**
 * Sets the vector defining the side/tick axis of the colour bar. The magnitude
 * of this vector gives the diameter of the bar.
 * The default side axis is (0.1,0,0) for vertical bar and horizontal ticks in
 * window coordinates.
 *
 * @param colour_bar  The colour bar glyph to modify.
 * @param valuesCount  The size of valuesIn array. Sets maximum of 3 values.
 * @param valuesIn  Array containing side axis vector. If fewer than 3 values
 * then assumes zero for remaining components.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_set_side_axis(
	Cmiss_glyph_colour_bar_id colour_bar, int valuesCount, const double *valuesIn);

/**
 * Gets the tick length.
 *
 * @param colour_bar  The colour bar glyph to query.
 * @return  The tick length, or 0.0 if error.
 */
ZINC_API double Cmiss_glyph_colour_bar_get_tick_length(
	Cmiss_glyph_colour_bar_id colour_bar);

/**
 * Sets the tick length measured from outside radius of the colour bar, in the
 * direction of the side axis.
 * The default tick length is 0.05.
 *
 * @param colour_bar  The colour bar glyph to modify.
 * @param tickLength  The new tick length. Must be non-negative.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_colour_bar_set_tick_length(
	Cmiss_glyph_colour_bar_id colour_bar, double tickLength);

#ifdef __cplusplus
}
#endif

#endif /* ZINC_GLYPH_H */
