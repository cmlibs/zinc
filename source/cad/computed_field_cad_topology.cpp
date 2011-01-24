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
#include <stdlib.h>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <Geom_BezierSurface.hxx>
#include <Handle_Geom_BezierSurface.hxx>
#include <Geom_BoundedSurface.hxx>
#include <Handle_Geom_BoundedSurface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Handle_Geom_BSplineSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Handle_Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Handle_Geom_CylindricalSurface.hxx>
#include <Geom_ElementarySurface.hxx>
#include <Handle_Geom_ElementarySurface.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Handle_Geom_OffsetSurface.hxx>
#include <Geom_Plane.hxx>
#include <Handle_Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Handle_Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Handle_Geom_SphericalSurface.hxx>
#include <Geom_Surface.hxx>
#include <Handle_Geom_Surface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Handle_Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Handle_Geom_SurfaceOfRevolution.hxx>
#include <Geom_SweptSurface.hxx>
#include <Handle_Geom_SweptSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Handle_Geom_ToroidalSurface.hxx>

extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
#include "computed_field/computed_field.h"
}

#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"

#include "cad/field_location.hpp"
#include "cad/computed_field_cad_topology.h"
#include "cad/computed_field_cad_topology_private.h"

namespace {

char computed_field_cad_topology_type_string[] = "cad_topology";

class Computed_field_cad_topology : public Computed_field_core
{
protected:
	TopologicalShape *m_shape;
	GeometricShape *m_geometric_shape;
	std::vector<int> surface_identifiers;

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
	int surface_point(Cmiss_cad_surface_identifier identifier, double u, double v, double *point, double *uDerivative, double *vDerivative);
	int curve_point(Cmiss_cad_curve_identifier identifier, double s, double *point);
	int surface_colour(Cmiss_cad_surface_identifier identifier, double u, double v, double *colour);
	int surface_uv_point(Cmiss_cad_surface_identifier identifier, Cmiss_cad_surface_point_identifier point_identifier, double &u, double &v);
	int curve_s_parameter(Cmiss_cad_curve_identifier identifier, Cmiss_cad_curve_point_identifier s_identifier, double &s);
	int surface_count() const;
	int curve_count() const;
	int surface_point_count(int i) const;
	int curve_point_count(int i) const;

	void information(int number) const;

private:
	Computed_field_core* copy();

	const char* get_type_string()
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

	if (field && dynamic_cast<Computed_field_cad_topology*>(other_core))
	{
		printf("Comparing computed field cad topology, not really\n" );
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_cad_topology::compare */


/***************************************************************************//**
 * Evaluate the values of the field at the supplied location.
 */
int Computed_field_cad_topology::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code = 0;

	if (m_geometric_shape && location)
	{
#if defined(DEBUG)
		printf("Hi from Computed_field_cad_topology::evaluate_cache_at_location ...  doing nothing\n");
#endif /* defined(DEBUG) */
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

	return (return_code);
} /* Computed_field_cad_topology::evaluate_cache_at_location */

/***************************************************************************//**
 * Writes type-specific details of the field to the console.
 */
int Computed_field_cad_topology::list()
{
	//char *field_name;
	int return_code = 0;

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

	return (return_code);
} /* list_Computed_field_cad_topology */

/***************************************************************************//**
 * Returns allocated command string for reproducing this field. Includes type.
 */
char *Computed_field_cad_topology::get_command_string()
{
	char *command_string, *field_name;
	int error;

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
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_cad_topology::get_domain.  Failed to add object to list" );
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
				"Computed_field_cad_topology::surface_point_count.  Surface index out of bounds (%d)", i);
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
				"Computed_field_cad_topology::curve_point_count.  Curve index out of bounds (%d)", i);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::curve_point_count.  Invalid geometric shape");
	}

	return point_count;
}

int Computed_field_cad_topology::surface_uv_point(Cmiss_cad_surface_identifier identifier, Cmiss_cad_surface_point_identifier point_identifier, double &u, double &v)
{
	int return_code = 0;
	if (field)
	{
		//u = m_geometric_shape->surface(surface_index)->uvPoints()[2 * point_index + 0];
		//v = m_geometric_shape->surface(surface_index)->uvPoints()[2 * point_index + 1];
		return_code = m_geometric_shape->surface(identifier)->uvPoints(point_identifier, u, v);
	}

	return return_code;
}

int Computed_field_cad_topology::curve_s_parameter(Cmiss_cad_curve_identifier identifier, Cmiss_cad_curve_point_identifier s_identifier, double &s)
{
	int return_code = 0;
	if (field)
	{
		//u = m_geometric_shape->surface(surface_index)->uvPoints()[2 * point_index + 0];
		//v = m_geometric_shape->surface(surface_index)->uvPoints()[2 * point_index + 1];
		return_code = m_geometric_shape->curve(identifier)->sParameter(s_identifier, s);
	}

	return return_code;
}

