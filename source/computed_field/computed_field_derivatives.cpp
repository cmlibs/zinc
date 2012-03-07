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
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif
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
       
	Computed_field_derivative_image_filter(Computed_field *source_field, 
		int xi_index, int derivative_operator_order);

	~Computed_field_derivative_image_filter()
	{
	};

private:
	virtual void create_functor();

	Computed_field_core *copy()
	{
		return new Computed_field_derivative_image_filter(field->source_fields[0], xi_index, derivative_operator_order);
	}

	const char *get_type_string()
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
	USE_PARAMETER(other_core);
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

	int set_filter(Cmiss_field_cache& cache)
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
			(cache, filter, this->outputImage,
			static_cast<ImageType*>(NULL), static_cast<FilterType*>(NULL));
		
		return (return_code);
	} /* set_filter */

}; /* template < class ImageType > class Computed_field_derivative_image_filter_Functor */

Computed_field_derivative_image_filter::Computed_field_derivative_image_filter(
	Computed_field *source_field, int xi_index, int derivative_operator_order) : 
	Computed_field_ImageFilter(source_field), 
	xi_index(xi_index), derivative_operator_order(derivative_operator_order)
/*******************************************************************************
LAST MODIFIED : 2 July 2007

DESCRIPTION :
Create the ITK implementation for a Computed_field_derivative.
==============================================================================*/
{
}

void Computed_field_derivative_image_filter::create_functor()
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

	Computed_field_derivative(int xi_index) : 
		Computed_field_core(), xi_index(xi_index)
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
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_derivative_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

};

Computed_field_core* Computed_field_derivative::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_derivative* core = new Computed_field_derivative(xi_index);

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
	USE_PARAMETER(other_core);
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

bool Computed_field_derivative::is_defined_at_location(Cmiss_field_cache& cache)
{
	// derivative values are only defined for element_xi locations, and only up to element dimension...
	Field_element_xi_location* element_xi_location;
	if ((element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())) &&
  		(xi_index < element_xi_location->get_dimension()))
	{
		// check the source fields
		return Computed_field_core::is_defined_at_location(cache);
	}
	// ... or image based field derivative with field location
	else if (dynamic_cast<Field_coordinate_location*>(cache.getLocation()))
	{
		/* This can only be valid if the input field has
			a native resolution as we will be using image filter. */
		if (Computed_field_core::is_defined_at_location(cache))
		{
			int dimension;
			int *sizes = static_cast<int *>(NULL);
			Computed_field *texture_coordinate_field;
			bool result = Computed_field_get_native_resolution(
				field->source_fields[0], &dimension, &sizes,
				&texture_coordinate_field);
			if (sizes)
			{
				DEALLOCATE(sizes);
			}
			return result;
		}
	}
	return false;
}

int Computed_field_derivative::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	/* Only works for element_xi locations, or field locations for image-based fields */
	Field_element_xi_location *element_xi_location =
		dynamic_cast<Field_element_xi_location*>(cache.getLocation());
	if (element_xi_location)
	{
		FE_element* element = element_xi_location->get_element();
		int element_dimension=get_FE_element_dimension(element);
		if (xi_index < element_dimension)
		{
			RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluateWithDerivatives(cache, element_dimension));
			if (sourceCache)
			{
				for (int i = 0 ; i < field->number_of_components ; i++)
				{
					valueCache.values[i] = sourceCache->derivatives[i * element_dimension + xi_index];
				}
				valueCache.derivatives_valid = 0;
				return 1;
			}
		}
	}
	else
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
					Computed_field_derivative_image_filter(field->source_fields[0],
						xi_index, /*derivative_operator_order*/1);
				// Note: following attaches another core to the same field
				if (derivative_image_filter)
				{
					derivative_image_filter->attach_to_field(field);
				}
			}
			if (sizes)
			{
				DEALLOCATE(sizes);
			}
			if (derivative_image_filter && derivative_image_filter->evaluate(cache, inValueCache))
			{
				return 1;
			}
		}
	}
	return 0;
}

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

