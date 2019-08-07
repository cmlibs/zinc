/**
 * FILE : finite_element_field.hpp
 *
 * Internal class for field interpolated over finite elements.
 * Private header to share struct definition.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_FIELD_PRIVATE_HPP)
#define FINITE_ELEMENT_FIELD_PRIVATE_HPP

#include "opencmiss/zinc/types/elementid.h"
#include <opencmiss/zinc/zincconfigure.h>
#include "finite_element/finite_element_field.hpp"
#include "finite_element/finite_element_private.h"
#include "general/cmiss_set.hpp"
#include "general/geometry.h"
#include "general/indexed_list_stl_private.hpp"
#include "general/value.h"

/*
Global types
------------
*/

/** Information common to a set of FE_fields.
 * Currently only maintains a non-ACCESSed pointer to the owning FE_region. */
struct FE_field_info
{
	/* the FE_region this FE_field_info and all FE_fields using it belong to */
	struct FE_region *fe_region;

	/* the number of structures that point to this field information.  The
		 field information cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct FE_field_info */

/** Internal class for field interpolated over finite elements. */
struct FE_field
{
	/* the name of the field, which is its identifier */
	const char *name;
	/* shared info for this FE_field including the FE_region it belongs to */
	struct FE_field_info *info;
	/* CMISS field type */
	enum CM_field_type cm_field_type;
	enum FE_field_type fe_field_type;
	/* following two for INDEXED_FE_FIELD only */
	struct FE_field *indexer_field;
	int number_of_indexed_values;
	/* the number of components (allows vector valued fields) */
	int number_of_components;
	/* the names of the different vector components */
	char **component_names;
	/* the coordinate system for the vector */
	struct Coordinate_system coordinate_system;
	/* the number of global values/derivatives that are stored with the field */
	int number_of_values;
	/* the type of the values returned by the field */
	enum Value_type value_type;
	/* for value_type== ELEMENT_XI_VALUE, host mesh, or 0 if not determined from legacy input */
	const FE_mesh *element_xi_host_mesh;
	/* array of global values/derivatives that are stored with the field.
	 * The actual values can be extracted using the <value_type> */
	Value_storage *values_storage;
	/* time series information.  If <number_of_times> is zero then constant in
	 * time */
	enum Value_type time_value_type;
	int number_of_times;
	Value_storage *times;

	// field definition and data on FE_mesh[dimension - 1]
	// Future: limit to being defined on a single mesh; requires change to current usage
	FE_mesh_field_data *meshFieldData[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	/* the number of computed fields wrapping this FE_field */
	int number_of_wrappers;
	/* the number of structures that point to this field.  The field cannot be
		destroyed while this is greater than 0 */
	int access_count;

public:

	inline FE_field *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(FE_field **field_address)
	{
		return DEACCESS(FE_field)(field_address);
	}

	bool isTypeCoordinate() const
	{
		return (CM_COORDINATE_FIELD == this->cm_field_type)
			&& (FE_VALUE_VALUE == this->value_type)
			&& (1 <= this->number_of_components)
			&& (3 >= this->number_of_components);
	}

	cmzn_element *getOrInheritOnElement(cmzn_element *element,
		int inheritFaceNumber, cmzn_element *topLevelElement,
		FE_value *coordinateTransformation);

}; /* struct FE_field */

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with cmzn_set.
 */
class FE_field_identifier : private FE_field
{
public:
	FE_field_identifier(const char *name)
	{
		FE_field::name = name;
	}

	~FE_field_identifier()
	{
		FE_field::name = NULL;
	}

	FE_field *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering cmzn_set<FE_field> by field name */
struct FE_field_compare_name
{
	bool operator() (const FE_field* field1, const FE_field* field2) const
	{
		return strcmp(field1->name, field2->name) < 0;
	}
};

typedef cmzn_set<FE_field *,FE_field_compare_name> cmzn_set_FE_field;

/*
Private functions
-----------------
*/

PROTOTYPE_INDEXED_LIST_STL_IDENTIFIER_CHANGE_FUNCTIONS(FE_field,name);

struct FE_field_info *CREATE(FE_field_info)(struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Creates a struct FE_field_info with a pointer to <fe_region>.
Note:
This should only be called by FE_region functions, and the FE_region must be
its own master. The returned object is owned by the FE_region.
It maintains a non-ACCESSed pointer to its owning FE_region which the FE_region
will clear before it is destroyed. If it becomes necessary to have other owners
of these objects, the common parts of it and FE_region should be extracted to a
common object.
==============================================================================*/

int DESTROY(FE_field_info)(
	struct FE_field_info **fe_field_info_address);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Destroys the FE_field_info at *<field_info_address>. Frees the
memory for the information and sets <*field_info_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_field_info);

int FE_field_info_clear_FE_region(struct FE_field_info *field_info);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Clears the pointer to FE_region in <field_info>.
Private function only to be called by destroy_FE_region.
==============================================================================*/

int FE_field_set_FE_field_info(struct FE_field *fe_field,
	struct FE_field_info *fe_field_info);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Changes the FE_field_info at <fe_field> to <fe_field_info>.
Private function only to be called by FE_region when merging FE_regions!
==============================================================================*/

int FE_field_set_indexer_field(struct FE_field *fe_field,
	struct FE_field *indexer_fe_field);
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
If <fe_field> is already indexed, substitutes <indexer_fe_field>.
Does not change any of the values currently stored in <fe_field>
Used to merge indexed fields into different FE_regions; should not be used for
any other purpose.
==============================================================================*/

int FE_field_log_FE_field_change(struct FE_field *fe_field,
	void *fe_field_change_log_void);
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION :
Logs the field in <fe_field> as RELATED_OBJECT_CHANGED in the
struct CHANGE_LOG(FE_field) pointed to by <fe_field_change_log_void>.
???RC Later may wish to allow more than just RELATED_OBJECT_CHANGED, or have
separate functions for each type.
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_FIELD_PRIVATE_HPP) */
