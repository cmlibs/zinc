/*******************************************************************************
FILE : computed_field_fast_marching_image_filter.cpp

LAST MODIFIED : 22 February 2007

DESCRIPTION :
Wraps itk::MeanImageFilter
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
extern "C" {
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
#include "image_processing/computed_field_ImageFilter.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "image_processing/computed_field_fast_marching_image_filter.h"
}

#include "itkImage.h"
#include "itkVector.h"
#include "itkFastMarchingImageFilter.h"

using namespace CMISS;

namespace {

char computed_field_fast_marching_image_filter_type_string[] = "fast_marching_filter";

class Computed_field_fast_marching_image_filter : public Computed_field_ImageFilter
{

public:
	double stopping_value;
	int num_seed_points;
	double *seed_points;  // seed positions
	double *seed_values;  // seed values at each position
	int *output_size;

	// Need something to represent index

	Computed_field_fast_marching_image_filter(Computed_field *field,
		double stopping_value, int num_seed_points, double *seed_points, 
    double *seed_values, int *output_size);

	~Computed_field_fast_marching_image_filter()
	{

		// delete memory used by arrays

		if (seed_points) {
			delete seed_points;
		}

		if (seed_values) {
			delete seed_values;
		}

		if (output_size) {
			delete output_size;
		}				

	}

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_fast_marching_image_filter(new_parent,
 		  stopping_value, num_seed_points, seed_points, seed_values, output_size);
	}

	char *get_type_string()
	{
		return(computed_field_fast_marching_image_filter_type_string);
	}

	int compare(Computed_field_core* other_field);

	int list();

	char* get_command_string();
};

int Computed_field_fast_marching_image_filter::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	Computed_field_fast_marching_image_filter* other;
	int return_code;
	int i;

	ENTER(Computed_field_fast_marching_image_filter::compare);
	if (field && (other = dynamic_cast<Computed_field_fast_marching_image_filter*>(other_core)))
	{

		if ((dimension == other->dimension)
				&& (stopping_value == other->stopping_value)
				&& (num_seed_points == other->num_seed_points)) {

			return_code = 1;

			// check that all seed_points match
			for (i = 0 ; return_code && (i < dimension*num_seed_points) ; i++) {
				if (seed_points[i] != other->seed_points[i]) {
					return_code = 0;
				}
			}

			// check that all seed_values match
			for (i = 0 ; return_code && (i < num_seed_points) ; i++) {
				if (seed_values[i] != other->seed_values[i]) {
					return_code = 0;
				}
			}

			// check that all output_size values match
			for (i = 0 ; return_code && (i < dimension) ; i++) {
				if (output_size[i] != other->output_size[i]) {
					return_code = 0;
				}
			}
			
		} else {
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_fast_marching_image_filter::compare */

