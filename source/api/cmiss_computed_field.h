/*******************************************************************************
FILE : cmiss_computed_field.h

LAST MODIFIED : 21 April 2008

DESCRIPTION :
The public interface to the Cmiss computed_fields.

Preferable to base new projects on the cmiss_function.h and 
cmiss_function_finite_element.h interface as this functionality should replace
computed fields.
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
#ifndef __CMISS_COMPUTED_FIELD_H__
#define __CMISS_COMPUTED_FIELD_H__

#include "api/cmiss_node.h"
#include "api/cmiss_element.h"
#include "general/object.h"
#include "general/value.h"

/*
Global types
------------
*/

struct Cmiss_region;
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/

#ifndef CMISS_REGION_ID_DEFINED
   typedef struct Cmiss_region * Cmiss_region_id;
   #define CMISS_REGION_ID_DEFINED
#endif /* CMISS_REGION_ID_DEFINED */

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_computed_field Computed_field

struct Cmiss_computed_field;
typedef struct Cmiss_computed_field *Cmiss_computed_field_id;
/*******************************************************************************
LAST MODIFIED : 31 March 2004

DESCRIPTION :
==============================================================================*/

/* Forward declared types */
/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_time_sequence FE_time_sequence
struct Cmiss_time_sequence;

struct Cmiss_node_field_creator;

/* Global Functions */

Cmiss_computed_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region, 
	const char *field_name);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Returns the computed_field of <field_name> from the <region> if it is defined.
==============================================================================*/

Cmiss_computed_field_id Cmiss_region_create_field(Cmiss_region_id region);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Creates a new field in <region>.
==============================================================================*/

int Cmiss_region_is_field_defined(Cmiss_region_id region,
	const char *field_name);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Tests if a field named <field_name> exists in <region>.
==============================================================================*/

int Cmiss_computed_field_get_number_of_components(Cmiss_computed_field_id field);
/*******************************************************************************
LAST MODIFIED : 23 December 1998

DESCRIPTION :
==============================================================================*/

int Cmiss_computed_field_destroy(Cmiss_computed_field_id *field);
/*******************************************************************************
LAST MODIFIED : 22 April 2008

DESCRIPTION :
Destroys this reference to the field (and sets it to NULL).
Internally this just decrements the reference count.
==============================================================================*/

int Cmiss_computed_field_evaluate_at_node(struct Cmiss_computed_field *field,
	struct Cmiss_node *node, float time, int number_of_values, float *values);
/*******************************************************************************
LAST MODIFIED : 29 March 2004

DESCRIPTION :
Returns the <values> of <field> at <node> and <time> if it is defined there.

The <values> array must be large enough to store as many floats as there are
number_of_components, the function checks that <number_of_values> is 
greater than or equal to the number of components.
==============================================================================*/

int Cmiss_computed_field_set_values_at_node(struct Cmiss_computed_field *field,
	struct Cmiss_node *node, float time, int number_of_values, float *values);
/*******************************************************************************
LAST MODIFIED : 21 April 2005

DESCRIPTION :
Sets the <values> of the computed <field> at <node>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the node are changed or a field
is reached for which its calculation is not reversible, or is not supported yet.
==============================================================================*/

int Cmiss_computed_field_evaluate_in_element(struct Cmiss_computed_field *field,
	struct Cmiss_element *element, float *xi, float time, 
	struct Cmiss_element *top_level_element, int number_of_values,
	float *values, int number_of_derivatives, float *derivatives);
/*******************************************************************************
LAST MODIFIED : 29 March 2004

DESCRIPTION :
Returns the values and derivatives (if <derivatives> != NULL) of <field> at
<element>:<xi>, if it is defined over the element.

The optional <top_level_element> may be supplied for the benefit of this or
any source fields that may require calculation on it instead of a face or line.
FIBRE_AXES and GRADIENT are examples of such fields, since they require
top-level coordinate derivatives. The term "top_level" refers to an ultimate
parent element for the face or line, eg. the 3-D element parent to 2-D faces.
If no such top level element is supplied and one is required, then the first
available parent element will be chosen - if the user requires a top-level
element in the same group as the face or with the face on the correct side,
then they should supply the top_level_element here.

The <values> and <derivatives> arrays must be large enough to store all the
values and derivatives for this field in the given element, ie. values is
number_of_components in size, derivatives has the element dimension times the
number_of_components
==============================================================================*/

char *Cmiss_computed_field_evaluate_as_string_at_node(
	struct Cmiss_computed_field *field, struct Cmiss_node *node, float time);
/*******************************************************************************
LAST MODIFIED : 17 January 2007

DESCRIPTION :
Returns a string describing the value/s of the <field> at the <node>. If the
field is based on an FE_field but not returning FE_values, it is asked to supply
the string. Otherwise, a string built up of comma separated values evaluated
for the field in Computed_field_evaluate_cache_at_node. The FE_value exception
is used since it is likely the values are already in the cache in most cases,
or can be used by other fields again if calculated now.
Creates a string which represents all the components.
Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/

int Cmiss_computed_field_is_defined_at_node(Cmiss_computed_field_id field,
	struct Cmiss_node *node);
/*******************************************************************************
LAST MODIFIED : 17 January 2007

DESCRIPTION :
Returns true if <field> can be calculated at <node>. If the field depends on
any other fields, this function is recursively called for them.
==============================================================================*/

