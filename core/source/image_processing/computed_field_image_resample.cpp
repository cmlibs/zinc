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
#include "zinc/fieldimageprocessing.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element_region.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "image_processing/computed_field_image_resample.h"

namespace {

	const char computed_field_image_resample_type_string[] = "image_resample";

	class Computed_field_image_resample : public Computed_field_core
	{

	public:
		int dimension;  // Should match the dimension of the source field,
						// kept here just to help with iterating through array
		int *sizes; // Resolution in each direction of <dimension>

		Computed_field_image_resample(int dimension, int *sizes_in) :
			Computed_field_core(), dimension(dimension)
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
		Computed_field_core *copy()
		{
			return new Computed_field_image_resample(dimension, sizes);
		}

		int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

		const char *get_type_string()
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
	int i, return_code = 0;

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

int Computed_field_image_resample::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		valueCache.copyValues(*sourceCache);
		return 1;
	}
	return 0;
}

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

Computed_field *Cmiss_field_module_create_image_resample(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, int dimension, int *sizes)
{
	Computed_field *field = NULL;
	if (source_field && dimension && sizes)
	{
		Computed_field *source_texture_coordinate_field = NULL;
		int source_field_dimension = 0;
		int *source_sizes = NULL;
		int return_code = Computed_field_get_native_resolution(
			source_field, &source_field_dimension, &source_sizes,
			&source_texture_coordinate_field);
		DEALLOCATE(source_sizes);
		if (return_code && (dimension == source_field_dimension))
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true,
				source_field->number_of_components,
				/*number_of_source_fields*/1, &source_field,
				/*number_of_source_values*/0, NULL,
				new Computed_field_image_resample(dimension, sizes));
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_field_module_create_image_resample.  "
				"Specified dimension and source field dimension do not match.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_image_resample.  Invalid argument(s)");
	}

	return (field);
}

int Cmiss_field_get_type_image_resample(struct Computed_field *field,
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

	ENTER(Cmiss_field_get_type_image_resample);
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
				"Cmiss_field_get_type_image_resample.  Unable to allocate array.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_get_type_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_field_get_type_image_resample */

