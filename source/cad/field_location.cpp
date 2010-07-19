
extern "C" {
#include "computed_field/computed_field.h"
}

#include "general/value.h"
#include "cad/field_location.hpp"

Field_cad_geometry_location::Field_cad_geometry_location(Cmiss_field_cad_topology_id id, FE_value time, int number_of_derivatives)
	: Field_location(time, number_of_derivatives)
	, id(id)
{
}

Field_cad_geometry_location::~Field_cad_geometry_location()
{
}

Field_cad_geometry_surface_location::Field_cad_geometry_surface_location(Cmiss_field_cad_topology_id id, Cmiss_cad_surface_identifier identifier, double u, double v, FE_value time, int number_of_derivatives)
	: Field_cad_geometry_location(id, time, number_of_derivatives)
	, identifier(identifier)
	, u(u)
	, v(v)
{
}

Field_cad_geometry_surface_location::~Field_cad_geometry_surface_location()
{
}

Field_cad_geometry_curve_location::Field_cad_geometry_curve_location(Cmiss_field_cad_topology_id id, Cmiss_cad_curve_identifier identifier, double s, FE_value time, int number_of_derivatives)
	: Field_cad_geometry_location(id, time, number_of_derivatives)
	, identifier(identifier)
	, s(s)
{
}

Field_cad_geometry_curve_location::~Field_cad_geometry_curve_location()
{
}

