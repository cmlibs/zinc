/*******************************************************************************
 * scenepicker.h
 *
 * Public interface to the cmzn_scene picker which represents a tool for
 * graphics primitives picking.
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

#ifndef CMZN_SCENEPICKER_H__
#define CMZN_SCENEPICKER_H__

#include "types/graphicid.h"
#include "types/graphicsfilterid.h"
#include "types/fieldgroupid.h"
#include "types/elementid.h"
#include "types/nodeid.h"
#include "types/sceneid.h"
#include "types/scenecoordinatesystem.h"
#include "types/scenepickerid.h"
#include "types/sceneviewerid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new reference to the scene picker with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param scene_picker  The scene_picker to obtain a new reference to.
 * @return  New scene_picker reference with incremented reference count.
 */
ZINC_API cmzn_scene_picker_id cmzn_scene_picker_access(
	cmzn_scene_picker_id scene_picker);

/**
 * Destroys this reference to the scene_picker (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param scene_picker_address  The address to the handle of the scene_picker
 *    to be destroyed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_picker_destroy(cmzn_scene_picker_id *scene_picker_address);

/**
 * Get the scene set for the scene picker to pick from.
 *
 * @param scene_picker  The scene picker to get the scene from.
 * @return  Valid handle to scene object on success, 0 on failure.
 */
ZINC_API cmzn_scene_id cmzn_scene_picker_get_scene(cmzn_scene_picker_id scene_picker);

/**
 * Set the scene for the scene picker to pick from.
 *
 * @param scene_picker  The scene picker to be modified.
 * @param scene  The scene to pick from.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_picker_set_scene(cmzn_scene_picker_id scene_picker,
	cmzn_scene_id scene);

/**
 * Get the graphics filter for the scene picker.
 *
 * @param scene_picker  The scene picker to get the graphics filter from.
 * @return  Valid handle to scene picker object on success, 0 on failure.
 */
ZINC_API cmzn_graphics_filter_id cmzn_scene_picker_get_graphics_filter(
	cmzn_scene_picker_id scene_picker);

/**
 * Set the graphics filter for the scene picker. This will affect
 * graphics to be picked,
 *
 * @param scene_picker  The scene picker to be modified.
 * @param scene  The scene to pick from.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_picker_set_graphics_filter(cmzn_scene_picker_id scene_picker,
	cmzn_graphics_filter_id filter_in);

/**
 * Set the bounding box of scene picker. Scene viewer will provide the preset
 * modelview and projection matrix from itself.
 * (x1,y1) and (x2,y2) are diagonally opposite corners of the rectangle.
 *
 * @param scene_picker  The scene picker to be modified.
 * @param scene_viewer_in  Scene viewer to get the modelview and projection
 * matrix from.
 * @param coordinate_system_in
 * @param x1 and y1 specify the location of a corner of the rectangle
 * @param x1 and y2 specify tthe location of he diagonally opposite corners
 * to (x1, y1) of the rectangle.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int  cmzn_scene_picker_set_scene_viewer_rectangle(
	cmzn_scene_picker_id scene_picker, cmzn_scene_viewer_id scene_viewer_in,
	enum cmzn_scene_coordinate_system coordinate_system_in, double x1,
		double y1, double x2, double y2);

/**
 * Get the nearest element in the defined bounding box on scene.
 *
 * @param scene_picker  The scene picker to pick the nearest element.
 * @return  a valid handle to the nearest picked element, otherwise null.
 */
ZINC_API cmzn_element_id cmzn_scene_picker_get_nearest_element(
	cmzn_scene_picker_id scene_picker);

/**
 * Get the nearest node in the defined bounding box on scene.
 *
 * @param scene_picker  The scene picker to pick the nearest node.
 * @return  a valid handle to the nearest picked node, otherwise null.
 */
ZINC_API cmzn_node_id cmzn_scene_picker_get_nearest_node(
	cmzn_scene_picker_id scene_picker);

/**
 * Get the nearest element graphic in the defined bounding box on scene.
 *
 * @param scene_picker  The scene picker to pick the nearest element graphic.
 * @return a valid handle to the nearest picked graphic, otherwise null.
 */
ZINC_API cmzn_graphic_id cmzn_scene_picker_get_nearest_element_graphic(
	cmzn_scene_picker_id scene_picker);

/**
 * Get the nearest node graphic in the defined bounding box on scene.
 *
 * @param scene_picker  The scene picker to pick the nearest node graphic.
 * @return a valid handle to the nearest picked graphic, otherwise null.
 */
ZINC_API cmzn_graphic_id cmzn_scene_picker_get_nearest_node_graphic(
	cmzn_scene_picker_id scene_picker);

/**
 * Get the nearest graphic in the defined bounding box on scene.
 *
 * @param scene_picker  The scene picker to pick the nearest graphic.
 * @return a valid handle to the nearest picked graphic, otherwise null.
 */
ZINC_API cmzn_graphic_id cmzn_scene_picker_get_nearest_graphic(
	cmzn_scene_picker_id scene_picker);

/**
 * Picked nodes belong to group field's owning region tree will
 * be addeed to group.
 * Additional subregion fields will be created when required.
 *
 * @param scene_picker  The scene picker to pick the nearest node.
 * @param group  nodes will be added to this group field
 * @return Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_picker_add_picked_nodes_to_group(
	cmzn_scene_picker_id scene_picker, cmzn_field_group_id group);

/**
 * Picked elements belong to group field's owning region tree will
 * be addeed to group.
 * Additional subregion fields will be created when required.
 *
 * @param scene_picker  The scene picker to pick the nearest element.
 * @param group  elements will be added to this group field
 * @return Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_picker_add_picked_elements_to_group(
	cmzn_scene_picker_id scene_picker, cmzn_field_group_id group);

#ifdef __cplusplus
}
#endif

#endif
