/*
 * computed_field_cad_colour.cpp
 *
 *  Created on: 12-Oct-2009
 *      Author: hsorby
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

#include "cad/computed_field_cad_colour.h"
#include "cad/field_location.hpp"
#include "cad/computed_field_cad_topology_private.h"

namespace {

char computed_field_cad_colour_type_string[] = "cad_colour";

class Computed_field_cad_colour : public Computed_field_core
{
public:

	Computed_field_cad_colour()
		: Computed_field_core()
	{
	}

	~Computed_field_cad_colour()
	{
	}

private:
	Computed_field_core* copy();

	const char* get_type_string()
	{
		return (computed_field_cad_colour_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate(cmzn_field_cache& cache, FieldValueCache& valueCache);

	int list();

	char* get_command_string();

};

/***************************************************************************//**
 * Copy the type specific data used by this type.
 */
Computed_field_core* Computed_field_cad_colour::copy()
{
	Computed_field_cad_colour* core = 
		new Computed_field_cad_colour();

	return (core);
} /* Computed_field_cad_colour::copy */

/***************************************************************************//**
 * Compare the type specific data.
 */
int Computed_field_cad_colour::compare(Computed_field_core *other_core)
{
	int return_code;

	ENTER(Computed_field_cad_colour::compare);
	if (field && dynamic_cast<Computed_field_cad_colour*>(other_core))
	{
		printf("Comparing a cad normal field\n");
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cad_colour::compare */

int Computed_field_cad_colour::evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	cmzn_field_cad_topology_id cad_topology = reinterpret_cast<cmzn_field_cad_topology_id>(getSourceField(0));
	// @TODO: prescribe location directly in cad_topology field's value cache
	Field_cad_geometry_surface_location *cad_surface_location;
	if (0 != (cad_surface_location = dynamic_cast<Field_cad_geometry_surface_location*>(cache.getLocation())))
	{
		//printf("need to populate some values here\n");
		double colour[3];
		if ((cad_topology == (cmzn_field_cad_topology_id *)cad_surface_location->get_id()) &&
			Computed_field_cad_topology_get_surface_colour(cad_topology,
				cad_surface_location->get_identifier(),
				cad_surface_location->get_u(),
				cad_surface_location->get_v(),
				colour))
		{
			valueCache.values[0] = colour[0];
			valueCache.values[1] = colour[1];
			valueCache.values[2] = colour[2];
			//printf("colour is: (%.2f,%.2f,%.2f)\n", colour[0], colour[1], colour[2]);
			return 1;
		}
		//Computed_field_cad_colour *cad_colour = static_cast<Computed_field_cad_colour*>(field->core);
		//field->source_fields[0]->core->evaluate_cache_at_location(location);
		//valueCache.values[0] = Computed_field_surface_point(field->source_fields[0], cad_colour_location->surface_index(), cad_colour_location->point_index(), 0);
		//valueCache.values[1] = Computed_field_surface_point(field->source_fields[0], cad_colour_location->surface_index(), cad_colour_location->point_index(), 1);
		//valueCache.values[2] = Computed_field_surface_point(field->source_fields[0], cad_colour_location->surface_index(), cad_colour_location->point_index(), 2);
		//printf( "START[%.2f, %.2f , %.2f] ", valueCache.values[0], valueCache.values[1], valueCache.values[2] );
	}
	return 0;
}

/***************************************************************************//**
 * Writes type-specific details of the field to the console.
 */
int Computed_field_cad_colour::list()
{
	//char *field_name;
	int return_code;

	ENTER(List_Computed_field_cad_colour);
	if (field)
	{
		double colour[3];
		cmzn_cad_surface_identifier surface_identifier = 0;
		cmzn_field_cad_topology_id cad_topology = cmzn_field_cast_cad_topology(field->source_fields[0]);
		if (Computed_field_cad_topology_get_surface_colour(cad_topology,
				surface_identifier,
				0.0,
				0.0,
				colour))
		{
			display_message(INFORMATION_MESSAGE, "    Cad colour field : [%.3f, %.3f. %.3f] ", colour[0], colour[1], colour[2]);
		}
		cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(&cad_topology));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cad_colour.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cad_colour */

/***************************************************************************//**
 * Returns allocated command string for reproducing this field. Includes type.
 */
char *Computed_field_cad_colour::get_command_string()
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_cad_colour::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, computed_field_cad_colour_type_string, &error);
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
			"Computed_field_cad_colour::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cad_colour::get_command_string */

} //namespace

int cmzn_field_is_cad_colour( struct Computed_field *field, void *not_in_use )
{
	int return_code = 0;
	USE_PARAMETER(not_in_use);
	if ( field )
	{
		if ( NULL != dynamic_cast<Computed_field_cad_colour*>(field->core) )
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_cad_colour.  Invalid argument(s)");
	}

	return return_code;
}

cmzn_field_cad_colour_id cmzn_field_cad_colour_cast(cmzn_field_id field)
{
	if (dynamic_cast<Computed_field_cad_colour*>(field->core))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_cad_colour_id>(field));
	}
	else
	{
		return (NULL);
	}
}

/**
 * @see cmzn_field_create_cad_colour
 */
cmzn_field_id Computed_field_module_create_cad_colour(cmzn_field_module_id field_module, cmzn_field_id cad_topology_source_field)
{
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
			new Computed_field_cad_colour());
	}
	else
	{
		display_message( ERROR_MESSAGE, "Computed_field_create_cad_colour.  "
			"Invalid argument(s)" );
	}

	return (field);
} /* Computed_field_create_cad_colour */


