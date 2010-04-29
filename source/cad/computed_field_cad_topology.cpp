/*****************************************************************************//**
 * FILE : computed_field_cad_topology.cpp
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
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}

#include "cad/field_location.hpp"
#include "cad/computed_field_cad_topology.h"
#include "cad/computed_field_cad_topology_private.h"
//#include "cad/topologicalshape.h"
//#include "cad/geometricshape.h"
/*
Module types
------------
*/
/*
class Computed_field_cad_topology_package : public Computed_field_type_package
{
public:
	Cmiss_region *root_region;

	Computed_field_cad_topology_package(Cmiss_region *root_region)
		: root_region(root_region)
	{
		ACCESS(Cmiss_region)(root_region);
	}

	~Computed_field_cad_topology_package()
	{
		DEACCESS(Cmiss_region)(&root_region);
	}
};
*/
namespace {

char computed_field_cad_topology_type_string[] = "cad_topology";

class Computed_field_cad_topology : public Computed_field_core
{
protected:
	TopologicalShape *m_shape;
	GeometricShape *m_geometric_shape;

public:
	Computed_field_cad_topology(TopologicalShape *shape)
		: Computed_field_core()
		, m_shape(shape)
		, m_geometric_shape(0)
	{
	}

	~Computed_field_cad_topology()
	{
		if ( m_shape )
			delete m_shape;

		if (m_geometric_shape)
			delete m_geometric_shape;
	}

	inline void topological_shape(TopologicalShape *shape) { m_shape = shape; }
	inline void geometric_shape(GeometricShape *shape) {m_geometric_shape = shape;}
	int get_domain( struct LIST(Computed_field) *domain_field_list ) const;
	int surface_point(int surface_index, double u, double v, double *point, double *uDerivative, double *vDerivative);
	int curve_point(int surface_index, double s, double *point);
	int surface_colour(int surface_index, double u, double v, double *colour);
	int surface_uv_point(int surface_index, int point_index, double &u, double &v);
	int curve_s_parameter(int curve_index, int parameter_index, double &s);
	int surface_count() const;
	int curve_count() const;
	int surface_point_count(int i) const;
	int curve_point_count(int i) const;

private:
	Computed_field_core* copy();

