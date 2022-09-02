/*******************************************************************************
FILE : computed_field_find_xi_private.hpp

LAST MODIFIED : 13 June 2008

DESCRIPTION :
Data structures and prototype functions needed for all find xi implementations.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_FIND_XI_PRIVATE_HPP)
#define COMPUTED_FIELD_FIND_XI_PRIVATE_HPP

#include "opencmiss/zinc/mesh.h"

class Computed_field_find_element_xi_cache
{
	cmzn_field *field;
	cmzn_mesh *searchMesh;

public:
	struct FE_element *element;
	int componentsCount;
	double time;
	FE_value *values;
	FE_value *workingValues;
	
	Computed_field_find_element_xi_cache(cmzn_field *fieldIn);
	
	virtual ~Computed_field_find_element_xi_cache();

	cmzn_field *getField() const
	{
		return this->field;
	}

	cmzn_mesh *getSearchMesh() const
	{
		return this->searchMesh;
	};

	void setSearchMesh(cmzn_mesh_id searchMeshIn)
	{
		if (searchMeshIn)
		{
			cmzn_mesh_access(searchMeshIn);
		}
		if (this->searchMesh)
		{
			cmzn_mesh_destroy(&this->searchMesh);
		}
		this->searchMesh = searchMeshIn;
	};
};

struct Computed_field_iterative_find_element_xi_data
/*******************************************************************************
LAST MODIFIED: 21 August 2002

DESCRIPTION:
Data for passing to Computed_field_iterative_element_conditional
Important note:
The <values> passed in this structure must not be a pointer to values
inside the field_cache otherwise they may be overwritten if that field
matches the <field> in this structure or one of its source fields.
==============================================================================*/
{
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	cmzn_fieldcache_id field_cache;
	cmzn_field *field;
	int number_of_values;
	FE_value *values;
	FE_value *workingValues;  // another array with correct number of components
	FE_value xi_tolerance;
	int find_nearest_location;
	struct FE_element *nearest_element;
	FE_value nearest_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	double nearest_element_distance_squared;
	int start_with_data_xi;
	double time;
}; /* Computed_field_iterative_find_element_xi_data */

int Computed_field_iterative_element_conditional(struct FE_element *element,
	struct Computed_field_iterative_find_element_xi_data *data);
/***************************************************************************//**
 * Searches element for location with matching field values.
 * Important note:
 * The <values> passed in the <data> structure must not be a pointer to values
 * inside a field cache otherwise they may be overwritten if the field is the
 * same as the <data> field or any of its source fields.
 *
 * @return  1 if a valid element xi is found.
 */

#endif /* !defined (COMPUTED_FIELD_FIND_XI_PRIVATE_HPP) */
