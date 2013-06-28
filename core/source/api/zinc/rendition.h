/*******************************************************************************
FILE : cmiss_rendition.h

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
 * Portions created by the Initial Developer are Copyright (C) 2010
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
#ifndef __CMISS_RENDITION_H__
#define __CMISS_RENDITION_H__

#include "types/fieldid.h"
#include "types/fieldgroupid.h"
#include "types/graphicid.h"
#include "types/graphicsfilterid.h"
#include "types/nodeid.h"
#include "types/regionid.h"
#include "types/renditionid.h"
#include "types/selectionid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Returns a new reference to the rendition with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param rendition  The rendition to obtain a new reference to.
 * @return  New rendition reference with incremented reference count.
 */
ZINC_API Cmiss_rendition_id Cmiss_rendition_access(Cmiss_rendition_id rendition);

/*******************************************************************************
 * Destroys this reference to the rendition (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param rendition Pointer to the handle to the rendition.
 * @return  status CMISS_OK if successfully remove rendition, any other value on
 * failure.
 */
ZINC_API int Cmiss_rendition_destroy(Cmiss_rendition_id * rendition);

/***************************************************************************//**
 * Use this function with Cmiss_rendition_end_change.
 *
 * Use this function before making multiple changes on the rendition, this
 * will stop rendition from executing any immediate changes made in
 * rendition. After multiple changes have been made, use
 * Cmiss_rendition_end_change to execute all changes made previously in rendition
 * at once.
 *
 * @param rendition  The handle to the rendition.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_rendition_begin_change(Cmiss_rendition_id rendition);

/**
 * Creates a cloud of points (nodes) in the supplied nodeset sampled at random
 * locations according to a Poisson distribution on the lines and surfaces that
 * are in the rendition tree (filtered by the optional filter), i.e. including
 * all its descendents. Points/nodes are created with the next available
 * identifier. The density of points is set by supplied arguments and may be
 * scaled by data values stored in each graphic.
 *
 * @param rendition  The root rendition containing graphics to convert.
 * @param filter  The filter determining which graphics from the rendition tree
 * are converted. If not supplied then all graphics are converted.
 * @param nodeset  The nodeset to add nodes to.
 * @param coordinate_field  The coordinate field to be defined and assigned on
 * the new nodes. Must be from the same region as the nodeset.
 * @param line_density  The expected number of points per unit length for lines.
 * @param line_density_scale_factor  If a line graphic has a data field the mean
 * value of its first component multiplied by this factor is added to the
 * expected value.
 * @param surface_density  The expected number of points per unit area of
 * surfaces.
 * @param surface_density_scale_factor  If a surface graphic has a data field
 * the mean value of its first component multiplied by this factor is added to
 * the expected value.
 * @return  Status CMISS_OK on success, otherwise some other error code
 * including CMISS_ERROR_ARGUMENT.
 */
ZINC_API int Cmiss_rendition_convert_to_point_cloud(Cmiss_rendition_id rendition,
	Cmiss_graphics_filter_id filter, Cmiss_nodeset_id nodeset,
	Cmiss_field_id coordinate_field,
	double line_density, double line_density_scale_factor,
	double surface_density, double surface_density_scale_factor);

/**
 * Create a graphic of the given type in the rendition.
 *
 * @param rendition  Handle to rendition the graphic is created in.
 * @param graphic_type  Enumerator for a specific graphic type.
 * @return  Handle to the new graphic on success, otherwise 0.
 */
ZINC_API Cmiss_graphic_id Cmiss_rendition_create_graphic(Cmiss_rendition_id rendition,
	enum Cmiss_graphic_type graphic_type);

/**
 * Create a contours graphic in the rendition. Contours create graphics showing
 * where the isoscalar field has fixed value(s): iso-surfaces for 3-D domains,
 * iso-lines for 2-D domains.
 *
 * @param rendition  Handle to rendition the graphic is created in.
 * @return  Handle to the new contours graphic on success, otherwise 0.
 */