	char* get_type_string()
	{
		return (computed_field_cad_topology_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

};

/***************************************************************************//**
 * Copy the type specific data used by this type.
 */
Computed_field_core* Computed_field_cad_topology::copy()
{
	Computed_field_cad_topology* core = 
		new Computed_field_cad_topology(m_shape);;

	return (core);
} /* Computed_field_cad_topology::copy */

/***************************************************************************//**
 * Compare the type specific data.
 */
int Computed_field_cad_topology::compare(Computed_field_core *other_core)
{
	int return_code;

	ENTER(Computed_field_cad_topology::compare);
	if (field && dynamic_cast<Computed_field_cad_topology*>(other_core))
	{
		printf("Comparing computed field cad topology, not really\n" );
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cad_topology::compare */


/***************************************************************************//**
 * Evaluate the values of the field at the supplied location.
 */
int Computed_field_cad_topology::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code = 0;

	ENTER(Computed_field_cad_topology::evaluate_cache_at_location);
	if (m_geometric_shape && location)
	{
		printf("Hi from Computed_field_cad_topology::evaluate_cache_at_location\n");
		//Field_cad_geometry_location *cad_geometry_location = static_cast<Field_cad_geometry_location*>(location);
		//field->values[0] = surface_point( cad_geometry_location->surface_index(), cad_geometry_location->point_index(), 0 );
		//field->values[1] = surface_point( cad_geometry_location->surface_index(), cad_geometry_location->point_index(), 1 );
		//field->values[2] = surface_point( cad_geometry_location->surface_index(), cad_geometry_location->point_index(), 2 );
		//printf( "START[%.2f, %.2f , %.2f] ", field->values[0], field->values[1], field->values[2] );
		//return_code = 1; always fail this function
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::evaluate_cache_at_location.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cad_topology::evaluate_cache_at_location */

/***************************************************************************//**
 * Writes type-specific details of the field to the console.
 */
int Computed_field_cad_topology::list()
{
	//char *field_name;
	int return_code = 0;

	ENTER(List_Computed_field_cad_topology);
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    Cad Shape field : ");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cad_topology.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cad_topology */

/***************************************************************************//**
 * Returns allocated command string for reproducing this field. Includes type.
 */
char *Computed_field_cad_topology::get_command_string()
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_cad_topology::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, computed_field_cad_topology_type_string, &error);
		append_string(&command_string, " field ", &error);
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
			"Computed_field_cad_topology::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cad_topology::get_command_string */

int Computed_field_cad_topology::get_domain( struct LIST(Computed_field) *domain_field_list ) const
{
	int return_code = 0;
	if (domain_field_list)
	{
		if ( ADD_OBJECT_TO_LIST(Computed_field)(field,domain_field_list) )
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::get_domain.  Invalid argument" );
	}

	return return_code;
}

int Computed_field_cad_topology::surface_count() const
{
	int surface_count = -1;
	if (m_geometric_shape)
	{
		surface_count = m_geometric_shape->surfaceCount();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_geometry::surface_count.  Invalid geometric shape");
	}

	return surface_count;
}

int Computed_field_cad_topology::curve_count() const
{
	int curve_count = -1;
	if (m_geometric_shape)
	{
		curve_count = m_geometric_shape->curveCount();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_geometry::curve_count.  Invalid geometric shape");
	}

	return curve_count;
}

int Computed_field_cad_topology::surface_point_count(int i) const
{
	int point_count = -1;
	if (m_geometric_shape)
	{
		int surface_count = m_geometric_shape->surfaceCount();
		if (i >= 0 && i < surface_count)
		{
			point_count = m_geometric_shape->surface(i)->pointCount();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_cad_topology::surface_point_count.  Surface index out of bounds");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::surface_point_count.  Invalid geometric shape");
	}

	return point_count;
}

int Computed_field_cad_topology::curve_point_count(int i) const
{
	int point_count = -1;
	if (m_geometric_shape)
	{
		int curve_count = m_geometric_shape->curveCount();
		if (i >= 0 && i < curve_count)
		{
			point_count = m_geometric_shape->curve(i)->pointCount();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_cad_topology::curve_point_count.  Curve index out of bounds");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::curve_point_count.  Invalid geometric shape");
	}

	return point_count;
}

int Computed_field_cad_topology::surface_uv_point(int surface_index, int point_index, double &u, double &v)
{
	int return_code = 0;
	if (field)
	{
		//u = m_geometric_shape->surface(surface_index)->uvPoints()[2 * point_index + 0];
		//v = m_geometric_shape->surface(surface_index)->uvPoints()[2 * point_index + 1];
		return_code = m_geometric_shape->surface(surface_index)->uvPoints(point_index, u, v);
	}

	return return_code;
}

int Computed_field_cad_topology::curve_s_parameter(int curve_index, int parameter_index, double &s)
{
	int return_code = 0;
	if (field)
	{
		//u = m_geometric_shape->surface(surface_index)->uvPoints()[2 * point_index + 0];
		//v = m_geometric_shape->surface(surface_index)->uvPoints()[2 * point_index + 1];
		return_code = m_geometric_shape->curve(curve_index)->sParameter(parameter_index, s);
	}

	return return_code;
}

int Computed_field_cad_topology::surface_point(int surface_index, double u, double v, double *point, double *uDerivative, double *vDerivative)
{
	int return_code = 0;
	if (field)
	{
		return_code = m_geometric_shape->surface(surface_index)->surfacePoint(u, v, point, uDerivative, vDerivative);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::surface_point.  Invalid shape");
	}

	return return_code;
}

int Computed_field_cad_topology::curve_point(int curve_index, double s, double *point)
{
	int return_code = 0;
	if (field)
	{
		return_code = m_geometric_shape->curve(curve_index)->curvePoint( s, point);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::surface_point.  Invalid shape");
	}

	return return_code;
}

int Computed_field_cad_topology::surface_colour(int surface_index, double u, double v, double *colour)
{
	USE_PARAMETER(surface_index);
	USE_PARAMETER(u);
	USE_PARAMETER(v);
	int return_code = 0;
	if (field)
	{
		return_code = m_shape->surfaceColour(colour);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::surface_colour.  Invalid shape");
	}

	return return_code;
}

} //namespace

int Cmiss_field_is_type_cad_topology(Cmiss_field_id field, void *not_in_use)
{
	int return_code = 0;
	USE_PARAMETER(not_in_use);
	//printf( "Checking domain ...\n" );
	if (field)
	{
		//printf( "Valid field ...\n" );
		if ( NULL != dynamic_cast<Computed_field_cad_topology*>(field->core) )
		{
			//printf( "Yes, it is a cad topology.\n" );
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_is_type_cad_topology.  Invalid argument(s)");
	}

	return return_code;
}

Cmiss_field_cad_topology_id Cmiss_field_cast_cad_topology(Cmiss_field_id field)
{
	if (dynamic_cast<Computed_field_cad_topology*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_cad_topology_id>(field));
	}
	else
	{
		return (NULL);
	}
}

/**
 * @see Cmiss_field_create_cad_topology
 */
Computed_field *Computed_field_create_cad_topology( Cmiss_field_module *field_module, TopologicalShape *shape )
{
	ENTER(Computed_field_create_cad_topology);
	Computed_field *field = (Computed_field *)NULL;

	if ( shape )
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/3,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_cad_topology( shape ));

		//cad_topology_field->topological_shape(shape);
	}
	else
	{
		display_message( ERROR_MESSAGE, "Computed_field_create_image.  "
			"Invalid argument(s)" );
	}
	LEAVE;

	return (field);
} /* Computed_field_create_cad_topology */

void Cmiss_field_cad_topology_set_geometric_shape(Cmiss_field_cad_topology_id field, GeometricShape *shape)
{
	if (field && shape)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = static_cast<Computed_field_cad_topology*>(computed_field->core);
		cad_topology->geometric_shape(shape);
	}
	else
	{
		display_message( ERROR_MESSAGE, "Computed_field_set_cad_geometry.  "
			"Invalid argument(s)" );
	}
}

int Cmiss_field_cad_topology_get_surface_count( Cmiss_field_cad_topology_id field )
{
	int surface_count = -1;

	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = static_cast<Computed_field_cad_topology*>(computed_field->core);
		surface_count = cad_topology->surface_count();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_surface_count.  Invalid field");
	}

