/*******************************************************************************
FILE : computed_field_image_resample.cpp

LAST MODIFIED : 7 March 2007

DESCRIPTION :
Field that changes the native resolution of a computed field.
Image processing fields use the native resolution to determine their image size.
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
extern "C" {
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element_region.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "image_processing/computed_field_image_resample.h"
}

class Computed_field_image_resample_package : public Computed_field_type_package
{
public:
	struct MANAGER(Computed_field) *computed_field_manager;
};

namespace {

	char computed_field_image_resample_type_string[] = "image_resample";
	
	class Computed_field_image_resample : public Computed_field_core
	{
		
	public:
		int dimension;  // Should match the dimension of the source field,
		                // kept here just to help with iterating through array
		int *sizes; // Resolution in each direction of <dimension>
		
		Computed_field_image_resample(Computed_field *field,
			int dimension, int *sizes_in) : Computed_field_core(field), dimension(dimension) 
		{
			int i;
			sizes = new int[dimension];
			for (i = 0 ; i < dimension ; i++)
			{
				sizes[i] = sizes_in[i];
			}
		}
		
		~Computed_field_image_resample()
		{
			if (sizes)
			{
				delete sizes;
			}
		}
		
	private:
		Computed_field_core *copy(Computed_field* new_parent)
		{
			return new Computed_field_image_resample(new_parent,
				dimension, sizes);
		}
		
		int evaluate_cache_at_location(Field_location* location);

		char *get_type_string()
		{
			return(computed_field_image_resample_type_string);
		}
		
		int compare(Computed_field_core* other_field);
		
		int list();
		
		char* get_command_string();

		int get_native_resolution(int *dimension,
			int **sizes, Computed_field **texture_coordinate_field);
	};
	
int Computed_field_image_resample::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	Computed_field_image_resample* other;
	int i, return_code;

	ENTER(Computed_field_image_resample::compare);
	if (field && (other = dynamic_cast<Computed_field_image_resample*>(other_core)))
	{
		if (dimension == other->dimension)
		{
			return_code = 1;
			i = dimension;
			while (return_code && (i < dimension))
			{
				if (sizes[i] != other->sizes[i])
				{
					return_code = 0;
				}
				i++;
			}
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_resample::compare */