template < class ImageType >
class Computed_field_fast_marching_image_filter_Functor :
	public Computed_field_ImageFilter_FunctorTmpl< ImageType >
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
This class actually does the work of processing images with the filter.
It is instantiated for each of the chosen ImageTypes.
==============================================================================*/
{
	Computed_field_fast_marching_image_filter *fast_marching_image_filter;

public:

	Computed_field_fast_marching_image_filter_Functor(
		Computed_field_fast_marching_image_filter *fast_marching_image_filter) :
		Computed_field_ImageFilter_FunctorTmpl< ImageType >(fast_marching_image_filter),
		fast_marching_image_filter(fast_marching_image_filter)
	{
	}

	int set_filter(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 12 September 2006

DESCRIPTION :
Create a filter of the correct type, set the filter specific parameters
and generate the outputImage.
==============================================================================*/
	{
		
		int return_code;
		int *image_size;
		int i, j, dimension, num_seed_points;

		dimension  = fast_marching_image_filter->dimension;
		num_seed_points = fast_marching_image_filter->num_seed_points;
		image_size = fast_marching_image_filter->sizes;
		
	  // create a fastmarching object

		typedef itk::FastMarchingImageFilter<ImageType,ImageType> FMType;
		
		typename FMType::Pointer filter = FMType::New();

		// set trial points
				
		// create a node container to hold the seeds
		typedef typename FMType::NodeType NodeType;
		typedef typename FMType::NodeContainer NodeContainer;	 
		typename NodeContainer::Pointer seeds = NodeContainer::New();		
		typename ImageType::IndexType seedPosition;
		
		NodeType node;
		double seedValue;
		
		// intialise seeds container
		seeds->Initialize();
		
		// add seeds to container
		// loop over number of seeds, setting the seed value and position for each seed		
		
		for (i = 0; i < num_seed_points; i++) {
		
			// intially we will just use zero for all seedValues
			// change this once the filter is working
			seedValue = fast_marching_image_filter->seed_values[i];

			for (j = 0; j < dimension; j++) {

				// convert seed position values from xi space to values in terms 
				// of image pixels (based on image size)
				seedPosition[j] = (long int)(fast_marching_image_filter->seed_points[i * dimension + j]*image_size[j]+0.5);

				//				printf("seed position %ld\n",seedPosition[j]);	

			}

			node.SetValue( seedValue );
			node.SetIndex( seedPosition );
			seeds->InsertElement( i, node );		
		}

		// set trial points to be those in the seeds container
		filter->SetTrialPoints(seeds);


		// The normalisation factor is used in the fast marching equations
		// to allow integer values
		// It appears that the default equations are designed to work on values
		// in the range 0 to 255.  If we wish to calculate a fast maching
		// solution based on values in the range 0 to 1, then the normalization
		// factor needs to be adjusted.  
		// Values are divided by the normalization factor so to scale from 1 to 
		// 255 requires setting a normalization factor of 1/255.

		filter->SetNormalizationFactor(1/255.0);

		// IMPORTANT: if we wish to use field values that vary between a range
		// other than 0 to 1, then the normilzation factor will need to be
		// set to a sensible value based on a user defined parameter,
		// as part of the gfx def field command for the fast marching filter


		// set the output size

		// initially we use image_size values as default output image_size
		// could eventutally	fetch output image size array from the 
		// fast_marching_image_filter options
		// seems to have no effect what the output size is set to.

		typename ImageType::SizeType output_size;
		for (i=0;i <dimension;i++) {
			output_size[i] = image_size[i];
		}
		output_size[0]=1;
		output_size[1]=1;

		filter->SetOutputSize(output_size);

		// set stopping value
		filter->SetStoppingValue( fast_marching_image_filter->stopping_value);
		//		printf("Stopping value is %f", fast_marching_image_filter->stopping_value);

		return_code = fast_marching_image_filter->update_output_image
			(location, filter, this->outputImage,
			static_cast<ImageType*>(NULL), static_cast<FMType*>(NULL));
		
		return (return_code);
	} /* set_filter */

}; /* template < class ImageType > class Computed_field_fast_marching_image_filter_Functor */

Computed_field_fast_marching_image_filter::Computed_field_fast_marching_image_filter(
	Computed_field *field,
	double stopping_value, int num_seed_points, double *seed_points_in, double *seed_values_in, int *output_size_in) :
	Computed_field_ImageFilter(field),
	stopping_value(stopping_value), num_seed_points(num_seed_points)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Create the computed_field representation of the fast marching image filter.
==============================================================================*/
{

	
	// NEED to allocate seed_points array by copying it to memory here.
	// see resample code for some idea
	int i;
	int length_seed_points;
	length_seed_points = dimension * num_seed_points;
	seed_points = new double[length_seed_points];
	seed_values = new double[num_seed_points];
	output_size = new int[dimension];

	for (i = 0 ; i < length_seed_points ; i++) {
		seed_points[i] = seed_points_in[i];
	}

	for (i = 0 ; i < num_seed_points ; i++) {
		seed_values[i] = seed_values_in[i];
	}

	for (i = 0 ; i < dimension ; i++) {
		output_size[i] = output_size_in[i];
	}
	
	
#if defined DONOTUSE_TEMPLATETEMPLATES
	create_filters_singlecomponent_multidimensions(
		Computed_field_fast_marching_image_filter_Functor, this);
#else
	create_filters_singlecomponent_multidimensions
		< Computed_field_fast_marching_image_filter_Functor,
		Computed_field_fast_marching_image_filter >
		(this);
#endif
}

int Computed_field_fast_marching_image_filter::list()
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	int i;

	ENTER(List_Computed_field_fast_marching_image_filter);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    lower_threshold : %g\n", stopping_value);
		display_message(INFORMATION_MESSAGE,
			"    upper_threshold : %d\n", num_seed_points);
		display_message(INFORMATION_MESSAGE,
			"    replace_value : %d\n", dimension);

		display_message(INFORMATION_MESSAGE,"    seed_points :");
		for (i = 0; i < dimension*num_seed_points; i++) {
			display_message(INFORMATION_MESSAGE," %g",seed_points[i]);
		}		

		display_message(INFORMATION_MESSAGE,"    seed_values :");
		for (i = 0; i < num_seed_points; i++) {
			display_message(INFORMATION_MESSAGE," %g",seed_values[i]);
		}		

		display_message(INFORMATION_MESSAGE,"    output_size :");
		for (i = 0; i < dimension; i++) {
			display_message(INFORMATION_MESSAGE," %d",output_size[i]);
		}		

		display_message(INFORMATION_MESSAGE,"\n");

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_fast_marching_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_fast_marching_image_filter */

char *Computed_field_fast_marching_image_filter::get_command_string()
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	int i;

	
	ENTER(Computed_field_fast_marching_image_filter::get_command_string);
	command_string = (char *)NULL;
	if (field) {

		error = 0;
		append_string(&command_string, get_type_string(), &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " stopping_value %g", stopping_value);
		append_string(&command_string, temp_string, &error);		
		sprintf(temp_string, " num_seed_points %d", num_seed_points);	
		append_string(&command_string, temp_string, &error);		
		sprintf(temp_string, " dimension %d", dimension);	
		append_string(&command_string, temp_string, &error);		

		append_string(&command_string, " seed_points", &error);
		for (i = 0; i < dimension*num_seed_points; i++)
		{
			sprintf(temp_string, " %g", seed_points[i]);
			append_string(&command_string, temp_string, &error);
		}

		append_string(&command_string, " seed_values", &error);
		for (i = 0; i < num_seed_points; i++)
		{
			sprintf(temp_string, " %g", seed_values[i]);
			append_string(&command_string, temp_string, &error);
		}

		append_string(&command_string, " output_size", &error);
		for (i = 0; i < dimension; i++)
		{
			sprintf(temp_string, " %d", output_size[i]);
			append_string(&command_string, temp_string, &error);
		}

	} else {

		display_message(ERROR_MESSAGE,
			"Computed_field_fast_marching_image_filter::get_command_string.  Invalid field");
		
	}
	LEAVE;

	return (command_string);
} /* Computed_field_fast_marching_image_filter::get_command_string */

} //namespace

