/*******************************************************************************
FILE : computed_field_histogram_image_filter.c

LAST MODIFIED : 26 March 2008

DESCRIPTION :
Wraps itk::Statistics::ScalarImageToHistogramGenerator
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
 *   Shane Blackett shane at blackett.co.nz
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
#include "image_processing/computed_field_histogram_image_filter.h"
}
#include "itkImage.h"
#include "itkVector.h"
#include "itkScalarImageToHistogramGenerator.h"
#include "itkImageToHistogramGenerator.h"

using namespace CMISS;

namespace {

char computed_field_histogram_image_filter_type_string[] = "histogram_filter";

class Computed_field_histogram_image_filter : public Computed_field_ImageFilter
{

public:
	/* Number of bins with respect to each component */
	int sourceNumberOfComponents;
	int *numberOfBins;
	double marginalScale;
	int totalPixels;
       
	Computed_field_histogram_image_filter(Computed_field *field,
		int *numberOfBins, double marginalScale);

	~Computed_field_histogram_image_filter()
	{
		if (numberOfBins)
		{
			delete [] numberOfBins;
		}
	};

	template <class ImageType, class HistogramGeneratorType >
	int update_histogram(Field_location* location, 
		typename HistogramGeneratorType::Pointer filter, 
		const typename HistogramGeneratorType::HistogramType *&outputHistogram,
		ImageType *dummytemplarg1, HistogramGeneratorType *dummytemplarg2);

	template <class ImageType, class HistogramGeneratorType >
	int evaluate_histogram(Field_location* location,
		const typename HistogramGeneratorType::HistogramType *outputHistogram,
		ImageType *dummytemplarg, HistogramGeneratorType *dummytemplarg2);

private:
	int get_native_resolution(
		int *native_dimension, int **native_sizes, 
		struct Computed_field **native_texture_coordinate_field)
	{
		int i, return_code;

		ENTER(Computed_field_default_get_native_resolution);
		if (field&&native_dimension&&native_sizes&&
			native_texture_coordinate_field&&
			ALLOCATE(*native_sizes, int, sourceNumberOfComponents))
		{
			*native_dimension = sourceNumberOfComponents;
			for (i = 0 ; i < sourceNumberOfComponents ; i++)
			{
				(*native_sizes)[i] = numberOfBins[i];
			}
			*native_texture_coordinate_field = texture_coordinate_field;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_histogram_image_filter::get_native_resolution.  "
				"Invalid argument(s)");
			return_code = 0;
		}
		LEAVE;
		
		return (return_code);
	} /* Computed_field_default_get_native_resolution */

	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_histogram_image_filter(new_parent,
			numberOfBins, marginalScale);
	}

	char *get_type_string()
	{
		return(computed_field_histogram_image_filter_type_string);
	}

	int compare(Computed_field_core* other_field);

	int list();

	char* get_command_string();

#if !defined (DONOTUSE_TEMPLATETEMPLATES)
	/* This is the normal implementation */
	template < class ComputedFieldFilter >
	int create_histogram_filters_multicomponent_multidimensions(
		ComputedFieldFilter* filter);
#endif // !defined (DONOTUSE_TEMPLATETEMPLATES)

};

template <class ImageType, class HistogramGeneratorType >
int Computed_field_histogram_image_filter::update_histogram(
	Field_location* location, 
	typename HistogramGeneratorType::Pointer filter, 
	const typename HistogramGeneratorType::HistogramType *&outputHistogram,
	ImageType *dummytemplarg1, HistogramGeneratorType *dummytemplarg2)
/*******************************************************************************
LAST MODIFIED : 25 March 2008

DESCRIPTION :
Evaluate the templated version of this filter
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_histogram_image_filter::update_histogram);
	if (field && location)
	{
		typename ImageType::Pointer inputImage;

		if (create_input_image(location, inputImage, dummytemplarg1))
		{
			try
			{
				filter->SetInput( inputImage );

				//Histogram generators are not pipeline objects so
				//we have to explicitly update the input image

				inputImage->Update();

				filter->Compute();
				
				outputHistogram = filter->GetOutput();
				
			} catch ( itk::ExceptionObject & err )
			{
				display_message(ERROR_MESSAGE,
					"ExceptionObject caught!");
				display_message(ERROR_MESSAGE,
					(char *)err.GetDescription());
			}
			
			if (outputHistogram)
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_histogram_image_filter::update_histogram.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_histogram_image_filter::update_histogram */
	
