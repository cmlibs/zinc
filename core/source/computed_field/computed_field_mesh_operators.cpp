/**
 * FILE : computed_field_mesh_operators.cpp
 *
 * Field operators that act on a mesh: integration etc.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <cmath>
#include <iostream>
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_mesh_operators.hpp"
#include "computed_field/field_module.hpp"
#include "mesh/cmiss_element_private.hpp"
#include "zinc/fieldmeshoperators.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_set.h"
#include "element/element_operations.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "finite_element/finite_element_region.h"

namespace {

const char computed_field_mesh_integral_type_string[] = "mesh_integral";

// assumes there are two source fields: 1. integrand and 2. coordinate
class Computed_field_mesh_integral : public Computed_field_core
{
protected:
	cmzn_mesh_id mesh;
	int order;

public:
	Computed_field_mesh_integral(cmzn_mesh_id meshIn) :
		Computed_field_core(),
		mesh(cmzn_mesh_access(meshIn)),
		order(1)
	{
	}

	virtual ~Computed_field_mesh_integral()
	{
		cmzn_mesh_destroy(&mesh);
	}

	Computed_field_core *copy()
	{
		return new Computed_field_mesh_integral(mesh);
	}

	const char *get_type_string()
	{
		return (computed_field_mesh_integral_type_string);
	}

	int compare(Computed_field_core* other_core)
	{
		Computed_field_mesh_integral *other =
			dynamic_cast<Computed_field_mesh_integral*>(other_core);
		if (other)
			return cmzn_mesh_match(mesh, other->mesh);
		return 0;
	}

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	int list();

	char* get_command_string();

	cmzn_mesh_id getMesh()
	{
		return mesh;
	}

	int setOrder(int order)
	{
		if ((1 <= order) && (order <= 4))
		{
			if (this->order != order)
			{
				this->order = order;
				if (this->field)
					Computed_field_changed(this->field);
			}
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

protected:
	/** @return  number_of_terms summed. 0 is not an error for mesh_integral */
	int evaluate_sum(cmzn_fieldcache& cache, FieldValueCache& inValueCache);
};

int Computed_field_mesh_integral::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	int result = 1;
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	FE_value *values = valueCache.values;
	cmzn_fieldcache* extraCache = inValueCache.getExtraCache();
	extraCache->setTime(cache.getTime());
	const int number_of_components = field->number_of_components;
	cmzn_field_id integrandField = this->getSourceField(0);
	cmzn_field_id coordinateField = this->getSourceField(1);
	const int coordinatesCount = coordinateField->number_of_components;
	int i;
	for (i = 0; i < number_of_components; i++)
		values[i] = 0;
	cmzn_elementiterator_id iterator = cmzn_mesh_create_elementiterator(mesh);
	cmzn_element_id element = 0;
	int elementDimension = cmzn_mesh_get_dimension(mesh);
	IntegrationPoints integration(this->order);
	int numPoints;
	double *points = 0;
	double *weights = 0;

	while (0 != (element = cmzn_elementiterator_next_non_access(iterator)))
	{
		numPoints = integration.getPoints(element, points, weights);
		if (0 == numPoints)
		{
			result = 0;
			break;
		}
		for (int p = 0; p < numPoints; ++p)
		{
			extraCache->setMeshLocation(element, points + p*elementDimension);
			RealFieldValueCache *integrandValueCache = RealFieldValueCache::cast(integrandField->evaluate(*extraCache));
			RealFieldValueCache *coordinateValueCache = coordinateField->evaluateWithDerivatives(*extraCache, elementDimension);
			if (integrandValueCache && coordinateValueCache)
			{
				// note dx_dxi cycles over xi fastest
				FE_value *dx_dxi = coordinateValueCache->derivatives;
				FE_value dLAV = 0.0; // dL (1-D), dA (2-D), dV (3-D)
				switch (elementDimension)
				{
				case 1:
					for (int c = 0; c < coordinatesCount; ++c)
						dLAV += dx_dxi[c]*dx_dxi[c];
					dLAV = sqrt(dLAV);
					break;
				case 2:
					if (coordinatesCount == 2)
						dLAV = dx_dxi[0]*dx_dxi[3] - dx_dxi[1]*dx_dxi[2];
					else
					{
						// dA = magnitude of dx_dxi1 (x) dx_dxi2
						FE_value n1 = dx_dxi[2]*dx_dxi[5] - dx_dxi[3]*dx_dxi[4];
						FE_value n2 = dx_dxi[4]*dx_dxi[1] - dx_dxi[5]*dx_dxi[0];
						FE_value n3 = dx_dxi[0]*dx_dxi[3] - dx_dxi[1]*dx_dxi[2];
						dLAV = n1*n1 + n2*n2 + n3*n3;
						dLAV = sqrt(dLAV);
					}
					break;
				case 3:
					dLAV = dx_dxi[0]*(dx_dxi[4]*dx_dxi[8] - dx_dxi[7]*dx_dxi[5]) +
					       dx_dxi[3]*(dx_dxi[7]*dx_dxi[2] - dx_dxi[1]*dx_dxi[8]) +
					       dx_dxi[6]*(dx_dxi[1]*dx_dxi[5] - dx_dxi[4]*dx_dxi[2]);
					break;
				}
				for (i = 0; i < number_of_components; ++i)
					values[i] += integrandValueCache->values[i]*weights[p]*dLAV;
			}
			else
				// ignore elements where integrand or coordinates not defined
				break;
		}
	}
	cmzn_elementiterator_destroy(&iterator);
	valueCache.derivatives_valid = 0;
	return result;
}

