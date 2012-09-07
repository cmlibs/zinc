/*******************************************************************************
FILE : computed_field_compose.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a computed_field that uses evaluates one field, does a
"find element_xi" look up on a field in a host element group to find the same 
values and then evaluates a third field at that location.
Essentially it is used to embed one mesh in the elements of another.
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
//-- extern "C" {
#include "api/cmiss_field_module.h"
#include "computed_field/computed_field.h"
//-- }
#include "computed_field/computed_field_private.hpp"
//-- extern "C" {
#include "computed_field/computed_field_set.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "computed_field/computed_field_compose.h"
//-- }
#include "computed_field/field_module.hpp"
#include "mesh/cmiss_element_private.hpp"

class Computed_field_compose_package : public Computed_field_type_package
{
public:
	struct Cmiss_region *root_region;
};

namespace {

char computed_field_compose_type_string[] = "compose";

class Computed_field_compose : public Computed_field_core
{
private:
	Cmiss_mesh_id mesh;
	int find_nearest;
	int use_point_five_when_out_of_bounds;

public:
	Computed_field_compose(Cmiss_mesh_id search_mesh,
			int find_nearest, int use_point_five_when_out_of_bounds = 0) :
		Computed_field_core(),
		mesh(Cmiss_mesh_access(search_mesh)),
		find_nearest(find_nearest),
		use_point_five_when_out_of_bounds(use_point_five_when_out_of_bounds)
	{		
	};
		
	virtual ~Computed_field_compose();

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			/* inherit coordinate system from third source field */
			Computed_field *calculate_values_field = field->source_fields[2];
			Computed_field_set_coordinate_system(field,
				Computed_field_get_coordinate_system(calculate_values_field));
		}
	}

	int get_type(
		struct Computed_field **texture_coordinate_field_address,
		struct Computed_field **find_element_xi_field_address,
		struct Computed_field **calculate_values_field_address,
		Cmiss_mesh_id *mesh_address, int *find_nearest_address,
		int *use_point_five_when_out_of_bounds_address);

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_compose_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

Computed_field_compose::~Computed_field_compose()
{
	Cmiss_mesh_destroy(&mesh);
}

Computed_field_core *Computed_field_compose::copy()
{
	return new Computed_field_compose(mesh, find_nearest, use_point_five_when_out_of_bounds);
}

int Computed_field_compose::compare(Computed_field_core *other_core)
{
	int return_code;
	Computed_field_compose *other;

	ENTER(Computed_field_compose::type_specific_contents_match);
	if (field && (0 != (other = dynamic_cast<Computed_field_compose*>(other_core))))
	{
		if (Cmiss_mesh_match(mesh, other->mesh) &&
			(find_nearest == other->find_nearest) &&
			(use_point_five_when_out_of_bounds == 
				other->use_point_five_when_out_of_bounds))
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
} /* Computed_field_compose::compare */

bool Computed_field_compose::is_defined_at_location(Cmiss_field_cache& cache)
{
	return (0 != field->evaluate(cache));
}

int Computed_field_compose::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	int return_code = 0;
	RealFieldValueCache *coordinateValueCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (coordinateValueCache)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		Cmiss_field_cache& extraCache = *valueCache.getExtraCache();
		extraCache.setTime(cache.getTime());
		/* The values from the first source field are inverted in the
			second source field to get element_xi which is evaluated with
			the third source field */
		Cmiss_element_id compose_element =0;
		FE_value compose_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		if (Computed_field_find_element_xi(getSourceField(1), &extraCache,
			coordinateValueCache->values,
			coordinateValueCache->componentCount,
			&compose_element, compose_xi,
			mesh, /*propagate_field*/0, find_nearest)
			&& compose_element)
		{
			/* calculate the third source_field at this new location */
			extraCache.setMeshLocation(compose_element, compose_xi);
			RealFieldValueCache *calculateValueCache = RealFieldValueCache::cast(getSourceField(2)->evaluate(extraCache));
			if (calculateValueCache)
			{
				return_code = 1;
				for (int i=0;i<field->number_of_components;i++)
				{
					valueCache.values[i] = calculateValueCache->values[i];
				}
			}
		}
		else
		{
			if (use_point_five_when_out_of_bounds)
			{
				/* Actually don't fail here, just make the values constant so that
					people can compose outside the valid range */
				return_code = 1;
				for (int i=0;i<field->number_of_components;i++)
				{
					valueCache.values[i]=0.5;
				}
			}
			else
			{
				return_code = 0;
			}
		}
		valueCache.derivatives_valid = 0;
	}
	return return_code;
}

