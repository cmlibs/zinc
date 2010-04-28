
extern "C" {
#include "computed_field/computed_field.h"
}

#include "general/value.h"
#include "cad/field_location.hpp"

Field_cad_geometry_location::Field_cad_geometry_location(Cmiss_field_cad_topology_id id, int index, FE_value time, int number_of_derivatives)
	: Field_location(time, number_of_derivatives)
	, m_id(id)
	, m_index(index)
{
}

Field_cad_geometry_location::~Field_cad_geometry_location()
{
}

Field_cad_geometry_surface_location::Field_cad_geometry_surface_location(Cmiss_field_cad_topology_id id, int index, double u, double v, FE_value time, int number_of_derivatives)
	: Field_cad_geometry_location(id, index, time, number_of_derivatives)
	, m_u(u)
	, m_v(v)
{
}

Field_cad_geometry_surface_location::~Field_cad_geometry_surface_location()
{
}

Field_cad_geometry_curve_location::Field_cad_geometry_curve_location(Cmiss_field_cad_topology_id id, int index, double s, FE_value time, int number_of_derivatives)
	: Field_cad_geometry_location(id, index, time, number_of_derivatives)
	, m_s(s)
{
}

Field_cad_geometry_curve_location::~Field_cad_geometry_curve_location()
{
}

