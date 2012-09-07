/*******************************************************************************
FILE : computed_field_binary_threshold_image_filter.cpp

LAST MODIFIED : 16 May 2008

DESCRIPTION :
Wraps itk::BinaryThresholdImageFilter
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
#include "image_processing/computed_field_image_filter.h"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "image_processing/computed_field_binary_threshold_image_filter.h"
}
#include "itkImage.h"
#include "itkVector.h"
#include "itkBinaryThresholdImageFilter.h"

using namespace CMISS;

namespace {

char computed_field_binary_threshold_image_filter_type_string[] = "binary_threshold_filter";

class Computed_field_binary_threshold_image_filter : public computed_field_image_filter
{

public:
	double lower_threshold;
	double upper_threshold;

	Computed_field_binary_threshold_image_filter(Computed_field *source_field,
		double lower_threshold, double upper_threshold);

	~Computed_field_binary_threshold_image_filter()
	{
	}

private:
	virtual void create_functor();

	Computed_field_core *copy()
	{
		return new Computed_field_binary_threshold_image_filter(field->source_fields[0],
			lower_threshold, upper_threshold);
	}

	const char *get_type_string()
	{
		return(computed_field_binary_threshold_image_filter_type_string);
	}

	int compare(Computed_field_core* other_field);

	int list();

	char* get_command_string();
};

/*****************************************************************************//**
 * Compare the type specific data
 * 
 * @param other_core Core of other field to compare against
 * @return Return code indicating field is the same (1) or different (0)
*/
int Computed_field_binary_threshold_image_filter::compare(Computed_field_core *other_core)
{
	Computed_field_binary_threshold_image_filter* other;
	int return_code;

	ENTER(Computed_field_binary_threshold_image_filter::compare);
	if (field && (other = dynamic_cast<Computed_field_binary_threshold_image_filter*>(other_core)))
	{
		if ((dimension == other->dimension)
			&& (lower_threshold == other->lower_threshold)
			&& (upper_threshold == other->upper_threshold))
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
} /* Computed_field_binary_threshold_image_filter::compare */

template < class ImageType >
class Computed_field_binary_threshold_image_filter_Functor :
	public computed_field_image_filter_FunctorTmpl< ImageType >
/*******************************************************************************
LAST MODIFIED : 12 September 2006

DESCRIPTION :
This class actually does the work of processing images with the filter.
It is instantiated for each of the chosen ImageTypes.
==============================================================================*/
{
	Computed_field_binary_threshold_image_filter *binary_threshold_image_filter;

public:

	Computed_field_binary_threshold_image_filter_Functor(
		Computed_field_binary_threshold_image_filter *binary_threshold_image_filter) :
		computed_field_image_filter_FunctorTmpl< ImageType >(binary_threshold_image_filter),
		binary_threshold_image_filter(binary_threshold_image_filter)
	{
	}

/*****************************************************************************//**
 * Create a filter of the correct type, set the filter specific parameters
 * and generate the output
 * 
 * @param location Field location
 * @return Return code indicating succes (1) or failure (0)
*/
	int set_filter(Cmiss_field_cache& cache)
	{
		int return_code;
		
		typedef itk::BinaryThresholdImageFilter< ImageType , ImageType > FilterType;
		
		typename FilterType::Pointer filter = FilterType::New();
		
		filter->SetLowerThreshold( binary_threshold_image_filter->lower_threshold );
		filter->SetUpperThreshold( binary_threshold_image_filter->upper_threshold );
		
		return_code = binary_threshold_image_filter->update_output_image
			(cache, filter, this->outputImage,
			static_cast<ImageType*>(NULL), static_cast<FilterType*>(NULL));
		
		return (return_code);
	} /* set_filter */

}; /* template < class ImageType > class Computed_field_binary_threshold_image_filter_Functor */


/*****************************************************************************//**
 * Constructor for a binary threshold image filter field
 * Creates the computed field representation of the binary threshold image filter
 * 
 * @param field Generic computed field
 * @param lower_threshold Lower threshold value (below which values will be set to 0)
 * @param upper_threshold Upper threshold value (above which values will be set to 0)
 * @return Return code indicating succes (1) or failure (0)
*/
Computed_field_binary_threshold_image_filter::Computed_field_binary_threshold_image_filter(
	Computed_field *source_field, double lower_threshold, double upper_threshold) :
	computed_field_image_filter(source_field),
	lower_threshold(lower_threshold), upper_threshold(upper_threshold)
{
}

void Computed_field_binary_threshold_image_filter::create_functor()
{
#if defined DONOTUSE_TEMPLATETEMPLATES
	create_filters_singlecomponent_multidimensions(
		Computed_field_binary_threshold_image_filter_Functor, this);
#else
	create_filters_singlecomponent_multidimensions
		< Computed_field_binary_threshold_image_filter_Functor,
		Computed_field_binary_threshold_image_filter >
		(this);
#endif
}

/*****************************************************************************//**
 * List field parameters
 * 
 * @return Return code indicating succes (1) or failure (0)
*/
int Computed_field_binary_threshold_image_filter::list()
{
	int return_code = 0;

	ENTER(List_Computed_field_binary_threshold_image_filter);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    lower_threshold : %g\n", lower_threshold);
		display_message(INFORMATION_MESSAGE,
			"    upper_threshold : %g\n", upper_threshold);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_binary_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_binary_threshold_image_filter */

/*****************************************************************************//**
 * Returns allocated command string for reproducing field. Includes type.
 * 
 * @return Command string to create field
*/
char *Computed_field_binary_threshold_image_filter::get_command_string()
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_binary_threshold_image_filter::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, get_type_string(), &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " lower_threshold %g", lower_threshold);
		append_string(&command_string, temp_string, &error);		
		sprintf(temp_string, " upper_threshold %g", upper_threshold);	
		append_string(&command_string, temp_string, &error);		
}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_binary_threshold_image_filter::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_binary_threshold_image_filter::get_command_string */

} //namespace

/*****************************************************************************//**
 * If field can be cast to a COMPUTED_FIELD_BINARY_THRESHOLD_IMAGE_FILTER do so
 * and return the field.  Otherwise return NULL.
 * 
 * @param field Id of the field to cast
 * @return Id of the cast field, or NULL
*/
Cmiss_field_binary_threshold_image_filter_id Cmiss_field_cast_binary_threshold_image_filter(Cmiss_field_id field)
{
	if (dynamic_cast<Computed_field_binary_threshold_image_filter*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_binary_threshold_image_filter_id>(field));
	}
	else
	{
		return (NULL);
	}
}

struct Computed_field *Cmiss_field_module_create_binary_threshold_image_filter(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, double lower_threshold,
	double upper_threshold)
{
	Computed_field *field = NULL;
	if (source_field && Computed_field_is_scalar(source_field, (void *)NULL))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_binary_threshold_image_filter(source_field,
				lower_threshold, upper_threshold));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_binary_threshold_image_filter.  Invalid argument(s)");
	}

	return (field);
}

int Cmiss_field_get_type_binary_threshold_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, double *lower_threshold,
	double *upper_threshold)
{
	Computed_field_binary_threshold_image_filter* core;
	int return_code;

	ENTER(Cmiss_field_get_type_binary_threshold_image_filter);
	if (field && (core = dynamic_cast<Computed_field_binary_threshold_image_filter*>(field->core))
		&& source_field)
	{
		*source_field = field->source_fields[0];
		*lower_threshold = core->lower_threshold;
		*upper_threshold = core->upper_threshold;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_get_type_binary_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_field_get_type_binary_threshold_image_filter */