struct Computed_field *Computed_field_create_derivative(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, int xi_index)
{
	Computed_field *field = NULL;
	if (source_field)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_derivative(xi_index));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_derivative.  Missing source field");
	}

	return (field);
}

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
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_derivative);
	USE_PARAMETER(computed_field_derivatives_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		xi_index = 1;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_derivative_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_derivative(field_modify->get_field(),
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
				field_modify->get_field_manager();
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
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_derivative(field_modify->get_field_module(),
						source_field, xi_index - 1));
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
	Computed_field_curl() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_curl();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

};

/***************************************************************************//**
 * Evaluates the curl of a vector field.
 * If function fails to invert the coordinate derivatives then the curl is
 * returned as 0 with a warning - as may happen at certain locations of the mesh.
 * Note currently requires vector_field to be RC.
 */
int Computed_field_curl::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	Field_element_xi_location* element_xi_location;
	//Field_node_location* node_location;
	Cmiss_field_id source_field = getSourceField(0);
	Cmiss_field_id coordinate_field = getSourceField(1);
	/* cannot calculate derivatives for curl yet */
	valueCache.derivatives_valid = 0;
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		FE_element* element = element_xi_location->get_element();
		int element_dimension = get_FE_element_dimension(element);
		FE_element *top_level_element = element_xi_location->get_top_level_element();
		FE_value top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		int top_level_element_dimension = 0;
		FE_element_get_top_level_element_and_xi(element,
			element_xi_location->get_xi(), element_dimension,
			&top_level_element, top_level_xi, &top_level_element_dimension);
		// use the normal cache if already on a top level element, otherwise use extra cache
		Cmiss_field_cache *workingCache = &cache;
		if (top_level_element != element)
		{
			workingCache = valueCache.getOrCreateExtraCache(cache);
			workingCache->setTime(cache.getTime());
			workingCache->setMeshLocation(top_level_element, top_level_xi);
		}

		RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(source_field->evaluateWithDerivatives(*workingCache, top_level_element_dimension));
		RealFieldValueCache *coordinateValueCache = RealFieldValueCache::cast(coordinate_field->evaluateWithDerivatives(*workingCache, top_level_element_dimension));
		if (sourceValueCache && sourceValueCache->derivatives_valid &&
			coordinateValueCache && coordinateValueCache->derivatives_valid)
		{
			FE_value curl,dx_dxi[9],dxi_dx[9],x[3],*source;
			/* curl is only valid in 3 dimensions */
			// Constructor already checked (3==element_dimension)&&(3==coordinate_number_of_components)
			/* only support RC vector fields */
			if (RECTANGULAR_CARTESIAN == source_field->coordinate_system.type)
			{
				Cmiss_field_id coordinate_field = getSourceField(1);
				if (convert_coordinates_and_derivatives_to_rc(&(coordinate_field->coordinate_system),
					coordinate_field->number_of_components, coordinateValueCache->values, coordinateValueCache->derivatives,
					top_level_element_dimension, x, dx_dxi))
				{
					if (invert_FE_value_matrix3(dx_dxi,dxi_dx))
					{
						source=sourceValueCache->derivatives;
						/* curl[0] = dVz/dy - dVy/dz */
						curl=0.0;
						for (int i=0;i<top_level_element_dimension;i++)
						{
							curl += (source[6+i]*dxi_dx[3*i+1] - source[3+i]*dxi_dx[3*i+2]);
						}
						valueCache.values[0]=curl;
						/* curl[1] = dVx/dz - dVz/dx */
						curl=0.0;
						for (int i=0;i<top_level_element_dimension;i++)
						{
							curl += (source[  i]*dxi_dx[3*i+2] - source[6+i]*dxi_dx[3*i  ]);
						}
						valueCache.values[1]=curl;
						/* curl[2] = dVy/dx - dVx/dy */
						curl=0.0;
						for (int i=0;i<top_level_element_dimension;i++)
						{
							curl += (source[3+i]*dxi_dx[3*i  ] - source[  i]*dxi_dx[3*i+1]);
						}
						valueCache.values[2]=curl;
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"Could not invert coordinate derivatives; setting curl to 0");
						for (int i=0;i<field->number_of_components;i++)
						{
							valueCache.values[i]=0.0;
						}
					}
					/* cannot calculate derivatives for curl yet */
					valueCache.derivatives_valid=0;
					return 1;
				}
			}
			else
			{
				//display_message(ERROR_MESSAGE,
				//	"Computed_field_evaluate_curl.  Vector field must be RC");
			}
		}
	}
	return 0;
}

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