	return surface_count;
}

int Cmiss_field_cad_topology_get_curve_count(Cmiss_field_cad_topology_id field)
{
	int curve_count = -1;

	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = static_cast<Computed_field_cad_topology *>(computed_field->core);
		curve_count = cad_topology->curve_count();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_curve_count.  Invalid field");
	}

	return curve_count;
}


int Cmiss_field_cad_topology_get_surface_point_count( Cmiss_field_cad_topology_id field, int surface_index )
{
	int point_count = -1;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		point_count = cad_topology->surface_point_count( surface_index );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_surface_point_count.  Invalid field");
	}

	return point_count;
}

int Cmiss_field_cad_topology_get_curve_point_count( Cmiss_field_cad_topology_id field, int curve_index )
{
	int point_count = -1;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		point_count = cad_topology->curve_point_count( curve_index );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_curve_point_count.  Invalid field");
	}

	return point_count;
}

int Cmiss_field_cad_topology_get_surface_point_uv_coordinates( Cmiss_field_cad_topology_id field, int surface_index, int uvPoint_index, double &u, double &v)
{
	int return_code = 0;
	u = 0.0;
	v = 0.0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->surface_uv_point(surface_index, uvPoint_index, u, v);
	}
	return return_code;
}

int Cmiss_field_cad_topology_get_curve_s_parameter( Cmiss_field_cad_topology_id field, int curve_index, int s_parameter_index, double &s)
{
	int return_code = 0;
	s = 0.0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->curve_s_parameter(curve_index, s_parameter_index, s);
	}
	return return_code;
}

int Computed_field_cad_topology_get_surface_point(Cmiss_field_cad_topology_id field, int surface_index, double u, double v, double *point, double *uDerivative, double *vDerivative)
{
	int return_code = 0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->surface_point(surface_index, u, v, point, uDerivative, vDerivative);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_surface_point.  Invalid field");
	}

	return return_code;
}

int Computed_field_cad_topology_get_curve_point(Cmiss_field_cad_topology_id field, int curve_index, double s, double *point)
{
	int return_code = 0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->curve_point(curve_index, s, point);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_surface_point.  Invalid field");
	}

	return return_code;
}

int Computed_field_cad_topology_get_surface_colour(Cmiss_field_cad_topology_id field, int surface_index, double u, double v, double *colour)
{
	int return_code = 0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->surface_colour(surface_index, u, v, colour);
	}

	return return_code;
}

