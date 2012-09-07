/***************************************************************************//**
 * FILE : field_module.cpp
 *
 * Internal implementation of field module api.
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#include <string>
//-- extern "C" {
#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_time_sequence.h"
//-- }
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_module.hpp"
//-- extern "C" {
#include "computed_field/computed_field_alias.h"
#include "computed_field/computed_field_arithmetic_operators.h"
#include "computed_field/computed_field_compose.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_conditional.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_deformation.h"
#if defined (USE_ITK)
#include "computed_field/computed_field_derivatives.h"
#endif
#include "computed_field/computed_field_fibres.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_format_output.h"
#include "computed_field/computed_field_function.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_logical_operators.h"
#include "computed_field/computed_field_lookup.h"
#include "computed_field/computed_field_string_constant.h"
#include "computed_field/computed_field_trigonometry.h"
#include "image_processing/computed_field_image_resample.h"
#include "general/mystring.h"
#include "finite_element/finite_element_region.h"
#if defined (USE_ITK)
#include "image_processing/computed_field_threshold_image_filter.h"
#include "image_processing/computed_field_binary_threshold_image_filter.h"
#include "image_processing/computed_field_canny_edge_detection_filter.h"
#include "image_processing/computed_field_mean_image_filter.h"
#include "image_processing/computed_field_sigmoid_image_filter.h"
#include "image_processing/computed_field_discrete_gaussian_image_filter.h"
#include "image_processing/computed_field_curvature_anisotropic_diffusion_image_filter.h"
#include "image_processing/computed_field_derivative_image_filter.h"
#include "image_processing/computed_field_rescale_intensity_image_filter.h"
#include "image_processing/computed_field_connected_threshold_image_filter.h"
#include "image_processing/computed_field_gradient_magnitude_recursive_gaussian_image_filter.h"
#include "image_processing/computed_field_histogram_image_filter.h"
#include "image_processing/computed_field_fast_marching_image_filter.h"
#include "image_processing/computed_field_binary_dilate_image_filter.h"
#include "image_processing/computed_field_binary_erode_image_filter.h"
#endif
#include "region/cmiss_region.h"
#include "general/message.h"
//-- }
#include "computed_field/computed_field_matrix_operators.hpp"
#include "computed_field/computed_field_nodeset_operators.hpp"
#include "computed_field/computed_field_vector_operators.hpp"

/***************************************************************************//**
 * Object to pass into field create functions, supplying region field is to
 * go into and other default parameters.
 */
struct Cmiss_field_module
{
	Cmiss_region *region;
	char *field_name;
	struct Coordinate_system coordinate_system;
	int coordinate_system_override; // true if coordinate system has been set
	Computed_field *replace_field;
	int access_count;
};

struct Cmiss_field_module *Cmiss_field_module_create(struct Cmiss_region *region)
{
	ENTER(Cmiss_field_module_create);
	Cmiss_field_module *field_module = NULL;
	if (region)
	{
		ALLOCATE(field_module, struct Cmiss_field_module, sizeof(struct Cmiss_field_module));
		if (field_module)
		{
			field_module->region = ACCESS(Cmiss_region)(region);
			field_module->field_name = (char *)NULL;
			field_module->replace_field = (Computed_field *)NULL;
			field_module->coordinate_system.type = RECTANGULAR_CARTESIAN;
			field_module->coordinate_system_override = 0;
			field_module->access_count = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_field_module_create.  Missing region");
	}
	LEAVE;

	return (field_module);
};

struct Cmiss_field_module *Cmiss_field_module_access(struct Cmiss_field_module *field_module)
{
	if (field_module)
	{
		field_module->access_count++;
	}
	return field_module;
}

int Cmiss_field_module_destroy(
	struct Cmiss_field_module **field_module_address)
{
	int return_code;
	struct Cmiss_field_module *field_module;

