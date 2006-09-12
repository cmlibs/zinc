/*******************************************************************************
FILE : computed_field_ImageFilter.hpp

LAST MODIFIED : 11 September 2006

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation 
equivalent to the scene_viewer assigned to it.
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
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
}
#include "itkImage.h"
#include "itkVector.h"
#include "itkImageRegionIteratorWithIndex.h"

namespace CMISS {

class Computed_field_ImageFilter_Functor
{
public:
   virtual int set_filter(Field_location* location) = 0;

	virtual int evaluate_filter(Field_location* location) = 0;

	virtual int update_and_evaluate_filter(Field_location* location) = 0;

	virtual ~Computed_field_ImageFilter_Functor()
	{
	}
};

class Computed_field_ImageFilter : public Computed_field_core
{

public:
	int dimension;
	int *sizes;

	Computed_field *texture_coordinate_field;

	Computed_field_ImageFilter_Functor* functor;

	Computed_field_ImageFilter(Computed_field *field) : Computed_field_core(field)
	{
		Computed_field_get_native_resolution(field->source_fields[0],
			&dimension, &sizes, &texture_coordinate_field);
		ACCESS(Computed_field)(texture_coordinate_field);
		functor = NULL;
	};

	~Computed_field_ImageFilter()
	{
		if (sizes)
		{
			DEALLOCATE(sizes);
		}
		if (texture_coordinate_field)
		{
			DEACCESS(Computed_field)(&texture_coordinate_field);
		}
	};

	template < class PixelType >
	inline void assign_field_values( PixelType pixel );

protected:

	int evaluate_cache_at_location(Field_location* location);

	template < template <class> class ComputedFieldImageFunctor,
				  class ComputedFieldFilter >
	int create_filters_multicomponent_multidimensions(
		ComputedFieldFilter* filter);

	template < template <class> class ComputedFieldImageFunctor,
				  class ComputedFieldFilter >
	int create_filters_singlecomponent_multidimensions(
		ComputedFieldFilter* filter);

	template < template <class> class ComputedFieldImageFunctor,
				  class ComputedFieldFilter >
	int create_filters_singlecomponent_twoormoredimensions(
		ComputedFieldFilter* filter);

public:
	template <class ImageType, class FilterType >
	int update_output_image(Field_location* location, 
		typename FilterType::Pointer filter,
		typename ImageType::Pointer &outputImage);

	template <class ImageType >
	int evaluate_output_image(Field_location* location,
		typename ImageType::Pointer &outputImage);

};

template < template <class> class ComputedFieldImageFunctor,
	class ComputedFieldFilter >
int Computed_field_ImageFilter::create_filters_multicomponent_multidimensions(
  ComputedFieldFilter* filter)
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
			switch (field->number_of_components)
			{
				case 1:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< float, 1 > >(filter);
					return_code = 1;
				} break;
				case 2:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 2>, 1 > >(filter);
					return_code = 1;
				} break;
				case 3:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 3>, 1 > >(filter);
					return_code = 1;
				} break;
				case 4:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 4>, 1 > >(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_multicomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						field->number_of_components);
					return_code = 0;
				} break;
			}
		} break;
		case 2:
		{
			switch (field->number_of_components)
			{
				case 1:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< float, 2 > >(filter);
					return_code = 1;
				} break;
				case 2:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 2>, 2 > >(filter);
					return_code = 1;
				} break;
				case 3:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 3>, 2 > >(filter);
					return_code = 1;
				} break;
				case 4:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 4>, 2 > >(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_multicomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						field->number_of_components);
					return_code = 0;
				} break;
			}
		} break;
		case 3:
		{
			switch (field->number_of_components)
			{
				case 1:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< float, 3 > >(filter);
					return_code = 1;
				} break;
				case 2:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 2>, 3 > >(filter);
					return_code = 1;
				} break;
				case 3:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 3>, 3 > >(filter);
					return_code = 1;
				} break;
				case 4:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< itk::Vector<float, 4>, 3 > >(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_multicomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						field->number_of_components);
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

template < template <class> class ComputedFieldImageFunctor,
	class ComputedFieldFilter >
int Computed_field_ImageFilter::create_filters_singlecomponent_multidimensions(
  ComputedFieldFilter* filter)
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
			switch (field->number_of_components)
			{
				case 1:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< float, 1 > >(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_singlecomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						field->number_of_components);
					return_code = 0;
				} break;
			}
		} break;
		case 2:
		{
			switch (field->number_of_components)
			{
				case 1:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< float, 2 > >(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_singlecomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						field->number_of_components);
					return_code = 0;
				} break;
			}
		} break;
		case 3:
		{
			switch (field->number_of_components)
			{
				case 1:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< float, 3 > >(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_singlecomponent_multidimensions.  "
						"Template invocation not declared for number of components %d.",
						field->number_of_components);
					return_code = 0;
				} break;
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_ImageFilter::create_filters_singlecomponent_multidimensions.  "
				"Template invocation not declared for dimension %d.", 
				dimension);
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_ImageFilter::create_filters_singlecomponent_multidimensions */
	