int Computed_field_compose::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;

	ENTER(Computed_field_compose::list);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    texture coordinates field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    find element xi field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE, "    mesh : ");
		char *mesh_name = Cmiss_mesh_get_name(mesh);
		display_message(INFORMATION_MESSAGE, "%s\n", mesh_name);
		DEALLOCATE(mesh_name);
		display_message(INFORMATION_MESSAGE,"    calculate values field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[2]->name);
		if (find_nearest)
		{
			display_message(INFORMATION_MESSAGE,"    find nearest match\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"    find exact match\n");
		}
		if (use_point_five_when_out_of_bounds)
		{
			display_message(INFORMATION_MESSAGE,"    use point five when out of bounds\n");
		}
		display_message(INFORMATION_MESSAGE,"    element dimension %d\n", Cmiss_mesh_get_dimension(mesh));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose::list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

char *Computed_field_compose::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_compose::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_compose_type_string, &error);
		append_string(&command_string, " texture_coordinates_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " find_element_xi_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " mesh ", &error);
		char *mesh_name = Cmiss_mesh_get_name(mesh);
		append_string(&command_string, mesh_name, &error);
		DEALLOCATE(mesh_name);
		append_string(&command_string, " calculate_values_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		if (find_nearest)
		{
			append_string(&command_string, " find_nearest", &error);
		}
		if (use_point_five_when_out_of_bounds)
		{
			append_string(&command_string, " use_point_five_when_out_of_bounds", &error);
		}
		sprintf(temp_string, " element_dimension %d", Cmiss_mesh_get_dimension(mesh));
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_compose::get_command_string */

} //namespace

/***************************************************************************//**
 * Creates a field of type COMPUTED_FIELD_COMPOSE. This field allows you to
 * evaluate one field to find "texture coordinates", use a find_element_xi field
 * to then calculate a corresponding element/xi and finally calculate values
 * using this element/xi and a third field.  You can then evaluate values on a
 * "host" mesh for any points "contained" inside.  The <search_mesh> is the
 * domain from which any returned element_xi will belong.
 * If <use_point_five_when_out_of_bounds> is true then if the
 * texture_coordinate_field values cannot be found in the find_element_xi_field,
 * then instead of returning failure, the values will be set to 0.5 and returned
 * as success.
 * NOTE: this field type has been superceded by find_mesh_location combined with
 * embedded field. DO NOT add to external API.
 */
Computed_field *Computed_field_create_compose(Cmiss_field_module *field_module,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *find_element_xi_field,
	struct Computed_field *calculate_values_field,
	Cmiss_mesh_id search_mesh,
	int find_nearest, int use_point_five_when_out_of_bounds)
{
	Computed_field *field = NULL;

	ENTER(Computed_field_create_compose);
	if (texture_coordinate_field && texture_coordinate_field->isNumerical() &&
		find_element_xi_field && find_element_xi_field->isNumerical() &&
		calculate_values_field && calculate_values_field->isNumerical() &&
		search_mesh &&
		(Cmiss_mesh_get_master_region_internal(search_mesh) ==
			Cmiss_field_module_get_master_region_internal(field_module)))
	{
		if (texture_coordinate_field->number_of_components ==
			find_element_xi_field->number_of_components)
		{
			if (Computed_field_is_find_element_xi_capable(
				find_element_xi_field, /*dummy*/NULL))
			{
				Computed_field *source_fields[3];
				source_fields[0] = texture_coordinate_field;
				source_fields[1] = find_element_xi_field;
				source_fields[2] = calculate_values_field;
				field = Computed_field_create_generic(field_module,
					/*check_source_field_regions*/true,
					calculate_values_field->number_of_components,
					/*number_of_source_fields*/3, source_fields,
					/*number_of_source_values*/0, NULL,
					new Computed_field_compose(search_mesh, find_nearest,
						use_point_five_when_out_of_bounds));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_compose.  "
					"The type of find_element_xi_field supplied has not "
					"been implemented for find_element_xi calculations.");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_compose.  "
				"The texture_coordinate_field and find_element_xi_field "
				"must have the same number of components");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_compose.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_compose */

/***************************************************************************//**
 * If the field is of type COMPUTED_FIELD_COMPOSE, the function returns the
 * three fields which define the field.
 * Note that the fields and mesh are not ACCESSed.
 */
int Computed_field_compose::get_type(
	struct Computed_field **texture_coordinate_field_address,
	struct Computed_field **find_element_xi_field_address,
	struct Computed_field **calculate_values_field_address,
	Cmiss_mesh_id *mesh_address, int *find_nearest_address,
	int *use_point_five_when_out_of_bounds_address)
{
	int return_code;
	if (field && texture_coordinate_field_address &&
		find_element_xi_field_address && calculate_values_field_address &&
		mesh_address && find_nearest_address &&
		use_point_five_when_out_of_bounds_address)
	{
		*texture_coordinate_field_address = field->source_fields[0];
		*find_element_xi_field_address = field->source_fields[1];
		*calculate_values_field_address = field->source_fields[2];
		*mesh_address = mesh;
		*find_nearest_address = find_nearest;
		*use_point_five_when_out_of_bounds_address = use_point_five_when_out_of_bounds;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose::get_type.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

