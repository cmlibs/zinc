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
 * Portions created by the Initial Developer are Copyright (C) 2005
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

#if !defined(CAD_CAD_GEOMETRY_TO_GRAPHICS_OBJECT_H)
#define CAD_CAD_GEOMETRY_TO_GRAPHICS_OBJECT_H

#include "cad/computed_field_cad_topology.h"
#include "cad/cad_element.h"

/**
 * Create a GT surface from the given surface identifier taken from the cad topology
 * @param cad_topology a cad topology field upon which the geometry of the cad
 *        shape is based
 * @param field_cache  Cache object for field evaluations.
 * @param coordinate_field the coordinate field
 * @param data_field the data field into which the colour information is placed
 * @param render_type the render type to be displayed wireframe or surface
 * @param surface_index the surface identifier of the surface that is to be created from the 
 *        cad topology
 * @returns a GT surface for display, or NULL if the surface cannot be created
 */
struct GT_surface *create_surface_from_cad_shape(Cmiss_field_cad_topology_id cad_topology,
	Cmiss_field_cache_id field_cache, struct Computed_field *coordinate_field,
	struct Computed_field *data_field, Cmiss_graphics_render_type render_type, Cmiss_cad_surface_identifier surface_index);

/**
 * Carete a curve from the given cad topology
 */
struct GT_polyline_vertex_buffers *create_curves_from_cad_shape(Cmiss_field_cad_topology_id cad_topology,
	Cmiss_field_cache_id field_cache, struct Computed_field *coordinate_field,
	struct Computed_field *data_field, struct GT_object *graphics_object);

#endif /* !defined(CAD_CAD_GEOMETRY_TO_GRAPHICS_OBJECT_H) */