template <class ImageType, class HistogramGeneratorType >
int Computed_field_histogram_image_filter::evaluate_histogram(
	Field_location* location,
	const typename HistogramGeneratorType::HistogramType *outputHistogram,
	ImageType *dummytemplarg, HistogramGeneratorType *dummytemplarg2)
{
	int i, return_code;

	ENTER(Computed_field_histogram_image_filter::evaluate_histogram);
	if (field && location)
	{
		Field_element_xi_location* element_xi_location;
		Field_coordinate_location* coordinate_location = NULL;
		FE_value* xi = NULL;

		if (element_xi_location = 
			dynamic_cast<Field_element_xi_location*>(location))
		{
			xi = element_xi_location->get_xi();
		}
		else if (coordinate_location = 
			dynamic_cast<Field_coordinate_location*>(location))
		{
			xi = coordinate_location->get_values();
		}

		if (xi && outputHistogram)
		{
			unsigned int bin = 0;
			unsigned int offset = 1;

			for (i = 0 ; i < sourceNumberOfComponents ; i++)
			{
				if (xi[i] >= 1.0)
				{
					bin += offset * (numberOfBins[i] - 1);
				}
				else if (xi[i] <= 0.0)
				{
					/* nothing to add */
				}
				else
				{
					bin += offset * static_cast<unsigned int>(
						floor((double)numberOfBins[i] * xi[i]));
				}
				offset *= numberOfBins[i];
			}

			field->values[0] = outputHistogram->GetFrequency( bin ) /
				(double)totalPixels;
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_histogram_image_filter::evaluate_histogram.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_histogram_image_filter::evaluate_histogram */

int Computed_field_histogram_image_filter::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	Computed_field_histogram_image_filter* other;
	int i, return_code;

	ENTER(Computed_field_histogram_image_filter::compare);
	if (field && (other = dynamic_cast<Computed_field_histogram_image_filter*>(other_core)))
	{
		if ((dimension == other->dimension)
			&& (sourceNumberOfComponents == other->sourceNumberOfComponents)
			&& (marginalScale == other->marginalScale))
		{
			return_code = 1;
			for (i = 0 ; i < sourceNumberOfComponents ; i++)
			{
				if (numberOfBins[i] != other->numberOfBins[i])
				{
					return_code = 0;
				}
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
	LEAVE;

	return (return_code);
} /* Computed_field_histogram_image_filter::compare */

int Computed_field_histogram_image_filter::list()
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_histogram_image_filter);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    filter number of bins :");
		for (i = 0 ; i < sourceNumberOfComponents ; i++)
		{
			display_message(INFORMATION_MESSAGE,
				" %d", numberOfBins[i]);
		}
		display_message(INFORMATION_MESSAGE,
			"    filter marginal scale : %g\n", marginalScale);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_histogram_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_histogram_image_filter */

char *Computed_field_histogram_image_filter::get_command_string()
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int i, error;

	ENTER(Computed_field_histogram_image_filter::get_command_string);
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
		sprintf(temp_string, " number_of_bins");
		append_string(&command_string, temp_string, &error);	
		for (i = 0 ; i < sourceNumberOfComponents ; i++)
		{
			sprintf(temp_string, " %d", numberOfBins[i]);
		}
		sprintf(temp_string, " marginal_scale  %g", marginalScale);
		append_string(&command_string, temp_string, &error);	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_histogram_image_filter::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_histogram_image_filter::get_command_string */

template < class ImageType, class HistogramGeneratorType >
class Computed_field_histogram_image_filter_Functor :
	public Computed_field_ImageFilter_Functor
/*******************************************************************************
LAST MODIFIED : 26 March 2008

DESCRIPTION :
This class actually does the work of processing images with the filter.
It is instantiated for each of the chosen ImageTypes.
==============================================================================*/
{
protected:
	Computed_field_histogram_image_filter *histogram_image_filter;

	const typename HistogramGeneratorType::HistogramType * histogram;

	typename HistogramGeneratorType::Pointer filter;

public:

	Computed_field_histogram_image_filter_Functor(
		Computed_field_histogram_image_filter *histogram_image_filter) :
		histogram_image_filter(histogram_image_filter),
		histogram(NULL), filter(NULL)
	{
	}

	int update_and_evaluate_filter(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 26 March 2008

DESCRIPTION :
Updates the outputImage if required and then evaluates the outputImage at the 
location.
==============================================================================*/
	{
		int return_code;
		if (!histogram)
		{
			if (return_code = set_filter(location))
			{
				return_code = histogram_image_filter->evaluate_histogram
					(location, histogram,
					 static_cast<ImageType*>(NULL),
					 static_cast<HistogramGeneratorType*>(NULL));
			}
		}
		else
		{
			return_code = histogram_image_filter->evaluate_histogram
				(location, histogram,
					static_cast<ImageType*>(NULL),
					static_cast<HistogramGeneratorType*>(NULL));
		}
		return(return_code);
	}

	int clear_cache()
	{
		histogram = NULL;
		return (1);
	}

}; /* template < class ImageType > class Computed_field_histogram_image_filter_Functor */

template < class ImageType >
class Computed_field_histogram_scalar_image_filter_Functor :
	public Computed_field_histogram_image_filter_Functor< ImageType,
		itk::Statistics::ScalarImageToHistogramGenerator< ImageType > >
/*******************************************************************************
LAST MODIFIED : 26 March 2008

DESCRIPTION :
This class actually does the work of processing images with the filter.
It is instantiated for each of the chosen ImageTypes.
==============================================================================*/
{
public:
	Computed_field_histogram_scalar_image_filter_Functor(
		Computed_field_histogram_image_filter *histogram_image_filter) :
	Computed_field_histogram_image_filter_Functor< ImageType,
		itk::Statistics::ScalarImageToHistogramGenerator< ImageType > >
	   (histogram_image_filter)
	{
	}

	int set_filter(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 26 March 2008

DESCRIPTION :
Create a filter of the correct type, set the filter specific parameters
and generate the outputImage.
==============================================================================*/
	{
		int return_code;

		typedef itk::Statistics::ScalarImageToHistogramGenerator< ImageType > HistogramGeneratorType;

		this->filter = HistogramGeneratorType::New();

		this->filter->SetNumberOfBins( this->histogram_image_filter->numberOfBins[0] );
		this->filter->SetMarginalScale( this->histogram_image_filter->marginalScale );
		
		return_code = this->histogram_image_filter->update_histogram
			(location, this->filter, this->histogram,
			static_cast<ImageType*>(NULL),
			static_cast<HistogramGeneratorType*>(NULL));
		
		return (return_code);
	} /* set_filter */

}; /* template < class ImageType > class Computed_field_histogram_image_filter_Functor */

template < class SizeType >
inline void setBinSizes( SizeType& binSizes, int* numberOfBins );

template < >
inline void setBinSizes( itk::Size< 2 >& binSizes, int* numberOfBins )
{
	binSizes[0] = numberOfBins[0];
	binSizes[1] = numberOfBins[1];
}

template < >
inline void setBinSizes( itk::Size< 3 >& binSizes, int* numberOfBins )
{
	binSizes[0] = numberOfBins[0];
	binSizes[1] = numberOfBins[1];
	binSizes[2] = numberOfBins[2];
}

template < >
inline void setBinSizes( itk::Size< 4 >& binSizes, int* numberOfBins )
{
	binSizes[0] = numberOfBins[0];
	binSizes[1] = numberOfBins[1];
	binSizes[2] = numberOfBins[2];
	binSizes[3] = numberOfBins[3];
}

template < class ImageType >
class Computed_field_histogram_nonscalar_image_filter_Functor :
	public Computed_field_histogram_image_filter_Functor< ImageType,
		itk::Statistics::ImageToHistogramGenerator< ImageType > >
/*******************************************************************************
LAST MODIFIED : 26 March 2008

DESCRIPTION :
This class actually does the work of processing images with the filter.
It is instantiated for each of the chosen ImageTypes.
==============================================================================*/
{
public:
	Computed_field_histogram_nonscalar_image_filter_Functor(
		Computed_field_histogram_image_filter *histogram_image_filter) :
	Computed_field_histogram_image_filter_Functor<ImageType,
		itk::Statistics::ImageToHistogramGenerator< ImageType > >
	   (histogram_image_filter)
	{
	}

	int set_filter(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 26 March 2008

DESCRIPTION :
Create a filter of the correct type, set the filter specific parameters
and generate the outputImage.
==============================================================================*/
	{
		int return_code;

		typedef itk::Statistics::ImageToHistogramGenerator< ImageType > HistogramGeneratorType;

		this->filter = HistogramGeneratorType::New();

		typename HistogramGeneratorType::SizeType binSizes;
		setBinSizes(binSizes, this->histogram_image_filter->numberOfBins);

		this->filter->SetNumberOfBins( binSizes );
		this->filter->SetMarginalScale(
			this->histogram_image_filter->marginalScale );

		return_code = this->histogram_image_filter->update_histogram
			(location, this->filter, this->histogram,
			static_cast<ImageType*>(NULL),
			static_cast<HistogramGeneratorType*>(NULL));		
		
		return (return_code);
	} /* set_filter */

}; /* template < class ImageType > class Computed_field_histogram_image_filter_Functor */

template < class ComputedFieldFilter >
int Computed_field_histogram_image_filter::create_histogram_filters_multicomponent_multidimensions(
  ComputedFieldFilter* const filter)
/*******************************************************************************
LAST MODIFIED : 7 September 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_ImageFilter::select_filter_single_component);

	switch (dimension)
	{
		case 1:
		{
			switch (sourceNumberOfComponents)
			{
				case 1:
				{
					functor = new Computed_field_histogram_scalar_image_filter_Functor
						< itk::Image< float, 1 > >
						(filter);
					return_code = 1;
				} break;
				case 2:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 2>, 1 > >
						(filter);
					return_code = 1;
				} break;
				case 3:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 3>, 1 > >
						(filter);
					return_code = 1;
				} break;
				case 4:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 4>, 1 > >
						(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_multicomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						sourceNumberOfComponents);
					return_code = 0;
				} break;
			}
		} break;
		case 2:
		{
			switch (sourceNumberOfComponents)
			{
				case 1:
				{
					functor = new Computed_field_histogram_scalar_image_filter_Functor
						< itk::Image< float, 2 > >
						(filter);
					return_code = 1;
				} break;
				case 2:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 2>, 2 > >
						(filter);
					return_code = 1;
				} break;
				case 3:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 3>, 2 > >
						(filter);
					return_code = 1;
				} break;
				case 4:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 4>, 2 > >
						(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_multicomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						sourceNumberOfComponents);
					return_code = 0;
				} break;
			}
		} break;
		case 3:
		{
			switch (sourceNumberOfComponents)
			{
				case 1:
				{
					functor = new Computed_field_histogram_scalar_image_filter_Functor
						< itk::Image< float, 3 > >
						(filter);
					return_code = 1;
				} break;
				case 2:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 2>, 3 > >
						(filter);
					return_code = 1;
				} break;
				case 3:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 3>, 3 > >
						(filter);
					return_code = 1;
				} break;
				case 4:
				{
					functor = new Computed_field_histogram_nonscalar_image_filter_Functor
						< itk::Image< itk::Vector<float, 4>, 3 > >
						(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_multicomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						sourceNumberOfComponents);
					return_code = 0;
				} break;
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_ImageFilter::create_filters_multicomponent_multidimensions.  "
				"Template invocation not declared for dimension %d.", 
				dimension);
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_ImageFilter::create_filters_multicomponent_multidimensions */

Computed_field_histogram_image_filter::Computed_field_histogram_image_filter(
	Computed_field *field, int *numberOfBinsIn, double marginalScale) : 
	Computed_field_ImageFilter(field), marginalScale(marginalScale)
/*******************************************************************************
LAST MODIFIED : 25 March 2008

DESCRIPTION :
Create the computed_field representation of the RescaleIntensityImageFilter.
==============================================================================*/
{
	int i;
	sourceNumberOfComponents = field->source_fields[0]->number_of_components;
	numberOfBins = new int [sourceNumberOfComponents];
	for (i = 0 ; i < sourceNumberOfComponents ; i++)
	{
		numberOfBins[i] = numberOfBinsIn[i];
	}
	/* We need to leave the sizes and dimension for 
		using the functions from Computed_field_ImageFilter::set_input_image */
	if (dimension > 0 && sizes)
	{
		totalPixels = sizes[0];
		for (i = 1 ; i < dimension ; i++)
		{
			totalPixels *= sizes[i];
		}
	}

#if defined DONOTUSE_TEMPLATETEMPLATES
	create_filters_singlecomponent_multidimensions(
		Computed_field_histogram_image_filter_Functor, this);
#else
	create_histogram_filters_multicomponent_multidimensions
		< Computed_field_histogram_image_filter >
		(this);
#endif
}

} //namespace

int Computed_field_set_type_histogram_image_filter(struct Computed_field *field,
	struct Computed_field *source_field, int *numberOfBins, double marginalScale)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
Converts <field> to type histogram.  The <numberOfBins> states how many
bins to divide the minimum and maximum range, the <marginalScale> adds 
1/marginalScale to the upper value.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_histogram_image_filter);
	if (field && source_field)
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 1;
		if (ALLOCATE(source_fields, struct Computed_field *,
			number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			/* Always 1 component returned from a histogram */
			field->number_of_components = 1;
			source_fields[0] = ACCESS(Computed_field)(source_field);
			field->source_fields = source_fields;
			field->number_of_source_fields = number_of_source_fields;			
			Computed_field_ImageFilter* filter_core = new Computed_field_histogram_image_filter(field, numberOfBins, marginalScale);
			if (filter_core->functor)
			{
				field->core = filter_core;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_histogram_image_filter.  "
					"Unable to create image filter.");
				return_code = 0;
			}
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
			"Computed_field_set_type_histogram_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_histogram_image_filter */

int Computed_field_get_type_histogram_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, int **numberOfBins, double *marginalScale)
/*******************************************************************************
LAST MODIFIED : 25 March 2008

DESCRIPTION :
If the field is of type histogram, the source_field and histogram_image_filter
used by it are returned - otherwise an error is reported.
==============================================================================*/
{
	Computed_field_histogram_image_filter* core;
	int i, return_code;

	ENTER(Computed_field_get_type_histogram_image_filter);
	if (field && (core = dynamic_cast<Computed_field_histogram_image_filter*>(field->core))
		&& source_field &&
		ALLOCATE(*numberOfBins, int, core->sourceNumberOfComponents))
	{
		*source_field = field->source_fields[0];
		for (i = 0 ; i < core->sourceNumberOfComponents ; i++)
		{
			(*numberOfBins)[i] = core->numberOfBins[i];
		}
		*marginalScale = core->marginalScale;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_histogram_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_histogram_image_filter */

int define_Computed_field_type_histogram_image_filter(struct Parse_state *state,
	void *field_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
Converts <field> into type DISCRETEGAUSSIAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int i, original_number_of_components, previous_state_index, return_code;
	int *numberOfBins;
	double marginalScale;
	struct Computed_field *field, *source_field;
	struct Computed_field_simple_package *computed_field_simple_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_histogram_image_filter);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_simple_package = (Computed_field_simple_package*)computed_field_simple_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		numberOfBins = (int *)NULL;
		marginalScale = 10.0;
		original_number_of_components = 0;
		if (computed_field_histogram_image_filter_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code =
				Computed_field_get_type_histogram_image_filter(field, &source_field,
					&numberOfBins, &marginalScale);
			original_number_of_components = source_field->number_of_components;
		}
		if (return_code)
		{
			/* must access objects for set functions */
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
					"The histogram_filter field uses the itk::ImageToHistogramGenerator code to generate binned values representing the relative frequency of the various pixel intensities.  There should be a number_of_bins for each component direction, and so the total number of bins will be a product of these, so that for a 3 component image you would get a volume histogram.  If you wanted a histogram for a single component then set the number_of_bins for the other components to 1.");

				/* field */
				set_source_field_data.computed_field_manager =
					computed_field_simple_package->get_computed_field_manager();
				set_source_field_data.conditional_function = Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				/* numberOfBins */
				int dummyNumberOfBins = 255;
				Option_table_add_int_positive_entry(option_table, "number_of_bins",
					&dummyNumberOfBins);
				/* marginalScale */
				Option_table_add_double_entry(option_table, "marginal_scale",
					&marginalScale);

				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}

			if (return_code)
			{
				// store previous state so that we can return to it
				previous_state_index = state->current_index;

				/* parse the source field so we know the dimension. */

				option_table = CREATE(Option_table)();
				/* field */
				set_source_field_data.computed_field_manager =
					computed_field_simple_package->get_computed_field_manager();
				set_source_field_data.conditional_function = Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);

				/* Ignore all the other entries */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);				

				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_histogram_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (source_field->number_of_components != original_number_of_components)
				{
					REALLOCATE(numberOfBins, numberOfBins, int,
						source_field->number_of_components);
					for (i = 0 ; i < source_field->number_of_components ; i++)
					{
						numberOfBins[i] = 64;
					}
				}

				option_table = CREATE(Option_table)();

				/* field, ignore as we have already got it */
				int field_expected_parameters = 1;
				Option_table_add_ignore_token_entry(
					option_table, "field", &field_expected_parameters);
				/* numberOfBins */
				Option_table_add_int_vector_entry(option_table, "number_of_bins",
					numberOfBins, &source_field->number_of_components);
				/* marginalScale */
				Option_table_add_double_entry(option_table, "marginal_scale",
					&marginalScale);

				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);

				if (return_code)
				{
					return_code = Computed_field_set_type_histogram_image_filter(
						field, source_field, numberOfBins, marginalScale);				
				}
			
				if (!return_code)
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_histogram_image_filter.  Failed");
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
			"define_Computed_field_type_histogram_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_histogram_image_filter */

int Computed_field_register_types_histogram_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_histogram_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_histogram_image_filter_type_string, 
			define_Computed_field_type_histogram_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_histogram_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_histogram_image_filter */
