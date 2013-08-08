/***************************************************************************//**
 * FILE : finite_element_to_iso_surfaces.h
 * 
 * Functions for creating graphical iso-surfaces from finite element fields.
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
 * Portions created by the Initial Developer are Copyright (C) 2009
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
#if !defined (FINITE_ELEMENT_TO_ISO_SURFACES_H)
#define FINITE_ELEMENT_TO_ISO_SURFACES_H

#include "graphics/graphics_object.h"

struct Iso_surface_specification;

/***************************************************************************//**
 * Creates sharable specification of iso-surfaces are to be generated.
 * 
 * @param number_of_iso_values  Number of iso-values to create surfaces from.
 * @param iso_values  If non-NULL lists the iso_values in create order. If NULL,
 * space iso-values regularly from first_iso_value to last_iso_value.
 * @param first_iso_value  First iso-value if iso_values not supplied.
 * @param last_iso_value  Last iso-value if iso_values not supplied.
 */
struct Iso_surface_specification *Iso_surface_specification_create(
	int number_of_iso_values, const double *iso_values,
	double first_iso_value, double last_iso_value,
	struct Computed_field *coordinate_field, struct Computed_field *data_field,
	struct Computed_field *scalar_field,
	struct Computed_field *texture_coordinate_field);

/***************************************************************************//**
 * Clean up iso-surface specification object. Clears pointer to object.
 * 
 * @param specification_address  Address where pointer to specification held.
 * @return  Non-zero on success.
 */
int Iso_surface_specification_destroy(
	struct Iso_surface_specification **specification_address);

/***************************************************************************//**
 * Converts a 3-D element into an iso_surface as a GT_surface
 */
int create_iso_surfaces_from_FE_element_new(struct FE_element *element,
	Cmiss_field_cache_id field_cache, Cmiss_mesh_id mesh,
	FE_value time,	int *number_in_xi,
	struct Iso_surface_specification *specification,
	struct GT_object *graphics_object, enum Cmiss_graphic_render_polygon_mode render_polygon_mode);

#endif /* !defined (FINITE_ELEMENT_TO_ISO_SURFACES_H) */
