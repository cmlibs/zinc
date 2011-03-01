/*****************************************************************************//**
 * FILE : computed_field_cad_geometry.cpp
 *
 * Implements a cmiss field which wraps an OpenCASCADE shape.
 *
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
#include <stdlib.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
#include "computed_field/computed_field.h"
}

#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"

#include "cad/computed_field_cad_geometry.h"
#include "cad/computed_field_cad_topology.h"
#include "cad/computed_field_cad_topology_private.h"
#include "cad/geometricshape.h"
#include "cad/field_location.hpp"
/*
Module types
------------
*/
/*
class Computed_field_cad_geometry_package : public Computed_field_type_package
{
public:
	Cmiss_region *root_region;

	Computed_field_cad_geometry_package(Cmiss_region *root_region)
	  : root_region(root_region)
	{
		ACCESS(Cmiss_region)(root_region);
	}

	~Computed_field_cad_geometry_package()
	{
		DEACCESS(Cmiss_region)(&root_region);
	}
};
*/
namespace {

char computed_field_cad_geometry_type_string[] = "cad_geometry";

class Computed_field_cad_geometry : public Computed_field_core
{
public:

	Computed_field_cad_geometry()
		: Computed_field_core()
	{
	}

	~Computed_field_cad_geometry()
	{
		for ( int i = 0; i < field->number_of_source_fields; i++ )
		{
			DEACCESS(Computed_field)(&(field->source_fields[i]));
		}
		DEALLOCATE(field->source_fields);
	}

private:
	Computed_field_core* copy();