template < template <class> class ComputedFieldImageFunctor,
	class ComputedFieldFilter >
int Computed_field_ImageFilter::create_filters_singlecomponent_twoormoredimensions(
  ComputedFieldFilter* filter)
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
		case 2:
		{
			switch (field->number_of_components)
			{
				case 1:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< float, 2 > >(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_singlecomponent_twoormoredimensions.  "
						"Template invocation not declared for number of components %d.",
						field->number_of_components);
					return_code = 0;
				} break;
			}
		} break;
		case 3:
		{
			switch (field->number_of_components)
			{
				case 1:
				{
					functor = new ComputedFieldImageFunctor
						< itk::Image< float, 3 > >(filter);
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_ImageFilter::create_filters_singlecomponent_twoormoredimensions.  "
						"Template invocation not declared for number of components %d.",
						field->number_of_components);
					return_code = 0;
				} break;
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_ImageFilter::create_filters_singlecomponent_twoormoredimensions.  "
				"Template invocation not declared for dimension %d.", 
				dimension);
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_ImageFilter::create_filters_singlecomponent_twoormoredimensions */

template <class ImageType, class FilterType >
int Computed_field_ImageFilter::update_output_image(Field_location* location, 
	typename FilterType::Pointer filter, typename ImageType::Pointer &outputImage)
