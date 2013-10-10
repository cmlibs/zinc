/*******************************************************************************
FILE : computed_field_threshold_image_filter.cpp

LAST MODIFIED : 26 September 2008

DESCRIPTION :
Wraps itk::ThresholdImageFilter

This enables the use of itk to do general thresholding.  The threshold filter
can be used in three different ways.
- specify one threshold value.  All pixels BELOW this value are set to a
  specified outside value
- specify one threshold value.  All pixels ABOVE this value are set to a
  specified outside value
- specify two threshold values.  All pixels OUTSIDE the range defined by the
  threshold values are set to a specified outside value

==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "zinc/fieldimageprocessing.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "image_processing/computed_field_image_filter.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
/* cannot use enumerator_private.h with c++ compiler, use cpp version instead
	eventually should replace enumerator macros with a template */
#include "general/enumerator_private.hpp"
#include "general/mystring.h"
#include "general/message.h"
#include "image_processing/computed_field_threshold_image_filter.h"
#include "itkImage.h"
#include "itkVector.h"
#include "itkThresholdImageFilter.h"

using namespace CMZN;

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(cmzn_field_imagefilter_threshold_mode)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(cmzn_field_imagefilter_threshold_mode));
	switch (enumerator_value)
	{
		case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_BELOW:
		{
			enumerator_string = "below";
		} break;
		case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_ABOVE:
		{
			enumerator_string = "above";
		} break;
		case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_OUTSIDE:
		{
			enumerator_string = "outside";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(cmzn_field_imagefilter_threshold_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(cmzn_field_imagefilter_threshold_mode)

namespace {

	char computed_field_threshold_image_filter_type_string[] = "threshold_filter";

	class Computed_field_threshold_image_filter : public computed_field_image_filter
	{

	public:
		/* to specify the threshold filter we use an enumerated type that
			can take values ABOVE, BELOW or OUTSIDE */
		enum cmzn_field_imagefilter_threshold_mode threshold_mode;

		double outside_value; // used by all modes
		double below_value;   // needed for both below and outside mode
		double above_value;   // neeeded for both above and outside mode

		Computed_field_threshold_image_filter(Computed_field *source_field,
			enum cmzn_field_imagefilter_threshold_mode threshold_mode = CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_BELOW,
			double oustide_value = 0.0, double below_value = 0.5, double above_value = 0.5);

		~Computed_field_threshold_image_filter()
		{
		}

		enum cmzn_field_imagefilter_threshold_mode getMode()
		{
			return threshold_mode;
		}

		int setMode(enum cmzn_field_imagefilter_threshold_mode thresholdModeIn)
		{
			if (threshold_mode != thresholdModeIn)
			{
				threshold_mode = thresholdModeIn;
				clear_cache();
			}
			return CMZN_OK;
		}

		double getOutsideValue()
		{
			return outside_value;
		}

		int setOutsideValue(double outsideValueIn)
		{
			if (outside_value != outsideValueIn)
			{
				outside_value = outsideValueIn;
				clear_cache();
			}
			return CMZN_OK;
		}

		double getBelow()
		{
			return below_value;
		}

		int setBelow(double belowIn)
		{
			if (below_value != belowIn)
			{
				below_value = belowIn;
				clear_cache();
			}
			return CMZN_OK;
		}

		double getAbove()
		{
			return above_value;
		}

		int setAbove(double aboveIn)
		{
			if (above_value != aboveIn)
			{
				above_value = aboveIn;
				clear_cache();
			}
			return CMZN_OK;
		}

	private:
		virtual void create_functor();

		Computed_field_core *copy()
		{
			return new Computed_field_threshold_image_filter(field->source_fields[0],
				threshold_mode, outside_value, below_value, above_value);
		}

		const char *get_type_string()
		{
			return(computed_field_threshold_image_filter_type_string);
		}

		int compare(Computed_field_core* other_field);

		int list();

		char* get_command_string();
	};

	inline Computed_field_threshold_image_filter *
		Computed_field_threshold_image_filter_core_cast(
			cmzn_field_imagefilter_threshold *imagefilter_threshold)
	{
		return (static_cast<Computed_field_threshold_image_filter*>(
			reinterpret_cast<Computed_field*>(imagefilter_threshold)->core));
	}

	int Computed_field_threshold_image_filter::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 8 December 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
	{
		Computed_field_threshold_image_filter* other;
		int return_code;

		ENTER(Computed_field_threshold_image_filter::compare);
		if (field && (other = dynamic_cast<Computed_field_threshold_image_filter*>(other_core)))
		{

			/* could get trickier here and check only the relevant above or below
				values depending on filter type.  eg for an above filter,
				the below value is irrelevant and could be ignored.
				Currently we just use a direct comparison of all set variables. */
			if ((dimension == other->dimension)
				&& (threshold_mode == other->threshold_mode)
				&& (outside_value == other->outside_value)
				&& (below_value == other->below_value)
				&& (above_value == other->above_value))
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
	} /* Computed_field_threshold_image_filter::compare */

	template < class ImageType >
	class Computed_field_threshold_image_filter_Functor :
		public computed_field_image_filter_FunctorTmpl< ImageType >
/*******************************************************************************
LAST MODIFIED : 12 September 2006

DESCRIPTION :
This class actually does the work of processing images with the filter.
It is instantiated for each of the chosen ImageTypes.
==============================================================================*/
	{
		Computed_field_threshold_image_filter *threshold_image_filter;

	public:

		Computed_field_threshold_image_filter_Functor(
			Computed_field_threshold_image_filter *threshold_image_filter) :
			computed_field_image_filter_FunctorTmpl< ImageType >(threshold_image_filter),
			threshold_image_filter(threshold_image_filter)
		{
		}

		int set_filter(cmzn_fieldcache& cache)
/*******************************************************************************
LAST MODIFIED : 8 December 2006

DESCRIPTION :
Create a filter of the correct type, set the filter specific parameters
and generate the outputImage.
==============================================================================*/
		{
			int return_code;

			typedef itk::ThresholdImageFilter< ImageType > FilterType;

			typename FilterType::Pointer filter = FilterType::New();

			filter->SetOutsideValue( threshold_image_filter->outside_value );

			// call appropriate threshold mode based on threshold_mode variable

			switch (threshold_image_filter->threshold_mode)
			{
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_BELOW:
				{
					filter->ThresholdBelow( threshold_image_filter->below_value );
				} break;
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_ABOVE:
				{
					filter->ThresholdAbove( threshold_image_filter->above_value );
				} break;
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_OUTSIDE:
				{
					filter->ThresholdOutside( threshold_image_filter->below_value, threshold_image_filter->above_value );
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Unknown threshold mode");
				} break;
			}

			return_code = threshold_image_filter->update_output_image
				(cache, filter, this->outputImage,
				 static_cast<ImageType*>(NULL),
				 static_cast<FilterType*>(NULL));

			return (return_code);
		} /* set_filter */

	}; /* template < class ImageType > class Computed_field_threshold_image_filter_Functor */

	Computed_field_threshold_image_filter::Computed_field_threshold_image_filter(
		Computed_field *source_field,
		enum cmzn_field_imagefilter_threshold_mode threshold_mode,
		double outside_value, double below_value, double above_value) :
		computed_field_image_filter(source_field),
		threshold_mode(threshold_mode), outside_value(outside_value),
		below_value(below_value),above_value(above_value)
/*******************************************************************************
LAST MODIFIED : 12 September 2006

DESCRIPTION :
Create the computed_field representation of the threshold_image_filter.
==============================================================================*/
	{
	}

	void Computed_field_threshold_image_filter::create_functor()
	{
#if defined DONOTUSE_TEMPLATETEMPLATES
		create_filters_singlecomponent_multidimensions(
			Computed_field_threshold_image_filter_Functor, this);
#else
		create_filters_singlecomponent_multidimensions
			< Computed_field_threshold_image_filter_Functor,
			Computed_field_threshold_image_filter >
			(this);
#endif
	}

	int Computed_field_threshold_image_filter::list()
/*******************************************************************************
LAST MODIFIED : 8 December 2006

DESCRIPTION : list out the threshold filter options
==============================================================================*/
	{
		int return_code = 0;

		ENTER(List_Computed_field_threshold_image_filter);
		if (field)
		{
			display_message(INFORMATION_MESSAGE,
				"    source field : %s\n",field->source_fields[0]->name);

			display_message(INFORMATION_MESSAGE,
			"    threshold mode: %s\n", ENUMERATOR_STRING(cmzn_field_imagefilter_threshold_mode)(threshold_mode));
			display_message(INFORMATION_MESSAGE,
				"    outside_value : %g\n", outside_value);

			switch (threshold_mode)
			{
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_BELOW:
				{
					display_message(INFORMATION_MESSAGE,
						"    below_value : %g\n", below_value);
				} break;
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_ABOVE:
				{
					display_message(INFORMATION_MESSAGE,
						"    above_value : %g\n", above_value);
				} break;
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_OUTSIDE:
				{
					display_message(INFORMATION_MESSAGE,
						"    below_value : %g\n", below_value);
					display_message(INFORMATION_MESSAGE,
						"    above_value : %g\n", above_value);
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"list_Computed_field_threshold_image_filter.  Invalid argument(s)");
			return_code = 0;
		}
		LEAVE;

		return (return_code);
	} /* list_Computed_field_threshold_image_filter */

	char *Computed_field_threshold_image_filter::get_command_string()
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
	{
		char *command_string, *field_name, temp_string[40];
		int error;

		ENTER(Computed_field_threshold_image_filter::get_command_string);
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
			sprintf(temp_string, " %s outside_value %g",
				ENUMERATOR_STRING(cmzn_field_imagefilter_threshold_mode)(threshold_mode), outside_value);
			append_string(&command_string, temp_string, &error);

			switch (threshold_mode)
			{
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_BELOW:
				{
					sprintf(temp_string, " below_value %g", below_value);
				} break;
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_ABOVE:
				{
					sprintf(temp_string, " above_value %g", above_value);
				} break;
				case CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_OUTSIDE:
				{
					sprintf(temp_string, " below_value %g above_value %g",
						below_value, above_value);
				} break;
			}
			append_string(&command_string, temp_string, &error);

		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_threshold_image_filter::get_command_string.  Invalid field");
		}
		LEAVE;

		return (command_string);
	} /* Computed_field_threshold_image_filter::get_command_string */

} //namespace


cmzn_field_imagefilter_threshold_id cmzn_field_cast_imagefilter_threshold(cmzn_field_id field)
{
	if (dynamic_cast<Computed_field_threshold_image_filter*>(field->core))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_imagefilter_threshold_id>(field));
	}
	else
	{
		return (NULL);
	}
}

int cmzn_field_imagefilter_threshold_destroy(
	cmzn_field_imagefilter_threshold_id *imagefilter_threshold_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(
		imagefilter_threshold_address));
}

enum cmzn_field_imagefilter_threshold_mode cmzn_field_imagefilter_threshold_get_mode(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold)
{
	if (imagefilter_threshold)
	{
		Computed_field_threshold_image_filter *filter_core =
			Computed_field_threshold_image_filter_core_cast(imagefilter_threshold);
		return filter_core->getMode();
	}
	return CMZN_FIELD_IMAGEFILTER_THRESHOLD_MODE_BELOW;
}

int cmzn_field_imagefilter_threshold_set_mode(cmzn_field_imagefilter_threshold_id
	imagefilter_threshold, enum cmzn_field_imagefilter_threshold_mode mode)
{
	if (imagefilter_threshold)
	{
		Computed_field_threshold_image_filter *filter_core =
			Computed_field_threshold_image_filter_core_cast(imagefilter_threshold);
		return filter_core->setMode(mode);
	}
	return CMZN_ERROR_ARGUMENT;
}

double cmzn_field_imagefilter_threshold_get_outside_value(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold)
{
	if (imagefilter_threshold)
	{
		Computed_field_threshold_image_filter *filter_core =
			Computed_field_threshold_image_filter_core_cast(imagefilter_threshold);
		return filter_core->getOutsideValue();
	}
	return 0.0;
}

int cmzn_field_imagefilter_threshold_set_outside_value(cmzn_field_imagefilter_threshold_id
	imagefilter_threshold, double outside_value)
{
	if (imagefilter_threshold)
	{
		Computed_field_threshold_image_filter *filter_core =
			Computed_field_threshold_image_filter_core_cast(imagefilter_threshold);
		return filter_core->setOutsideValue(outside_value);
	}
	return CMZN_ERROR_ARGUMENT;
}

double cmzn_field_imagefilter_threshold_get_below(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold)
{
	if (imagefilter_threshold)
	{
		Computed_field_threshold_image_filter *filter_core =
			Computed_field_threshold_image_filter_core_cast(imagefilter_threshold);
		return filter_core->getBelow();
	}
	return 0.0;
}

int cmzn_field_imagefilter_threshold_set_below(cmzn_field_imagefilter_threshold_id
	imagefilter_threshold, double below_value)
{
	if (imagefilter_threshold)
	{
		Computed_field_threshold_image_filter *filter_core =
			Computed_field_threshold_image_filter_core_cast(imagefilter_threshold);
		return filter_core->setBelow(below_value);
	}
	return CMZN_ERROR_ARGUMENT;
}

double cmzn_field_imagefilter_threshold_get_above(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold)
{
	if (imagefilter_threshold)
	{
		Computed_field_threshold_image_filter *filter_core =
			Computed_field_threshold_image_filter_core_cast(imagefilter_threshold);
		return filter_core->getAbove();
	}
	return 0.0;
}

int cmzn_field_imagefilter_threshold_set_above(cmzn_field_imagefilter_threshold_id
	imagefilter_threshold, double above_value)
{
	if (imagefilter_threshold)
	{
		Computed_field_threshold_image_filter *filter_core =
			Computed_field_threshold_image_filter_core_cast(imagefilter_threshold);
		return filter_core->setAbove(above_value);
	}
	return CMZN_ERROR_ARGUMENT;
}

struct Computed_field *cmzn_fieldmodule_create_field_imagefilter_threshold(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field)
{
	Computed_field *field = NULL;
	if (source_field && Computed_field_is_scalar(source_field, (void *)NULL))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_threshold_image_filter(source_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_imagefilter_threshold.  Invalid argument(s)");
	}

	return (field);
}

int cmzn_field_get_type_threshold_image_filter(struct Computed_field *field,
	struct Computed_field **source_field,
	enum cmzn_field_imagefilter_threshold_mode *threshold_mode,
	double *outside_value, double *below_value,	double *above_value)
/*******************************************************************************
LAST MODIFIED : 8 December 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FILTER,
the source_field and thresholds used by it are returned -
otherwise an error is reported.
==============================================================================*/
{
	Computed_field_threshold_image_filter* core;
	int return_code;

	ENTER(cmzn_field_get_type_threshold_image_filter);
	if (field && (core = dynamic_cast<Computed_field_threshold_image_filter*>(field->core))
		&& source_field)
	{
		*source_field = field->source_fields[0];
		*threshold_mode = core->threshold_mode;
		*outside_value = core->outside_value;
		*below_value = core->below_value;
		*above_value = core->above_value;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_get_type_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_field_get_type_threshold_image_filter */

