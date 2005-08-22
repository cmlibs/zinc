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

#if defined (OLD_CODE)
struct Interactive_streamline;
/*******************************************************************************
LAST MODIFIED : 18 October 1997

DESCRIPTION :
Interactive_streamline type is private.
==============================================================================*/

DECLARE_LIST_TYPES(Interactive_streamline);

DECLARE_MANAGER_TYPES(Interactive_streamline);
#endif /* defined (OLD_CODE) */

struct Streampoint;
/*******************************************************************************
LAST MODIFIED : 11 November 1997

DESCRIPTION :
Streampoint type is private.
==============================================================================*/

struct Element_to_streamline_data
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Data for converting a 3-D element into an array of streamlines.
==============================================================================*/
{
	enum Streamline_type type;
	enum Streamline_data_type data_type;
	FE_value seed_xi[3], time;
	struct Computed_field *coordinate_field,*data_field,*stream_vector_field;
	float length, width;
	/* reverse_track = track -vector and store -travel_scalar */
	int reverse_track;
	/* graphics object must be of correct type for Streamline_type */
	struct GT_object *graphics_object;
}; /* struct Element_to_streamline_data */

struct Node_to_streamline_data
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Data for converting a node with an Element_xi field into a streamline.
==============================================================================*/
{
	enum Streamline_type type;
	enum Streamline_data_type data_type;
	struct Computed_field *coordinate_field,*data_field,*stream_vector_field;
	/* The seed_element and element_groups are now passed so that when used
		with a seed_data_group the data actually drawn can be filtered by
		these parameters */
	struct FE_element *seed_element;
	struct FE_region *fe_region;
	struct FE_field *seed_data_field;
	float length, width, time;
	/* reverse_track = track -vector and store -travel_scalar */
	int reverse_track;
	/* graphics object must be of correct type for Streamline_type */
	struct GT_object *graphics_object;
}; /* struct Element_to_streamline_data */

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
	struct Computed_field *coordinate_field,*stream_vector_field;
	int index;
	Triple **pointlist;
}; /* struct Element_to_particle_data */

/*
Global functions
----------------
*/

#if defined (OLD_CODE)
PROTOTYPE_OBJECT_FUNCTIONS(Interactive_streamline);

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Interactive_streamline);

PROTOTYPE_COPY_OBJECT_FUNCTION(Interactive_streamline);

PROTOTYPE_LIST_FUNCTIONS(Interactive_streamline);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Interactive_streamline,name,
	char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Interactive_streamline,name,char *);

PROTOTYPE_MANAGER_FUNCTIONS(Interactive_streamline);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Interactive_streamline,name,char *);

void interactive_streamline_callback(Widget caller,XtPointer window_void,
	XtPointer caller_data);
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Handles the mouse events which may affect interactive streamlines.
==============================================================================*/

int interactive_streamline_set_changed(
	struct Interactive_streamline *streamline);
/*******************************************************************************
LAST MODIFIED : 29 October 1997

DESCRIPTION :
Sets the graphics object status as uncreated so that they get regenrated when
window is updated.
==============================================================================*/

struct Interactive_streamline *CREATE(Interactive_streamline)(char *name,
	enum Streamline_type type,struct FE_element *element,FE_value *xi,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,float width,enum Streamline_data_type data_type,
	struct Computed_field *data_field,struct GT_object *graphics_object,
	struct GT_object *streamline_graphics_object);
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Allocates memory and assigns fields for a Interactive_streamline object.
==============================================================================*/

int DESTROY(Interactive_streamline)(
	struct Interactive_streamline **streamline_ptr);
/*******************************************************************************
LAST MODIFIED : 24 October 1997

DESCRIPTION :
Frees the memory for the fields of <**Interactive_streamline>, frees the memory
for <**Interactive_streamline> and sets <*Interactive_streamline> to NULL.
==============================================================================*/

int get_streamline_gt_object(struct Interactive_streamline *streamline,
	struct GT_object **gt_object);
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Returns the GT_object which represents the streamline.
==============================================================================*/
#endif /* defined (OLD_CODE) */

struct GT_polyline *create_GT_polyline_streamline_FE_element(
	struct FE_element *element,FE_value *start_xi,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,enum Streamline_data_type data_type,
	struct Computed_field *data_field, FE_value time,
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 23 June 2004

DESCRIPTION :
Creates a <GT_polyline> streamline from the <coordinate_field> following
<stream_vector_field> (with 3, 6 or 9 components) starting in the element and at
the xi coordinates supplied. If <reverse_track> is true, the reverse of the
stream vector is tracked, and the travel_scalar is made negative.
If <fe_region> is not NULL then the function will restrict itself to elements
in that region.
==============================================================================*/

struct GT_surface *create_GT_surface_streamribbon_FE_element(
	struct FE_element *element,FE_value *start_xi,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,float width,enum Streamline_type type,
	enum Streamline_data_type data_type,struct Computed_field *data_field,
	FE_value time, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 23 June 2004

DESCRIPTION :
Creates a <GT_surface> streamline from the <coordinate_field> following
<stream_vector_field> (with 3, 6 or 9 components) starting in the element and at
the xi coordinates supplied. If <reverse_track> is true, the reverse of the
stream vector is tracked, and the travel_scalar is made negative.
If <fe_region> is not NULL then the function will restrict itself to elements
in that region.
==============================================================================*/

struct GT_pointset *create_interactive_streampoint(struct FE_element *element,
	struct Computed_field *coordinate_field,float length,FE_value *xi,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Creates a <GT_pointset> streampoint which can be manipulated with the mouse.
==============================================================================*/

#if defined (OLD_CODE)
int update_interactive_streamline(struct Interactive_streamline *streamline,
	FE_value *new_point,
	struct MANAGER(Interactive_streamline) *streamline_manager);
/*******************************************************************************
LAST MODIFIED : 16 May 1998

DESCRIPTION :
Moves a streampoint to the new_point position.  This function works by an
incremental step change to allow it to calculate the correct element and the
xi coordinates.  To be accurate the change in position must be small.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int add_flow_particle(struct Streampoint **list,FE_value *xi,
	struct FE_element *element,Triple **pointlist,int index,
	struct Computed_field *coordinate_field,gtObject *graphics_object);
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Adds a new flow particle structure to the start of the Streampoint list
==============================================================================*/

int update_flow_particle_list(struct Streampoint *point,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,FE_value step,float time);
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Uses RungeKutta integration to update the position of the given streampoints
using the vector/gradient field and stepsize.  If time is 0 then the previous
point positions are updated adding no new objects.  Otherwise a new pointset is
created with the given timestamp.
==============================================================================*/

int element_to_streamline(struct FE_element *element,
	void *void_element_to_streamline_data);
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Converts a 3-D element into an array of streamlines.
==============================================================================*/

int node_to_streamline(struct FE_node *node,
	void *void_node_to_streamline_data);
/*******************************************************************************
LAST MODIFIED : 4 May 1999

DESCRIPTION :
Converts a node into an array of a streamline.
==============================================================================*/

int element_to_particle(struct FE_element *element,
	void *void_element_to_particle_data);
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
Converts a 3-D element into an array of particles.
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_TO_STREAMLINES_H) */
