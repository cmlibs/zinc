/*******************************************************************************
FILE : computed_field_meanImageFilter.c

LAST MODIFIED : 30 August 2006

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
#include "computed_field/computed_field_meanImageFilter.h"
}
#include "itkImage.h"
#include "itkVector.h"
#include "itkMeanImageFilter.h"
#include <itkDataObject.h>
#include "itkImageRegionIteratorWithIndex.h"

namespace CMISS {

class Computed_field_ImageFilter : public Computed_field_core
{

public:
	int dimension;
	int *sizes;

	Computed_field *texture_coordinate_field;

	itk::Object::Pointer outputImagePtr;

	Computed_field_ImageFilter(Computed_field *field) : Computed_field_core(field)
	{
		Computed_field_get_native_resolution(field->source_fields[0],
			&dimension, &sizes, &texture_coordinate_field);
		ACCESS(Computed_field)(texture_coordinate_field);
		outputImagePtr = NULL;
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

protected:
	template < class ImageType, class FilterType >
	int update_output(Field_location* location, typename FilterType::Pointer filter);

	template < class PixelType >
	inline void assign_field_values( PixelType pixel );

	template < class ImageType >
	int evaluate_filter(Field_location* location);

	template < class ComputedFieldFilter >
	static int select_filter(
		ComputedFieldFilter *filter_class, Field_location* location);

	template < class ComputedFieldFilter >
	static int select_filter_single_component(
		ComputedFieldFilter *filter_class, Field_location* location);

	template < class ComputedFieldFilter >
	static int select_filter_single_component_two_dimensions_plus(
		ComputedFieldFilter *filter_class, Field_location* location);
};

template < class ImageType >
int set_filter(Field_location* location);

template < class ComputedFieldFilter >
int Computed_field_ImageFilter::select_filter(
	ComputedFieldFilter *filter_class, Field_location* location)
/*******************************************************************************
LAST MODIFIED : 7 September 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_meanImageFilter::evaluate_cache_at_location);

	if (filter_class->field && location)
	{
		switch (filter_class->dimension)
		{
			case 1:
			{
				switch (filter_class->field->number_of_components)
				{
					case 1:
					{
						typedef itk::Image< float, 1 > ImageType_1_1;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_1_1 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_1_1 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_1_1 >(location);
						}
					} break;
					case 2:
					{
						typedef itk::Image< itk::Vector< float, 2 > , 1 > ImageType_1_2;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_1_2 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_1_2 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_1_2 >(location);
						}
					} break;
					case 3:
					{
						typedef itk::Image< itk::Vector< float, 3 > , 1 > ImageType_1_3;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_1_3 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_1_3 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_1_3 >(location);
						}
					} break;
					case 4:
					{
						typedef itk::Image< itk::Vector< float, 4 > , 1 > ImageType_1_4;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_1_4 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_1_4 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_1_4 >(location);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
							"Template invocation not declared for number of components %d.",
							filter_class->field->number_of_components);
					} break;
				}
			} break;
			case 2:
			{
				switch (filter_class->field->number_of_components)
				{
					case 1:
					{
						typedef itk::Image< float, 2 > ImageType_2_1;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_2_1 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_2_1 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_2_1 >(location);
						}
					} break;
					case 2:
					{
						typedef itk::Image< itk::Vector< float, 2 > , 2 > ImageType_2_2;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_2_2 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_2_2 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_2_2 >(location);
						}
					} break;
					case 3:
					{
						typedef itk::Image< itk::Vector< float, 3 > , 2 > ImageType_2_3;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_2_3 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_2_3 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_2_3 >(location);
						}
					} break;
					case 4:
					{
						typedef itk::Image< itk::Vector< float, 4 > , 2 > ImageType_2_4;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_2_4 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_2_4 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_2_4 >(location);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
							"Template invocation not declared for number of components %d.",
							filter_class->field->number_of_components);
					} break;
				}
			} break;
			case 3:
			{
				switch (filter_class->field->number_of_components)
				{
					case 1:
					{
						typedef itk::Image< float , 3 > ImageType_3_1;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_3_1 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_3_1 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_3_1 >(location);
						}
					} break;
					case 2:
					{
						typedef itk::Image< itk::Vector< float, 2 > , 3 > ImageType_3_2;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_3_2 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_3_2 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_3_2 >(location);
						}
					} break;
					case 3:
					{
						typedef itk::Image< itk::Vector< float, 3 > , 3 > ImageType_3_3;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_3_3 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_3_3 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_3_3 >(location);
						}
					} break;
					case 4:
					{
						typedef itk::Image< itk::Vector< float, 4 > , 3 > ImageType_3_4;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_3_4 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_3_4 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_3_4 >(location);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
							"Template invocation not declared for number of components %d.",
							filter_class->field->number_of_components);
					} break;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
					"Template invocation not declared for dimension %d.", 
					filter_class->dimension);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_meanImageFilter::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_meanImageFilter::evaluate_cache_at_location */

