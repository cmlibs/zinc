/*
 * FILE : computed_field_cad_topology.h
 *
 * Implements a cmiss field which wraps an OpenCASCADE shape.
 *
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
#if !defined (COMPUTED_FIELD_CAD_TOPOLOGY_H)
#define COMPUTED_FIELD_CAD_TOPOLOGY_H

extern "C" {
#include "api/cmiss_field.h"
#include "api/cmiss_field_cad.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
}
#include "cad/topologicalshape.h"
#include "cad/geometricshape.h"
#include "cad/cad_element.h"


//#define Computed_field_module_create_cad_topology cmzn_field_create_cad_topology
//#define cmzn_field_cad_topology Computed_field_cad_topology

//class cmzn_field_cad_topology;


cmzn_field_id cmzn_field_module_create_cad_topology(cmzn_field_module *field_module, TopologicalShape *shape );

cmzn_field_cad_topology_id cmzn_field_cast_cad_topology(cmzn_field_id cad_topology_field );

int cmzn_field_cad_topology_destroy(cmzn_field_cad_topology_id *cad_topology_field_address);

cmzn_field_cad_topology_id cmzn_field_cad_topology_access(cmzn_field_cad_topology_id cad_topology_field);

int cmzn_field_cad_topology_get_surface_count(cmzn_field_cad_topology_id cad_topology_field);

int cmzn_field_cad_topology_get_curve_count(cmzn_field_cad_topology_id cad_topology_field);

int cmzn_field_cad_topology_get_surface_point_count(cmzn_field_cad_topology_id cad_topology_field, cmzn_cad_surface_identifier identifier);

int cmzn_field_cad_topology_get_curve_point_count(cmzn_field_cad_topology_id cad_topology_field, cmzn_cad_curve_identifier identifier);

int cmzn_field_cad_topology_get_surface_point_uv_coordinates(cmzn_field_cad_topology_id cad_topology_field,
	cmzn_cad_surface_identifier identifier,
	cmzn_cad_surface_point_identifier uv_identifier,
	double &u, double &v);

int cmzn_field_cad_topology_get_curve_point_s_coordinate(cmzn_field_cad_topology_id cad_topology_field,
	cmzn_cad_curve_identifier identifier,
	cmzn_cad_curve_point_identifier s_identifier,
	double &s);

void cmzn_field_cad_topology_set_geometric_shape(cmzn_field_cad_topology_id cad_topology_field, GeometricShape *shape);

int cmzn_field_is_type_cad_topology(cmzn_field_id field, void *not_in_use);

void Cad_topology_information( cmzn_field_cad_topology_id cad_topology_field, Cad_primitive_identifier information );

int gfx_list_cad_entity(struct Parse_state *state, void *cad_element_type_void, void *root_region_void);

#endif /* !defined (COMPUTED_FIELD_CAD_SHAPE_H) */

