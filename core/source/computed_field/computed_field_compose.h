/*******************************************************************************
FILE : computed_field_compose.h

LAST MODIFIED : 7 January 2003

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_COMPOSE_H)
#define COMPUTED_FIELD_COMPOSE_H

#include "finite_element/finite_element.h"
#include "region/cmiss_region.h"
#include "computed_field/computed_field_private.hpp"


class Computed_field_compose_package : public Computed_field_type_package
{
public:
	struct cmzn_region *root_region;
};

const char computed_field_compose_type_string[] = "compose";

class Computed_field_compose : public Computed_field_core
{
private:
	cmzn_mesh_id mesh;
	int find_nearest;
	int use_point_five_when_out_of_bounds;

public:
	Computed_field_compose(cmzn_mesh_id search_mesh,
			int find_nearest, int use_point_five_when_out_of_bounds = 0) :
		Computed_field_core(),
		mesh(cmzn_mesh_access(search_mesh)),
		find_nearest(find_nearest),
		use_point_five_when_out_of_bounds(use_point_five_when_out_of_bounds)
	{
	}

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
		cmzn_mesh_id *mesh_address, int *find_nearest_address,
		int *use_point_five_when_out_of_bounds_address);

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_compose_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual bool is_defined_at_location(cmzn_field_cache& cache);

	virtual FieldValueCache *createValueCache(cmzn_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

#endif /* !defined (COMPUTED_FIELD_COMPOSE_H) */
