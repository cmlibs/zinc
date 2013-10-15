/**
 * FILE : scene.h
 *
 * The public interface to a zinc scene, which gives the graphical
 * representation of a region
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_SCENE_H__
#define CMZN_SCENE_H__

#include "types/fieldid.h"
#include "types/fieldgroupid.h"
#include "types/graphicsid.h"
#include "types/scenefilterid.h"
#include "types/nodeid.h"
#include "types/regionid.h"
#include "types/sceneid.h"
#include "types/scenepickerid.h"
#include "types/selectionid.h"
#include "types/spectrumid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new reference to the scene with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param scene  The scene to obtain a new reference to.
 * @return  New scene reference with incremented reference count.
 */
ZINC_API cmzn_scene_id cmzn_scene_access(cmzn_scene_id scene);

/**
 * Destroys this reference to the scene (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param scene Pointer to the handle to the scene.
 * @return  status CMZN_OK if successfully remove scene, any other value on
 * failure.
 */
ZINC_API int cmzn_scene_destroy(cmzn_scene_id * scene);

/**
 * Use this function with cmzn_scene_end_change.
 *
 * Use this function before making multiple changes on the scene, this
 * will stop scene from executing any immediate changes made in
 * scene. After multiple changes have been made, use
 * cmzn_scene_end_change to execute all changes made previously in scene
 * at once.
 *
 * @param scene  The handle to the scene.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_begin_change(cmzn_scene_id scene);

/**
 * Creates a cloud of points (nodes) in the supplied nodeset sampled at random
 * locations according to a Poisson distribution on the lines and surfaces that
 * are in the scene tree (filtered by the optional filter), i.e. including
 * all its descendents. Points/nodes are created with the next available
 * identifier. The density of points is set by supplied arguments and may be
 * scaled by data values stored in each graphics.
 *
 * @param scene  The root scene containing graphics to convert.
 * @param filter  The filter determining which graphics from the scene tree
 * are converted. If not supplied then all graphics are converted.
 * @param nodeset  The nodeset to add nodes to.
 * @param coordinate_field  The coordinate field to be defined and assigned on
 * the new nodes. Must be from the same region as the nodeset.
 * @param line_density  The expected number of points per unit length for lines.
 * @param line_density_scale_factor  If a lines graphics has a data field the
 * mean value of its first component multiplied by this factor is added to the
 * expected value.
 * @param surface_density  The expected number of points per unit area of
 * surfaces.
 * @param surface_density_scale_factor  If a surfaces graphics has a data field
 * the mean value of its first component multiplied by this factor is added to
 * the expected value.
 * @return  Status CMZN_OK on success, otherwise some other error code
 * including CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_scene_convert_to_point_cloud(cmzn_scene_id scene,
	cmzn_scenefilter_id filter, cmzn_nodeset_id nodeset,
	cmzn_field_id coordinate_field,
	double line_density, double line_density_scale_factor,
	double surface_density, double surface_density_scale_factor);

/**
 * Create a graphics of the given type in the scene.
 *
 * @param scene  Handle to scene the graphics is created in.
 * @param graphics_type  Enumerator for a specific graphics type.
 * @return  Handle to the new graphics on success, otherwise 0.
 */
ZINC_API cmzn_graphics_id cmzn_scene_create_graphics(cmzn_scene_id scene,
	enum cmzn_graphics_type graphics_type);

/**
 * Create a contours graphics in the scene. Contours create graphics showing
 * where the isoscalar field has fixed value(s): iso-surfaces for 3-D domains,
 * iso-lines for 2-D domains.
 *
 * @param scene  Handle to scene the graphics is created in.
 * @return  Handle to the new graphics on success, otherwise 0.
 */
ZINC_API cmzn_graphics_id cmzn_scene_create_graphics_contours(
	cmzn_scene_id scene);

/**
 * Create a lines graphics in the scene. Used to visualise 1-D elements and
 * lines/faces of elements.
 *
 * @param scene  Handle to scene the graphics is created in.
 * @return  Handle to the new graphics on success, otherwise 0.
 */