struct Computed_field *Computed_field_create_curl(
	struct Cmiss_field_module *field_module,
	struct Computed_field *vector_field, struct Computed_field *coordinate_field)
{
	struct Computed_field *field = NULL;
	if (vector_field && (3 == vector_field->number_of_components) &&
		coordinate_field && (3 == coordinate_field->number_of_components) &&
		(RECTANGULAR_CARTESIAN == vector_field->coordinate_system.type))
	{
		Computed_field *source_fields[2];
		source_fields[0] = vector_field;
		source_fields[1] = coordinate_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			vector_field->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_curl());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_curl.  Invalid argument(s)");
	}

	return (field);
}

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
	struct Computed_field *coordinate_field,*vector_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_curl);
	USE_PARAMETER(computed_field_derivatives_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field = (struct Computed_field *)NULL;
		vector_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_curl_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_curl(field_modify->get_field(),
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
				field_modify->get_field_manager();
			set_coordinate_field_data.conditional_function=
				Computed_field_has_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
				field_modify->get_field_manager();
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
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_curl(field_modify->get_field_module(),
						vector_field, coordinate_field));
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
	Computed_field_divergence() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_divergence();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

};

/***************************************************************************//**
 * Evaluates the divergence of a vector field.
 * If function fails to invert the coordinate derivatives then the divergence is
 * returned as 0 with a warning - as may happen at certain locations of the mesh.
 * Note currently requires vector_field to be RC.
 */
