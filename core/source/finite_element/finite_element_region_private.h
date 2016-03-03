/*******************************************************************************
FILE : finite_element_region_private.h

LAST MODIFIED : 21 April 2005

DESCRIPTION :
Private interfaces to FE_region types and functions to be included only by
privileged finite_element modules.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_REGION_PRIVATE_H)
#define FINITE_ELEMENT_REGION_PRIVATE_H

#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region.h"

/*
Private types
-------------
*/

/**
 * Reference counted sets of all element bases and shapes, shared by all regions in context.
 */
class FE_region_bases_and_shapes
{
	struct MANAGER(FE_basis) *basis_manager;
	struct LIST(FE_element_shape) *element_shape_list;
	int access_count;

	FE_region_bases_and_shapes();
	~FE_region_bases_and_shapes();

public:
	static FE_region_bases_and_shapes *create()
	{
		return new FE_region_bases_and_shapes();
	}

	FE_region_bases_and_shapes *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(FE_region_bases_and_shapes* &bases_and_shapes)
	{
		if (!bases_and_shapes)
			return;
		--(bases_and_shapes->access_count);
		if (bases_and_shapes->access_count <= 0)
			delete bases_and_shapes;
		bases_and_shapes = 0;
	}

	inline struct MANAGER(FE_basis) *getBasisManager() const
	{
		return this->basis_manager;
	}

	inline struct LIST(FE_element_shape) *getElementShapeList() const
	{
		return this->element_shape_list;
	}
};

struct FE_region
{
	/* pointer to the cmzn_region this FE_region is in: NOT ACCESSED */
	struct cmzn_region *cmiss_region;

	/* field information */
	struct FE_time_sequence_package *fe_time;
	struct LIST(FE_field) *fe_field_list;
	struct FE_field_info *fe_field_info;

	/* FE bases and shapes shared by all regions */
	FE_region_bases_and_shapes *bases_and_shapes;

	/* lists of nodes and elements in this region */
	FE_nodeset *nodesets[2];
	FE_mesh *meshes[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	/* change_level: if zero, change messages are sent with every change,
		 otherwise incremented/decremented for nested changes */
	int change_level;
	/* internal record of changes from which FE_field_changes is constructed */
	/* fields added, removed or otherwise changed are put in the following list.
		 Note this records changes to the FE_field structure itself, NOT changes to
		 values and representations of the field in nodes and elements -- these are
		 recorded separately as described below */
	struct CHANGE_LOG(FE_field) *fe_field_changes;

	/* initially false, these flags are set to true once wrapping computed field
	 * manager is informed that it should create cmiss_number and xi fields
	 * See: FE_region_need_add_cmiss_number_field, FE_region_need_add_xi_field */
	bool informed_make_cmiss_number_field;
	bool informed_make_xi_field;

	/* number of objects using this region */
	int access_count;

	FE_region(FE_region *base_fe_region);

	~FE_region();

	void createChangeLogs();

	void update();

	inline void FE_field_change(FE_field *fe_field, enum CHANGE_LOG_CHANGE(FE_field) change)
	{
		CHANGE_LOG_OBJECT_CHANGE(FE_field)(this->fe_field_changes, fe_field,  change);
		this->update();
	}

};

/*
Private functions
-----------------
*/

struct FE_field_info *FE_region_get_FE_field_info(
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns a struct FE_field_info for <fe_region>.
This is an object private to FE_region that is common between all fields
owned by FE_region. FE_fields access this object, but this object maintains
a non-ACCESSed pointer to <fe_region> so fields can determine which FE_region
they belong to.
==============================================================================*/

/**
 * Private function for use by computed_field_finite_element field wrapping
 * code, to be called in response to region changes - tells it whether it to
 * create a cmiss_number field. Informs only once and requires the existence
 * of a node, element or data point (in the matching data_hack_fe_region).
 * 
 * @param fe_region the finite_element region to check for.
 * @return true if cmiss_number field needs to be made, 0 if not
 */
bool FE_region_need_add_cmiss_number_field(struct FE_region *fe_region);

/**
 * Private function for use by computed_field_finite_element field wrapping
 * code, to be called in response to region changes - tells it whether to
 * create an xi coordinates field. Informs only once - if any elements exist.
 * 
 * @param fe_region the finite_element region to check for.
 * @return true if cmiss_number field needs to be made, 0 if not
 */
bool FE_region_need_add_xi_field(struct FE_region *fe_region);

#endif /* !defined (FINITE_ELEMENT_REGION_PRIVATE_H) */