int Computed_field_image_resample::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_image_resample::evaluate_cache_at_location);
	if (field)
	{
		return_code = Computed_field_evaluate_cache_at_location(field->source_fields[0],
			location);
		for (i=0;i<field->number_of_components;i++)
		{
			field->values[i] = field->source_fields[0]->values[i];
		}
		if (field->source_fields[0]->derivatives_valid)
		{
			temp=field->derivatives;
			temp2=field->source_fields[0]->derivatives;
			field->derivatives_valid = 1;
			element_dimension=get_FE_element_dimension(field->source_fields[0]->element);
			for (i=0;i<field->number_of_components;i++)
			{
				for (j=0;j<element_dimension;j++)
				{
					(*temp)=(*temp2);
					temp++;
					temp2++;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_value::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_resample::evaluate_cache_at_location */

int Computed_field_image_resample::get_native_resolution(int *return_dimension,
	int **return_sizes, Computed_field **return_texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 7 March 2007

DESCRIPTION :
==============================================================================*/
{       
	int i, return_code, source_dimension, *source_sizes;
	
	ENTER(Computed_field_image_resample::get_native_resolution);
	if (field)
	{
		return_code = Computed_field_get_native_resolution(
			field->source_fields[0], &source_dimension, &source_sizes, 
			return_texture_coordinate_field);
		if (dimension == source_dimension)
		{
			*return_dimension = dimension;
			// The source sizes is allocated and now ours to pass back
			for (i = 0 ; i < dimension ; i++)
			{
				source_sizes[i] = sizes[i];
			}
 			*return_sizes = source_sizes;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_image_resample::get_native_resolution.  "
				"Source dimension and field dimension do not match.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_resample::get_native_resolution.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_resample::get_native_resolution */

int Computed_field_image_resample::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_time_image_resample);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field : %s\n",
			field->source_fields[0]->name);
		
		display_message(INFORMATION_MESSAGE,
			"    sizes :");
		for (i = 0 ; i < dimension ; i++)
		{
			display_message(INFORMATION_MESSAGE,
				" %d", sizes[i]);
		}
		display_message(INFORMATION_MESSAGE,
			"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_image_resample.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_image_resample */

char *Computed_field_image_resample::get_command_string(
  )
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_time_image_resample::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_image_resample_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " sizes ", &error);
		for (i = 0 ; i < dimension ; i++)
		{
			sprintf(temp_string, " %d", sizes[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_resample::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_image_resample::get_command_string */

} //namespace

int Computed_field_set_type_image_resample(struct Computed_field *field,
	struct Computed_field *source_field, int dimension, int *sizes)
/*******************************************************************************
LAST MODIFIED : 7 March 2007

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_IMAGE_RESAMPLE with the supplied
field, <source_field>, and overrides the sizes that will be used for subsequent
image processing operations.  The texture_coordinate field could also be 
overridden.  The minimums and maximums are not implemented at all, which
would allow a windowing, and could also be overridden here.
==============================================================================*/
{
	int number_of_source_fields, return_code, source_field_dimension, *source_sizes;
	Computed_field **source_fields, *source_texture_coordinate_field;

	ENTER(Computed_field_set_type_time_image_resample);
	if (field && source_field && dimension && sizes)
	{
		if (Computed_field_get_native_resolution(
			source_field, &source_field_dimension, &source_sizes,
			&source_texture_coordinate_field) && 
			(dimension == source_field_dimension))
		{
			DEALLOCATE(source_sizes);
			return_code=1;
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=1;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->number_of_components = source_field->number_of_components;
				source_fields[0]=ACCESS(Computed_field)(source_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;
				field->core = new Computed_field_image_resample(field, dimension, sizes);
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_image_resample.  "
				"Specified dimension and source field dimension do not match.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_image_resample */

int Computed_field_get_type_image_resample(struct Computed_field *field,
	struct Computed_field **source_field, int *dimension, int **sizes)
/*******************************************************************************
LAST MODIFIED : 7 March 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_IMAGE_RESAMPLE, the function returns the source
<image_resample_field>, <dimension> and <sizes> used in each image direction.
==============================================================================*/
{
	Computed_field_image_resample* core;
	int i, return_code = 0;

	ENTER(Computed_field_get_type_image_resample);
	if (field && (core = dynamic_cast<Computed_field_image_resample*>(field->core)))
	{
		if (ALLOCATE(*sizes, int, core->dimension))
		{
			for (i = 0 ; i < core->dimension ; i++)
			{
				(*sizes)[i] = core->sizes[i];
			}
			*dimension = core->dimension;
			*source_field = field->source_fields[0];
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_image_resample.  Unable to allocate array.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_image_resample */

int define_Computed_field_type_image_resample(struct Parse_state *state,
	void *field_void,void *computed_field_image_resample_package_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2007

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_IMAGE_RESAMPLE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int dimension, return_code, original_dimension, *sizes, *original_sizes;
	Computed_field *field,*source_field, *texture_coordinate_field;
	Computed_field_image_resample_package *computed_field_image_resample_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_image_resample);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_image_resample_package=
		(Computed_field_image_resample_package *)
		computed_field_image_resample_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		original_dimension = 0;
		original_sizes = (int *)NULL;
		if (computed_field_image_resample_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_image_resample(field, 
				&source_field, &original_dimension, &original_sizes);
			ACCESS(Computed_field)(source_field);
		}
		if (return_code)
		{
			if (state->current_token &&
				(!(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token))))
			{
				/* Handle help separately */
				option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The image_resample field resamples the field to a new user specified size. It is especially useful for resizing image based fields.  The new size of the field is specified by using the <sizes> option with a list of values for the new size in each dimension.  See a/testing/image_processing_2D for an example of using this field.");

				/* source field */
				set_source_field_data.computed_field_manager=
					computed_field_image_resample_package->computed_field_manager;
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"field",&source_field,
					&set_source_field_data,set_Computed_field_conditional);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", original_sizes, &original_dimension);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		if (return_code)
		{
			option_table = CREATE(Option_table)();
			/* source field */
			set_source_field_data.computed_field_manager=
				computed_field_image_resample_package->computed_field_manager;
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);

			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			if (!source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_time_image_resample.  "
					"You must specify the source field first.");
				return_code = 0;
			}
		}
		if (return_code)
		{
			return_code = Computed_field_get_native_resolution(
				source_field, &dimension, &sizes, &texture_coordinate_field);
			if (original_sizes && (original_dimension == dimension))
			{
				DEALLOCATE(sizes);
				sizes = original_sizes;
			}
			else
			{
				DEALLOCATE(original_sizes);
			}

			/* parse the rest of the table */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				return_code = Computed_field_set_type_image_resample(field,
					source_field, dimension, sizes);
			}
			
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_time_image_resample.  Failed");
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
			DEALLOCATE(sizes);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_image_resample */

int Computed_field_register_types_image_resample(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_image_resample_package
		*computed_field_image_resample_package =
		new Computed_field_image_resample_package;

	ENTER(Computed_field_register_types_image_resample);
	if (computed_field_package)
	{
		computed_field_image_resample_package->computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_image_resample_type_string,
			define_Computed_field_type_image_resample,
			computed_field_image_resample_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_image_resample */

