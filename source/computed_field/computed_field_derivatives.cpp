/*******************************************************************************
FILE : computed_field_derivatives.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements computed_fields for calculating various derivative quantities such
as derivatives w.r.t. Xi, gradient, curl, divergence etc.
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
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_derivatives.h"
}
#include "itkImage.h"
#include "itkVector.h"
#include "itkDerivativeImageFilter.h"

class Computed_field_derivatives_package : public Computed_field_type_package
{
};

using namespace CMISS;

namespace {

char computed_field_derivative_type_string[] = "derivative";

class Computed_field_derivative_image_filter : public Computed_field_ImageFilter
{
	/* This class is only used when the input is deemed to be grid based, 
		the derivative is not calculable on the input field and the 
		get_native_resolution method is implemented. */

public:
	int xi_index;
	int derivative_operator_order;
       
	Computed_field_derivative_image_filter(Computed_field *field,
		int xi_index, int derivative_operator_order);

	~Computed_field_derivative_image_filter()
	{
	};

	int clear_cache()
	{
		int return_code = Computed_field_ImageFilter::clear_cache();
		return (return_code);
	};

	int evaluate_cache_at_location(Field_location* location)
	{
		int return_code =
			Computed_field_ImageFilter::evaluate_cache_at_location(location);
		return (return_code);
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_derivative_image_filter(new_parent, xi_index, derivative_operator_order);
	}

	char *get_type_string()
	{
		return(computed_field_derivative_type_string);
	}

	int compare(Computed_field_core* other_field);

	int list();

	char* get_command_string();

};

int Computed_field_derivative_image_filter::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 2 July 2007

DESCRIPTION :
Not needed but required to construct object.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_derivative_image_filter::compare);
	display_message(ERROR_MESSAGE,
		"Computed_field_derivative_image_filter::compare.  Not implemented.");
	return_code = 0;
	LEAVE;

	return (return_code);
} /* Computed_field_derivative_image_filter::compare */

int Computed_field_derivative_image_filter::list()
/*******************************************************************************
LAST MODIFIED : 2 July 2007

DESCRIPTION :
Not needed but required to construct object.
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_derivative_image_filter);
	display_message(ERROR_MESSAGE,
		"Computed_field_derivative_image_filter::list.  Not implemented.");
	return_code = 0;
	LEAVE;

	return (return_code);
} /* list_Computed_field_derivative_image_filter */

char *Computed_field_derivative_image_filter::get_command_string()
/*******************************************************************************
LAST MODIFIED : 2 July 2007

DESCRIPTION :
Not needed but required to construct object.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_derivative_image_filter::get_command_string);
	display_message(ERROR_MESSAGE,
		"Computed_field_derivative_image_filter::get_command_string.  Not implemented.");
	command_string = static_cast<char *>(NULL);
	LEAVE;

	return (command_string);
} /* Computed_field_derivative_image_filter::get_command_string */

template < class ImageType >
class Computed_field_derivative_image_filter_Functor :
	public Computed_field_ImageFilter_FunctorTmpl< ImageType >
