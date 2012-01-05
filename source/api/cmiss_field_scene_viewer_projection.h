/***************************************************************************//**
 * FILE : cmiss_field_scene_viewer_projection.h
 *
 * A field which extract a transformation matrix from a scene viewer for use in
 * field expressions.
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#ifndef __CMISS_FIELD_SCENE_VIEWER_PROJECTION_H__
#define __CMISS_FIELD_SCENE_VIEWER_PROJECTION_H__

#include "types/cmiss_field_id.h"
#include "types/cmiss_field_module_id.h"
#include "types/cmiss_graphics_coordinate_system.h"
#include "types/cmiss_scene_viewer_id.h"

/***************************************************************************//**
 * Creates a field whose values are the 4x4 transformation matrix mapping
 * coordinates between two graphics coordinate systems for a scene viewer.
 * The matrix maps homogeneous coordinates (x,y,z,1) to (x',y',z',h') suitable
 * for passing to a projection field. The values are continuously updated with
 * changes to the scene_viewer and become invalid if the scene_viewer is
 * destroyed.
 * Note CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL gives the local coordinate system
 * of the rendition for the owning region of field, which is transformed from
 * world coordinates by the cumulative transformation matrices of all renditions
 * down from the root region of the scene viewer's scene.
 * @see Cmiss_field_module_create_projection.
 *
 * @param field_module  Region field module which will own new field.
 * @param scene_viewer  Handle to Cmiss_scene_viewer object.
 * @param from_coordinate_system  The input coordinate system for the
 * transformation.
 * @param to_coordinate_system  The output coordinate system for the
 * transformation.
 * @return  Newly created field with 16 components.
 */
Cmiss_field_id Cmiss_field_module_create_scene_viewer_projection(
	Cmiss_field_module_id field_module,
	Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_graphics_coordinate_system from_coordinate_system,
	enum Cmiss_graphics_coordinate_system to_coordinate_system);

#endif