template < class ComputedFieldFilter >
int Computed_field_ImageFilter::select_filter_single_component(
	ComputedFieldFilter *filter_class, Field_location* location)
/*******************************************************************************
LAST MODIFIED : 7 September 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_meanImageFilter::evaluate_cache_at_location);

	if (filter_class->field && location)
	{
		switch (filter_class->dimension)
		{
			case 1:
			{
				switch (filter_class->field->number_of_components)
				{
					case 1:
					{
						typedef itk::Image< float, 1 > ImageType_1_1;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_1_1 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_1_1 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_1_1 >(location);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
							"Template invocation not declared for number of components %d.",
							filter_class->field->number_of_components);
					} break;
				}
			} break;
			case 2:
			{
				switch (filter_class->field->number_of_components)
				{
					case 1:
					{
						typedef itk::Image< float, 2 > ImageType_2_1;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_2_1 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_2_1 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_2_1 >(location);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
							"Template invocation not declared for number of components %d.",
							filter_class->field->number_of_components);
					} break;
				}
			} break;
			case 3:
			{
				switch (filter_class->field->number_of_components)
				{
					case 1:
					{
						typedef itk::Image< float , 3 > ImageType_3_1;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_3_1 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_3_1 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_3_1 >(location);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
							"Template invocation not declared for number of components %d.",
							filter_class->field->number_of_components);
					} break;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
					"Template invocation not declared for dimension %d.", 
					filter_class->dimension);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_meanImageFilter::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_meanImageFilter::evaluate_cache_at_location */

template < class ComputedFieldFilter >
int Computed_field_ImageFilter::select_filter_single_component_two_dimensions_plus(
	ComputedFieldFilter *filter_class, Field_location* location)
/*******************************************************************************
LAST MODIFIED : 7 September 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_meanImageFilter::evaluate_cache_at_location);

	if (filter_class->field && location)
	{
		switch (filter_class->dimension)
		{
			case 2:
			{
				switch (filter_class->field->number_of_components)
				{
					case 1:
					{
						typedef itk::Image< float, 2 > ImageType_2_1;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_2_1 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_2_1 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_2_1 >(location);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
							"Template invocation not declared for number of components %d.",
							filter_class->field->number_of_components);
					} break;
				}
			} break;
			case 3:
			{
				switch (filter_class->field->number_of_components)
				{
					case 1:
					{
						typedef itk::Image< float , 3 > ImageType_3_1;
						if (!filter_class->outputImagePtr)
						{
							if (return_code = filter_class->set_filter< ImageType_3_1 >
								(location))
							{
								return_code = filter_class->evaluate_filter< ImageType_3_1 >(location);
							}
						}
						else
						{
							return_code = filter_class->evaluate_filter< ImageType_3_1 >(location);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
							"Template invocation not declared for number of components %d.",
							filter_class->field->number_of_components);
					} break;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_meanImageFilter::Computed_field_meanImageFilter.  "
					"Template invocation not declared for dimension %d.", 
					filter_class->dimension);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_meanImageFilter::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_meanImageFilter::evaluate_cache_at_location */

template <class ImageType, class FilterType >
int Computed_field_ImageFilter::update_output(Field_location* location, 
	typename FilterType::Pointer filter)
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
		typename ImageType::Pointer outputImage;

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
				outputImagePtr = outputImage.GetPointer();
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
int Computed_field_ImageFilter::evaluate_filter(Field_location* location)
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
		typename ImageType::Pointer outputImage;

		if (element_xi_location = 
			dynamic_cast<Field_element_xi_location*>(location))
		{
			FE_value* xi  = element_xi_location->get_xi();

			outputImage = dynamic_cast<ImageType*>(outputImagePtr.GetPointer());
			
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

} //CMISS namespace

