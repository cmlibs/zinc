/*******************************************************************************
FILE : finite_element_to_streamlines.h

LAST MODIFIED : 14 January 2003

DESCRIPTION :
Functions for calculating streamlines in finite elements.
???DB.  Put into finite_element_to_graphics_object or split
	finite_element_to_graphics_object further ?
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
#if !defined (FINITE_ELEMENT_TO_STREAMLINES_H)
#define FINITE_ELEMENT_TO_STREAMLINES_H
#include "finite_element/finite_element.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"

/*
Global types
----------------
*/

struct Streampoint;
/*******************************************************************************
LAST MODIFIED : 11 November 1997

DESCRIPTION :
Streampoint type is private.
==============================================================================*/

struct Element_to_particle_data
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Data for converting a 3-D element into an array of particles.
==============================================================================*/
{
	FE_value xi[3];
	int element_number;
	int number_of_particles;
	struct GT_object *graphics_object;
	struct Streampoint **list;
	cmzn_field_cache_id field_cache;
	struct Computed_field *coordinate_field,*stream_vector_field;
	int index;
	Triple **pointlist;
}; /* struct Element_to_particle_data */

/*
Global functions
----------------
*/

struct GT_polyline *create_GT_polyline_streamline_FE_element(
	struct FE_element *element,FE_value *start_xi,
	cmzn_field_cache_id field_cache, struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	FE_value length,enum Streamline_data_type data_type,
	struct Computed_field *data_field, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 23 June 2004

DESCRIPTION :
Creates a <GT_polyline> streamline from the <coordinate_field> following
<stream_vector_field> (with 3, 6 or 9 components) starting in the element and at
the xi coordinates supplied. If <reverse_track> is true, the reverse of the
stream vector is tracked, and the travel_scalar is made negative.
If <fe_region> is not NULL then the function will restrict itself to elements
in that region.
@param field_cache  cmzn_field_cache for evaluating fields with. Time is
expected to have been set in the field_cache if needed.
==============================================================================*/

/**
 * Creates a <GT_surface> streamline from the <coordinate_field> following
 * <stream_vector_field> (with 3, 6 or 9 components) starting in the element and at
 * the xi coordinates supplied. If <reverse_track> is true, the reverse of the
 * stream vector is tracked, and the travel_scalar is made negative.
 * If <fe_region> is not NULL then the function will restrict itself to elements
 * in that region.
 * @param field_cache  cmzn_field_cache for evaluating fields with. Time is
 * expected to have been set in the field_cache if needed.
 * @param line_shape  LINE, RIBBON, CIRCLE_EXTRUSION or SQUARE_EXTRUSION.
 * @param line_base_size  width and thickness of line, use depends on shape.
 * @param line_scale_factors  Ignored. For future use.
 * @param line_orientation_scale_field  Ignored. For future use.
 */
struct GT_surface *create_GT_surface_streamribbon_FE_element(
	struct FE_element *element,FE_value *start_xi,
	cmzn_field_cache_id field_cache, struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track, FE_value length,
	enum cmzn_graphic_line_attributes_shape line_shape, int circleDivisions,
	FE_value *line_base_size, FE_value *line_scale_factors,
	struct Computed_field *line_orientation_scale_field,
	enum Streamline_data_type data_type,struct Computed_field *data_field,
	struct FE_region *fe_region, enum cmzn_graphic_render_polygon_mode render_polygon_mode);

int add_flow_particle(struct Streampoint **list,FE_value *xi,
	struct FE_element *element,Triple **pointlist,int index,
	cmzn_field_cache_id field_cache, struct Computed_field *coordinate_field,
	gtObject *graphics_object);
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Adds a new flow particle structure to the start of the Streampoint list
==============================================================================*/

int update_flow_particle_list(struct Streampoint *point,
	cmzn_field_cache_id field_cache, struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,FE_value step,FE_value time);
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Uses RungeKutta integration to update the position of the given streampoints
using the vector/gradient field and stepsize.  If time is 0 then the previous
point positions are updated adding no new objects.  Otherwise a new pointset is
created with the given timestamp.
@param field_cache  cmzn_field_cache for evaluating fields with. Time is
expected to have been set in the field_cache if needed.
==============================================================================*/

int element_to_particle(struct FE_element *element,
	void *void_element_to_particle_data);
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
Converts a 3-D element into an array of particles.
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_TO_STREAMLINES_H) */