int Computed_field_divergence::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	Field_element_xi_location* element_xi_location;
	//Field_node_location* node_location;
	Cmiss_field_id source_field = getSourceField(0);
	Cmiss_field_id coordinate_field = getSourceField(1);
	/* cannot calculate derivatives for curl yet */
	valueCache.derivatives_valid = 0;
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		FE_element* element = element_xi_location->get_element();
		int element_dimension = get_FE_element_dimension(element);
		FE_element *top_level_element = element_xi_location->get_top_level_element();
		FE_value top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		int top_level_element_dimension = 0;
		FE_element_get_top_level_element_and_xi(element,
			element_xi_location->get_xi(), element_dimension,
			&top_level_element, top_level_xi, &top_level_element_dimension);
		// use the normal cache if already on a top level element, otherwise use extra cache
		Cmiss_field_cache *workingCache = &cache;
		if (top_level_element != element)
		{
			workingCache = valueCache.getOrCreateExtraCache(cache);
			workingCache->setTime(cache.getTime());
			workingCache->setMeshLocation(top_level_element, top_level_xi);
		}

		RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(source_field->evaluateWithDerivatives(*workingCache, top_level_element_dimension));
		RealFieldValueCache *coordinateValueCache = RealFieldValueCache::cast(coordinate_field->evaluateWithDerivatives(*workingCache, top_level_element_dimension));
		if (sourceValueCache && sourceValueCache->derivatives_valid &&
			coordinateValueCache && coordinateValueCache->derivatives_valid)
		{
			FE_value divergence,dx_dxi[9],dxi_dx[9],x[3],*source;
			int coordinate_components = coordinate_field->number_of_components;
			/* Following asks: can dx_dxi be inverted? */
			if (((3==top_level_element_dimension)&&(3==coordinate_components))||
				((RECTANGULAR_CARTESIAN==coordinate_field->coordinate_system.type)
					&&(coordinate_components==top_level_element_dimension))||
				((CYLINDRICAL_POLAR==coordinate_field->coordinate_system.type)&&
					(2==top_level_element_dimension)&&(2==coordinate_components)))
			{
				/* only support RC vector fields */
				if (RECTANGULAR_CARTESIAN == source_field->coordinate_system.type)
				{
					if (convert_coordinates_and_derivatives_to_rc(&(coordinate_field->coordinate_system),
						coordinate_field->number_of_components, coordinateValueCache->values, coordinateValueCache->derivatives,
						top_level_element_dimension, x, dx_dxi))
					{
						/* if the element_dimension is less than 3, put ones on the main
							 diagonal to allow inversion of dx_dxi */
						if (3>top_level_element_dimension)
						{
							dx_dxi[8]=1.0;
							if (2>top_level_element_dimension)
							{
								dx_dxi[4]=1.0;
							}
						}
						if (invert_FE_value_matrix3(dx_dxi,dxi_dx))
						{
							divergence=0.0;
							source=sourceValueCache->derivatives;
							for (int i=0;i<top_level_element_dimension;i++)
							{
								for (int j=0;j<top_level_element_dimension;j++)
								{
									divergence += (*source) * dxi_dx[3*j+i];
									source++;
								}
							}
							valueCache.values[0]=divergence;
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"Could not invert coordinate derivatives; "
								"setting divergence to 0");
							valueCache.values[0]=0.0;
						}
						return 1;
					}
				}
				else
				{
					//display_message(ERROR_MESSAGE,
					//	"Computed_field_divergence::evaluate.  Vector field must be RC");
				}
			}
			else
			{
				//display_message(ERROR_MESSAGE,
				//	"Computed_field_divergence::evaluate.  Elements of wrong dimension");
			}
		}
	}
	return 0;
}

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

