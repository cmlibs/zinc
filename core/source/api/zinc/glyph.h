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
 * Creates a selection of standard glyphs for visualising points, vectors etc.
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
 * Note: for now all circles are approximated by 12 line segments.
 *
 * @param glyph_module  The glyph module to create the glyph in.
 * @return  CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_module_create_standard_glyphs(
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
 * @return  1 (true) if glyph is managed, otherwise 0 (false).
 */
ZINC_API int Cmiss_glyph_is_managed(Cmiss_glyph_id glyph);

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
 * @param value  The new value for the managed flag: 0 or 1.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_glyph_set_managed(Cmiss_glyph_id glyph, int value);

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

#ifdef __cplusplus
}
#endif

#endif /* ZINC_GLYPH_H */