int Cmiss_computed_field_evaluate_at_field_coordinates(
	Cmiss_computed_field_id field,
	Cmiss_computed_field_id reference_field, int number_of_input_values,
	float *input_values, float time, float *values);
/*******************************************************************************
LAST MODIFIED : 25 March 2008

DESCRIPTION :
Returns the <values> of <field> at the location of <input_values>
with respect to the <reference_field> if it is defined there.

The <values> array must be large enough to store as many FE_values as there are
number_of_components.
==============================================================================*/

int Cmiss_computed_field_is_type_finite_element(Cmiss_computed_field_id field);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/

int Cmiss_computed_field_get_name(Cmiss_computed_field_id field,
	char **name);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Get the name of a field.
==============================================================================*/

int Cmiss_computed_field_set_name(Cmiss_computed_field_id field,
	const char *name);
/*******************************************************************************
LAST MODIFIED : 17 April 2008

DESCRIPTION :
Change the name of a field.
==============================================================================*/

int Cmiss_computed_field_finite_element_set_string_at_node(
	Cmiss_computed_field_id field, int component_number, Cmiss_node_id node, 
	float time, const char *string);
/*******************************************************************************
LAST MODIFIED : 24 May 2006

DESCRIPTION :
Special function for Computed_field_finite_element fields only.
Allows the setting of a string if that is the type of field represented.
==============================================================================*/

int Cmiss_computed_field_finite_element_define_at_node(
	Cmiss_computed_field_id field, Cmiss_node_id node,
	struct Cmiss_time_sequence *time_sequence,
	struct Cmiss_node_field_creator *node_field_creator);
/*******************************************************************************
LAST MODIFIED : 25 May 2006

DESCRIPTION :
Special function for Computed_field_finite_element fields only.
Defines the field at the specified node.
<fe_time_sequence> optionally defines multiple times for the <field>.  If it is
NULL then the field will be defined as constant for all times.
<node_field_creator> optionally defines different versions and/or derivative types.
If it is NULL then a single nodal value for each component will be defined.
==============================================================================*/

int Cmiss_computed_field_set_type_binary_threshold_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field,
	double lower_threshold, double upper_threshold);
/*******************************************************************************
LAST MODIFIED : 9 September 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_BINARYTHRESHOLDFILTER, returning the value of
<binary_threshold_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/


int Cmiss_computed_field_get_type_binary_dilate_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field,
	int *radius, double *dilate_value);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_BINARYDILATEFILTER, the source_field and binary_dilate_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_binary_dilate_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field,
	int radius, double dilate_value);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_BINARYDILATEFILTER, returning the value of
<binary_dilate_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_binary_erode_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field,
	int *radius, double *erode_value);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_BINARYERODEFILTER, the source_field and binary_erode_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_binary_erode_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field,
	int radius, double erode_value);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_BINARYERODEFILTER, returning the value of
<binary_erode_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_binary_threshold_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field,
	double *lower_threshold, double *upper_threshold);
/*******************************************************************************
LAST MODIFIED : 9 September 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_BINARYTHRESHOLDFILTER, the source_field and binary_threshold_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_canny_edge_detection_image_filter(Cmiss_computed_field_id field,
         Cmiss_computed_field_id source_field, double variance, double maximumError,
         double upperThreshold, double lowerThreshold);
/*******************************************************************************
LAST MODIFIED : 9 September 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CANNYEDGEDETECTIONFILTER, returning the value of
<canny_edge_detection_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_canny_edge_detection_image_filter(Cmiss_computed_field_id field,
         Cmiss_computed_field_id *source_field, double *variance, double *maximumError,
         double *upperThreshold, double *lowerThreshold);
/*******************************************************************************
LAST MODIFIED : 9 September 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CANNYEDGEDETECTIONFILTER, the source_field and canny_edge_detection_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_connected_threshold_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field,
  double lower_threshold, double upper_threshold, double replace_value, 
  int num_seed_points, int dimension, double *seed_points);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CONNECTED_THRESHOLD_IMAGE_FILTER, returning the value of
<connected_threshold_image_filter> at the time/parameter value given by scalar <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_connected_threshold_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field,
  double *lower_threshold, double *upper_threshold, double *replace_value,
  int *num_seed_points, int *dimension, double **seed_points);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CONNECTED_THRESHOLD_IMAGE_FILTER, the source_field and connected_threshold_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_curvature_anisotropic_diffusion_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, double timeStep, double conductance, int numIterations);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CURVATUREANISOTROPICDIFFUSIONIMAGEFILTER, returning the value of
<curvature_anisotropic_diffusion_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_curvature_anisotropic_diffusion_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, double *timeStep, double *conductance, int *numIterations);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CURVATUREANISOTROPICDIFFUSIONIMAGEFILTER, the source_field and curvature_anisotropic_diffusion_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_derivative_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, int order, int direction);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DERIVATIVEIMAGEFILTER, returning the value of
<derivative_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
=============================================================================*/

