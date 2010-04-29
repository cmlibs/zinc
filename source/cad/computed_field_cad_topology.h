/*****************************************************************************//**
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

#include "api/cmiss_field.h"
#include "cad/topologicalshape.h"
#include "cad/geometricshape.h"

#define Computed_field_create_cad_topology Cmiss_field_create_cad_topology
//#define Cmiss_field_cad_topology Computed_field_cad_topology

class Cmiss_field_cad_topology;

typedef struct Cmiss_field_cad_topology *Cmiss_field_cad_topology_id;

Cmiss_field_id Computed_field_create_cad_topology(Cmiss_field_module *field_module, TopologicalShape *shape );

Cmiss_field_cad_topology_id Cmiss_field_cast_cad_topology(Cmiss_field_id cad_topology_field );

int Cmiss_field_cad_topology_get_surface_count(Cmiss_field_cad_topology_id cad_topology_field);

int Cmiss_field_cad_topology_get_curve_count(Cmiss_field_cad_topology_id cad_topology_field);

int Cmiss_field_cad_topology_get_surface_point_count(Cmiss_field_cad_topology_id cad_topology_field, int surface_index);

int Cmiss_field_cad_topology_get_curve_point_count(Cmiss_field_cad_topology_id cad_topology_field, int curve_index);

int Cmiss_field_cad_topology_get_surface_point_uv_coordinates(Cmiss_field_cad_topology_id cad_topology_field, int surface_index, int uvPoint_index, double &u, double &v);

int Cmiss_field_cad_topology_get_curve_s_parameter(Cmiss_field_cad_topology_id cad_topology_field, int curve_index, int s_parameter_index, double &s);

void Cmiss_field_cad_topology_set_geometric_shape(Cmiss_field_cad_topology_id cad_topology_field, GeometricShape *shape);

int Cmiss_field_is_type_cad_topology(Cmiss_field_id field, void *not_in_use);

#endif /* !defined (COMPUTED_FIELD_CAD_SHAPE_H) */