int Computed_field_set_type_fast_marching_image_filter(struct Computed_field *field,
  struct Computed_field *source_field, double stopping_value, int num_seed_points, 
  int dimension, double *seed_points, double *seed_values, int *output_size)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FAST_MARCHING_IMAGE_FILTER.
The <lower_threshold> and <upper_threshold> specify at what value thresholding
occurs.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_fast_marching_image_filter);
	if (field && source_field &&
		Computed_field_is_scalar(source_field, (void *)NULL))
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 1;


		// do some validation here to make sure the dimension passed in matches up to the dimension of the filter
		if (ALLOCATE(source_fields, struct Computed_field *,
								 number_of_source_fields))
			{
			// need to check that (dimension == dimension)
		
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0] = ACCESS(Computed_field)(source_field);
			field->source_fields = source_fields;
			field->number_of_source_fields = number_of_source_fields;			
			field->core = new Computed_field_fast_marching_image_filter(field,stopping_value, 
				num_seed_points, seed_points, seed_values, output_size);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_fast_marching_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_fast_marching_image_filter */

int Computed_field_get_type_fast_marching_image_filter(struct Computed_field *field,
  struct Computed_field **source_field, double *stopping_value, 
		int *num_seed_points, int *dimension, double **seed_points, 
		double **seed_values, int **output_size)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FAST_MARCHING_IMAGE_FILTER, 
