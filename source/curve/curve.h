/*******************************************************************************
FILE : curve.h

LAST MODIFIED : 3 September 2004

DESCRIPTION :
Definition of struct Curve used to describe parameter-value functions.
Simulation/animation parameters are controlled over time by these curves.
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
#if !defined (CONTROL_CURVE_H)
#define CONTROL_CURVE_H
#include "finite_element/finite_element.h"
#include "general/io_stream.h"
#include "general/list.h"
#include "general/manager.h"
/*
Global types
------------
*/
enum Curve_extend_mode
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Switch determining what value the curve returns at a parameter outside the range
the curve is defined over. These add extra animation flexibility.
CONTROL_CURVE_EXTEND_CLAMP uses the nearest value in the range.
CONTROL_CURVE_EXTEND_LOOP uses the curve data an integer number of ranges
	away,causing the curve to repeat.
CONTROL_CURVE_EXTEND_SWING is similar except alternate ranges are played in
	reverse.
Make sure any new options supported by
Curve_extend_mode_string
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
for automatic creation of choose_enumerator widgets.

???RC Other options that could be added in future:
CONTROL_CURVE_EXTEND_TANGENT keeps values outside the range along the last
tangent	at the edge of the range.
Also could have different conditions before and after curve.
==============================================================================*/
{
	CONTROL_CURVE_EXTEND_MODE_INVALID,
	CONTROL_CURVE_EXTEND_MODE_BEFORE_FIRST,
	CONTROL_CURVE_EXTEND_CLAMP,
	CONTROL_CURVE_EXTEND_CYCLE,
	CONTROL_CURVE_EXTEND_SWING,
	CONTROL_CURVE_EXTEND_MODE_AFTER_LAST
}; /* enum Curve_extend_mode */

enum Curve_type
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :
SAB
Users of Curves can create a type here and use it to distinguish
between curves for different uses.  This way if a copy is made of
a curve the MANAGER_ADD callbacks can tell if the copy is intended
for their application or not.  The default type is assigned on CREATE.
The type is not intended to be seen by the editor at this point
==============================================================================*/
{
	CONTROL_CURVE_TYPE_DEFAULT,
	CONTROL_CURVE_TYPE_EMOTER_MODES,
	CONTROL_CURVE_TYPE_EMOTER_COMBINE,
	CONTROL_CURVE_TYPE_EMOTER_TIMEBASE
}; /* enum Curve_extend_mode */

enum Curve_continuity_mode
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Used with Curve_enforce_continuity to adjust derivatives and scaling
factors on boundary nodes of cubic Hermite elements to enforce a specified
degree of continuity. Note that Hermite elements have G1 continuity except when
scale factors are zero.
==============================================================================*/
{
	CONTROL_CURVE_CONTINUITY_SLOPE, /* dvalue/dparameter continuous */
	CONTROL_CURVE_CONTINUITY_C1,    /* C1 continuity */
	CONTROL_CURVE_CONTINUITY_G1     /* G1 (slope) continuity */
}; /* enum Curve_continuity_mode */

struct Curve;

DECLARE_LIST_TYPES(Curve);
DECLARE_MANAGER_TYPES(Curve);

struct Curve_command_data
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Objects for the control curve commands.
==============================================================================*/
{
	struct MANAGER(Curve) *curve_manager;
	struct IO_stream_package *io_stream_package;
}; /* struct Curve_command_data */