struct Computed_field *Computed_field_create_divergence(
	struct Cmiss_field_module *field_module,
	struct Computed_field *vector_field, struct Computed_field *coordinate_field)
{
	struct Computed_field *field = NULL;
	if (vector_field && coordinate_field &&
		(3 >= coordinate_field->number_of_components) &&
		(vector_field->number_of_components ==
			coordinate_field->number_of_components) &&
		(RECTANGULAR_CARTESIAN == vector_field->coordinate_system.type))
	{
		Computed_field *source_fields[2];
		source_fields[0] = vector_field;
		source_fields[1] = coordinate_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_divergence());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_divergence.  Invalid argument(s)");
	}

	return (field);
}

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
	struct Computed_field *coordinate_field,*vector_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_divergence);
	USE_PARAMETER(computed_field_derivatives_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		vector_field=(struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_divergence_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_divergence(field_modify->get_field(),
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
				field_modify->get_field_manager();
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
				field_modify->get_field_manager();
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
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_divergence(field_modify->get_field_module(),
						vector_field, coordinate_field));
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
	Computed_field_gradient() : Computed_field_core()
	{
	};

private:
	~Computed_field_gradient()
	{
	}
	
	Computed_field_core *copy()
	{
		return new Computed_field_gradient();
	}

	const char *get_type_string()
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

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

/***************************************************************************//**
 * Implemented for element_xi and node locations
 */
bool Computed_field_gradient::is_defined_at_location(Cmiss_field_cache& cache)
{
	return ((0 != dynamic_cast<Field_element_xi_location*>(cache.getLocation())) ||
		(0 != dynamic_cast<Field_node_location*>(cache.getLocation()))) &&
		Computed_field_core::is_defined_at_location(cache);
}

int Computed_field_gradient::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	int return_code = 0;
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	Field_element_xi_location* element_xi_location;
	//Field_node_location* node_location;
	Cmiss_field_id source_field = getSourceField(0);
	Cmiss_field_id coordinate_field = getSourceField(1);
	int source_number_of_components = source_field->number_of_components;
	int coordinate_number_of_components = coordinate_field->number_of_components;
	/* cannot calculate derivatives for gradient yet */
	valueCache.derivatives_valid = 0;
	if (0 != (element_xi_location = dynamic_cast<Field_element_xi_location*>(cache.getLocation())))
	{
		FE_element* element = element_xi_location->get_element();
		int element_dimension = get_FE_element_dimension(element);
		FE_element *top_level_element = element_xi_location->get_top_level_element();
		FE_value top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		int top_level_element_dimension = 0;
		FE_element_get_top_level_element_and_xi(element,
			element_xi_location->get_xi(), element_dimension,
			&top_level_element, top_level_xi, &top_level_element_dimension);
		// use the normal cache if already on a top level element, otherwise use extra cache
		Cmiss_field_cache *workingCache = &cache;
		if (top_level_element != element)
		{
			workingCache = valueCache.getOrCreateExtraCache(cache);
			workingCache->setTime(cache.getTime());
			workingCache->setMeshLocation(top_level_element, top_level_xi);
		}

		RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(source_field->evaluateWithDerivatives(*workingCache, top_level_element_dimension));
		RealFieldValueCache *coordinateValueCache = RealFieldValueCache::cast(coordinate_field->evaluateWithDerivatives(*workingCache, top_level_element_dimension));
		if (sourceValueCache && sourceValueCache->derivatives_valid &&
			coordinateValueCache && coordinateValueCache->derivatives_valid)
		{
			/*  Calculate the field. First verify we can invert the derivatives of
				the coordinate field, which we can if dx_dxi is square */
			double a[9], b[3], d;
			int i, indx[3], j;
			if (top_level_element_dimension == coordinate_number_of_components)
			{
				/* fill square matrix a with coordinate field derivatives w.r.t. xi */
				for (i = 0; i < coordinate_number_of_components; i++)
				{
					for (j = 0; j < coordinate_number_of_components; j++)
					{
						a[i*coordinate_number_of_components + j] =
							coordinateValueCache->derivatives[j*coordinate_number_of_components+i];
					}
				}
				/* invert to get derivatives of xi w.r.t. coordinates */
				FE_value *destination = valueCache.values;
				if (LU_decompose(coordinate_number_of_components,a,indx,&d,/*singular_tolerance*/1.0e-12))
				{
					return_code = 1;
					FE_value *source = sourceValueCache->derivatives;
					for (i = 0; i < source_number_of_components; i++)
					{
						for (j = 0; j < coordinate_number_of_components; j++)
						{
							b[j] = (double)(*source);
							source++;
						}
						if (LU_backsubstitute(coordinate_number_of_components,a,indx,b))
						{
							for (j = 0; j < coordinate_number_of_components; j++)
							{
								*destination = (FE_value)b[j];
								destination++;
							}
						}
						else
						{
							return_code = 0;
							break;
						}
					}
				}
				if (!return_code)
				{
					/* cannot invert at apex of heart, so set to zero to allow values
						to be viewed elsewhere in mesh */
					display_message(WARNING_MESSAGE,
						"Computed_field_gradient::evaluate.  "
						"Could not invert coordinate derivatives; setting gradient to 0");
					for (i = 0; i < field->number_of_components; i++)
					{
						destination[i] = 0.0;
					}
					return_code = 1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_gradient::evaluate.  "
					"Cannot invert coordinate derivatives");
			}
		}
	}
	else // if (0 != (node_location = dynamic_cast<Field_node_location*>(cache.getLocation()))) // should work with any location now
	{
		// small efficiency gain can be made by allocating following in valueCache:
		FE_value *down_values = new FE_value[source_number_of_components];
		FE_value *coordinate_values = new FE_value[coordinate_number_of_components];

		Cmiss_field_cache& extraCache = *valueCache.getOrCreateExtraCache(cache);
		Field_location *location = cache.cloneLocation();
		extraCache.setLocation(location);

		/* Do a finite difference calculation varying the coordinate field */
		int i, j, k;
		const FE_value perturb_scale = 1e-5;

		RealFieldValueCache *coordinateValueCache = RealFieldValueCache::cast(coordinate_field->evaluate(extraCache));
		if (coordinateValueCache)
		{
			return_code = 1;
			for (j = 0 ; j < coordinate_number_of_components ; j++)
			{
				coordinate_values[j] = coordinateValueCache->values[j];
			}
			for (i = 0 ; i < coordinate_number_of_components ; i++)
			{
				for (k = 0 ; k < 2 ; k++)
				{
					double sign;
					FE_value *field_values;
					if (k == 0)
					{
						sign = -1;
						field_values = down_values;
					}
					else
					{
						sign = 1;
						field_values = valueCache.values + i * source_number_of_components;
					}
					for (j = 0 ; j < coordinate_number_of_components ; j++)
					{
						if (i == j + 1)
						{
							// Set back the perturbed value from last time
							coordinate_values[j] = coordinateValueCache->values[j];
						}
						if (i == j)
						{
							coordinate_values[j] += sign * perturb_scale * coordinate_values[j];
						}
					}
					/* Set the coordinate field values in cache only and evaluate the source field */
					extraCache.setAssignInCacheOnly(true);
					if (Cmiss_field_assign_real(coordinate_field, &extraCache,
						coordinate_number_of_components, coordinate_values))
					{
						RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(source_field->evaluate(extraCache));
						if (sourceValueCache)
						{
							for (int m = 0; m < source_number_of_components; ++m)
							{
								field_values[m] = sourceValueCache->values[m];
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"Computed_field_gradient::evaluate.  "
								"Unable to evaluate source field when evaluating nodal finite difference.");
							return_code = 0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"Computed_field_gradient::evaluate.  "
							"Unable to set coordinate field when evaluating nodal finite difference.");
						return_code = 0;
					}
					extraCache.setAssignInCacheOnly(false);
				}
				for (j = 0 ; j < source_number_of_components ; j++)
				{
					valueCache.values[i * source_number_of_components + j] =
						(valueCache.values[i * source_number_of_components + j] - down_values[j]) / (2.0 * perturb_scale);
				}
			}
		}
		delete[] down_values;
		delete[] coordinate_values;
	}
	return (return_code);
}

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

struct Computed_field *Computed_field_create_gradient(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, struct Computed_field *coordinate_field)
{
	struct Computed_field *field = NULL;
	if (source_field && coordinate_field &&
		(3 >= coordinate_field->number_of_components))
	{
		int number_of_components = source_field->number_of_components *
			coordinate_field->number_of_components;
		Computed_field *source_fields[2];
		source_fields[0] = source_field;
		source_fields[1] = coordinate_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_gradient());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_gradient.  Invalid argument(s)");
	}

	return (field);
}

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
	struct Computed_field *coordinate_field,*source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_source_field_data;

	ENTER(define_Computed_field_type_gradient);
	USE_PARAMETER(computed_field_derivatives_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		source_field=(struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_gradient_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_gradient(field_modify->get_field(),
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
			Option_table_add_help(option_table,
				"The gradient field calculates the partial derivatives of each of the "
				"source field components with respect to each of the coordinate field components.  "
				"The first values are each of the source field components with respect to the first "
				"coordinate component and then each of the source field components wrt to the second "
				"coordinate component and so on.  "
				"This field can now be used at both element_xi and nodal locations.  At element_xi "
				"locations the basis function supplied xi derivative is multiplied by the basis function "
				"coordinate field jacobian.  For nodal locations a finite difference approximation is "
				"calculated by perturbing each of the coordinate field values and reevaluating the source field.  "
				"See a/graph_axes for an example of using the gradient field to calculate the number of pixels "
				"per ndc coordinate.");

			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* field */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
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
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_gradient(field_modify->get_field_module(),
						source_field, coordinate_field));
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