int Cmiss_computed_field_get_type_derivative_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, int *order, int *direction);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DERIVATIVEIMAGEFILTER, the source_field and derivative_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/


int Cmiss_computed_field_set_type_discrete_gaussian_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, double variance, int maxKernelWidth);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DISCRETEGAUSSIANIMAGEFILTER, returning the value of
<discrete_gaussian_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_discrete_gaussian_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, double *variance, int *maxKernelWidth);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DISCRETEGAUSSIANIMAGEFILTER_H, the source_field and discrete_gaussian_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_gradient_magnitude_recursive_gaussian_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, double sigma);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_GRADIENT_MAGNITUDE_RECURSIVE_GAUSSIAN_IMAGE_FILTER, returning the value of
<discrete_gaussian_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_gradient_magnitude_recursvie_gaussian_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, double *sigma);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_GRADIENT_MAGNITUDE_RECURSIVE_GAUSSIAN_IMAGE_FILTER, 
the source_field and discrete_gaussian_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_mean_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, int *radius_sizes);
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MEANIMAGEFILTER, returning the value of
<mean_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_mean_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, int **radius_sizes);
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MEANIMAGEFILTER, the source_field and mean_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_rescale_intensity_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, double outputMin, double outputMax);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_RESCALEINTENSITYIMAGEFILTER, returning the value of
<rescale_intensity_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_rescale_intensity_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, double *outputMin, double *outputMax);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_RESCALEINTENSITYIMAGEFILTER, the source_field and rescale_intensity_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/


int Cmiss_computed_field_set_type_sigmoid_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, double min, double max, double alpha, double beta);
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SIGMOIDIMAGEFILTER, returning the value of
<sigmoid_image_filter> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <source_field>.
==============================================================================*/

int Cmiss_computed_field_get_type_sigmoid_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, double *min, double *max, double *alpha, double *beta);
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SIGMOIDIMAGEFILTER, the source_field and sigmoid_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_get_type_sum_components(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, FE_value **weights);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SUM_COMPONENTS, the 
<source_field> and <weights> used by it are returned.
==============================================================================*/

int Cmiss_computed_field_set_type_sum_components(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, FE_value *weights);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SUM_COMPONENTS with the supplied which
returns a scalar weighted sum of the components of <source_field>.
The <weights> array must therefore contain as many FE_values as there are
components in <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

/**
int Cmiss_computed_field_set_type_threshold_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field, 
	enum General_threshold_filter_mode threshold_mode, 
	double outside_value, double below_value, double above_value); **/
/*******************************************************************************
LAST MODIFIED : 8 December 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_THRESHOLDFILTER, returning the value of
<threshold_image_filter> at the time/parameter value given by scalar <source_field>.
==============================================================================*/

/**int Cmiss_computed_field_get_type_threshold_image_filter(Cmiss_computed_field_id field,
	Cmiss_computed_field_id *source_field, 
	enum General_threshold_filter_mode *threshold_mode, 
	double *outside_value, double *below_value,double *above_value); **/
/*******************************************************************************
LAST MODIFIED : 8 December 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_THRESHOLDFILTER, the source_field and threshold_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/

int Cmiss_computed_field_set_type_constant(Cmiss_computed_field_id field,
	int number_of_values, FE_value *values);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Changes <field> into type composite with <number_of_values> values listed in
the <values> array.
==============================================================================*/

int Cmiss_computed_field_set_type_add(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field_one,
	Cmiss_computed_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_subtract(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field_one,
	Cmiss_computed_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD (with a -1 weighting for the
second field) with the supplied fields, <source_field_one> and 
<source_field_two>.  Sets the number of components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_weighted_add(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field_one, double scale_factor1,
	Cmiss_computed_field_id source_field_two, double scale_factor2);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_multiply(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field_one,
	Cmiss_computed_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MULTIPLY with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_divide(Cmiss_computed_field_id field,
	Cmiss_computed_field_id source_field_one,
	Cmiss_computed_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVIDE with the supplied fields,
<source_field_one> and <source_field_two>.
Sets the number of components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_identity(struct Computed_field *field,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Changes <field> into type composite with one input field, the <source_field>.
==============================================================================*/

int Cmiss_computed_field_set_type_exp(struct Computed_field *field,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXP with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_log(struct Computed_field *field,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LOG with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_power(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_POWER with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
For each component the result is source_field_one to the power of 
source_field_two.
==============================================================================*/

int Cmiss_computed_field_set_type_sqrt(struct Computed_field *field,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SQRT with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_less_than(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LESS_THAN with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_greater_than(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_GREATER_THAN with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Cmiss_computed_field_set_type_if(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two,
	struct Computed_field *source_field_three);
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_IF with the supplied
fields, <source_field_one>, <source_field_two> and <source_field_three>.
Sets the number of components equal to the source_fields.
For each component, if the value of source_field_one is TRUE (non-zero) then
the result will be the value of source_field_two, otherwise the result will
be source_field_three.
==============================================================================*/

#endif /* __CMISS_COMPUTED_FIELD_H__ */