int Computed_field_cad_topology::surface_point(Cmiss_cad_surface_identifier identifier, double u, double v, double *point, double *uDerivative, double *vDerivative)
{
	int return_code = 0;
	if (field)
	{
		return_code = m_geometric_shape->surface(identifier)->surfacePoint(u, v, point, uDerivative, vDerivative);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::surface_point.  Invalid shape");
	}

	return return_code;
}

int Computed_field_cad_topology::curve_point(Cmiss_cad_curve_identifier identifier, double s, double *point)
{
	int return_code = 0;
	if (field)
	{
		return_code = m_geometric_shape->curve(identifier)->curvePoint( s, point);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::surface_point.  Invalid shape");
	}

	return return_code;
}

int Computed_field_cad_topology::surface_colour(Cmiss_cad_surface_identifier identifier, double u, double v, double *colour)
{
	USE_PARAMETER(u);
	USE_PARAMETER(v);
	int return_code = 0;
	if (field)
	{
		return_code = m_shape->surfaceColour(identifier, colour);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::surface_colour.  Invalid shape");
	}

	return return_code;
}

void Computed_field_cad_topology::information(int surface_index) const
{
	//String info;
	display_message(INFORMATION_MESSAGE,
		"Computed_field_cad_topology %s.  Information\n", Cmiss_field_get_name(field));
	TopExp_Explorer faceExplorer(m_shape->shape() , TopAbs_FACE);
	TopoDS_Face face;
	int count = 0;
	for (TopExp_Explorer faceExplorer(m_shape->shape() , TopAbs_FACE) ; faceExplorer.More() && count <= surface_index; faceExplorer.Next())
	{
		face = TopoDS::Face(faceExplorer.Current());
		count++;
	}
	
	//Check if <aFace> is the top face of the bottle's neck
	Handle(Geom_Surface) surface = BRep_Tool::Surface(face);

	if (surface->DynamicType() == STANDARD_TYPE(Geom_BezierSurface))
	{
		display_message(INFORMATION_MESSAGE, "  Bezier surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_BoundedSurface))
	{
		display_message(INFORMATION_MESSAGE, "  Bounded surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_BSplineSurface))
	{
		display_message(INFORMATION_MESSAGE, "  BSpline surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface))
	{
		display_message(INFORMATION_MESSAGE, "  Conical surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface))
	{
		Handle_Geom_CylindricalSurface cylinder = Handle_Geom_CylindricalSurface::DownCast(surface);
		display_message(INFORMATION_MESSAGE, "  Cylinder radius = %.3f\n", cylinder->Radius());
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_ElementarySurface))
	{
		display_message(INFORMATION_MESSAGE, "  Elementary surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_OffsetSurface))
	{
		display_message(INFORMATION_MESSAGE, "  Offset surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_Plane))
	{
		Handle_Geom_Plane plane = Handle_Geom_Plane::DownCast(surface);
		Standard_Real a, b, c, d;
		plane->Coefficients(a, b, c, d);
		display_message(INFORMATION_MESSAGE, "  Plane equation: %.3f x + %.3f y + %.3f z + %.3f = 0\n", a, b, c, d);
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
	{
		//Handle_Geom_RectangularTrimmedSurface rect = Handle_Geom_RectangularTrimmedSurface::DownCast(surface);
		display_message(INFORMATION_MESSAGE, "  Rectangular trimmed surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_SphericalSurface))
	{
		Handle_Geom_SphericalSurface sphere = Handle_Geom_SphericalSurface::DownCast(surface);
		display_message(INFORMATION_MESSAGE, "  Sphere area: %.3f\n", sphere->Area());
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_Surface))
	{
		display_message(INFORMATION_MESSAGE, "  Surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion))
	{
		display_message(INFORMATION_MESSAGE, "  Linear extrusion surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_SurfaceOfRevolution))
	{
		display_message(INFORMATION_MESSAGE, "  Surface of revolution\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_SweptSurface))
	{
		display_message(INFORMATION_MESSAGE, "  Swept surface\n");
	}
	else if (surface->DynamicType() == STANDARD_TYPE(Geom_ToroidalSurface))
	{
		display_message(INFORMATION_MESSAGE, "  Toroidal surface\n");
	}
	else
	{
		display_message(INFORMATION_MESSAGE, "  Unknown surface type\n");
	}
}

char computed_field_cad_group_type_string[] = "cad_group";
/**
 * A group of cad element identifiers
 * Implemented using a std::list
 */
class Field_cad_element_group : public Computed_field_core
{
private:
	Computed_field_cad_topology *topology;
	std::list<Cad_topology_primitive_identifier> element_identifiers;

private:
	Computed_field_core* copy();

	const char* get_type_string()
	{
		return (computed_field_cad_group_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

};

/**
 * Copy the type specific data used by this type.
 */
Computed_field_core* Field_cad_element_group::copy()
{
	Field_cad_element_group* core = 
		new Field_cad_element_group();

	return (core);
} /* Field_cad_element_group::copy */

/***************************************************************************//**
 * Compare the type specific data.
 */
int Field_cad_element_group::compare(Computed_field_core *other_core)
{
	int return_code;

	if (field && dynamic_cast<Field_cad_element_group*>(other_core))
	{
		printf("Comparing computed field cad group, not really\n" );
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}

	return (return_code);
} /* Field_cad_element_group::compare */


/***************************************************************************//**
 * Evaluate the values of the field at the supplied location.
 */
int Field_cad_element_group::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code = 0;

	if (location)
	{
		printf("Hi from Field_cad_element_group::evaluate_cache_at_location ...  doing nothing\n");
		return_code = 0; // always fail this function
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cad_topology::evaluate_cache_at_location.  Invalid argument(s)");
	}

	return (return_code);
} /* Field_cad_element_group::evaluate_cache_at_location */

/***************************************************************************//**
 * Writes type-specific details of the field to the console.
 */
int Field_cad_element_group::list()
{
	//char *field_name;
	int return_code = 0;

	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    Cad Group field : ");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cad_topology.  Invalid argument(s)");
	}

	return (return_code);
} /* Field_cad_element_group::list */

/***************************************************************************//**
 * Returns allocated command string for reproducing this field. Includes type.
 */
char *Field_cad_element_group::get_command_string()
{
	char *command_string, *field_name;
	int error;

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

	return (command_string);
} /* Field_cad_element_group::get_command_string */

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
 * @see Cmiss_field_module_create_cad_topology
 */
Computed_field *Cmiss_field_module_create_cad_topology( Cmiss_field_module *field_module, TopologicalShape *shape )
{
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

	return (field);
} /* Cmiss_field_module_create_cad_topology */

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


int Cmiss_field_cad_topology_get_surface_point_count( Cmiss_field_cad_topology_id field, Cmiss_cad_surface_identifier identifier )
{
	int point_count = -1;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		point_count = cad_topology->surface_point_count( identifier );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_surface_point_count.  Invalid field");
	}

	return point_count;
}

int Cmiss_field_cad_topology_get_curve_point_count( Cmiss_field_cad_topology_id field, Cmiss_cad_curve_identifier identifier )
{
	int point_count = -1;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		point_count = cad_topology->curve_point_count( identifier );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_curve_point_count.  Invalid field");
	}

	return point_count;
}

int Cmiss_field_cad_topology_get_surface_point_uv_coordinates( Cmiss_field_cad_topology_id field,
	Cmiss_cad_surface_identifier identifier,
	Cmiss_cad_surface_point_identifier uv_identifier,
	double &u, double &v)
{
	int return_code = 0;
	u = 0.0;
	v = 0.0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->surface_uv_point(identifier, uv_identifier, u, v);
	}
	return return_code;
}

int Cmiss_field_cad_topology_get_curve_point_s_coordinate( Cmiss_field_cad_topology_id field, Cmiss_cad_curve_identifier identifier,
	Cmiss_cad_curve_point_identifier s_identifier, double &s)
{
	int return_code = 0;
	s = 0.0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->curve_s_parameter(identifier, s_identifier, s);
	}
	return return_code;
}

int Computed_field_cad_topology_get_surface_point(Cmiss_field_cad_topology_id field,
	Cmiss_cad_surface_identifier identifier,
	double u, double v, double *point, double *uDerivative, double *vDerivative)
{
	int return_code = 0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->surface_point(identifier, u, v, point, uDerivative, vDerivative);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_surface_point.  Invalid field");
	}

	return return_code;
}

int Computed_field_cad_topology_get_curve_point(Cmiss_field_cad_topology_id field, Cmiss_cad_curve_identifier identifier, double s, double *point)
{
	int return_code = 0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->curve_point(identifier, s, point);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_surface_point.  Invalid field");
	}

	return return_code;
}

int Computed_field_cad_topology_get_surface_colour(Cmiss_field_cad_topology_id field, Cmiss_cad_surface_identifier identifier, double u, double v, double *colour)
{
	int return_code = 0;
	if (field)
	{
		Computed_field *computed_field = reinterpret_cast<Computed_field *>(field);
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(computed_field->core);
		return_code = cad_topology->surface_colour(identifier, u, v, colour);
	}

	return return_code;
}

void Cad_topology_information( Cmiss_field_id cad_topology_field, Cad_primitive_identifier information )
{
	if (cad_topology_field)
	{
		Computed_field_cad_topology *cad_topology = reinterpret_cast<Computed_field_cad_topology*>(cad_topology_field->core);
		cad_topology->information(information.number);
	}
}

