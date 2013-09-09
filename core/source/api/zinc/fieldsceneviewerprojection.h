/***************************************************************************//**
 * FILE : fieldsceneviewerprojection.h
 *
 * A field which extract a transformation matrix from a scene viewer for use in
 * field expressions.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELD_SCENE_VIEWER_PROJECTION_H__
#define CMZN_FIELD_SCENE_VIEWER_PROJECTION_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/scenecoordinatesystem.h"
#include "types/sceneviewerid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Creates a field whose values are the 4x4 transformation matrix mapping
 * coordinates between two scene coordinate systems for a scene viewer.
 * The matrix maps homogeneous coordinates (x,y,z,1) to (x',y',z',h') suitable
 * for passing to a projection field. The values are continuously updated with
 * changes to the scene_viewer and become invalid if the scene_viewer is
 * destroyed.
 * Note CMZN_SCENE_COORDINATE_SYSTEM_LOCAL gives the local coordinate system
 * of the scene for the owning region of field, which is transformed from
 * world coordinates by the cumulative transformation matrices of all scenes
 * down from the root region of the scene viewer's scene.
 * @see cmzn_field_module_create_projection.
 *
 * @param field_module  Region field module which will own new field.
 * @param scene_viewer  Handle to cmzn_scene_viewer object.
 * @param from_coordinate_system  The input coordinate system for the
 * transformation.
 * @param to_coordinate_system  The output coordinate system for the
 * transformation.
 * @return  Newly created field with 16 components.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_scene_viewer_projection(
	cmzn_field_module_id field_module,
	cmzn_scene_viewer_id scene_viewer,
	enum cmzn_scene_coordinate_system from_coordinate_system,
	enum cmzn_scene_coordinate_system to_coordinate_system);

#ifdef __cplusplus
}
#endif

#endif