/*******************************************************************************
LAST MODIFIED : 2 July 2007

DESCRIPTION :
This class actually does the work of processing images with the filter.
It is instantiated for each of the chosen ImageTypes.
==============================================================================*/
{
	Computed_field_derivative_image_filter *derivative_image_filter;

public:

	Computed_field_derivative_image_filter_Functor(
		Computed_field_derivative_image_filter *derivative_image_filter) :
		Computed_field_ImageFilter_FunctorTmpl< ImageType >(derivative_image_filter),
		derivative_image_filter(derivative_image_filter)
	{
	}

	int set_filter(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 2 July 2007

DESCRIPTION :
Create a filter of the correct type, set the filter specific parameters
and generate the outputImage.
==============================================================================*/
	{
		int return_code;

		typedef itk::DerivativeImageFilter< ImageType , ImageType > FilterType;
		
		typename FilterType::Pointer filter = FilterType::New();

		filter->SetDirection( derivative_image_filter->xi_index );
		filter->SetOrder( derivative_image_filter->derivative_operator_order );
		
		return_code = derivative_image_filter->update_output_image
			(location, filter, this->outputImage,
			static_cast<ImageType*>(NULL), static_cast<FilterType*>(NULL));
		
		return (return_code);
	} /* set_filter */

}; /* template < class ImageType > class Computed_field_derivative_image_filter_Functor */

Computed_field_derivative_image_filter::Computed_field_derivative_image_filter(
	Computed_field *field, int xi_index, int derivative_operator_order) : 
	Computed_field_ImageFilter(field), 
	xi_index(xi_index), derivative_operator_order(derivative_operator_order)
/*******************************************************************************
LAST MODIFIED : 2 July 2007

DESCRIPTION :
Create the ITK implementation for a Computed_field_derivative.
==============================================================================*/
{
#if defined DONOTUSE_TEMPLATETEMPLATES
	create_filters_singlecomponent_multidimensions(
		Computed_field_derivative_image_filter_Functor, this);
#else
	create_filters_singlecomponent_multidimensions
		< Computed_field_derivative_image_filter_Functor, Computed_field_derivative_image_filter >
		(this);
#endif
}

class Computed_field_derivative : public Computed_field_core
{
	/* This is the actual Computed_field implementation.
		Normally this derivative is the xi derivative taken from
		the input_field.  If the derivatives are not defined on the
		input field but the get_native_resolution function is then
		an image based derivative is calculated using ITK.
		It creates a Computed_field_derivative_image_filter if and
		when required. */

public:

	int xi_index;

	Computed_field_derivative_image_filter *derivative_image_filter;

	Computed_field_derivative(Computed_field *field, int xi_index) : 
		Computed_field_core(field), xi_index(xi_index)
	{
		/* Only construct the image filter version if it is required */
		derivative_image_filter = (Computed_field_derivative_image_filter *)NULL;
	};

	~Computed_field_derivative()
	{
		if (derivative_image_filter)
		{
			delete derivative_image_filter;
		}
	};

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_derivative_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int is_defined_at_location(Field_location* location);

	int clear_cache()
	{
		/* Call the parent */
		int return_code = Computed_field_core::clear_cache();
		if (derivative_image_filter)
		{
			return_code = derivative_image_filter->clear_cache();
		}
		return (return_code);
	};

};

Computed_field_core* Computed_field_derivative::copy(Computed_field* new_parent)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_derivative* core;

	ENTER(Computed_field_derivative::copy);
	if (new_parent)
	{
		core = new Computed_field_derivative(new_parent, xi_index);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative::copy.  "
			"Invalid arguments.");
		core = (Computed_field_derivative*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_derivative::copy */

int Computed_field_derivative::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_derivative* other;
	int return_code;

	ENTER(Computed_field_derivative::compare);
	if (field && (other = dynamic_cast<Computed_field_derivative*>(field->core)))
	{
		if ((xi_index == other->xi_index))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative::compare */

int Computed_field_derivative::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_derivative::is_defined_at_location);
	if (field && location)
	{
		Field_element_xi_location* element_xi_location;
		/* Derivative values are only defined for element_xi locations */
		if ((element_xi_location  = 
				dynamic_cast<Field_element_xi_location*>(location))
			/* Derivatives can only be calculated up to element dimension */
  			&& (xi_index < get_FE_element_dimension(
				element_xi_location->get_element())))
		{
			/* Check the source field */
			return_code = Computed_field_core::is_defined_at_location(location);
		}
		else if (dynamic_cast<Field_coordinate_location*>(location))
		{
			/* This can only be valid if the input field has 
				a native resolution as we will be using image filter. */
			if (Computed_field_core::is_defined_at_location(location))
			{
				int dimension;
				int *sizes = static_cast<int *>(NULL);
				Computed_field *texture_coordinate_field;

				return_code = Computed_field_get_native_resolution(
					field->source_fields[0], &dimension, &sizes, 
					&texture_coordinate_field);

				if (sizes)
				{
					DEALLOCATE(sizes);
				}
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative::is_defined_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative::is_defined_at_location */

int Computed_field_derivative::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_derivative::evaluate_cache_at_location);
	if (field && Computed_field_has_numerical_components(field, NULL) && 
		location)
	{
		Field_element_xi_location* element_xi_location;
		return_code = 0;
		/* Only works for element_xi locations */
		if (element_xi_location  = 
			dynamic_cast<Field_element_xi_location*>(location))
		{
			FE_element* element = element_xi_location->get_element();
			int element_dimension=get_FE_element_dimension(element);
			if (xi_index < element_dimension)
			{
				Field_element_xi_location location_with_derivatives =
					Field_element_xi_location(element,
						element_xi_location->get_xi(), element_xi_location->get_time(), 
						element_xi_location->get_top_level_element(),
						element_dimension);

				/* 1. Precalculate any source fields that this field depends on,
					we always want the derivatives */
				if (Computed_field_evaluate_cache_at_location(
						 field->source_fields[0], &location_with_derivatives) && 
					field->source_fields[0]->derivatives_valid)
				{
					return_code = 1;
					/* 2. Calculate the field */
					for (i = 0 ; i < field->number_of_components ; i++)
					{
						field->values[i] = field->source_fields[0]->
							derivatives[i * element_dimension + xi_index];
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_derivative::evaluate_cache_at_location.  "
					"Derivative %d not defined on element.",
					xi_index + 1);
			}
			field->derivatives_valid = 0;
		}
		if (!return_code)
		{
			int dimension;
			int *sizes = static_cast<int *>(NULL);
			Computed_field *texture_coordinate_field;

			/* If it isn't calculated then try with the ImageFilter */
			if (Computed_field_get_native_resolution(field->source_fields[0],
					&dimension, &sizes, &texture_coordinate_field))
			{
				if (!derivative_image_filter)
				{
					/* Hard coding the default first order operator for now */
					derivative_image_filter = new 
						Computed_field_derivative_image_filter(field, xi_index,
							/*derivative_operator_order*/1);
				}
				return_code = derivative_image_filter->evaluate_cache_at_location(
					location);
			}
			
			if (sizes)
			{
				DEALLOCATE(sizes);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative::evaluate_cache_at_location */


int Computed_field_derivative::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_derivative);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    xi number : %d\n",xi_index+1);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_derivative.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_derivative */

char *Computed_field_derivative::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_derivative::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_derivative_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " xi_index %d", xi_index + 1);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_derivative::get_command_string */

} //namespace

int Computed_field_set_type_derivative(struct Computed_field *field,
	struct Computed_field *source_field, int xi_index)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DERIVATIVE with the supplied
<source_field> and <xi_index>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_derivative);
	if (field && source_field)
	{
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
			field->core = new Computed_field_derivative(field, xi_index);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_derivative */

int Computed_field_get_type_derivative(struct Computed_field *field,
	struct Computed_field **source_field, int *xi_index)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DERIVATIVE, the 
<source_field> and <xi_index> used by it are returned.
==============================================================================*/
{
	Computed_field_derivative* derivative_core;
	int return_code;

	ENTER(Computed_field_get_type_derivative);
	if (field &&
		(derivative_core = dynamic_cast<Computed_field_derivative*>(field->core))
		&& source_field)
	{
		*source_field = field->source_fields[0];
		*xi_index = derivative_core->xi_index;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_derivative */

int define_Computed_field_type_derivative(struct Parse_state *state,
	void *field_modify_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DERIVATIVE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code, xi_index;
	struct Computed_field *field,*source_field;
	Computed_field_derivatives_package 
		*computed_field_derivatives_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_derivative);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_derivatives_package=
		(Computed_field_derivatives_package *)
		computed_field_derivatives_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		xi_index = 1;
		if (computed_field_derivative_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_derivative(field,
				&source_field, &xi_index);
			xi_index++;
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The derivative field has two modes of operation.  For normal "
				"finite element fields it simply promotes the derivative "
				"values corresponding to <xi_index> calculated by the input "
				"<field> to be the field values.  These derivatives are with "
				"respect to xi. "
				"If the input <field> cannot cannot calculate element based "
				"derivatives then if the input field has a native resolution "
				"then this field uses the ITK DerivativeImageFilter to calculate "
				"a pixel based derivative at that same resolution.  "
				"The derivative filter will use the image pixel physical spacing "
				"if that is defined for ITK.  Note that as the derivative is a "
				"signed value you may want to offset and scale the resultant "
				"values if you intend to store them in an unsigned pixel format.");

			/* field */
			set_source_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_source_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			/* xi_index */
			Option_table_add_entry(option_table,"xi_index",&xi_index,
				NULL,set_int_positive);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_derivative(field,
					source_field, xi_index - 1);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_derivative.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_derivative */

namespace {

char computed_field_curl_type_string[] = "curl";

class Computed_field_curl : public Computed_field_core
{
public:
	Computed_field_curl(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_curl(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_curl_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_curl*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int evaluate(int element_dimension);
};

int Computed_field_curl::evaluate(int element_dimension)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute the curl.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
If function fails to invert the coordinate derivatives then the curl is
returned as 0 with a warning - as may happen at certain locations of the mesh.
Note currently requires vector_field to be RC.
==============================================================================*/
{
	FE_value curl,dx_dxi[9],dxi_dx[9],x[3],*source;
	int coordinate_components,i,return_code;
	
	ENTER(Computed_field_evaluate_curl);
	if (field&&(dynamic_cast<Computed_field_curl*>(field->core)))
	{
		coordinate_components=field->source_fields[1]->number_of_components;
		/* curl is only valid in 3 dimensions */
		if ((3==element_dimension)&&(3==coordinate_components))
		{
			/* only support RC vector fields */
			if (RECTANGULAR_CARTESIAN==
				field->source_fields[0]->coordinate_system.type)
			{
				if (return_code=Computed_field_extract_rc(field->source_fields[1],
					element_dimension,x,dx_dxi))
				{
					if (invert_FE_value_matrix3(dx_dxi,dxi_dx))
					{
						source=field->source_fields[0]->derivatives;
						/* curl[0] = dVz/dy - dVy/dz */
						curl=0.0;
						for (i=0;i<element_dimension;i++)
						{
							curl += (source[6+i]*dxi_dx[3*i+1] - source[3+i]*dxi_dx[3*i+2]);
						}
						field->values[0]=curl;
						/* curl[1] = dVx/dz - dVz/dx */
						curl=0.0;
						for (i=0;i<element_dimension;i++)
						{
							curl += (source[  i]*dxi_dx[3*i+2] - source[6+i]*dxi_dx[3*i  ]);
						}
						field->values[1]=curl;
						/* curl[2] = dVy/dx - dVx/dy */
						curl=0.0;
						for (i=0;i<element_dimension;i++)
						{
							curl += (source[3+i]*dxi_dx[3*i  ] - source[  i]*dxi_dx[3*i+1]);
						}
						field->values[2]=curl;
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"Could not invert coordinate derivatives; setting curl to 0");
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=0.0;
						}
					}
					/* cannot calculate derivatives for curl yet */
					field->derivatives_valid=0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_curl.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_curl.  Vector field must be RC");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_curl.  Elements of wrong dimension");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_curl.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_curl */

int Computed_field_curl::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_element *top_level_element;
	FE_value top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, return_code, top_level_element_dimension;

	ENTER(Computed_field_curl::evaluate_cache_at_location);
	if (field && location)
	{
		Field_element_xi_location* element_xi_location;
		/* Only works for element_xi locations */
		if ((element_xi_location  = 
				dynamic_cast<Field_element_xi_location*>(location)))
		{
			element_dimension = get_FE_element_dimension(
				element_xi_location->get_element());
			/* 1. Precalculate any source fields that this field depends on,
				we always want the derivatives and want to use the top_level element */
			FE_element_get_top_level_element_and_xi(element_xi_location->get_element(),
				element_xi_location->get_xi(), element_dimension,
				&top_level_element, top_level_xi, &top_level_element_dimension);

			Field_element_xi_location top_level_location(top_level_element,
				top_level_xi, element_xi_location->get_time(), top_level_element,
				get_FE_element_dimension(top_level_element));
			if (return_code = 
				Computed_field_evaluate_source_fields_cache_at_location(field,
					&top_level_location))
			{
				/* 2. Calculate the field */
				return_code=evaluate(top_level_element_dimension);
			}
			field->derivatives_valid = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_curl::evaluate_cache_at_location.  "
				"Only calculable in elements.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curl::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curl::evaluate_cache_at_location */


int Computed_field_curl::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_curl);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    vector field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_curl.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_curl */

char *Computed_field_curl::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_curl::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_curl_type_string, &error);
		append_string(&command_string, " coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " vector ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curl::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_curl::get_command_string */

} //namespace

int Computed_field_set_type_curl(struct Computed_field *field,
	struct Computed_field *vector_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CURL, combining a vector and
coordinate field to return the curl scalar.
Sets the number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
<vector_field> and <coordinate_field> must both have exactly 3 components.
The vector field must also be RECTANGULAR_CARTESIAN.
Note that an error will be reported on calculation if the xi-dimension of the
element and the number of components in coordinate_field & vector_field differ.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_curl);
	if (field&&vector_field&&(3==vector_field->number_of_components)&&
		coordinate_field&&(3==coordinate_field->number_of_components))
	{
		/* only support RC vector fields */
		if (RECTANGULAR_CARTESIAN==vector_field->coordinate_system.type)
		{
			return_code=1;
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=2;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->number_of_components=3;
				source_fields[0]=ACCESS(Computed_field)(vector_field);
				source_fields[1]=ACCESS(Computed_field)(coordinate_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_curl(field);
			}
			else
			{
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_curl.  Vector field must be RC");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_curl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_curl */

int Computed_field_get_type_curl(struct Computed_field *field,
	struct Computed_field **vector_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CURL, the 
<source_field> and <coordinate_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_curl);
	if (field&&(dynamic_cast<Computed_field_curl*>(field->core))
		&&vector_field&&coordinate_field)
	{
		/* source_fields: 0=vector, 1=coordinate */
		*vector_field = field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_curl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_curl */

int define_Computed_field_type_curl(struct Parse_state *state,
	void *field_modify_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CURL (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*field,*vector_field;
	Computed_field_derivatives_package 
		*computed_field_derivatives_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_curl);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_derivatives_package=
		(Computed_field_derivatives_package *)
		computed_field_derivatives_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field = (struct Computed_field *)NULL;
		vector_field = (struct Computed_field *)NULL;
		if (computed_field_curl_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_curl(field,
				&vector_field,&coordinate_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (vector_field)
			{
				ACCESS(Computed_field)(vector_field);
			}

			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_vector_field_data.conditional_function=
				Computed_field_has_3_components;
			set_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&vector_field,
				(void *)&set_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_curl(field,
					vector_field,coordinate_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_curl.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (vector_field)
			{
				DEACCESS(Computed_field)(&vector_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_curl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_curl */

namespace {

char computed_field_divergence_type_string[] = "divergence";

class Computed_field_divergence : public Computed_field_core
{
public:
	Computed_field_divergence(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_divergence(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_divergence_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_divergence*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int evaluate(int element_dimension);
};

int Computed_field_divergence::evaluate(int element_dimension)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute the divergence.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
If function fails to invert the coordinate derivatives then the divergence is
returned as 0 with a warning - as may happen at certain locations of the mesh.
Note currently requires vector_field to be RC.
==============================================================================*/
{
	FE_value divergence,dx_dxi[9],dxi_dx[9],x[3],*source;
	int coordinate_components,i,j,return_code;
	
	ENTER(Computed_field_evaluate_divergence);
	if (field&&(dynamic_cast<Computed_field_divergence*>(field->core)))
	{
		coordinate_components=field->source_fields[1]->number_of_components;
		/* Following asks: can dx_dxi be inverted? */
		if (((3==element_dimension)&&(3==coordinate_components))||
			((RECTANGULAR_CARTESIAN==field->source_fields[1]->coordinate_system.type)
				&&(coordinate_components==element_dimension))||
			((CYLINDRICAL_POLAR==field->source_fields[1]->coordinate_system.type)&&
				(2==element_dimension)&&(2==coordinate_components)))
		{
			/* only support RC vector fields */
			if (RECTANGULAR_CARTESIAN==
				field->source_fields[0]->coordinate_system.type)
			{
				if (return_code=Computed_field_extract_rc(field->source_fields[1],
					element_dimension,x,dx_dxi))
				{
					/* if the element_dimension is less than 3, put ones on the main
						 diagonal to allow inversion of dx_dxi */
					if (3>element_dimension)
					{
						dx_dxi[8]=1.0;
						if (2>element_dimension)
						{
							dx_dxi[4]=1.0;
						}
					}
					if (invert_FE_value_matrix3(dx_dxi,dxi_dx))
					{
						divergence=0.0;
						source=field->source_fields[0]->derivatives;
						for (i=0;i<element_dimension;i++)
						{
							for (j=0;j<element_dimension;j++)
							{
								divergence += (*source) * dxi_dx[3*j+i];
								source++;
							}
						}
						field->values[0]=divergence;
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"Could not invert coordinate derivatives; "
							"setting divergence to 0");
						field->values[0]=0.0;
					}
					/* cannot calculate derivatives for divergence yet */
					field->derivatives_valid=0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_divergence.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_divergence.  Vector field must be RC");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_divergence.  Elements of wrong dimension");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_divergence.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_divergence */

int Computed_field_divergence::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_element *top_level_element;
	FE_value top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, return_code, top_level_element_dimension;

	ENTER(Computed_field_divergence::evaluate_cache_at_location);
	if (field && location)
	{
		Field_element_xi_location* element_xi_location;
		/* Only works for element_xi locations */
		if ((element_xi_location  = 
				dynamic_cast<Field_element_xi_location*>(location)))
		{
			element_dimension = get_FE_element_dimension(
				element_xi_location->get_element());
			/* 1. Precalculate any source fields that this field depends on,
				we always want the derivatives and want to use the top_level element */
			FE_element_get_top_level_element_and_xi(element_xi_location->get_element(),
				element_xi_location->get_xi(), element_dimension,
				&top_level_element, top_level_xi, &top_level_element_dimension);

			Field_element_xi_location top_level_location(top_level_element,
				top_level_xi, element_xi_location->get_time(), top_level_element,
				get_FE_element_dimension(top_level_element));
			if (return_code = 
				Computed_field_evaluate_source_fields_cache_at_location(field,
					&top_level_location))
			{
				/* 2. Calculate the field */
				return_code=evaluate(top_level_element_dimension);
			}
			field->derivatives_valid = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_divergence::evaluate_cache_at_location.  "
				"Only calculable in elements.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divergence::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divergence::evaluate_cache_at_location */


int Computed_field_divergence::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_divergence);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    vector field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_divergence.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_divergence */

char *Computed_field_divergence::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_divergence::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_divergence_type_string, &error);
		append_string(&command_string, " coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " vector ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divergence::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_divergence::get_command_string */

} //namespace

int Computed_field_set_type_divergence(struct Computed_field *field,
	struct Computed_field *vector_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVERGENCE, combining a vector and
coordinate field to return the divergence scalar.
Sets the number of components to 1.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
The number of components of <vector_field> and <coordinate_field> must be the
same and less than or equal to 3
The vector field must also be RECTANGULAR_CARTESIAN.
Note that an error will be reported on calculation if the xi-dimension of the
element and the number of components in coordinate_field & vector_field differ.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_divergence);
	if (field&&vector_field&&coordinate_field&&
		(3>=coordinate_field->number_of_components)&&
		(vector_field->number_of_components==
			coordinate_field->number_of_components))
	{
		/* only support RC vector fields */
		if (RECTANGULAR_CARTESIAN==vector_field->coordinate_system.type)
		{
			return_code=1;
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=2;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->number_of_components=1;
				source_fields[0]=ACCESS(Computed_field)(vector_field);
				source_fields[1]=ACCESS(Computed_field)(coordinate_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_divergence(field);
			}
			else
			{
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_divergence.  Vector field must be RC");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_divergence.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_divergence */

int Computed_field_get_type_divergence(struct Computed_field *field,
	struct Computed_field **vector_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DIVERGENCE, the 
<source_field> and <coordinate_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_divergence);
	if (field&&(dynamic_cast<Computed_field_divergence*>(field->core))
		&&vector_field&&coordinate_field)
	{
		/* source_fields: 0=vector, 1=coordinate */
		*vector_field = field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_divergence.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_divergence */

int define_Computed_field_type_divergence(struct Parse_state *state,
	void *field_modify_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DIVERGENCE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*field,*vector_field;
	Computed_field_derivatives_package 
		*computed_field_derivatives_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_divergence);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_derivatives_package=
		(Computed_field_derivatives_package *)
		computed_field_derivatives_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		vector_field=(struct Computed_field *)NULL;
		if (computed_field_divergence_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_divergence(field,
				&vector_field,&coordinate_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (vector_field)
			{
				ACCESS(Computed_field)(vector_field);
			}

			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_vector_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&vector_field,
				(void *)&set_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_divergence(field,
					vector_field,coordinate_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_divergence.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (vector_field)
			{
				DEACCESS(Computed_field)(&vector_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_divergence.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_divergence */

namespace {

char computed_field_gradient_type_string[] = "gradient";

class Computed_field_gradient : public Computed_field_core
{
public:
	Computed_field_gradient(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_gradient(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_gradient_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_gradient*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_gradient::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	double a[9], b[3], d;
	FE_element *top_level_element;
	FE_value *destination, *source, top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int coordinate_components, i, indx[3], j, return_code,
		source_components, top_level_element_dimension;
	struct Computed_field *coordinate_field, *source_field;

	ENTER(Computed_field_gradient::evaluate_cache_at_location);
	return_code = 0;

	Field_element_xi_location* element_xi_location;
	/* Only works for element_xi locations */
	if (field && location && (element_xi_location  = 
			dynamic_cast<Field_element_xi_location*>(location)))
	{
		/* cannot calculate derivatives for gradient yet */
		field->derivatives_valid = 0;
		if (!location->get_number_of_derivatives())
		{
			FE_element* element = element_xi_location->get_element();
			int element_dimension=get_FE_element_dimension(element);

			FE_element_get_top_level_element_and_xi(element_xi_location->get_element(),
				element_xi_location->get_xi(), element_dimension,
				&top_level_element, top_level_xi, &top_level_element_dimension);

			Field_element_xi_location top_level_location(top_level_element,
				top_level_xi, element_xi_location->get_time(), top_level_element,
				get_FE_element_dimension(top_level_element));

			/* 1. Precalculate any source fields that this field depends on,
				we always want the derivatives and want to use the top_level element */
			if (Computed_field_evaluate_source_fields_cache_at_location(
					 field, &top_level_location))
			{
				/* 2. Calculate the field. First verify we can invert the derivatives of
					the coordinate field, which we can if dx_dxi is square */
				source_field = field->source_fields[0];
				source_components = source_field->number_of_components;
				coordinate_field = field->source_fields[1];
				coordinate_components = coordinate_field->number_of_components;
				if (top_level_element_dimension == coordinate_components)
				{
					/* fill square matrix a with coordinate field derivatives w.r.t. xi */
					for (i = 0; i < coordinate_components; i++)
					{
						for (j = 0; j < coordinate_components; j++)
						{
							a[i*coordinate_components + j] =
								coordinate_field->derivatives[j*coordinate_components+i];
						}
					}
					/* invert to get derivatives of xi w.r.t. coordinates */
					if (LU_decompose(coordinate_components,a,indx,&d,/*singular_tolerance*/1.0e-12))
					{
						return_code = 1;
						destination = field->values;
						source = source_field->derivatives;
						for (i = 0; (i < source_components) && return_code; i++)
						{
							for (j = 0; j < coordinate_components; j++)
							{
								b[j] = (double)(*source);
								source++;
							}
							if (LU_backsubstitute(coordinate_components,a,indx,b))
							{
								for (j = 0; j < coordinate_components; j++)
								{
									*destination = (FE_value)b[j];
									destination++;
								}
							}
							else
							{
								return_code = 0;
							}
						}
					}
					if (!return_code)
					{
						/* cannot invert at apex of heart, so set to zero to allow values
							to be viewed elsewhere in mesh */
						display_message(WARNING_MESSAGE,
							"Computed_field_gradient::evaluate_cache_at_location.  "
							"Could not invert coordinate derivatives; setting gradient to 0");
						for (i = 0; i < field->number_of_components; i++)
						{
							field->values[i] = 0.0;
						}
						return_code = 1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_gradient::evaluate_cache_at_location.  "
						"Cannot invert coordinate derivatives");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_gradient::evaluate_cache_at_location.  "
					"Cannot evaluate source fields");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_gradient::evaluate_cache_at_location.  "
				"Derivatives not available for gradient field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_gradient::evaluate_cache_at_location.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_gradient::evaluate_cache_at_location */


int Computed_field_gradient::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_gradient);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_gradient.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_gradient */

char *Computed_field_gradient::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_gradient::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_gradient_type_string, &error);
		append_string(&command_string, " coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_gradient::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_gradient::get_command_string */

} //namespace

int Computed_field_set_type_gradient(struct Computed_field *field,
	struct Computed_field *source_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type 'gradient' which returns the gradient of <source_field>
w.r.t. <coordinate_field>. Calculation will only succeed in any element with
xi-dimension equal to the number of components in the <coordinate_field>.
Sets the number of components to the product of the number of components in the
<source_field> and <coordinate_field>.
Note the <source_field> does not have to be a scalar. If it has more than 1
component, all the derivatives of its first component w.r.t. the components of
<coordinate_field> will be returned first, followed by those of the second
component, etc. Hence, this function can return the standard gradient of a
scalar source_field, and the deformation gradient if a deformed coordinate field
is passed as the source_field.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_gradient);
	if (field && source_field && coordinate_field &&
		(3 >= coordinate_field->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components *
				coordinate_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_gradient(field);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_gradient */

int Computed_field_get_type_gradient(struct Computed_field *field,
	struct Computed_field **source_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type 'gradient', the <source_field> and <coordinate_field>
used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_gradient);
	if (field && (dynamic_cast<Computed_field_gradient*>(field->core)) &&
		source_field && coordinate_field)
	{
		/* source_fields: 0=source, 1=coordinate */
		*source_field = field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_gradient */

int define_Computed_field_type_gradient(struct Parse_state *state,
	void *field_modify_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type 'gradient', if not already, and allows its contents
to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*field,*source_field;
	Computed_field_derivatives_package *computed_field_derivatives_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_source_field_data;

	ENTER(define_Computed_field_type_gradient);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_derivatives_package=
		(Computed_field_derivatives_package *)
		computed_field_derivatives_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		source_field=(struct Computed_field *)NULL;
		if (computed_field_gradient_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_gradient(field,
				&source_field, &coordinate_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* field */
			set_source_field_data.computed_field_manager=
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				(void *)&set_source_field_data,set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_gradient(field,
					source_field,coordinate_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_gradient.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_gradient */

int Computed_field_register_types_derivatives(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_derivatives_package
		*computed_field_derivatives_package =
		new Computed_field_derivatives_package;

	ENTER(Computed_field_register_type_derivative);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_derivative_type_string, 
			define_Computed_field_type_derivative,
			computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_curl_type_string, 
			define_Computed_field_type_curl,
			computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_divergence_type_string, 
			define_Computed_field_type_divergence,
			computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_gradient_type_string, 
			define_Computed_field_type_gradient,
			computed_field_derivatives_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_derivative */