ZINC_API cmzn_graphics_id cmzn_scene_create_graphics_lines(
	cmzn_scene_id scene);

/**
 * Create a points graphics in the scene. Used to visualise static points,
 * nodes, data and sampled element points. Must set the domain type after
 * creation.
 * @see cmzn_graphics_set_domain_type
 *
 * @param scene  Handle to scene the graphics is created in.
 * @return  Handle to the new graphics on success, otherwise 0.
 */
ZINC_API cmzn_graphics_id cmzn_scene_create_graphics_points(
	cmzn_scene_id scene);

/**
 * Create a streamlines graphics in the scene.
 *
 * @param scene  Handle to scene the graphics is created in.
 * @return  Handle to the new graphics on success, otherwise 0.
 */
ZINC_API cmzn_graphics_id cmzn_scene_create_graphics_streamlines(
	cmzn_scene_id scene);

/**
 * Create a surfaces graphics in the scene. Used to visualise 2-D elements
 * and faces.
 *
 * @param scene  Handle to scene the graphics is created in.
 * @return  Handle to the new graphics on success, otherwise 0.
 */
ZINC_API cmzn_graphics_id cmzn_scene_create_graphics_surfaces(
	cmzn_scene_id scene);

/**
 * Return a handle to selection handler for this scene. User can add and
 * remove callback functions of the selection handler. The callback functions
 * will be called when selection on the scene has changed. Please see
 * selection.h for more detail
 *
 * @param scene  Handle to a scene object.
 * @return  selection handler of this scene if successful, otherwise NULL.
 */
ZINC_API cmzn_selection_handler_id cmzn_scene_create_selection_handler(
	cmzn_scene_id scene);

/**
 * Use this function with cmzn_scene_begin_change.
 *
 * Use cmzn_scene_begin_change before making multiple changes on the
 * scene, it will stop scene from executing any immediate changes made in
 * scene. After multiple changes have been made, use
 * this function to execute all changes made previously in scene
 * at once.
 *
 * @param scene  The handle to the scene.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_end_change(cmzn_scene_id scene);

/**
 * Returns the graphics of the specified name from the scene. Beware that
 * graphics in the same scene may have the same name and this function will
 * only return the first graphics found with the specified name;
 *
 * @param scene  Scene in which to find the graphics.
 * @param name  The name of the graphics to find.
 * @return  New reference to graphics of specified name, or NULL if not found.
 */
ZINC_API cmzn_graphics_id cmzn_scene_find_graphics_by_name(cmzn_scene_id scene,
	const char *name);

/**
 * Get the first graphics on the graphics list of <scene>.

 * @param scene  Handle to a scene object.
 * @return  Handle to a graphics object if successful, otherwise NULL;
 */
ZINC_API cmzn_graphics_id cmzn_scene_get_first_graphics(cmzn_scene_id scene);

/**
 * Get the next graphics after <ref_graphics> on the graphics list of <scene>.

 * @param scene  Handle to a scene object.
 * @param ref_graphics  Handle to a graphics object.
 * @return  Handle to a graphics object if successful, otherwise NULL;
 */
ZINC_API cmzn_graphics_id cmzn_scene_get_next_graphics(cmzn_scene_id scene,
	cmzn_graphics_id ref_graphics);

/**
 * Get the graphics before <ref_graphics> on the graphics list of <scene>.

 * @param scene  Handle to a scene object.
 * @param ref_grpahic  Handle to a graphics object.
 * @return  Handle to a graphics object if successful, otherwise NULL;
 */
ZINC_API cmzn_graphics_id cmzn_scene_get_previous_graphics(cmzn_scene_id scene,
	cmzn_graphics_id ref_graphics);

/**
 * Returns the number of graphics in <scene>.
 *
 * @param scene  The handle to the scene
 * @return  Returns the number of graphics in scene.
 */
ZINC_API int cmzn_scene_get_number_of_graphics(cmzn_scene_id scene);