ZINC_API Cmiss_graphic_contours_id Cmiss_rendition_create_graphic_contours(
	Cmiss_rendition_id rendition);

/**
 * Create a lines graphic in the rendition. Used to visualise 1-D elements and
 * lines/faces of elements.
 *
 * @param rendition  Handle to rendition the graphic is created in.
 * @return  Handle to the new lines graphic on success, otherwise 0.
 */
ZINC_API Cmiss_graphic_lines_id Cmiss_rendition_create_graphic_lines(
	Cmiss_rendition_id rendition);

/**
 * Create a points graphic in the rendition. Used to visualise static points,
 * nodes, data and sampled element points. Must set the domain type after
 * creation.
 * @see Cmiss_graphic_set_domain_type
 *
 * @param rendition  Handle to rendition the graphic is created in.
 * @return  Handle to the new points graphic on success, otherwise 0.
 */
ZINC_API Cmiss_graphic_points_id Cmiss_rendition_create_graphic_points(
	Cmiss_rendition_id rendition);

/**
 * Create a streamlines graphic in the rendition.
 *
 * @param rendition  Handle to rendition the graphic is created in.
 * @return  Handle to the new steamlines graphic on success, otherwise 0.
 */
ZINC_API Cmiss_graphic_streamlines_id Cmiss_rendition_create_graphic_streamlines(
	Cmiss_rendition_id rendition);

/**
 * Create a surfaces graphic in the rendition. Used to visualise 2-D elements
 * and faces.
 *
 * @param rendition  Handle to rendition the graphic is created in.
 * @return  Handle to the new surfaces graphic on success, otherwise 0.
 */