/*******************************************************************************
LAST MODIFIED : 4 September 2006

DESCRIPTION :
Evaluate the templated version of this filter
==============================================================================*/
{
	int return_code;
	int i;

	ENTER(Computed_field_ImageFilter::evaluate_filter);
	if (field && location)
	{
		Field_element_xi_location* element_xi_location;

		typename ImageType::Pointer inputImage;

		if (element_xi_location = 
			dynamic_cast<Field_element_xi_location*>(location))
		{
			FE_element* element = element_xi_location->get_element();
			FE_value time = element_xi_location->get_time();

			inputImage = ImageType::New();
			typename ImageType::IndexType start;
			for (i = 0 ; i < dimension ; i++)
			{
				start[i] = 0; // first index on X
			}
			typename ImageType::SizeType size;
			for (i = 0 ; i < dimension ; i++)
			{
				size[i] = sizes[i];
			}
			typename ImageType::RegionType region;
			region.SetSize( size );
			region.SetIndex( start );
			
			inputImage->SetRegions(region);
			inputImage->Allocate();
			
			itk::ImageRegionIteratorWithIndex< ImageType >
				generateInput( inputImage, region );
			for ( generateInput.GoToBegin(); !generateInput.IsAtEnd(); ++generateInput)
			{
				typename ImageType::IndexType idx = generateInput.GetIndex();
				
				/* Find element xi for idx, 
					assuming xi field for now. */
				float pixel_xi[3];
				for (i = 0 ; i < dimension ; i++)
				{
					pixel_xi[i] = ((float)idx[i] + 0.5) / (float)sizes[i];
				}
				for (; i < 3 ; i++)
				{
					pixel_xi[i] = 0.0;
				}

				Field_element_xi_location pixel_location(element, pixel_xi, 
					time, /*top_level_element*/(struct FE_element *)NULL);
				Computed_field_evaluate_cache_at_location(
					field->source_fields[0], &pixel_location);
				
				generateInput.Set( field->source_fields[0]->values[0] );
			}
			
			filter->SetInput( inputImage );
			
			try
			{
				filter->Update();
				
				outputImage = filter->GetOutput();
				
			} catch ( itk::ExceptionObject & err )
			{
				display_message(ERROR_MESSAGE,
					"ExceptionObject caught!");
				display_message(ERROR_MESSAGE,
					(char *)err.GetDescription());
			}
			
			if (outputImage)
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
			"Computed_field_ImageFilter::update_output.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_ImageFilter::evaluate_filter */

template < >
inline void Computed_field_ImageFilter::assign_field_values( float pixel )
{
	field->values[0] = pixel;
}

template < >
inline void Computed_field_ImageFilter::assign_field_values( itk::Vector< float, 2 > pixel )
{
	field->values[0] = pixel[0];
	field->values[1] = pixel[1];
}

template < >
inline void Computed_field_ImageFilter::assign_field_values( itk::Vector< float, 3 > pixel )
{
	field->values[0] = pixel[0];
	field->values[1] = pixel[1];
	field->values[2] = pixel[2];
}

template < >
inline void Computed_field_ImageFilter::assign_field_values( itk::Vector< float, 4 > pixel )
{
	field->values[0] = pixel[0];
	field->values[1] = pixel[1];
	field->values[2] = pixel[2];
	field->values[3] = pixel[3];
}

template <class ImageType >
int Computed_field_ImageFilter::evaluate_output_image(Field_location* location,
	typename ImageType::Pointer &outputImage)
/*******************************************************************************
LAST MODIFIED : 4 September 2006

DESCRIPTION :
Evaluate the templated version of this filter
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_ImageFilter::evaluate_filter);
	if (field && location)
	{
		Field_element_xi_location* element_xi_location;

		if (element_xi_location = 
			dynamic_cast<Field_element_xi_location*>(location))
		{
			FE_value* xi  = element_xi_location->get_xi();

			if (outputImage)
			{
				typename ImageType::IndexType index;
				for (i = 0 ; i < dimension ; i++)
				{
					index[i] = (long int)(xi[i] * sizes[i] - 0.5);
				}
				assign_field_values( outputImage->GetPixel( index ) );
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
			"Computed_field_ImageFilter::evaluate_filter.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_ImageFilter::evaluate_filter */

template < class ImageType >
class Computed_field_ImageFilter_FunctorTmpl :
	public Computed_field_ImageFilter_Functor
{
protected:
	typename ImageType::Pointer outputImage;

	Computed_field_ImageFilter* image_filter;

public:

	Computed_field_ImageFilter_FunctorTmpl(
		Computed_field_ImageFilter* image_filter) :
		image_filter(image_filter)
	{
		outputImage = NULL;
	};

	inline int evaluate_filter(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 12 September 2006

DESCRIPTION :
Evaluate a pixel from the outputImage.
==============================================================================*/
	{
		return(image_filter->evaluate_output_image< ImageType >(location,
		   outputImage));
	} /* evaluate_filter */

	int update_and_evaluate_filter(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 12 September 2006

DESCRIPTION :
Updates the outputImage if required and then evaluates the outputImage at the 
location.
==============================================================================*/
	{
		int return_code;
		if (!outputImage)
		{
			if (return_code = set_filter(location))
			{
				return_code = evaluate_filter(location);
			}
		}
		else
		{
			return_code = evaluate_filter(location);
		}
		return(return_code);
	}

};

} //CMISS namespace