	const char* get_type_string()
	{
		return (computed_field_cad_geometry_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	//int has_multiple_times() {return 1;}

	char* get_command_string();

	virtual int get_attribute_integer(enum Cmiss_field_attribute_id attribute_id) const
	{
		if (attribute_id == CMISS_FIELD_ATTRIBUTE_IS_COORDINATE)
			return 1;
		return 0;
	}

};

/***************************************************************************//**
 * Copy the type specific data used by this type.
 */
Computed_field_core* Computed_field_cad_geometry::copy()
{
	Computed_field_cad_geometry* core = 
		new Computed_field_cad_geometry();

	return (core);
} /* Computed_field_cad_geometry::copy */

/***************************************************************************//**
 * Compare the type specific data.
 */
int Computed_field_cad_geometry::compare(Computed_field_core *other_core)
{
	int return_code;

	ENTER(Computed_field_cad_geometry::compare);
	if (field && dynamic_cast<Computed_field_cad_geometry*>(other_core))
	{
		printf("Comparing a cad geometry field\n");
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cad_geometry::compare */

/***************************************************************************//**
 * Evaluate the values of the field at the supplied location.
 */
int Computed_field_cad_geometry::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code = 0;

	if (field && field->source_fields[0])
	{
		Field_cad_geometry_surface_location *cad_geometry_surface_location = dynamic_cast<Field_cad_geometry_surface_location*>(location);
		Field_cad_geometry_curve_location *cad_geometry_curve_location = dynamic_cast<Field_cad_geometry_curve_location*>(location);
		if (cad_geometry_surface_location)
		{
			//printf("compare? 0x%x and 0x%x\n", field->source_fields[0], cad_geometry_location->id());
			//Computed_field_cad_geometry *cad_geometry = static_cast<Computed_field_cad_geometry*>(field->core);
			//field->source_fields[0]->core->evaluate_cache_at_location(location);
			double point[3], uDerivative[3], vDerivative[3];
			Cmiss_field_cad_topology_id cad_topology = Cmiss_field_cast_cad_topology(field->source_fields[0]);
			if ((field->source_fields[0] == (Computed_field *)cad_geometry_surface_location->get_id()) &&
				Computed_field_cad_topology_get_surface_point(cad_topology,
					cad_geometry_surface_location->get_identifier(),
					cad_geometry_surface_location->get_u(),
					cad_geometry_surface_location->get_v(),
					point, uDerivative, vDerivative))
			{
				field->values[0] = point[0];
				field->values[1] = point[1];
				field->values[2] = point[2];
				if (location->get_number_of_derivatives())
				{
					//printf("START ", location->get_number_of_derivatives());
					//printf("(%.2f,%.2f,%.2f) -", uDerivative[0], uDerivative[1], uDerivative[2]);
					//printf(" (%.2f,%.2f,%.2f)\n", vDerivative[0], vDerivative[1], vDerivative[2]);
					field->derivatives[0] = uDerivative[0];
					field->derivatives[2] = uDerivative[1];
					field->derivatives[4] = uDerivative[2];
					field->derivatives[1] = vDerivative[0];
					field->derivatives[3] = vDerivative[1];
					field->derivatives[5] = vDerivative[2];
					field->derivatives_valid = 1;
				}
				//printf( "START[%.2f, %.2f , %.2f] ", field->values[0], field->values[1], field->values[2] );
				return_code = 1;
			}
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(&cad_topology));
		}
		else if (cad_geometry_curve_location)
		{
			double point[3];
			Cmiss_field_cad_topology_id cad_topology = Cmiss_field_cast_cad_topology(field->source_fields[0]);
			if ((field->source_fields[0] == (Computed_field *)cad_geometry_curve_location->get_id()) &&
				Computed_field_cad_topology_get_curve_point(cad_topology,
					cad_geometry_curve_location->get_identifier(),
					cad_geometry_curve_location->get_s(),
					point))
			{
				field->values[0] = point[0];
				field->values[1] = point[1];
				field->values[2] = point[2];
				//printf("START: (%.2f,%.2f,%.2f)\n", point[0], point[1], point[2]);
				return_code = 1;
			}
			Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(&cad_topology));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_geometry::evaluate_cache_at_location.  Invalid argument(s)");
	}

	return (return_code);
} /* Computed_field_cad_geometry::evaluate_cache_at_location */

/***************************************************************************//**
 * Writes type-specific details of the field to the console.
 */
int Computed_field_cad_geometry::list()
{
	//char *field_name;
	int return_code;

	ENTER(List_Computed_field_cad_geometry);
	if (field)
	{
		Cmiss_field_cad_topology_id cad_topology = Cmiss_field_cast_cad_topology(field->source_fields[0]);
		int surface_count = Cmiss_field_cad_topology_get_surface_count(cad_topology);
		int curve_count = Cmiss_field_cad_topology_get_curve_count(cad_topology);
		display_message(INFORMATION_MESSAGE, "    Cad geometry field : # of surfaces: %d, # of curves: %d", surface_count, curve_count);
		Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(&cad_topology));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cad_geometry.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cad_geometry */

/***************************************************************************//**
 * Returns allocated command string for reproducing this field. Includes type.
 */
char *Computed_field_cad_geometry::get_command_string()
{
	char *command_string = 0, *field_name = 0;
	int error = 0;

	ENTER(Computed_field_cad_geometry::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, computed_field_cad_geometry_type_string, &error);
		append_string(&command_string, " field has no command string ", &error);
		if (GET_NAME(Computed_field)(field, &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_geometry::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cad_geometry::get_command_string */

} //namespace

int Cmiss_field_is_cad_geometry( struct Computed_field *field, void *not_in_use )
{
	int return_code = 0;
	USE_PARAMETER(not_in_use);
	//printf( "Checking domain ...\n" );
	if ( field )
	{
		//printf( "Valid field ...\n" );
		if ( NULL != dynamic_cast<Computed_field_cad_geometry*>(field->core) )
		{
			//printf( "Yes, it is a cad geometry field.\n" );
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_cad_geometry.  Invalid argument(s)");
	}

	return return_code;
}

Cmiss_field_cad_geometry_id Cmiss_field_cad_geometry_cast(Cmiss_field_id field)
{
	if (dynamic_cast<Computed_field_cad_geometry*>(field->core))
	{
		return (reinterpret_cast<Cmiss_field_cad_geometry_id>(field));
	}
	else
	{
		return (NULL);
	}
}

/**
 * @see Cmiss_field_create_cad_geometry
 */
Cmiss_field_id Computed_field_module_create_cad_geometry(Cmiss_field_module_id field_module, Cmiss_field_id cad_topology_source_field)
//Cmiss_field_id Computed_field_create_cad_geometry( Cmiss_field_id source_field, GeometricShape *shape )
{
	ENTER(Computed_field_create_cad_geometry);
	struct Computed_field *field = NULL;
	Computed_field *source_fields[1];

	if ( cad_topology_source_field )
	{

		source_fields[0] = cad_topology_source_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/3,
			/*number_of_source_fields*/1, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_cad_geometry());

	}
	else
	{
		display_message( ERROR_MESSAGE, "Computed_field_create_cad_geometry.  "
			"Invalid argument(s)" );
	}
	LEAVE;

	return (field);
} /* Computed_field_create_cad_geometry */