ZINC_API Cmiss_graphic_surfaces_id Cmiss_rendition_create_graphic_surfaces(
	Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Return a handle to selection handler for this rendition. User can add and
 * remove callback functions of the selection handler. The callback functions
 * will be called when selection on the rendition has changed. Please see
 * cmiss_selection.h for more detail
 *
 * @param rendition  Handle to a cmiss_rendition object.
 * @return  selection handler of this rendition if successful, otherwise NULL.
 */
ZINC_API Cmiss_selection_handler_id Cmiss_rendition_create_selection_handler(
	Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Use this function with Cmiss_rendition_begin_change.
 *
 * Use Cmiss_rendition_begin_change before making multiple changes on the
 * rendition, it will stop rendition from executing any immediate changes made in
 * rendition. After multiple changes have been made, use
 * this function to execute all changes made previously in rendition
 * at once.
 *
 * @param rendition  The handle to the rendition.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_rendition_end_change(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Returns the graphic of the specified name from the rendition. Beware that
 * graphics in the same rendition may have the same name and this function will
 * only return the first graphic found with the specified name;
 *
 * @param rendition  Rendition in which to find the graphic.
 * @param graphic_name  The name of the graphic to find.
 * @return  New reference to graphic of specified name, or NULL if not found.
 */
ZINC_API Cmiss_graphic_id Cmiss_rendition_find_graphic_by_name(Cmiss_rendition_id rendition,
	const char *graphic_name);

/***************************************************************************//**
 * Get the first graphic on the graphics list of <rendition>.

 * @param rendition  Handle to a cmiss_rendition object.
 * @return  Handle to a cmiss_graphic object if successful, otherwise NULL;
 */
ZINC_API Cmiss_graphic_id Cmiss_rendition_get_first_graphic(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Get the next graphic after <ref_graphic> on the graphics list of <rendition>.

 * @param rendition  Handle to a cmiss_rendition object.
 * @param ref_graphic  Handle to a cmiss_graphic object.
 * @return  Handle to a cmiss_graphic object if successful, otherwise NULL;
 */
ZINC_API Cmiss_graphic_id Cmiss_rendition_get_next_graphic(Cmiss_rendition_id rendition,
	Cmiss_graphic_id ref_graphic);

/***************************************************************************//**
 * Get the graphic before <ref_graphic> on the graphics list of <rendition>.

 * @param rendition  Handle to a cmiss_rendition object.
 * @param ref_grpahic  Handle to a cmiss_graphic object.
 * @return  Handle to a cmiss_graphic object if successful, otherwise NULL;
 */
ZINC_API Cmiss_graphic_id Cmiss_rendition_get_previous_graphic(Cmiss_rendition_id rendition,
	Cmiss_graphic_id ref_graphic);

/***************************************************************************//**
 * Returns the number of graphics in <rendition>.
 *
 * @param rendition  The handle to the rendition
 * @return  Returns the number of graphic in rendition.
 */
ZINC_API int Cmiss_rendition_get_number_of_graphics(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Get and return an accessed handle to the selection group of rendition.
 * This function will only return selection group that is still being managed.
 * Caller must destroy the reference to the handler.
 *
 * @param cmiss_rendition  pointer to the cmiss_rendition.
 *
 * @return Return selection group if successfully otherwise null.
 */
ZINC_API Cmiss_field_group_id Cmiss_rendition_get_selection_group(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Set the specified selection field to be the highlighting and selection group
 * of the specified rendition. This function will also set the selection field
 * for all of its subregion renditions if the a corresponding subregion selection
 * group is found in the selection field, otherwise the selection group of
 * the child rendition will be set to NULL;
 * Selection field set in the rendition using this function will not have its
 * access count increased.
 *
 * @param cmiss_rendition  pointer to the cmiss_rendition.
 * @param selection_field  selection field to be set for this group.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_rendition_set_selection_group(Cmiss_rendition_id rendition,
	Cmiss_field_group_id selection_field);

/***************************************************************************//**
 * Returns the state of the rendition's visibility flag. Note this only affects
 * graphics filters that act on the state of this flag.
 *
 * @param rendition  The handle to the rendition.
 * @return  1 for visible, 0 for not visible.
 */
ZINC_API int Cmiss_rendition_get_visibility_flag(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Set the state of the rendition's visibility flag. Note this only affects
 * visibility of graphics when a graphics filter is using it.
 *
 * @param rendition  The handle to the rendition.
 * @param visibility_flag  integer value to be set for the value of visibility flag.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_rendition_set_visibility_flag(Cmiss_rendition_id rendition,
	int visibility_flag);

/***************************************************************************//**
 * Move an existing graphic in rendition before ref_graphic. Both <graphic> and
 * <ref_graphic> must be from the same region.
 *
 * @param rendition  The handle to the rendition.
 * @param graphic  Cmiss_graphic to be moved.
 * @param ref_graphic  <graphic> will be moved into the current position of this
 * 		Cmiss_graphic
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_rendition_move_graphic_before(Cmiss_rendition_id rendition,
	Cmiss_graphic_id graphic, Cmiss_graphic_id ref_graphic);

/***************************************************************************//**
 * Removes all graphics from the rendition.
 *
 * @param rendition  The handle to the rendition of which the graphic is removed
 *   from.
 * @return  Status CMISS_OK if successfully remove all graphics from rendition,
 * any other value on failure.
 */
ZINC_API int Cmiss_rendition_remove_all_graphics(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Removes <graphic> from <rendition> and decrements the position
 * of all subsequent graphics.
 *
 * @param rendition  The handle to the rendition of which the graphic is removed
 *   from.
 * @param graphic  The handle to a cmiss graphic object which will be removed
 *   from the rendition.
 * @return  Status CMISS_OK if successfully remove graphic from rendition,
 * any other value on failure.
 */
ZINC_API int Cmiss_rendition_remove_graphic(Cmiss_rendition_id rendition,
	Cmiss_graphic_id graphic);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_RENDITION_H__ */