	ENTER(Cmiss_field_module_destroy);
	if (field_module_address && (NULL != (field_module = *field_module_address)))
	{
		field_module->access_count--;
		if (0 == field_module->access_count)
		{
			DEACCESS(Cmiss_region)(&field_module->region);
			DEALLOCATE(field_module->field_name)
			REACCESS(Computed_field)(&field_module->replace_field, NULL);
			DEALLOCATE(*field_module_address);
		}
		*field_module_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_destroy.  Missing field module");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_field_module_destroy */

char *Cmiss_field_module_get_unique_field_name(
	struct Cmiss_field_module *field_module)
{
	struct MANAGER(Computed_field) *manager;
	if (field_module &&
		(manager = Cmiss_region_get_Computed_field_manager(field_module->region)))
	{
		return Computed_field_manager_get_unique_field_name(manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_get_unique_field_name.  Invalid argument(s)");
	}
	return NULL;
}

struct Computed_field *Cmiss_field_module_find_field_by_name(
	struct Cmiss_field_module *field_module, const char *field_name)
{
	struct Computed_field *field;
	struct MANAGER(Computed_field) *manager;

	ENTER(Cmiss_field_module_find_field_by_name);
	if (field_module && field_name && 
		(manager = Cmiss_region_get_Computed_field_manager(field_module->region)))
	{
		field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			(char *)field_name, manager);
		if (field)
		{
			ACCESS(Computed_field)(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_find_field_by_name.  Invalid argument(s)");
		field = (struct Computed_field *)NULL;
	}
	LEAVE;

	return (field);
}

int Cmiss_field_module_contains_field(Cmiss_field_module_id field_module,
	Cmiss_field_id field)
{
	if (field_module && field)
	{
		return (Cmiss_field_module_get_master_region_internal(field_module) ==
			Computed_field_get_region(field));
	}
	return 0;
}

struct Cmiss_region *Cmiss_field_module_get_region_internal(
	struct Cmiss_field_module *field_module)
{
	if (field_module)
	{
		return field_module->region;
	}
	return NULL;
}

struct Cmiss_region *Cmiss_field_module_get_master_region_internal(
	struct Cmiss_field_module *field_module)
{
	if (!field_module)
		return 0;
	Cmiss_region_id region = field_module->region;
	return region;
}

struct Cmiss_region *Cmiss_field_module_get_region(
	struct Cmiss_field_module *field_module)
{
	if (field_module)
	{
		return ACCESS(Cmiss_region)(field_module->region);
	}
	return NULL;
}

int Cmiss_field_module_set_field_name(
	struct Cmiss_field_module *field_module, const char *field_name)
{
	int return_code = 0;
	if (field_module)
	{
		if (field_module->field_name)
		{
			DEALLOCATE(field_module->field_name);
		}
		field_module->field_name = field_name ? duplicate_string(field_name) : NULL;
		return_code = 1;
	}
	return (return_code);
}

char *Cmiss_field_module_get_field_name(
	struct Cmiss_field_module *field_module)
{
	if (field_module && field_module->field_name)
	{
		return duplicate_string(field_module->field_name);
	}
	return NULL;
}

int Cmiss_field_module_set_coordinate_system(
	struct Cmiss_field_module *field_module,
	struct Coordinate_system coordinate_system)
{
	if (field_module)
	{
		field_module->coordinate_system = coordinate_system;
		field_module->coordinate_system_override = 1;
		return 1;
	}
	return 0;
}

struct Coordinate_system Cmiss_field_module_get_coordinate_system(
	struct Cmiss_field_module *field_module)
{
	if (field_module)
	{
		return field_module->coordinate_system;
	}
	// return dummy
	struct Coordinate_system coordinate_system;
	coordinate_system.type = RECTANGULAR_CARTESIAN;
	return (coordinate_system);
}

int Cmiss_field_module_coordinate_system_is_set(
	struct Cmiss_field_module *field_module)
{
	if (field_module)
	{
		return field_module->coordinate_system_override;
	}
	return 0;
}

int Cmiss_field_module_set_replace_field(
	struct Cmiss_field_module *field_module,
	struct Computed_field *replace_field)
{
	int return_code;
	
	if (field_module && ((NULL == replace_field) ||
		(field_module->region == Computed_field_get_region(replace_field))))
	{
		REACCESS(Computed_field)(&field_module->replace_field, replace_field);
		if (replace_field)
		{
			// copy settings from replace_field to be new defaults
			char *field_name = NULL;
			if (GET_NAME(Computed_field)(replace_field, &field_name))
			{
				if (field_module->field_name)
				{
					DEALLOCATE(field_module->field_name);
				}
				field_module->field_name = field_name;
			}
			field_module->coordinate_system = replace_field->coordinate_system;
			field_module->coordinate_system_override = 1;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_set_replace_field.  Invalid arguments");		
		return_code = 0;
	}
	
	return (return_code);
}

struct Computed_field *Cmiss_field_module_get_replace_field(
	struct Cmiss_field_module *field_module)
{
	Computed_field *replace_field = NULL;
	if (field_module)
	{
		replace_field = field_module->replace_field;
	}
	return (replace_field);
}

Cmiss_field_iterator_id Cmiss_field_module_create_field_iterator(
	Cmiss_field_module_id field_module)
{
	if (!field_module)
		return 0;
	MANAGER(Computed_field) *manager = Cmiss_region_get_Computed_field_manager(field_module->region);
	return Computed_field_manager_create_iterator(manager);
}

Cmiss_time_sequence_id Cmiss_field_module_get_matching_time_sequence(
	Cmiss_field_module_id field_module, int number_of_times, double *times)
{
	if (!field_module)
		return NULL;
	return (Cmiss_time_sequence_id)FE_region_get_FE_time_sequence_matching_series(
		Cmiss_region_get_FE_region(field_module->region), number_of_times, times);
}
