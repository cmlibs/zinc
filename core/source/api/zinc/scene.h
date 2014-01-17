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
#include "types/fontid.h"
#include "types/glyphid.h"
#include "types/graphicsid.h"
#include "types/materialid.h"
#include "types/nodeid.h"
#include "types/regionid.h"
#include "types/scenefilterid.h"
#include "types/sceneviewerid.h"
#include "types/sceneid.h"
#include "types/scenepickerid.h"
#include "types/selectionid.h"
#include "types/spectrumid.h"
#include "types/tessellationid.h"
#include "types/timekeeperid.h"

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
 * @see cmzn_graphics_set_field_domain_type
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
 * Return a handle to selection notifier for this scene. User can add and
 * remove callback functions from the selection notifier. The callback functions
 * will be called when selection on the scene has changed. Please see
 * selection.h for more details.
 *
 * @param scene  Handle to a scene object.
 * @return  On success, handle to selection notifier of this scene, otherwise
 * NULL. Up to caller to destroy handle.
 */
ZINC_API cmzn_selectionnotifier_id cmzn_scene_create_selectionnotifier(
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
 * Gets the region this scene visualises.
 *
 * @param scene  The scene to query.
 * @return  Handle to the region for this scene. Up to caller to destroy.
 */
ZINC_API cmzn_region_id cmzn_scene_get_region(cmzn_scene_id scene);

/**
 * Get the font module which manages fonts for rendering text in graphics.
 *
 * @param scene  The scene to request the module from.
 * @return  Handle to the font module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_fontmodule_id cmzn_scene_get_fontmodule(cmzn_scene_id scene);

/**
 * Get the glyph module which stores static graphics for visualising points,
 * vectors, axes etc. Note on startup no glyphs are defined and glyph module
 * functions need to be called to define standard glyphs.
 *
 * @param scene  The scene to request the module from.
 * @return  Handle to the glyph module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_glyphmodule_id cmzn_scene_get_glyphmodule(cmzn_scene_id scene);

/**
 * Return the material module which manages materials used to colour, texture
 * and shade graphics. Note on startup only materials "default" and
 * "default_selected" are defined, as white and red, respectively. Additional
 * standard and custom materials can be defined using material module functions.
 *
 * @param scene  The scene to request the module from.
 * @return  Handle to the material module, or NULL on error. Up to caller to
 * destroy.
 */
ZINC_API cmzn_materialmodule_id cmzn_scene_get_materialmodule(
	cmzn_scene_id scene);

/**
 * Get the scene filter module which manages scenefilter objects for filtering
 * contents of scenes with scenepicker and sceneviewer etc.
 *
 * @param scene  The scene to request the module from.
 * @return  Handle to the scene filter module, or 0 on error. Up to caller to
 * destroy.
 */
ZINC_API cmzn_scenefiltermodule_id cmzn_scene_get_scenefiltermodule(
	cmzn_scene_id scene);

/**
 * Returns a handle to a sceneviewer module which manages sceneviewer objects
 * for rendering 3-D scenes into rectangular windows or canvases using OpenGL.
 *
 * @param scene  The scene to request the module from.
 * @return  Handle to the sceneviewer module, or 0 on error. Up to caller to
 * destroy.
 */
ZINC_API cmzn_sceneviewermodule_id cmzn_scene_get_sceneviewermodule(
	cmzn_scene_id scene);

/**
 * Get the spectrum module which manages spectrum objects controlling how
 * graphics data fields are converted into colours.
 *
 * @param scene  The scene to request the module from.
 * @return  Handle to the spectrum module, or 0 on error. Up to caller to
 * destroy.
 */
ZINC_API cmzn_spectrummodule_id cmzn_scene_get_spectrummodule(
	cmzn_scene_id scene);

/**
 * Get the tessellation module which manages objects controlling how curves are
 * approximated by line segments in graphics.
 *
 * @param scene  The scene to request the module from.
 * @return  Handle to the tesselation module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_tessellationmodule_id cmzn_scene_get_tessellationmodule(
	cmzn_scene_id scene);

/**
 * Get the timekeeper module which manages objects for synchronising time across
 * zinc objects.
 *
 * @param scene  The scene to request the module from.
 * @return  Handle to the timekeeper module, or 0 on error. Up to caller to destroy.
 */
ZINC_API cmzn_timekeepermodule_id cmzn_scene_get_timekeepermodule(
	cmzn_scene_id scene);

/**
 * Get the selection field for the scene, if any.
 *
 * @param scene  The scene to query.
 * @return  Handle to selection field or NULL if none or error. Up to caller to
 * destroy returned handle.
 */
ZINC_API cmzn_field_id cmzn_scene_get_selection_field(cmzn_scene_id scene);

/**
 * Set the field giving selection and highlighting in the scene.
 * Currently restricted to 'field_group' type fields.
 * This function will also set the selection field for all of its subregion
 * scenes if the a corresponding subregion field_group exists, otherwise the
 * selection group of the child scene will be set to NULL.
 *
 * @param scene  The scene to modify.
 * @param selection_field  Group field to be used as the selection.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_set_selection_field(cmzn_scene_id scene,
	cmzn_field_id selection_field);

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
ZINC_API cmzn_scenepicker_id cmzn_scene_create_scenepicker(cmzn_scene_id scene);

#ifdef __cplusplus
}
#endif

#endif