/**
 * Get and return an accessed handle to the selection group of scene.
 * This function will only return selection group that is still being managed.
 * Caller must destroy the reference to the handler.
 *
 * @param scene  pointer to the scene.
 *
 * @return Return selection group if successfully otherwise null.
 */
ZINC_API cmzn_field_group_id cmzn_scene_get_selection_group(cmzn_scene_id scene);

/**
 * Set the specified selection field to be the highlighting and selection group
 * of the specified scene. This function will also set the selection field
 * for all of its subregion scenes if the a corresponding subregion selection
 * group is found in the selection field, otherwise the selection group of
 * the child scene will be set to NULL;
 * Selection field set in the scene using this function will not have its
 * access count increased.
 *
 * @param scene  pointer to the scene.
 * @param selection_field  selection field to be set for this group.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_set_selection_group(cmzn_scene_id scene,
	cmzn_field_group_id selection_field);

/**
 * Get the range of graphics data field values rendered with the spectrum.
 * Search is limited to the scene and its sub-scenes with an optional filter.
 * Spectrum colour bar glyphs do not contribute to the range.
 *
 * @param scene  Handle to the root scene to search.
 * @param filter  Optional filter on which graphics to get data range from. If
 * omitted, then all graphics within the scene tree contribute.
 * @param spectrum  The spectrum to get the range for. Only data for graphics
 * using this spectrum contribute to the range.
 * @param valuesCount  The number of data values to get the minimum and maximum
 * range for, at least 1.
 * @param minimumValuesOut  Array to receive the data value minimums. Must be at
 * least as large as valuesCount.
 * @param maximumValuesOut  Array to receive the data value maximums. Must be at
 * least as large as valuesCount.
 * @return  The number of data components for which a valid range is obtainable,
 * which can be more or less than the valuesCount so must be tested if more than
 * one component range requested. Returns 0 on any other error including bad
 * arguments.
 */
ZINC_API int cmzn_scene_get_spectrum_data_range(cmzn_scene_id scene,
	cmzn_scenefilter_id filter, cmzn_spectrum_id spectrum,
	int valuesCount, double *minimumValuesOut, double *maximumValuesOut);

/**
 * Returns the state of the scene's visibility flag.
 *
 * @param scene  The handle to the scene.
 * @return  Boolean value true if visibility flag is set, false if not.
 */
ZINC_API bool cmzn_scene_get_visibility_flag(cmzn_scene_id scene);

/**
 * Set the state of the scene's visibility flag. Note this only affects
 * visibility of graphics when a scene filter is acting on it.
 *
 * @param scene  The handle to the scene.
 * @param visibility_flag  Boolean true to set, false to clear.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_scene_set_visibility_flag(cmzn_scene_id scene,
	bool visibility_flag);

/**
 * Move a graphics to the position before ref_graphics in the scene's
 * graphics list, or last in the list if no reference graphics supplied.
 * Earlier graphics are drawn first and in hardware rendering their pixel
 * fragments are displayed in priority over later graphics at the same depth.
 *
 * @param scene  The scene to modify.
 * @param graphics  The graphics to be moved. Must be from scene.
 * @param ref_graphics  Another graphics from this scene to insert before, or
 * NULL to move graphics to last position in list.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_move_graphics_before(cmzn_scene_id scene,
	cmzn_graphics_id graphics, cmzn_graphics_id ref_graphics);

/**
 * Removes all graphics from the scene.
 *
 * @param scene  The scene to modify.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_remove_all_graphics(cmzn_scene_id scene);

/**
 * Removes a graphics from scene and decrements the position of all subsequent graphics.
 *
 * @param scene  The scene to modify.
 * @param graphics  The graphics to be removed.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_scene_remove_graphics(cmzn_scene_id scene,
	cmzn_graphics_id graphics);

/**
 * Create a scene picker which user can use to define a picking volume and
 * find the onjects included in this volume.
 *
 * @param scene  Scene to create the scene picker for.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API cmzn_scene_picker_id cmzn_scene_create_picker(cmzn_scene_id scene);

#ifdef __cplusplus
}
#endif

#endif