bool Computed_field_mesh_integral::is_defined_at_location(cmzn_fieldcache& cache)
{
	return true;
}

/** Lists a description of the mesh_operator arguments */
int Computed_field_mesh_integral::list()
{
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			getSourceField(0)->name);
		display_message(INFORMATION_MESSAGE,"    coordinate field : %s\n",
			getSourceField(1)->name);
		char *mesh_name = cmzn_mesh_get_name(mesh);
		display_message(INFORMATION_MESSAGE,"    mesh : %s\n", mesh_name);
		DEALLOCATE(mesh_name);
		return 1;
	}
	return 0;
}

/** Returns allocated command string for reproducing field. Includes type. */
char *Computed_field_mesh_integral::get_command_string()
{
	char *command_string = 0;
	if (field)
	{
		int error = 0;
		append_string(&command_string, get_type_string(), &error);
		append_string(&command_string, " field ", &error);
		append_string(&command_string, getSourceField(0)->name, &error);
		append_string(&command_string, " coordinates ", &error);
		append_string(&command_string, getSourceField(1)->name, &error);
		char *mesh_name = cmzn_mesh_get_name(mesh);
		append_string(&command_string, " mesh ", &error);
		make_valid_token(&mesh_name);
		append_string(&command_string, mesh_name, &error);
		DEALLOCATE(mesh_name);
	}
	return (command_string);
}

} // namespace

cmzn_field_id cmzn_fieldmodule_create_field_mesh_integral(
	cmzn_fieldmodule_id field_module, cmzn_field_id integrand_field,
	cmzn_field_id coordinate_field, cmzn_mesh_id mesh)
{
	cmzn_field_id field = 0;
	if (integrand_field && integrand_field->isNumerical() &&
		coordinate_field && coordinate_field->isNumerical() && mesh)
	{
		int numCoordinates = Computed_field_get_number_of_components(coordinate_field);
		int meshDimension = cmzn_mesh_get_dimension(mesh);
		if ((numCoordinates >= meshDimension) && (numCoordinates <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
		{
			cmzn_field_id source_fields[2] = { integrand_field, coordinate_field };
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true,
				integrand_field->number_of_components,
				/*number_of_source_fields*/2, source_fields,
				/*number_of_source_values*/0, NULL,
				new Computed_field_mesh_integral(mesh));
		}
	}
	return field;
}

cmzn_field_mesh_integral_id cmzn_field_cast_mesh_integral(cmzn_field_id field)
{
	if (field && (dynamic_cast<Computed_field_mesh_integral*>(field->core)))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_mesh_integral_id>(field));
	}
	return 0;
}

int cmzn_field_mesh_integral_destroy(
	cmzn_field_mesh_integral_id *mesh_integral_field_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(mesh_integral_field_address));
}

inline Computed_field_mesh_integral *Computed_field_mesh_integral_core_cast(
	cmzn_field_mesh_integral *mesh_integral_field)
{
	return (static_cast<Computed_field_mesh_integral*>(
		reinterpret_cast<Computed_field*>(mesh_integral_field)->core));
}

int cmzn_field_mesh_integral_set_order(
	cmzn_field_mesh_integral_id mesh_integral_field, int order)
{
	if (mesh_integral_field)
	{
		Computed_field_mesh_integral *mesh_integral_core = Computed_field_mesh_integral_core_cast(mesh_integral_field);
		return mesh_integral_core->setOrder(order);
	}
	return CMZN_ERROR_ARGUMENT;
}