/*
Global functions
----------------
*/
char **Curve_FE_basis_type_get_valid_strings(
	int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for all
Fe_basis_types valid with Curves - obtained from function
FE_basis_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum FE_basis_type Curve_FE_basis_type_from_string(
	char *fe_basis_type_string);
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns the FE_basis_type described by <fe_basis_type_string>, if valid for
use in Curves.
==============================================================================*/

char *Curve_extend_mode_string(
	enum Curve_extend_mode extend_mode);
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns a pointer to a static string describing the extend_mode, eg.
CONTROL_CURVE_EXTEND_CLAMP == "extend_clamp".
The calling function must not deallocate the returned string.
==============================================================================*/

char **Curve_extend_mode_get_valid_strings(
	int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Curve_extend_modes - obtained from function
Curve_extend_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Curve_extend_mode Curve_extend_mode_from_string(
	char *extend_mode_string);
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Returns the <Curve_extend_mode> described by <extend_mode_string>.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Curve);
PROTOTYPE_LIST_FUNCTIONS(Curve);
PROTOTYPE_MANAGER_FUNCTIONS(Curve);

struct Curve *CREATE(Curve)(char *name,
	enum FE_basis_type fe_basis_type,int number_of_components);
/*******************************************************************************
LAST MODIFIED : 15 September 1997

DESCRIPTION :
Allocates memory and assigns fields for a struct Curve using the given
basis type (if supported) and number of components (if not to large).
The curve starts off with no elements and no nodes defined, so its returned
value will be zero in its initial state.
==============================================================================*/

int DESTROY(Curve)(struct Curve **curve_ptr);
/*******************************************************************************
LAST MODIFIED : 27 August 1997

DESCRIPTION :
Frees the memory for the fields of <**curve_ptr>, frees the memory for
<**curve_ptr> and sets <*curve_ptr> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Curve);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Curve);
PROTOTYPE_COPY_OBJECT_FUNCTION(Curve);
PROTOTYPE_LIST_FUNCTIONS(Curve);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Curve,name,char *);
PROTOTYPE_MANAGER_COPY_FUNCTIONS(Curve,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Curve);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Curve,name,char *);

int Curve_unitize_vector(FE_value *vector,int number_of_components,
	FE_value *norm);
/*******************************************************************************
LAST MODIFIED : 14 October 1997

DESCRIPTION :
Turns <vector> into a unit vector, returning its former magnitude as <norm>.
If <vector> is a zero it is made into a unit vector in the direction [1,1,1...]
while norm is returned as zero.
==============================================================================*/

int Curve_get_node_scale_factor_dparameter(struct Curve *curve,
	int element_no,int local_node_no,FE_value *sf_dparam);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the scale factor at the node divided by the parameter change over the
element. If the parameter_change is zero, try to calculate this quantity from
the neighbouring element, if any. Otherwise zero is returned.
This value can be used to maintain continuity across element boundaries.
==============================================================================*/

int Curve_enforce_continuity(struct Curve *curve,
	int element_no,int local_node_no,int equal_priority,
	enum Curve_continuity_mode continuity_mode);
/*******************************************************************************
LAST MODIFIED : 14 October 1997

DESCRIPTION :
Used to adjust derivatives and scaling factors on boundary nodes of cubic
Hermite elements to enforce a specified degree of continuity.
If the <equal_priority> flag is set, scaling factors on both sides of the
boundary node will be adjusted to enforce continuity. Otherwise, the specified
<element_no,node_no> will be unchanged and its parter adjusted.
==============================================================================*/

int Curve_has_1_component(struct Curve *curve,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
Returns true if the curve has 1 component - used with choosers.
==============================================================================*/

int Curve_is_in_use(struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Returns true if the curve is accessed more than once; ie. it is in use
somewhere else in the program - apart from being accessed by its manager.
==============================================================================*/

int Curve_get_number_of_elements(struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the number of elements in the curve.
==============================================================================*/

int Curve_add_element(struct Curve *curve,int element_no);
/*******************************************************************************
LAST MODIFIED : 9 September 1997

DESCRIPTION :
Adds an element to the list of elements in the curve at position element_no.
Valid element_nos are from 1 to the number of elements in the curve plus 1.
New elements occupy a point (ie. zero size and the same parameter at the start
and end of the element). If the element is added at the end of the list, the
last node will be copied to achieve this. The nodes in elements added before the
end of the list are copies of the first node in the element formerly at that
space (and that element has its element_no incremented, as do all subsequent
elements). Neighbouring elements are made to share the node on their common
boundary.
If there are no existing elements, an element is created spanning from (0,0) to
(0,0) with zero derivatives.
In all cases subsequent calls to change coordinate and derivative values are
expected to make the element have a finite size.
==============================================================================*/

int Curve_subdivide_element(struct Curve *curve,
	int element_no,FE_value xi);
/*******************************************************************************
LAST MODIFIED : 8 October 1997

DESCRIPTION :
Subdivides element_no at point xi, making the 2 new elements follow the curve
of the original.
==============================================================================*/

int Curve_delete_element(struct Curve *curve,int element_no,
	int local_node_no);
/*******************************************************************************
LAST MODIFIED : 14 October 1997

DESCRIPTION :
Deletes the element with with the given number, cleaning up links with
neighbouring elements.
If the local_node_no is in either the first or the last element and not shared
by a neighbouring element, that element is deleted outright with the parameter
range reducing accordingly.
Interior local_nodes are treated as if local_node_no=0. Neighbouring elements
which share the specified local_node_no have that local node changed to the one
at the other end of the element being deleted.
==============================================================================*/

int Curve_get_number_of_components(struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the number of components in the curve field.
==============================================================================*/

int Curve_set_number_of_components(struct Curve *curve,
	int number_of_components);
/*******************************************************************************
LAST MODIFIED : 13 October 1997

DESCRIPTION :
Changes the number of components in the curve. Component information is
copied to the new curve that is created to replace the current one. If the
new number of components is less than the current number, information is lost.
If it is greater, new components values are set to default values.
Returns true if the conversion is successful..
==============================================================================*/

int Curve_get_nodes_per_element(struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the number of nodes in each element in the curve.
==============================================================================*/

int Curve_get_derivatives_per_node(struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the number of derivatives the curve stores at each node.
==============================================================================*/

enum FE_basis_type Curve_get_fe_basis_type(struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the FE_basis_type used by the curve.
==============================================================================*/

int Curve_set_fe_basis_type(struct Curve *curve,
	enum FE_basis_type fe_basis_type);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Allows the basis to be changed for an existing curve. The elements and node
values will be transformed into the new basis function, sometimes with a loss
of information, and when changing to cubic Hermite the slopes will be smoothed.
==============================================================================*/

struct FE_field *Curve_get_value_field(struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the value field used by the curve. Needed to access component names.
???RC Don't like this being available!
==============================================================================*/

int Curve_get_node_values(struct Curve *curve,
	int element_no,int local_node_no,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Gets the field->number_of_components of value field at a node in the <curve>,
identified by the <element> it is in and the <local_node_no>. Calling function
must ensure <values> array can contain number_of_components FE_values.
==============================================================================*/

int Curve_set_node_values(struct Curve *curve,
	int element_no,int local_node_no,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Sets the value field of a node in the <curve>, identified by the <element>
it is in and the <local_node_no>.
Call Curve_number_of_components to get the number of components to pass.
==============================================================================*/

int Curve_get_node_derivatives(struct Curve *curve,
	int element_no,int local_node_no,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Gets the field->number_of_components derivatives of a node in the <curve>,
identified by the <element> it is in and the <local_node_no>. Calling function
must ensure <values> array can contain number_of_components FE_values.
==============================================================================*/

int Curve_set_node_derivatives(struct Curve *curve,
	int element_no,int local_node_no,FE_value *derivatives);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Sets the derivatives of a node in the <curve>, identified by the <element>
it is in and the <local_node_no>.
Call Curve_number_of_components to get the number of components to pass.
Note no checks are made on the derivatives. A normal way to use the derivatives
is for them to be a unit vector and scaling factors at each local node adjusting
the length of the vector.
==============================================================================*/

int Curve_get_scale_factor(struct Curve *curve,
	int element_no,int local_node_no,FE_value *scale_factor);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Gets the scale factor that the derivatives at <local_node_no> in <element_no>
are multiplied by - cubic Hermite basis only.
==============================================================================*/

int Curve_set_scale_factor(struct Curve *curve,
	int element_no,int local_node_no,FE_value scale_factor);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Sets the scale factor that the derivatives at <local_node_no> in <element_no>
are multiplied by - cubic Hermite basis only. Apply continuous velocities by
adjusting these values for the parameter change over the element.
See also Curve_set_node_derivatives.
==============================================================================*/

int Curve_is_node_parameter_modifiable(struct Curve *curve,
	int local_node_no);
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns true if the parameter can be changed for the given local_node_no.
Parameter may NOT be independently set for interior nodes, since parameter is
kept proportional to xi over each element.
==============================================================================*/

int Curve_get_parameter(struct Curve *curve,
	int element_no,int local_node_no,FE_value *parameter);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the parameter at the given <element_no> <local_node_no> in <curve>.
==============================================================================*/

int Curve_set_parameter(struct Curve *curve,
	int element_no,int local_node_no,FE_value parameter);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Sets the parameter at the given <element_no> <local_node_no> in <curve>.
==============================================================================*/

int Curve_get_edit_component_range(struct Curve *curve,
	int comp_no,FE_value *min_range,FE_value *max_range);
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the range of values allowed in component <comp_no> of the <curve>.
These values are used by the interactive curve editor.
==============================================================================*/

int Curve_set_edit_component_range(struct Curve *curve,
	int comp_no,FE_value min_range,FE_value max_range);
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Sets the range of values allowed in component <comp_no> of the <curve>.
These values are used by the interactive curve editor.
==============================================================================*/

int Curve_get_element_parameter_change(struct Curve *curve,
	int element_no,FE_value *parameter_change);
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the change in parameter over <element_no> of <curve>.
==============================================================================*/

int Curve_get_parameter_range(struct Curve *curve,
	FE_value *min_parameter,FE_value *max_parameter);
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the minimum and maximum parameter values in <curve>.
==============================================================================*/

int Curve_get_parameter_grid(struct Curve *curve,
	FE_value *parameter_grid);
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Returns the <parameter_grid> of the <curve> = smallest change in parameter
allowed by the curve editor. Also controls display of grids in the editor.
==============================================================================*/

int Curve_set_parameter_grid(struct Curve *curve,
	FE_value parameter_grid);
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Sets the <parameter_grid> of the <curve> = smallest change in parameter
allowed by the curve editor. Also controls display of grids in the editor.
==============================================================================*/

int Curve_get_value_grid(struct Curve *curve,
	FE_value *value_grid);
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Returns the <value_grid> of the <curve> = smallest change in value
allowed by the curve editor. Also controls display of grids in the editor.
==============================================================================*/

int Curve_set_value_grid(struct Curve *curve,
	FE_value value_grid);
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Sets the <value_grid> of the <curve> = smallest change in value
allowed by the curve editor. Also controls display of grids in the editor.
==============================================================================*/

enum Curve_extend_mode Curve_get_extend_mode(
	struct Curve *curve);
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns the mode used for evaluating the values returned for a parameter
outside the range of parameters defined for a curve. The definition of
enum Curve_extend_mode gives more information.
==============================================================================*/

int Curve_set_extend_mode(struct Curve *curve,
	enum Curve_extend_mode extend_mode);
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Sets the mode used for evaluating the values returned for a parameter outside
the range of parameters defined for a curve.  The definition of
enum Curve_extend_mode gives more information.
==============================================================================*/

int Curve_get_type(struct Curve *curve,
	enum Curve_type *type);
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :
Returns the type which applications can use to distinguish between
Curves for other purposes and the Curves applicable to them.
==============================================================================*/

int Curve_set_type(struct Curve *curve,
	enum Curve_type type);
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :
Sets the type which applications can use to distinguish between Curves
for other purposes and the Curves applicable to them.  Applications
can define their own type in the enum Curve_type in curve/curve.h
==============================================================================*/

int Curve_get_values_at_parameter(struct Curve *curve,
	FE_value parameter,FE_value *values,FE_value *derivatives);
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Returns <values> and optional <derivatives> - w.r.t. the parameter - of the
<curve> at <parameter>. Note that if the parameter change over an element is not
positive, zero derivatives are returned instead with a warning.
Call Curve_number_of_components to get number of components returned.
Calling function must ensure values and derivatives each allocated with enough
space for number_of_components in the curve.
==============================================================================*/

int Curve_get_values_in_element(struct Curve *curve,
	int element_no,FE_value xi,FE_value *values,FE_value *derivatives);
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Returns <values> and optional <derivatives> - w.r.t. xi - of the <curve> in
element <element_no> at <xi> (from 0.0 to 1.0).
Call Curve_number_of_components to get number of components returned.
Calling function must ensure values and derivatives each allocated with enough
space for number_of_components in the curve.
==============================================================================*/

int Curve_calculate_component_over_element(
	struct Curve *curve,int element_no,int component_no,
	int num_segments,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Calculates the value of the given <component_no> of the <curve> in element
<element_no> at <num_segments>+1 points evenly spaced in xi from 0 to 1.
<values> must be preallocated with enough space for the FE_values.
Used for efficiently drawing the curve in the Curve editor.
==============================================================================*/

int Curve_get_parameter_in_element(struct Curve *curve,
	int element_no,FE_value xi,FE_value *parameter);
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Returns the <parameter> at <element_no> <xi> of <curve>.
==============================================================================*/

int Curve_find_node_at_parameter(struct Curve *curve,
	FE_value parameter, int *element_no, int *local_node_no );
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
If there is a node in <curve> exactly at <parameter>, the
<element_no> <local_node_no> for the first such node is returned.
If no such node exists the return code is zero.
==============================================================================*/

int Curve_find_element_at_parameter(struct Curve *curve,
	FE_value parameter, int *element_no, FE_value *xi );
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
If there is an element in <curve> defined at <parameter>, returns the
<element_no> and <xi> value for that parameter in the first such element.
==============================================================================*/

int gfx_define_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *curve_command_data_void);
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
==============================================================================*/

int gfx_destroy_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *Curve_manager_void);
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DESTROY CURVE command.
==============================================================================*/

int gfx_list_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *Curve_manager_void);
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX SHOW CURVE.
==============================================================================*/

int list_Curve(struct Curve *curve,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Curve iterator function for writing out the names of curves.
==============================================================================*/

int set_Curve(struct Parse_state *state,void *curve_address_void,
	void *curve_manager_void);
/*******************************************************************************
LAST MODIFIED : 5 November 1999

DESCRIPTION :
Modifier function to set the curve from a command.
==============================================================================*/

int write_Curve(struct Curve *curve,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Writes <curve> to filename(s) stemming from the name of the curve,
eg. "name" -> name.curve.com name.curve.exnode name.curve.exelem
==============================================================================*/

struct Curve *create_Curve_from_file(char *curve_name,
	char *file_name_stem, struct IO_stream_package *io_stream_package);
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Appends extension '.curve.exnode' on to <file_name_stem> and reads in nodes from
it. Similarly reads elements in from <file_name_stem>.curve.exelem. Creates
temporary managers to use import_finite_element functions. Mesh is checked for
appropriateness to curve usage.
???RC Later autorange
==============================================================================*/
#endif /* !defined (CONTROL_CURVE_H) */