the source_field and thresholds used by it are returned - 
otherwise an error is reported.
==============================================================================*/
{
	Computed_field_fast_marching_image_filter* core;
	int return_code;
	int i, length_seed_points;


	ENTER(Computed_field_get_type_fast_marching_image_filter);
	if (field && (core = dynamic_cast<Computed_field_fast_marching_image_filter*>(field->core))
		&& source_field)
	{
		*source_field = field->source_fields[0];
		*stopping_value = core->stopping_value;
		*num_seed_points = core->num_seed_points;
		*dimension = core->dimension;

		length_seed_points = core->dimension * core->num_seed_points;

		ALLOCATE(*seed_points, double, length_seed_points);
		for (i=0; i < length_seed_points ;i++) {
			(*seed_points)[i]=core->seed_points[i];
		}

		ALLOCATE(*seed_values, double, *num_seed_points);
		for (i=0; i < *num_seed_points ;i++) {
			(*seed_values)[i]=core->seed_values[i];
		}

		ALLOCATE(*output_size, int, *dimension);
		for (i=0; i < *dimension ;i++) {
			(*output_size)[i]=core->output_size[i];
		}
		
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_fast_marching_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_fast_marching_image_filter */

int define_Computed_field_type_fast_marching_image_filter(struct Parse_state *state,
	void *field_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FAST_MARCHING_IMAGE_FILTER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	double stopping_value;
	int num_seed_points;
	int dimension;
	double *seed_points;
	double *seed_values;
	int *output_size;
	int length_seed_points;
	int return_code;
	int previous_state_index, expected_parameters;
	int i;

	struct Computed_field *field, *source_field;
	struct Computed_field_simple_package *computed_field_simple_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_fast_marching_image_filter);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_simple_package = (Computed_field_simple_package*)computed_field_simple_package_void))
	{
		return_code = 1;

		source_field = (struct Computed_field *)NULL;
		stopping_value = 100;
		num_seed_points = 1;
		dimension  = 2;
		length_seed_points = num_seed_points * dimension;

		seed_points = (double *)NULL;
		seed_values = (double *)NULL;
		output_size = (int *)NULL;

		ALLOCATE(seed_points, double, length_seed_points);
		ALLOCATE(seed_values, double, num_seed_points);
		ALLOCATE(output_size, int, dimension);

		// should probably default to having 1 seed 
		seed_points[0] = 0.5;  // pjb: is this ok?
		seed_points[1] = 0.5;
		seed_values[0] = 0.0;
		output_size[0] = 128;
		output_size[1] = 128;

		/* get valid parameters for projection field */
		if (computed_field_fast_marching_image_filter_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code =
				Computed_field_get_type_fast_marching_image_filter(field, &source_field,
				  &stopping_value, &num_seed_points, &dimension, &seed_points, &seed_values, &output_size);
		}
		if (return_code)
		{

			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			if (state->current_token &&
				(!(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token))))
			{
				/* Handle help separately */
				option_table = CREATE(Option_table)();
				Option_table_add_help(option_table,
					"The fast_marching_filter field uses the itk::FastMarchingImageFilter code to segment a field. The segmentation is based on a level set algorithm.  The <field> it operates on is used as a speed term, to govern where the level set curve grows quickly.  The speed term is usually some function (eg a sigmoid) of an image gradient field.  The output field is a time crossing map, where the value at is each pixel is the time take to reach that location from the specified seed points.  Values typically range from 0 through to extremely large (10 to the 38).  To convert the time cross map into a segmented region use a binary threshold filter. To specify the seed points first set the <num_seed_points> and the <dimension> of the image.  The <seed_points> are a list of the coordinates for the first and any subsequent seed points.   It is also possible to specify non-zero initial <seed_values> if desired and to set the <output_size> of the time crossing map. See a/segmentation for an example of using this field.  For more information see the itk software guide.");

				/* field */
				set_source_field_data.computed_field_manager =
					computed_field_simple_package->get_computed_field_manager();
				set_source_field_data.conditional_function = Computed_field_is_scalar;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
															 &set_source_field_data, set_Computed_field_conditional);
				/* stopping_value */
				Option_table_add_double_entry(option_table, "stopping_value",
																			&stopping_value);
				/* num_seed_points */
				Option_table_add_int_positive_entry(option_table, "num_seed_points",
																						&num_seed_points);
				/* dimension */
				Option_table_add_int_positive_entry(option_table, "dimension",
																						&dimension);
				/* seed_points */
				Option_table_add_double_vector_entry(option_table, "seed_points", 
																						 seed_points, &length_seed_points);
				/* seed_values */
				Option_table_add_double_vector_entry(option_table, "seed_values", 
																						 seed_values, &num_seed_points);
				/* output_size */
				Option_table_add_int_vector_entry(option_table, "output_size", 
																						 output_size, &dimension);

				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}

			if (return_code)
			{
				
				// store previous state so that we can return to it
				previous_state_index = state->current_index;

				/* parse the two options which determine the number of seed values
					 and hence the size of the array that must be created to read in
					 the number of seed values. */

				option_table = CREATE(Option_table)();
				/* num_seed_points */
				Option_table_add_int_positive_entry(option_table, "num_seed_points",
																						&num_seed_points);
				/* dimension */
				Option_table_add_int_positive_entry(option_table, "dimension",
																						&dimension);
				/* Ignore all the other entries */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);				

				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
				
			}

			/* parse the rest of the table */
			if (return_code)
			{
				length_seed_points = num_seed_points * dimension;
					
				/* reallocate memory for arrays */
				REALLOCATE(seed_points, seed_points, double, length_seed_points);
				REALLOCATE(seed_values, seed_values, double, num_seed_points);
				REALLOCATE(output_size, output_size, int, dimension);

				/* set default values, in case options are not specified in field definition. */
				for (i=0; i < length_seed_points ;i++) {
					seed_points[i] = 0;
				}				
				for (i=0; i < num_seed_points ;i++) {
					seed_values[i] = 0;
				}				
				for (i=0; i < dimension ;i++) {
					output_size[i] = 128;
				}

				option_table = CREATE(Option_table)();
				/* field */
				set_source_field_data.computed_field_manager =
					computed_field_simple_package->get_computed_field_manager();
				set_source_field_data.conditional_function = Computed_field_is_scalar;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
															 &set_source_field_data, set_Computed_field_conditional);
				/* stopping_value */
				Option_table_add_double_entry(option_table, "stopping_value",
																			&stopping_value);
				expected_parameters = 1;
				Option_table_add_ignore_token_entry(option_table, "num_seed_points", 
																						&expected_parameters);
				expected_parameters = 1;
				Option_table_add_ignore_token_entry(option_table, "dimension", 
																						&expected_parameters);
				Option_table_add_double_vector_entry(option_table, "seed_points", 
																						 seed_points, &length_seed_points);
				Option_table_add_double_vector_entry(option_table, "seed_values", 
																						 seed_values, &num_seed_points);
				Option_table_add_int_vector_entry(option_table, "output_size", 
																						 output_size, &dimension);

				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);

			
				printf("seed_values after reading option table :");
				for (i = 0; i < num_seed_points; i++) {
					printf(" %g",seed_values[i]);
				}		
			}

			/* no errors,not asking for help */
			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_fast_marching_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = Computed_field_set_type_fast_marching_image_filter(field, 
				  source_field, stopping_value, num_seed_points, dimension, 
				  seed_points, seed_values, output_size);				
			}
			
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_fast_marching_image_filter.  Failed");
				}
			}

			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}

			if (seed_points) {
				DEALLOCATE(seed_points);
			}

			if (seed_values) {
				DEALLOCATE(seed_values);
			}

			if (output_size) {
				DEALLOCATE(output_size);
			}

		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_fast_marching_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_fast_marching_image_filter */

int Computed_field_register_types_fast_marching_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_fast_marching_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_fast_marching_image_filter_type_string, 
			define_Computed_field_type_fast_marching_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_fast_marching_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_fast_marching_image_filter */
