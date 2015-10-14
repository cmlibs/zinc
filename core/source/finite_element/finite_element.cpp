/**
 * FILE : finite_element.cpp
 *
 * Finite element data structures and functions.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*???DB.  Testing */
#define DOUBLE_FOR_DOT_PRODUCT

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include "zinc/status.h"
#include "general/cmiss_set.hpp"
#include "general/indexed_list_stl_private.hpp"
#include "general/list_btree_private.hpp"
#include <math.h>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region_private.h"
#include "general/change_log_private.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/indexed_list_private.h"
#include "general/list_private.h"
#include "general/matrix_vector.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/value.h"
#include "general/message.h"

/*
Module Constants
----------------
*/

/* need following to handle 64-bit alignment problems of 64-bit quantities in
   64-bit version */
#if defined (O64)
#define VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE 8
#define ADJUST_VALUE_STORAGE_SIZE( new_values_storage_size ) \
/* round size up to nearest 64-bit boundary */ \
if (new_values_storage_size % 8) \
{ \
	new_values_storage_size += (8 - (new_values_storage_size % 8)); \
}
#else /* defined (O64) */
#define VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE sizeof(int)
#define ADJUST_VALUE_STORAGE_SIZE( new_values_storage_size )
/* do nothing */
#endif /* defined (O64) */

#define VALUE_STORAGE_NUMBER_OF_TIMES_BLOCK (30)
/*
Module types
------------
*/

struct FE_field_info
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Information common to a set of FE_fields.
Currently only maintains a non-ACCESSed pointer to the owning FE_region.
==============================================================================*/
{
	/* the FE_region this FE_field_info and all FE_fields using it belong to */
  struct FE_region *fe_region;

	/* the number of structures that point to this field information.  The
		 field information cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct FE_field_info */

struct FE_field
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
Stores the information for calculating the value of a field at a point.  The
position of the point should be specified by giving the element that contains
the point and the Xi coordinates of the point within the element.
==============================================================================*/
{
	/* the name of the field, which is its identifier */
	const char *name;
	/* shared info for this FE_field including the FE_region it belongs to */
	struct FE_field_info *info;
	/* CMISS field type */
	enum CM_field_type cm_field_type;
	struct FE_field_external_information *external;
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
	/* for value_type== ELEMENT_XI_VALUE, fixed mesh dimension, or 0 for any */
	int element_xi_mesh_dimension;
	/* array of global values/derivatives that are stored with the field.
	 * The actual values can be extracted using the <value_type> */
	Value_storage *values_storage;
	/*???DB.  Have something like enum FE_nodal_value_type ? */
	/*???DB.  Needs to be clarified for constant fields */
	/* time series information.  If <number_of_times> is zero then constant in
	 * time */
	enum Value_type time_value_type;
	int number_of_times;
	Value_storage *times;
	/* the number of computed fields wrapping this FE_field */
	int number_of_wrappers;
	/* the number of structures that point to this field.  The field cannot be
		destroyed while this is greater than 0 */
	int access_count;

	inline FE_field *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(FE_field **field_address)
	{
		return DEACCESS(FE_field)(field_address);
	}
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

FULL_DECLARE_CHANGE_LOG_TYPES(FE_field);

/**
 * Specifies how to find the value and derivatives for a field component at a
 * node. This only appears as part of a FE_node_field.
 */
struct FE_node_field_component
{
	/* the offset for the global value within the list of all global values and
		derivatives at the node */
	int value;
	/* the number of global derivatives.  NB The global derivatives are assumed to
		follow directly after the global value in the list of all global values and
		derivatives at the node */
	int number_of_derivatives;
	/* the number of versions at the node.  Different fields may use different
		versions eg if the node is on the axis in prolate spheroidal coordinates
		(mu=0 or mu=pi).  NB The different versions are assumed to follow directly
		after each other (only need one starting <value>) and to have the same
		<number_of derivatives> */
	int number_of_versions;
	/* the types the nodal values */
	enum FE_nodal_value_type *nodal_value_types;
};

/**
 * Describes storage of global values and derivatives for a field at a node.
 */
struct FE_node_field
{
	/* the field which this accesses values and derivatives for */
	struct FE_field *field;
	/* an array with <number_of_components> node field components */
	struct FE_node_field_component *components;
	/* the time dependence of all components below this point,
	   if it is non-NULL then every value storage must be an array of
	   values that matches this fe_time_sequence */
	struct FE_time_sequence *time_sequence;
	/* the number of structures that point to this node field.  The node field
		cannot be destroyed while this is greater than 0 */
	int access_count;
};

FULL_DECLARE_INDEXED_LIST_TYPE(FE_node_field);

/**
 * Describes how fields are defined for a node.
 */
struct FE_node_field_info
{
	/* the total number of values and derivatives */
	/*???RC not convinced number_of_values needs to be in this structure */
	int number_of_values;

	/* the size of the data in node->values->storage */
	int values_storage_size;

	/* list of the node fields */
	struct LIST(FE_node_field) *node_field_list;

	/* the FE_nodeset this FE_node_field_info and all nodes using it belong to */
	FE_nodeset *fe_nodeset;

	/* the number of structures that point to this node field information.  The
		node field information cannot be destroyed while this is greater than 0 */
	int access_count;
};

FULL_DECLARE_LIST_TYPE(FE_node_field_info);

struct FE_node_field_creator
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
==============================================================================*/
{
	int number_of_components;
	int *numbers_of_versions;
	int *numbers_of_derivatives;
	enum FE_nodal_value_type **nodal_value_types;
}; /* struct FE_node_field_creator */

struct FE_node
/*******************************************************************************
LAST MODIFIED : 11 February 2003

DESCRIPTION :
==============================================================================*/
{
	/* the unique number that identifies the node */
	int cm_node_identifier;

	/* the number of structures that point to this node.  The node cannot be
		destroyed while this is greater than 0 */
	int access_count;

	/* the fields defined at the node */
	struct FE_node_field_info *fields;
	/* the global values and derivatives for the fields defined at the node */
	Value_storage *values_storage;

	inline FE_node *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(FE_node **node_address)
	{
		return DEACCESS(FE_node)(node_address);
	}

	inline int get_identifier() const
	{
		return cm_node_identifier;
	}
}; /* struct FE_node */

/** can tweak this to vary performance */
const int CMZN_NODE_BTREE_ORDER = 10;

typedef cmzn_btree<cmzn_node,int,CMZN_NODE_BTREE_ORDER> cmzn_set_cmzn_node;

struct cmzn_nodeiterator : public cmzn_set_cmzn_node::ext_iterator
{
	int access_count;
	cmzn_nodeiterator(cmzn_set_cmzn_node *container) :
		cmzn_set_cmzn_node::ext_iterator(container),
		access_count(1)
	{
	}

	cmzn_nodeiterator_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_nodeiterator_id &iterator)
	{
		if (!iterator)
			return 0;
		--(iterator->access_count);
		if (iterator->access_count <= 0)
			delete iterator;
		iterator = 0;
		return 1;
	}
};

FULL_DECLARE_CHANGE_LOG_TYPES(FE_node);

/**
 * Stores the information for calculating element values by choosing nodal
 * values and applying a diagonal scale factor matrix.
 */
struct Standard_node_to_element_map
{
	/* the index, within the list of nodes for the element, of the node at which
		the values are stored */
	int node_index;
	/* the number of nodal values used (which is also the number of element values
		calculated) */
	int number_of_nodal_values;
	/* Legacy array only allocated and used when reading legacy EX files.
		array of indices for the nodal values, within the list of all values at
		the node - index is an offset in whole FE_values relative to the absolute
		value offset in the FE_node_field_component for the associated node (which
		is a values storage/unsigned char). */
	int *nodal_value_indices;
	// Arrays specifying the nodal value type and version for the nodal values.
	// Special case: FE_NODAL_UNKNOWN is used to mean use a zero parameter
	// Internally, the first version is 0.
	// When reading legacy EX files they are determined from the nodal value indices.
	// @see FE_element_field_info_check_field_node_value_labels
	FE_nodal_value_type *nodal_value_types;
	int *nodal_versions;
	/* array of indices for the scale factors.
		 Note a negative index indicates a unit scale factor */
	int *scale_factor_indices;
};

struct FE_element_node_scale_field_info;

namespace {

class ElementDOFMapEvaluationCache;
class ElementDOFMapMatchCache;

class ElementDOFMap
{
public:
	virtual ~ElementDOFMap()
	{
	}

	virtual ElementDOFMap *clone() = 0;

	virtual ElementDOFMap *cloneWithNewNodeIndices(
		FE_element_node_scale_field_info *mergeInfo,
		FE_element_node_scale_field_info *sourceInfo) = 0;

	/**
	 * Must override to evaluate DOF.
	 * @return  true if successfully evaluated, otherwise false
	 */
	virtual bool evaluate(ElementDOFMapEvaluationCache& cache, FE_value& value) = 0;

	/**
	 * Override to set node which DOF is mapped from, or set to zero if none.
	 * Default implementation is for non-node-based map.
	 * @return  true if successfully evaluated, otherwise false.
	 */
	virtual bool evaluateNode(ElementDOFMapEvaluationCache& /*cache*/, FE_node*& node)
	{
		node = 0;
		return true;
	}

	/**
	 * Override to return whether this map is defined identically to another map,
	 * i.e. is the same class with equal members.
	 * @param otherMap  Map to compare with, usually the one in global use.
	 * @return  true if maps match, otherwise false.
	 */
	virtual bool matches(ElementDOFMap *otherMap) = 0;

	/**
	 * Override to return whether this map is defined identically to another map,
	 * i.e. is the same class with equivalent definition with supporting node
	 * scale info.
	 * @param otherMap  Map to compare with, usually the one in global use.
	 * @param cache  Supporting node scale info and cache for matching.
	 * @return  true if maps match, otherwise false.
	 */
	virtual bool matchesWithInfo(ElementDOFMap *otherMap, ElementDOFMapMatchCache& cache) = 0;

	/** override to set flag for each local node in use */
	virtual void setLocalNodeInUse(int /*numberOfLocalNodes*/, int * /*localNodeInUse*/)
	{
	}
};

} // namespace { }

struct FE_element_field_component
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating element values, with respect to the
<basis>, from global values (this calculation includes the application of scale
factors).  There are two types - <NODE_BASED_MAP> and <GENERAL_LINEAR_MAP>.  For
a node based map, the global values are associated with nodes.  For a general
linear map, the global values do not have to be associated with nodes.  The node
based maps could be specified as general linear maps, but the node based
specification (required by CMISS) cannot be recovered from the general linear
map specification (important when the front end is being used to create meshs).
The <modify> function is called after the element values have been calculated
with respect to the <basis> and before the element values are blended to be with
respect to the standard basis.  The <modify> function is to allow for special
cases, such as nodes that have multiple theta values in cylindrical polar,
spherical polar, prolate spheroidal or oblate spheroidal coordinate systems -
either lying on the z-axis or being the first and last node in a circle.
==============================================================================*/
{
	/* the type of the global to element map */
	enum Global_to_element_map_type type;
	union
	{
		/* for a standard node based map */
		struct
		{
			/* the number of nodes */
			int number_of_nodes;
			/* how to get the element values from the nodal values */
			struct Standard_node_to_element_map **node_to_element_maps;
		} standard_node_based;
		/* for a general map */
		struct
		{
			int number_of_maps; // redundant; equals number of basis functions
			ElementDOFMap **maps;
		} general_map_based;
		/* for an element grid */
		struct
		{
			/* the element is covered by a regular grid with <number_in_xi>
				sub-elements in each direction */
			int *number_in_xi;
			/* the component is linear over each sub-element and the grid point values
				are stored with the element starting at <value_index> */
				/*???DB.  Other bases for the sub-elements are possible, but are not
					currently used and would add to the complexity/compute time */
			int value_index;
		} element_grid_based;
	} map;
	/* the basis that the element values are with respect to */
	struct FE_basis *basis;
	/* the function for modifying element values */
	FE_element_field_component_modify modify;
private:
	// accessed pointer to scale factor set; not used for ELEMENT_GRID_MAP
	cmzn_mesh_scale_factor_set *scale_factor_set;
public:

	/** Note: incomplete constructor
	 * @see CREATE(FE_element_field_component)
	 */
	FE_element_field_component() :
		basis(0),
		modify(0),
		scale_factor_set(0)
	{
	}

	/** Note: incomplete destructor
	 * @see DESTROY(FE_element_field_component)
	 */
	~FE_element_field_component()
	{
		DEACCESS(FE_basis)(&(this->basis));
		cmzn_mesh_scale_factor_set::deaccess(this->scale_factor_set);
	}

	/** @return  Non-accessed pointer to scale factor set, if any */
	cmzn_mesh_scale_factor_set *get_scale_factor_set()
	{
		return this->scale_factor_set;
	}

	int set_scale_factor_set(cmzn_mesh_scale_factor_set *scale_factor_set_in)
	{
		if (this->type != ELEMENT_GRID_MAP)
		{
			if (scale_factor_set_in)
				scale_factor_set_in->access();
			if (this->scale_factor_set)
				cmzn_mesh_scale_factor_set::deaccess(this->scale_factor_set);
			this->scale_factor_set = scale_factor_set_in;
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}
}; /* struct FE_element_field_component */

struct FE_element_field
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating the value of a field at a point within a
element.  The position of the point should be specified by Xi coordinates of the
point within the element.
==============================================================================*/
{
	/* the field which this is part of */
	struct FE_field *field;
	/* an array with <field->number_of_components> pointers to element field
		components */
	struct FE_element_field_component **components;
	/* the number of structures that point to this element field.  The element
		field cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct FE_element_field */

FULL_DECLARE_INDEXED_LIST_TYPE(FE_element_field);

struct FE_element_field_values
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
The values need to calculate a field on an element.  These structures are
calculated from the element field as required and are then destroyed.
==============================================================================*/
{
	/* the field these values are for */
	struct FE_field *field;
	/* the element these values are for */
	struct FE_element *element;
	/* the element the field was inherited from */
	struct FE_element *field_element;
	/* whether or not these values depend on time */
	int time_dependent;
	/* if the values are time dependent, the time at which they were calculated */
	FE_value time;
	/* number of sub-elements in each xi-direction of element for each component.
	 * If NULL (for a component) then field is not grid based for that component.
		 Notes:
		1.  Grid sub-elements are linear in each direction.  This means that
			<component_number_of_values> is not used
		2.  the grid-point values are not blended (to monomial) and so
			<component_standard_basis_functions> and
			<component_standard_basis_function_arguments> are not used
		3.  for grid-based <destroy_standard_basis_arguments> is used to specify
			if the <component_values> should be destroyed (element field has been
			inherited) */
	int **component_number_in_xi;
	/* a flag to specify whether or not values have also been calculated for the
		derivatives of the field with respect to the xi coordinates */
	char derivatives_calculated;
	/* a flag added to specify if the element field component modify function is
		ignored */
		/*???DB.  Added for calculating derivatives with respect to nodal values.
			See FE_element_field_values_set_no_modify */
	char no_modify;
	/* specify whether the standard basis arguments should be destroyed (element
		field has been inherited) or not be destroyed (element field is defined for
		the element and the basis arguments are being used) */
	char destroy_standard_basis_arguments;
	/* the number of field components */
	int number_of_components;
	/* the number of values for each component */
	int *component_number_of_values;
	/* the values_storage for each component if grid-based */
	Value_storage **component_grid_values_storage;
	/* grid_offset_in_xi is allocated with 2^number_of_xi_coordinates integers
		 giving the increment in index into the values stored with the top_level
		 element for the grid. For top_level_elements the first value is 1, the
		 second is (number_in_xi[0]+1), the third is
		 (number_in_xi[0]+1)*(number_in_xi[1]+1) etc. The base_grid_offset is 0 for
		 top_level_elements. For faces and lines these values are adjusted to get
		 the correct index for the top_level_element */
	int *component_base_grid_offset,**component_grid_offset_in_xi;
	/* following allocated with 2^number_of_xi for grid-based fields for use in
		 calculate_FE_element_field */
	int *element_value_offsets;
	/* the values for each component */
	FE_value **component_values;
	/* the standard basis function for each component */
	Standard_basis_function **component_standard_basis_functions;
	/* the arguments for the standard basis function for each component */
	int **component_standard_basis_function_arguments;
	/* working space for evaluating basis */
	FE_value *basis_function_values;

	int access_count;
}; /* struct FE_element_field_values */

FULL_DECLARE_INDEXED_LIST_TYPE(FE_element_field_values);

/**
 * The element fields defined on an element and how to calculate them.
 */
struct FE_element_field_info
{
	/* list of the  element fields */
	struct LIST(FE_element_field) *element_field_list;

	/* the FE_mesh this FE_element_field_info and all elements using it belong to */
	FE_mesh *fe_mesh;

	/* the number of structures that point to this information.  The information
		cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct FE_element_field_info */

FULL_DECLARE_LIST_TYPE(FE_element_field_info);

/**
 * The field values, nodes and scale factors for an element.
 * The element stores its FE_element_field_info, and optionally this structure
 * if fields are defined and need this supplemental information.
 */
struct FE_element_node_scale_field_info
{
public:
	/* values_storage.  Element based maps have indices into this array */
	int values_storage_size;
	Value_storage *values_storage;
	/* nodes.  Node to element maps have indices into this array */
	int number_of_nodes;
	struct FE_node **nodes;
private:
	/* there may be a number of sets of scale factors */
	int number_of_scale_factor_sets;
	/* unique identifiers for scale factors stored here. All scale factor set
	 * identifiers are listed with the mesh / FE_region. Accessed pointers */
	cmzn_mesh_scale_factor_set **scale_factor_set_identifiers;
	int *numbers_in_scale_factor_sets;
	/* all scale factors are stored in this array.  Global to element maps have
		indices into this array. General element map has relative offset into its
		scale factor set */
public:
	int number_of_scale_factors;
	FE_value *scale_factors;

private:

	FE_element_node_scale_field_info();

	/**
	 * Note that destructor does not clean up any dynamic values stored within
	 * the values_storage; call destroyDynamic with FE_element_field_info if there are
	 * dynamic arrays allocated in the values_storage, e.g. grid-based parameters.
	 */
	~FE_element_node_scale_field_info();

public:

	static FE_element_node_scale_field_info *create()
	{
		return new FE_element_node_scale_field_info();
	}

	static void destroy(FE_element_node_scale_field_info* &info)
	{
		delete info;
		info = 0;
	}

	/**
	 * Variant of destroy which cleans up dynamic arrays allocated in the
	 * values_storage, e.g. grid-based parameters.
	 */
	static void destroyDynamic(FE_element_node_scale_field_info* &info,
		FE_element_field_info *field_info);

	/**
	 * Creates a clone of this info without values storage array.
	 * Used exclusively by merge_FE_element.
	 */
	FE_element_node_scale_field_info *cloneWithoutValuesStorage();

	static FE_element_node_scale_field_info *createMergeWithoutValuesStorage(
		FE_element_node_scale_field_info& targetInfo,
		FE_element_node_scale_field_info& sourceInfo,
		std::vector<cmzn_mesh_scale_factor_set*> &changedExistingScaleFactorSets);

	/**
	 * Must supply element field info to copy dynamic values storage arrays.
	 */
	FE_element_node_scale_field_info *clone(FE_element_field_info *field_info);

	/**
	 * @param numberOfNodes  Number of nodes >= current number.
	 * @return  CMZN_OK on success, otherwise an error code.
	 */
	int setNumberOfNodes(int numberOfNodes);

	int setNode(int nodeNumber, cmzn_node *node);

	/**
	 * Set all scale factor set identifiers and numbers and allocate storage for
	 * scale factors.
	 * @param  Array of scale factors to copy. If omitted values are initialised to 0.
	 * @return CMZN_OK on success, otherwise any other error code.
	 */
	int setScaleFactorSets(int numberOfScaleFactorSetsIn,
		cmzn_mesh_scale_factor_set **scaleFactorSetIdentifiersIn,
		int *numbersInScaleFactorSetsIn, FE_value *scaleFactorsIn);

	int getNumberOfScaleFactorSets() const
	{
		return number_of_scale_factor_sets;
	}

	int getNumberInScaleFactorSetAtIndex(int index)
	{
		if ((0 <= index) && (index < this->number_of_scale_factor_sets))
		{
			return this->numbers_in_scale_factor_sets[index];
		}
		return 0;
	}

	/** @return  Non-accessed pointer to set identifier, or 0 id none or error */
	cmzn_mesh_scale_factor_set *getScaleFactorSetIdentifierAtIndex(int index)
	{
		if ((0 <= index) && (index < this->number_of_scale_factor_sets))
		{
			return this->scale_factor_set_identifiers[index];
		}
		return 0;
	}

	/** use with care only when merging from another region */
	int setScaleFactorSetIdentifierAtIndex(int index, cmzn_mesh_scale_factor_set *scaleFactorSet)
	{
		if ((0 <= index) && (index < this->number_of_scale_factor_sets) && scaleFactorSet)
		{
			scaleFactorSet->access();
			cmzn_mesh_scale_factor_set::deaccess(this->scale_factor_set_identifiers[index]);
			this->scale_factor_set_identifiers[index] = scaleFactorSet;
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	int getScaleFactorSetOffset(cmzn_mesh_scale_factor_set *scaleFactorSet, int &numberOfScaleFactors)
	{
		int offset = 0;
		for (int i = 0; i < this->number_of_scale_factor_sets; ++i)
		{
			if (this->scale_factor_set_identifiers[i] == scaleFactorSet)
			{
				numberOfScaleFactors = this->numbers_in_scale_factor_sets[i];
				return offset;
			}
			offset += this->numbers_in_scale_factor_sets[i];
		}
		numberOfScaleFactors = 0;
		return 0;
	}

	FE_value *getScaleFactorsForSet(cmzn_mesh_scale_factor_set *scaleFactorSet, int &numberOfScaleFactors)
	{
		int offset = this->getScaleFactorSetOffset(scaleFactorSet, numberOfScaleFactors);
		if (numberOfScaleFactors)
			return this->scale_factors + offset;
		return 0;
	}

};

struct FE_element_shape
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
A description of the shape of an element in Xi space.  It includes how to
calculate face coordinates from element coordinates and how to calculate element
coordinates from face coordinates.
==============================================================================*/
{
	/* the number of xi coordinates */
	int dimension;
	/* the structure description.  Similar to type for a FE_basis
		- doesn't have the dimension in the first position (type[0])
		- the diagonal entry is the shape type
		- non-zero off-diagonal entries indicate that the dimensions are linked.
			For a polygon, it is the number of vertices
		eg. a 5-gon in dimensions 1 and 2 and linear in the third dimension
			POLYGON_SHAPE 5             0
										POLYGON_SHAPE 0
																	LINE_SHAPE
		eg. tetrahedron
			SIMPLEX_SHAPE 1             1
										SIMPLEX_SHAPE 1
																	SIMPLEX_SHAPE */
	int *type;
	/* the number of faces */
	int number_of_faces;
	/* the equations for the faces of the element.  This is a linear system
		b = A xi, where A is <number_of_faces> by <dimension> matrix whose entries
		are either 0 or 1 and b is a vector whose entries are
		0,...number_of_faces_in_this_dimension-1.  For a cube the system would be
			0   1 0 0  xi1       2
			1   1 0 0  xi2       3
			0 = 0 1 0  xi3       4
			1   0 1 0            5
			0   0 0 1            8
			1   0 0 1            9
		For a 5-gon by linear above the system would be
			0   1 0 0  xi1       5
			1   1 0 0  xi2       6
			2   1 0 0  xi3       7
			3 = 1 0 0            8
			4   1 0 0            9
			0   0 0 1           10
			1   0 0 1           11
			The "equations" for the polygon faces, don't actually describe the faces,
				but are in 1 to 1 correspondence - first represents 0<=xi1<=1/5 and
				xi2=1.
		For a tetrahedron the system would be
			0   1 0 0  xi1       2
			0 = 0 1 0  xi2       4
			0   0 0 1  xi3       8
			1   1 1 1           15
		A unique number is calculated for each number as follows
		- a value is calculated for each column by multiplying the number for the
			previous column (start with 1, left to right) by
			- the number_of_vertices for the first polygon column
			- 1 for the second polygon column
			- 2 otherwise
		- a value for each row by doing the dot product of the row and the column
			values
		- the entry of b for that row is added to the row value to give the unique
			number
		These numbers are stored in <faces>.
		The values for the above examples are given on the right */
	int *faces;
	/* for each face an outwards pointing normal stored as a vector of size <dimension> */
	FE_value *face_normals;
	/* for each face an affine transformation, b + A xi for calculating the
		element xi coordinates from the face xi coordinates.  For each face there is
		a <number_of_xi_coordinates> by <number_of_xi_coordinates> array with the
		translation b in the first column.  For a cube the translations could be
		(not unique)
		face 2 :  0 0 0 ,  face 3 : 1 0 0 ,  face 4 : 0 0 1 , 1(face) goes to 3 and
							0 1 0             0 1 0             0 0 0   2(face) goes to 1 to
							0 0 1             0 0 1             0 1 0   maintain right-
																													handedness (3x1=2)
		face 5 :  0 0 1 ,  face 8 : 0 1 0 ,  face 9 : 0 1 0
							1 0 0             0 0 1             0 0 1
							0 1 0             0 0 0             1 0 0
		For the 5-gon by linear the faces would be
		face 5 :  0 1/5 0 ,  face 6 : 1/5 1/5 0 ,  face 7 : 2/5 1/5 0
							1 0   0             1   0   0             1   0   0
							0 0   1             0   0   1             0   0   1
		face 8 :  3/5 1/5 0 ,  face 9 : 4/5 1/5 0
							1   0   0             1   0   0
							0   0   1             0   0   1
		face 10 : 0 1 0 ,  face 11 : 0 1 0
							0 0 1              0 0 1
							0 0 0              1 0 0
		For the tetrahedron the faces would be
		face 2 :  0 0 0 ,  face 4 : 1 -1 -1 ,  face 8 : 0  0  1
							0 1 0             0  0  0             1 -1 -1
							0 0 1             0  1  0             0  0  0
		face 15 : 0  1  0
							0  0  1
							1 -1 -1
		The transformations are stored by row (ie. column number varying fastest) */
	FE_value *face_to_element;
	/* the number of structures that point to this shape.  The shape cannot be
		destroyed while this is greater than 0 */
	int access_count;
}; /* struct FE_element_shape */

FULL_DECLARE_LIST_TYPE(FE_element_shape);

/**
 * A region in space with functions defined on the region.  The region is
 * parameterized and the functions are known in terms of the parameterized
 * variables.
 * Note the shape is now held by the owning FE_mesh at the index.
 */
struct FE_element
{
	/* index into mesh labels, maps to unique identifier */
	DsLabelIndex index;
	/* the number of structures that point to this element.  The element cannot be
		destroyed while this is greater than 0 */
	int access_count;

	/* the faces (finite elements of dimension 1 less) of the element.  The number
		of faces is known from the <shape> */
	struct FE_element **faces;

	/* the field information for the element, also linking to owning FE_mesh */
	struct FE_element_field_info *fields;
	/* the nodes, scale factors and valuess for the element.  This is only set if
		 the element has fields that require this supplemental information */
	struct FE_element_node_scale_field_info *information;

	/* the parent elements are of dimension 1 greater and are the elements for
		which this element is a face */
	/* parent pointers are non-ACCESSed so parent element able to be destroyed */
	struct FE_element **parents;
	int number_of_parents;

	inline FE_element *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(FE_element **element_address)
	{
		return DEACCESS(FE_element)(element_address);
	}

	inline DsLabelIdentifier get_identifier() const
	{
		if (this->fields)
			return this->fields->fe_mesh->getElementIdentifier(this->index);
		return DS_LABEL_IDENTIFIER_INVALID;
	}

	inline int getDimension()
	{
		if (this->fields)
			return this->fields->fe_mesh->getDimension();
		display_message(ERROR_MESSAGE, "cmzn_element::getDimension.  Invalid element");
		return 0;
	}
}; /* struct FE_element */

/**
 * @see struct FE_element_type_node_sequence.
 */
struct FE_element_type_node_sequence_identifier
{
	/* node_numbers must be ordered from lowest to highest to work! */
	int *node_numbers,number_of_nodes;
};

struct FE_element_type_node_sequence
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Structure for storing an element with its identifier being its cm_type and the
number and list - in ascending order - of the nodes referred to by the default
coordinate field of the element. Indexed lists, indexed using function
compare_FE_element_type_node_sequence_identifier ensure that recalling a line or
face with the same nodes is extremely rapid. FE_mesh
uses them to find faces and lines for elements without them, if they exist.

???RC Can only match faces correctly for coordinate fields with standard node
to element maps and no versions. A grid-based coordinate field would fail
utterly since it has no nodes. A possible future solution for all cases is to
match the geometry exactly either by using the FE_element_field_values
(coefficients of the monomial basis functions), although there is a problem with
xi-directions not matching up, or actual centre positions of the face being a
trivial rejection, narrowing down to a single face or list of faces to compare
against.
==============================================================================*/
{
	struct FE_element_type_node_sequence_identifier identifier;
	struct FE_element *element;
	int dimension; // can be different from element dimension if created with face number
	int access_count;
}; /* struct FE_element_line_face_node_sequence */

FULL_DECLARE_INDEXED_LIST_TYPE(FE_element_type_node_sequence);

struct FE_node_field_iterator_and_data
{
	FE_node_field_iterator_function *iterator;
	struct FE_node *node;
	void *user_data;
}; /* struct FE_node_field_iterator_and_data */

struct FE_element_field_iterator_and_data
{
	FE_element_field_iterator_function *iterator;
	struct FE_element *element;
	void *user_data;
}; /* struct FE_element_field_iterator_and_data */

struct FE_field_order_info
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Stores a list of fields in the order they are added.
???RC move up in file.
==============================================================================*/
{
	int allocated_number_of_fields, number_of_fields;
	struct FE_field **fields;
}; /* FE_field_order_info */

/*
Module functions
----------------
*/

static int get_Value_storage_size(enum Value_type value_type,
	struct FE_time_sequence *time_sequence)
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Given the value type, returns the size in bytes of the memory required to store
the following:
For non-array type, the actual data.
For array types, an integer storing the number of array values, and a pointer to
the array values.
for time depedant types, a pointer to the values.
==============================================================================*/
{
	int size;

	ENTER(get_Value_storage_size);
	if (time_sequence)
	{
		switch (value_type)
		{
			case DOUBLE_VALUE:
			{
				size = sizeof(double *);
			} break;
			case FE_VALUE_VALUE:
			{
				size = sizeof(FE_value *);
			} break;
			case FLT_VALUE:
			{
				size = sizeof(float *);
			} break;
			case SHORT_VALUE:
			{
				size = sizeof(short *);
			} break;
			case INT_VALUE:
			{
				size = sizeof(int *);
			} break;
			case UNSIGNED_VALUE:
			{
				size = sizeof(unsigned *);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"get_Value_storage_size.  Not implemented time array value type.");
				size =0;
			} break;
		}
	}
	else
	{
		switch (value_type)
		{
			case DOUBLE_VALUE:
			{
				size = sizeof(double);
			} break;
			case ELEMENT_XI_VALUE:
			{
				/* need this to handle 64-bit alignment problems of 64-bit quantities in
					64-bit version */
#if defined (O64)
				size = (sizeof(struct FE_element *) + sizeof(FE_value) *
					MAXIMUM_ELEMENT_XI_DIMENSIONS) -
					((sizeof(struct FE_element *) + sizeof(FE_value) *
						MAXIMUM_ELEMENT_XI_DIMENSIONS)%8) + 8;
#else /* defined (O64) */
				size = sizeof(struct FE_element *) + sizeof(FE_value) *
					MAXIMUM_ELEMENT_XI_DIMENSIONS;
#endif /* defined (O64) */
			} break;
			case FE_VALUE_VALUE:
			{
				size = sizeof(FE_value);
			} break;
			case FLT_VALUE:
			{
				size = sizeof(float);
			} break;
			case SHORT_VALUE:
			{
				size = sizeof(short);
			} break;
			case INT_VALUE:
			{
				size = sizeof(int);
			} break;
			case UNSIGNED_VALUE:
			{
				size = sizeof(unsigned);
			} break;
			case DOUBLE_ARRAY_VALUE:
			{
				/* VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE space for number of array values */
				/* (*double) to store pointer to data*/
				size = VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE+sizeof(double *);
			} break;
			case FE_VALUE_ARRAY_VALUE:
			{
				size = VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE+sizeof(FE_value *);
			} break;
			case FLT_ARRAY_VALUE:
			{
				size = VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE+sizeof(float *);
			} break;
			case SHORT_ARRAY_VALUE:
			{
				size = VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE+sizeof(short *);
			} break;
			case INT_ARRAY_VALUE:
			{
				size = VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE+sizeof(int *);
			} break;
			case UNSIGNED_ARRAY_VALUE:
			{
				size = VALUE_STORAGE_ARRAY_LENGTH_INT_SIZE+sizeof(unsigned *);
			} break;
			case STRING_VALUE:
			{
				size = sizeof(char *);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"get_Value_storage_size.  Unknown value_type");
				size =0;
			} break;
		}
	}
	LEAVE;

	return (size);
} /* get_Value_storage_size */

static int free_value_storage_array(Value_storage *values_storage,
	enum Value_type value_type, struct FE_time_sequence *time_sequence,
	int number_of_values)
/*******************************************************************************
LAST MODIFIED: 20 November 2001

DESCRIPTION:
DEACCESSes objects and DEALLOCATEs dynamic storage in use by <values_storage>,
which is assumed to have <number_of_values> of the given <value_type> and
whether <time_sequence> is set.
Note that the values_storage array itself is not DEALLOCATED - up to the
calling function to do this.
Only certain value types, eg. arrays, strings, element_xi require this.
==============================================================================*/
{
	Value_storage *the_values_storage;
	int i,return_code,size;

	ENTER(free_value_storage_array);
	if (values_storage&&(size=get_Value_storage_size(value_type, time_sequence))&&
		(0<number_of_values))
	{
		return_code=1;
		the_values_storage = values_storage;
		if (time_sequence)
		{
			switch (value_type)
			{
				case DOUBLE_VALUE:
				{
					double **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (double **)the_values_storage;
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case FE_VALUE_VALUE:
				{
					FE_value **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (FE_value **)the_values_storage;
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case FLT_VALUE:
				{
					float **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (float **)the_values_storage;
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case INT_VALUE:
				{
					int **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (int **)the_values_storage;
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case UNSIGNED_VALUE:
				{
					unsigned **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (unsigned **)the_values_storage;
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				default:
				{
					display_message(WARNING_MESSAGE,
						"free_value_storage_array.  Time array not cleaned up for value_type");
				} break;
			}
		}
		else
		{
			switch (value_type)
			{
				case DOUBLE_ARRAY_VALUE:
				{
					double **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (double **)(the_values_storage+sizeof(int));
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case ELEMENT_XI_VALUE:
				{
					struct FE_element **element_address;

					for (i=0;i<number_of_values;i++)
					{
						element_address = (struct FE_element **)the_values_storage;
						if (*element_address)
						{
							DEACCESS(FE_element)(element_address);
						}
						the_values_storage += size;
					}
				} break;
				case FE_VALUE_ARRAY_VALUE:
				{
					FE_value **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (FE_value **)(the_values_storage+sizeof(int));
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case FLT_ARRAY_VALUE:
				{
					float **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (float **)(the_values_storage+sizeof(int));
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case INT_ARRAY_VALUE:
				{
					int **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (int **)(the_values_storage+sizeof(int));
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case UNSIGNED_ARRAY_VALUE:
				{
					unsigned **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (unsigned **)(the_values_storage+sizeof(int));
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				case STRING_VALUE:
				{
					char **array_address;

					for (i=0;i<number_of_values;i++)
					{
						array_address = (char **)(the_values_storage);
						DEALLOCATE(*array_address);
						the_values_storage += size;
					}
				} break;
				default:
				{
					// nothing to do for other types
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"free_value_storage_array. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* free_value_storage_array */

static int allocate_and_copy_values_storage_array(Value_storage *source,
	enum Value_type value_type, Value_storage *dest)
/************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION
Allocate an array of type value_type, of the length stored in source.
Copy the data from the array referenced by the pointer in source to the
alocated array. Copy the number of array values and the pointer to the
allocated array into dest.

Therefore must must free dest in calling function, and dest must be
an unallocated pointer when this function is called.

NOTE:
For array types, the contents of values_storage is:
 | int (number of array values) | pointer to array (eg double *) |
  x number_of_values
Assumes that sizeof(int) = 1 DWORD, so that the pointers are a DWORD aligned
in memory. If pointers weren't DWORD aligned get bus errors on SGIs.
=======================================================================*/
{
	int number_of_array_values,array_size,return_code;


	ENTER(allocate_and_copy_values_storage_array);
	if (source)
	{
		return_code = 1;
		switch (value_type)
		{
			case DOUBLE_ARRAY_VALUE:
			{
				double *dest_array,*source_array,**array_address;
				/* get number of array values from source */
				number_of_array_values = *((int *)source);
				if (number_of_array_values) /* no array values, source array is NULL*/
				{
					/* get address of array from source */
					array_address = (double **)(source+sizeof(int));
					source_array = *array_address;
					array_size = (sizeof(double))*(number_of_array_values);
					/* allocate the dest array */
					if (ALLOCATE(dest_array,double,number_of_array_values))
					{
						/* copy values into the dest array */
						memcpy(dest_array,source_array,array_size);
						/* copy the number of array values into the dest values_storage */
						*((int *)dest) = number_of_array_values;
						/* copy the address of the new array into the dest values_storage*/
						array_address = (double **)(dest+sizeof(int));
						*array_address = dest_array;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"allocate_and_copy_values_storage_array. Out of memory");
						return_code = 0;
					}
				}
				else
				{
					/* copy the number of array values = 0 into the dest values_storage */
					*((int *)dest) = 0;
					/* copy the  NULL address of the new array into the dest
						values_storage*/
					array_address = (double **)(dest+sizeof(int));
					*array_address = (double *)NULL;
				}
			} break;
			case FE_VALUE_ARRAY_VALUE:
			{
				FE_value *dest_array,*source_array,**array_address;
				/* get number of array values from source */
				number_of_array_values = *((int *)source);
				if (number_of_array_values) /* no array values, source array is NULL*/
				{
					/* get address of array from source */
					array_address = (FE_value **)(source+sizeof(int));
					source_array = *array_address;
					array_size = (sizeof(FE_value))*(number_of_array_values);
					/* allocate the dest array */
					if (ALLOCATE(dest_array,FE_value,number_of_array_values))
					{
						/* copy values into the dest array */
						memcpy(dest_array,source_array,array_size);
						/* copy the number of array values into the dest values_storage */
						*((int *)dest) = number_of_array_values;
						/* copy the address of the new array into the dest values_storage*/
						array_address = (FE_value **)(dest+sizeof(int));
						*array_address = dest_array;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"allocate_and_copy_values_storage_array. Out of memory");
						return_code = 0;
					}
				}
				else
				{
					/* copy the number of array values = 0 into the dest values_storage */
					*((int *)dest) = 0;
					/* copy the  NULL address of the new array into the dest values_storage*/
					array_address = (FE_value **)(dest+sizeof(int));
					*array_address = (FE_value *)NULL;
				}
			} break;
			case FLT_ARRAY_VALUE:
			{
				float *dest_array,*source_array,**array_address;
				/* get number of array values from source */
				number_of_array_values = *((int *)source);
				if (number_of_array_values) /* no array values, source array is NULL*/
				{
					/* get address of array from source */
					array_address = (float **)(source+sizeof(int));
					source_array = *array_address;
					array_size = (sizeof(float))*(number_of_array_values);
					/* allocate the dest array */
					if (ALLOCATE(dest_array,float,number_of_array_values))
					{
						/* copy values into the dest array */
						memcpy(dest_array,source_array,array_size);
						/* copy the number of array values into the dest values_storage */
						*((int *)dest) = number_of_array_values;
						/* copy the address of the new array into the dest values_storage*/
						array_address = (float **)(dest+sizeof(int));
						*array_address = dest_array;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"allocate_and_copy_values_storage_array. Out of memory");
						return_code = 0;
					}
				}
				else
				{
					/* copy the number of array values = 0 into the dest values_storage */
					*((int *)dest) = 0;
					/* copy the  NULL address of the new array into the dest values_storage*/
					array_address = (float **)(dest+sizeof(int));
					*array_address = (float *)NULL;
				}
			} break;
			case SHORT_ARRAY_VALUE:
			{
				short *dest_array,*source_array,**array_address;
				/* get number of array values from source */
				number_of_array_values = *((int *)source);
				if (number_of_array_values) /* no array values, source array is NULL*/
				{
					/* get address of array from source */
					array_address = (short **)(source+sizeof(int));
					source_array = *array_address;
					array_size = (sizeof(short))*(number_of_array_values);
					/* allocate the dest array */
					if (ALLOCATE(dest_array,short,number_of_array_values))
					{
						/* copy values into the dest array */
						memcpy(dest_array,source_array,array_size);
						/* copy the number of array values into the dest values_storage */
						*((int *)dest) = number_of_array_values;
						/* copy the address of the new array into the dest values_storage*/
						array_address = (short **)(dest+sizeof(int));
						*array_address = dest_array;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"allocate_and_copy_values_storage_array. Out of memory");
						return_code = 0;
					}
				}
				else
				{
					/* copy the number of array values = 0 into the dest values_storage */
					*((int *)dest) = 0;
					/* copy the  NULL address of the new array into the dest values_storage*/
					array_address = (short **)(dest+sizeof(int));
					*array_address = (short *)NULL;
				}
			} break;
			case INT_ARRAY_VALUE:
			{
				int *dest_array,*source_array,**array_address;
				/* get number of array values from source */
				number_of_array_values = *((int *)source);
				if (number_of_array_values) /* no array values, source array is NULL*/
				{
					/* get address of array from source */
					array_address = (int **)(source+sizeof(int));
					source_array = *array_address;
					array_size = (sizeof(int))*(number_of_array_values);
					/* allocate the dest array */
					if (ALLOCATE(dest_array,int,number_of_array_values))
					{
						/* copy values into the dest array */
						memcpy(dest_array,source_array,array_size);
						/* copy the number of array values into the dest values_storage */
						*((int *)dest) = number_of_array_values;
						/* copy the address of the new array into the dest values_storage*/
						array_address = (int **)(dest+sizeof(int));
						*array_address = dest_array;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"allocate_and_copy_values_storage_array. Out of memory");
						return_code = 0;
					}
				}
				else
				{
					/* copy the number of array values = 0 into the dest values_storage */
					*((int *)dest) = 0;
					/* copy the  NULL address of the new array into the dest values_storage*/
					array_address = (int **)(dest+sizeof(int));
					*array_address = (int *)NULL;
				}
			} break;
			case UNSIGNED_ARRAY_VALUE:
			{
				unsigned *dest_array,*source_array,**array_address;
				/* get number of array values from source */
				number_of_array_values = *((int *)source);
				if (number_of_array_values) /* no array values, source array is NULL*/
				{
					/* get address of array from source */
					array_address = (unsigned **)(source+sizeof(int));
					source_array = *array_address;
					array_size = (sizeof(unsigned))*(number_of_array_values);
					/* allocate the dest array */
					if (ALLOCATE(dest_array,unsigned,number_of_array_values))
					{
						/* copy values into the dest array */
						memcpy(dest_array,source_array,array_size);
						/* copy the number of array values into the dest values_storage */
						*((int *)dest) = number_of_array_values;
						/* copy the address of the new array into the dest values_storage*/
						array_address = (unsigned **)(dest+sizeof(int));
						*array_address = dest_array;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"allocate_and_copy_values_storage_array. Out of memory");
						return_code = 0;
					}
				}
				else
				{
					/* copy the number of array values = 0 into the dest values_storage */
					*((int *)dest) = 0;
					/* copy the  NULL address of the new array into the dest values_storage*/
					array_address = (unsigned **)(dest+sizeof(int));
					*array_address = (unsigned *)NULL;
				}
			} break;
			case STRING_VALUE:
			{
				char *dest_array,*source_array,**array_address;
				/* get address of array from source */
				array_address = (char **)(source);
				if (*array_address)/* if we have a source array*/
				{
					source_array = *array_address;
					array_size = static_cast<int>(strlen(source_array)) + 1; /* +1 for null termination */
					/* allocate the dest array */
					if (ALLOCATE(dest_array,char,array_size))
					{
						/* copy values into the dest array */
						memcpy(dest_array,source_array,array_size);
						/* copy the address of the new array into the dest values_storage*/
						array_address = (char **)(dest);
						*array_address = dest_array;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"allocate_and_copy_values_storage_array. Out of memory");
						return_code = 0;
					}
				}
				else	/* copy NULL into the dest values_storage*/
				{
					array_address = (char **)(dest);
					*array_address = (char *)NULL;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"allocate_and_copy_values_storage_array. Invalid type");
				return_code = 0;
			} break;
		} /*switch (the_value_type) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"allocate_and_copy_values_storage_array."
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* allocate_and_copy_values_storage_array */

static int copy_time_sequence_values_storage_array(Value_storage *source,
	enum Value_type value_type, struct FE_time_sequence *source_time_sequence,
	struct FE_time_sequence *destination_time_sequence, Value_storage *dest)
/************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION
Copy the data from the array referenced by the pointer in <source> to the
array referenced by the pointer in <dest>.

The destination must already have arrays allocated corresponding to the
<destination_time_sequence>.

NOTE:
For array types, the contents of values_storage is:
 | pointer to array (eg double *) |
  x number_of_values
Assumes that sizeof(int) = 1 DWORD, so that the pointers are a DWORD aligned
in memory. If pointers weren't DWORD aligned get bus errors on SGIs.
=======================================================================*/
{
	int destination_time_index, source_number_of_times,
		source_time_index, return_code;
	FE_value time;

	ENTER(copy_time_sequence_values_storage_array);
	if (source)
	{
		return_code = 1;
		source_number_of_times = FE_time_sequence_get_number_of_times(
			source_time_sequence);
		/* Copy the values into the correct places */
		for (source_time_index = 0 ; source_time_index < source_number_of_times
			 ; source_time_index++)
		{
			if (FE_time_sequence_get_time_for_index(source_time_sequence,
					 source_time_index, &time) &&
				FE_time_sequence_get_index_for_time(destination_time_sequence,
					time, &destination_time_index))
			{
				switch (value_type)
				{
					case DOUBLE_VALUE:
					{
						double *dest_array,*source_array,**array_address;
						/* get address of array from source */
						array_address = (double **)source;
						source_array = *array_address;
						array_address = (double **)dest;
						dest_array = *array_address;
						dest_array[destination_time_index] =
							source_array[source_time_index];
					} break;
					case FE_VALUE_VALUE:
					{
						FE_value *dest_array,*source_array,**array_address;
						/* get address of array from source */
						array_address = (FE_value **)source;
						source_array = *array_address;
						array_address = (FE_value **)dest;
						dest_array = *array_address;
						dest_array[destination_time_index] =
							source_array[source_time_index];
					} break;
					case FLT_VALUE:
					{
						float *dest_array,*source_array,**array_address;
						/* get address of array from source */
						array_address = (float **)source;
						source_array = *array_address;
						array_address = (float **)dest;
						dest_array = *array_address;
						dest_array[destination_time_index] =
							source_array[source_time_index];
					} break;
					case SHORT_VALUE:
					{
						short *dest_array,*source_array,**array_address;
						/* get address of array from source */
						array_address = (short **)source;
						source_array = *array_address;
						array_address = (short **)dest;
						dest_array = *array_address;
						dest_array[destination_time_index] =
							source_array[source_time_index];
					} break;
					case INT_VALUE:
					{
						int *dest_array,*source_array,**array_address;
						/* get address of array from source */
						array_address = (int **)source;
						source_array = *array_address;
						array_address = (int **)dest;
						dest_array = *array_address;
						dest_array[destination_time_index] =
							source_array[source_time_index];
					} break;
					case UNSIGNED_ARRAY_VALUE:
					{
						unsigned *dest_array,*source_array,**array_address;
						/* get address of array from source */
						array_address = (unsigned **)source;
						source_array = *array_address;
						array_address = (unsigned **)dest;
						dest_array = *array_address;
						dest_array[destination_time_index] =
							source_array[source_time_index];
					} break;
					case STRING_VALUE:
					{
						display_message(ERROR_MESSAGE,
							"copy_time_sequence_values_storage_array.  "
							"String type not implemented for multiple times yet.");
						return_code = 0;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"copy_time_sequence_values_storage_array.  Invalid type");
						return_code = 0;
					} break;
				} /*switch (the_value_type) */

			}
			else
			{
				display_message(ERROR_MESSAGE,"copy_time_sequence_values_storage_array.  "
					"Unable to find destination space for source time index %d",
					source_time_index);
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"copy_time_sequence_values_storage_array."
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* copy_time_sequence_values_storage_array */

int copy_time_sequence_values_storage_arrays(Value_storage *destination,
	enum Value_type value_type, struct FE_time_sequence *destination_time_sequence,
	struct FE_time_sequence *source_time_sequence, int number_of_values,
	Value_storage *source)
/*******************************************************************************
LAST MODIFIED : 1 November 2002

DESCRIPTION :
Calls copy_time_sequence_values_storage for all <number_of_values> items of
the appropriate value size to transfer time value arrays from <source> with
<source_time_sequence> to <destination> with <destination_time_sequence>.
==============================================================================*/
{
	int i, return_code, value_size;
	Value_storage *src,*dest;

	ENTER(copy_time_sequence_values_storage_arrays);
	if (destination && destination_time_sequence && source_time_sequence &&
		(0 < number_of_values) && source)
	{
		return_code = 1;
		dest = destination;
		src = source;
		value_size = get_Value_storage_size(value_type, destination_time_sequence);
		for (i = 0; (i < number_of_values) && return_code; i++)
		{
			if (!copy_time_sequence_values_storage_array(src, value_type,
				source_time_sequence, destination_time_sequence, dest))
			{
				display_message(ERROR_MESSAGE,
					"copy_time_sequence_values_storage_arrays.  Failed to copy array");
				return_code = 0;
			}
			dest += value_size;
			src += value_size;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"copy_time_sequence_values_storage_arrays.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* copy_time_sequence_values_storage_arrays */

static int allocate_time_values_storage_array(enum Value_type value_type,
	struct FE_time_sequence *destination_time_sequence, Value_storage *dest,
	int initialise_storage)
/************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION
Allocate an array of type value_type with size determined by the
destination_time_sequence.  If <initialise_storage> is true then the
values in the array are set to the zero.

NOTE:
For time types, the contents of values_storage is:
 | pointer to array (eg double *) |
  x number_of_values
Assumes that sizeof(int) = 1 DWORD, so that the pointers are a DWORD aligned
in memory. If pointers weren't DWORD aligned get bus errors on SGIs.
=======================================================================*/
{
	int number_of_times,j,return_code;

	ENTER(allocate_time_values_storage_array);
	if (dest)
	{
		return_code = 1;
		number_of_times = FE_time_sequence_get_number_of_times(
			destination_time_sequence);
		/* Allocate the array */
		switch (value_type)
		{
			case DOUBLE_VALUE:
			{
				double *dest_array,**array_address;
				/* allocate the dest array */
				if (ALLOCATE(dest_array,double,number_of_times))
				{
					if (initialise_storage)
					{
						for (j = 0 ; j < number_of_times ; j++)
						{
							dest_array[j] = 0.0;
						}
					}
					/* copy the address of the new array into the dest values_storage*/
					array_address = (double **)dest;
					*array_address = dest_array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case FE_VALUE_VALUE:
			{
				FE_value *dest_array,**array_address;
				/* allocate the dest array */
				if (ALLOCATE(dest_array,FE_value,number_of_times))
				{
					if (initialise_storage)
					{
						for (j = 0 ; j < number_of_times ; j++)
						{
							dest_array[j] = FE_VALUE_INITIALIZER;
						}
					}
					/* copy the address of the new array into the dest values_storage*/
					array_address = (FE_value **)dest;
					*array_address = dest_array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case FLT_VALUE:
			{
				float *dest_array,**array_address;
				/* allocate the dest array */
				if (ALLOCATE(dest_array,float,number_of_times))
				{
					if (initialise_storage)
					{
						for (j = 0 ; j < number_of_times ; j++)
						{
							dest_array[j] = 0.0f;
						}
					}
					/* copy the address of the new array into the dest values_storage*/
					array_address = (float **)dest;
					*array_address = dest_array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case SHORT_VALUE:
			{
				short *dest_array,**array_address;
				/* allocate the dest array */
				if (ALLOCATE(dest_array,short,number_of_times))
				{
					if (initialise_storage)
					{
						for (j = 0 ; j < number_of_times ; j++)
						{
							dest_array[j] = 0;
						}
					}
					/* copy the address of the new array into the dest values_storage*/
					array_address = (short **)dest;
					*array_address = dest_array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case INT_VALUE:
			{
				int *dest_array,**array_address;
				/* allocate the dest array */
				if (ALLOCATE(dest_array,int,number_of_times))
				{
					if (initialise_storage)
					{
						for (j = 0 ; j < number_of_times ; j++)
						{
							dest_array[j] = 0;
						}
					}
					/* copy the address of the new array into the dest values_storage*/
					array_address = (int **)dest;
					*array_address = dest_array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case STRING_VALUE:
			{
				display_message(ERROR_MESSAGE,
					"allocate_time_values_storage_array. String type not implemented for multiple times yet.");
				return_code = 0;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"allocate_time_values_storage_array. Invalid type");
				return_code = 0;
			} break;
		} /*switch (the_value_type) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"allocate_time_values_storage_array."
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* allocate_time_values_storage_array */

static int reallocate_time_values_storage_array(enum Value_type value_type,
	int new_number_of_values, Value_storage *new_array,
	Value_storage *previous_array,
	int initialise_storage, int previous_number_of_values)
/************************************************************************
LAST MODIFIED : 20 December 2005

DESCRIPTION
Reallocate an array of type value_type with number_of_values.  If
<initialise_storage> is true then the values from
<previous_number_of_values> + 1 to <new_number_of_values> are set to zero.
The routine will potentially overallocate the array to accelerate when
these arrays are expanded out one value at a time, many times over.
=======================================================================*/
{
	int allocate_number_of_values, j, return_code;

	ENTER(allocate_time_values_storage_array);
	if (new_array && previous_array)
	{
		return_code = 1;

		allocate_number_of_values = (new_number_of_values + VALUE_STORAGE_NUMBER_OF_TIMES_BLOCK) -
			(new_number_of_values % VALUE_STORAGE_NUMBER_OF_TIMES_BLOCK);
		switch (value_type)
		{
			case DOUBLE_VALUE:
			{
				double *array;
				if (REALLOCATE(array,*(double **)previous_array,double,allocate_number_of_values))
				{
					if (initialise_storage)
					{
						for (j = previous_number_of_values ; j < new_number_of_values ; j++)
						{
							array[j] = 0.0;
						}
					}
					*(double **)new_array = array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case FE_VALUE_VALUE:
			{
				FE_value *array;
				if (REALLOCATE(array,*(FE_value **)previous_array,FE_value,allocate_number_of_values))
				{
					if (initialise_storage)
					{
						for (j = previous_number_of_values ; j < new_number_of_values ; j++)
						{
							array[j] = FE_VALUE_INITIALIZER;
						}
					}
					*(FE_value **)new_array = array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case FLT_VALUE:
			{
				float *array;
				if (REALLOCATE(array,*(float **)previous_array,float,allocate_number_of_values))
				{
					if (initialise_storage)
					{
						for (j = previous_number_of_values ; j < new_number_of_values ; j++)
						{
							array[j] = 0.0f;
						}
					}
					*(float **)new_array = array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case SHORT_VALUE:
			{
				short *array;
				if (REALLOCATE(array,*(short **)previous_array,short,allocate_number_of_values))
				{
					if (initialise_storage)
					{
						for (j = previous_number_of_values ; j < new_number_of_values ; j++)
						{
							array[j] = 0;
						}
					}
					*(short **)new_array = array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case INT_VALUE:
			{
				int *array;
				if (REALLOCATE(array,*(int **)previous_array,int,allocate_number_of_values))
				{
					if (initialise_storage)
					{
						for (j = previous_number_of_values ; j < new_number_of_values ; j++)
						{
							array[j] = 0;
						}
					}
					*(int **)new_array = array;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_time_values_storage_array. Out of memory");
					return_code = 0;
				}
			} break;
			case STRING_VALUE:
			{
				display_message(ERROR_MESSAGE,
					"allocate_time_values_storage_array. String type not implemented for multiple times yet.");
				return_code = 0;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"allocate_time_values_storage_array. Invalid type");
				return_code = 0;
			} break;
		} /*switch (the_value_type) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"allocate_time_values_storage_array."
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* allocate_time_values_storage_array */

static int copy_value_storage_array(Value_storage *destination,
	enum Value_type value_type, struct FE_time_sequence *destination_time_sequence,
	struct FE_time_sequence *source_time_sequence,int number_of_values,
	Value_storage *source, int optimised_merge)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Copies the <number_of_values> of <value_type> stored in <source> into
<destination>. Arrays, strings and other dynamic values are allocated afresh
for <destination>. <destination> is assumed to be blank and large enough to
contain such values.  If <source_time_sequence> and <destination_time_sequence>
are non NULL then the value storage correspond to an arrays of values at those
times.
If <optimised_merge> is set then time sequences may be transferred from source
to destination in certain cases instead of copied; only call from merge_FE_node.
==============================================================================*/
{
	enum FE_time_sequence_mapping time_sequence_mapping;
	int destination_number_of_times, i, return_code;

	ENTER(copy_value_storage_array);
	if (destination&&(0<number_of_values)&&source)
	{
		return_code=1;
		if (source_time_sequence || destination_time_sequence)
		{
			if (source_time_sequence && destination_time_sequence)
			{
				int value_size;
				Value_storage *src,*dest;

				dest = destination;
				src = source;
				value_size=get_Value_storage_size(value_type,
					destination_time_sequence);
				if (optimised_merge)
				{
					time_sequence_mapping =
						FE_time_sequences_mapping(source_time_sequence, destination_time_sequence);
				}
				else
				{
					time_sequence_mapping = FE_TIME_SEQUENCE_MAPPING_UNKNOWN;
				}
				switch (time_sequence_mapping)
				{
					case FE_TIME_SEQUENCE_MAPPING_IDENTICAL:
					case FE_TIME_SEQUENCE_MAPPING_APPEND:
					{
						destination_number_of_times = FE_time_sequence_get_number_of_times(destination_time_sequence);
						for (i=0;(i<number_of_values)&&return_code;i++)
						{
							reallocate_time_values_storage_array(value_type,
								destination_number_of_times, dest, src,
								/*initialise_storage*/0, /*previous_number_of_values*/0);
							*(void **)src = 0x0;
							dest += value_size;
							src += value_size;
						}
					} break;
					default:
					{
						/* Fallback default implementation */
						for (i=0;(i<number_of_values)&&return_code;i++)
						{
							if (!(allocate_time_values_storage_array(value_type,
										destination_time_sequence,dest,/*initialise_storage*/0)&&
									copy_time_sequence_values_storage_array(src,value_type,
										source_time_sequence,destination_time_sequence,dest)))
							{
								display_message(ERROR_MESSAGE,
									"copy_value_storage_array.  Failed to copy array");
								if (0<i)
								{
									/* free any arrays allocated to date */
									free_value_storage_array(destination,value_type,
										destination_time_sequence,i);
								}
									return_code = 0;
							}
							dest += value_size;
							src += value_size;
						}
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"copy_value_storage_array.  Copying time values to or from non"
					"non time based values not implemented");
			}
		}
		else
		{
			switch (value_type)
			{
				case DOUBLE_VALUE:
				{
					double *src,*dest;

					src = (double *)source;
					dest = (double *)destination;
					for (i=0;i<number_of_values;i++)
					{
						*dest = *src;
						dest++;
						src++;
					}
				} break;
				case ELEMENT_XI_VALUE:
				{
					int i,j;
					Value_storage *src,*dest;

					src = source;
					dest = destination;
					for (i=0;i<number_of_values;i++)
					{
						/* copy accessed element pointer */
						if (*((struct FE_element **)src))
						{
							(*(struct FE_element **)dest)
								= ACCESS(FE_element)(*((struct FE_element **)src));
						}
						else
						{
							(*(struct FE_element **)dest) = (struct FE_element *)NULL;
						}
						dest += sizeof(struct FE_element *);
						src += sizeof(struct FE_element *);
						/* copy the xi location */
						for (j=0;j<MAXIMUM_ELEMENT_XI_DIMENSIONS;j++)
						{
							*((FE_value *)dest) = *((FE_value *)src);
							dest += sizeof(FE_value);
							src += sizeof(FE_value);
						}
					}
				} break;
				case FE_VALUE_VALUE:
				{
					FE_value *src,*dest;

					src = (FE_value *)source;
					dest = (FE_value *)destination;
					for (i=0;i<number_of_values;i++)
					{
						*dest = *src;
						dest++;
						src++;
					}
				} break;
				case FLT_VALUE:
				{
					float *src,*dest;

					src = (float *)source;
					dest = (float *)destination;
					for (i=0;i<number_of_values;i++)
					{
						*dest = *src;
						dest++;
						src++;
					}
				} break;
				case SHORT_VALUE:
				{
					display_message(ERROR_MESSAGE,"copy_value_storage_array.  "
						"SHORT_VALUE: Haven't written code yet. Beware pointer alignment problems!");
					return_code = 0;
				} break;
				case INT_VALUE:
				{
					int *src,*dest;

					src = (int *)source;
					dest = (int *)destination;
					for (i=0;i<number_of_values;i++)
					{
						*dest = *src;
						dest++;
						src++;
					}
				} break;
				case UNSIGNED_VALUE:
				{
					unsigned *src,*dest;

					src = (unsigned *)source;
					dest = (unsigned *)destination;
					for (i=0;i<number_of_values;i++)
					{
						*dest = *src;
						dest++;
						src++;
					}
				} break;
				case DOUBLE_ARRAY_VALUE:
				case FE_VALUE_ARRAY_VALUE:
				case FLT_ARRAY_VALUE:
				case SHORT_ARRAY_VALUE:
				case INT_ARRAY_VALUE:
				case UNSIGNED_ARRAY_VALUE:
				case STRING_VALUE:
				{
					int value_size;
					Value_storage *src,*dest;

					dest = destination;
					src = source;
					value_size=get_Value_storage_size(value_type,
						(struct FE_time_sequence *)NULL);
					for (i=0;(i<number_of_values)&&return_code;i++)
					{
						if (!allocate_and_copy_values_storage_array(src,value_type,dest))
						{
							display_message(ERROR_MESSAGE,
								"copy_value_storage_array.  Failed to copy array");
							if (0<i)
							{
								/* free any arrays allocated to date */
								free_value_storage_array(destination,value_type,
									(struct FE_time_sequence *)NULL,i);
							}
							return_code = 0;
						}
						dest += value_size;
						src += value_size;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"copy_value_storage_array.  Unknown value_type");
					return_code = 0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"copy_value_storage_array.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* copy_value_storage_array */

static int initialise_value_storage_array(Value_storage *values_storage,
	enum Value_type value_type, struct FE_time_sequence *time_sequence,
	int number_of_values)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
Clears values_storage array to suitable defaults for the <value_type> and
<time_sequence>.
<number_of_values> of the given <value_type>.
For non-array types, the contents of field->values_storage is:
   | data type (eg FE_value) | x number_of_values
For array types, the contents of field->values_storage is:
   ( | int (number of array values) | pointer to array (eg double *) |
   x number_of_values )
For time types where the <time_sequence> is nonNULL then then values_storage is:
   ( | pointer to array (eg double *) | x number_of_values )
Sets data in this memory to 0, pointers to NULL.
==============================================================================*/
{
	int i, j, return_code, size;
	Value_storage *new_value, *temp_values_storage;

	ENTER(initialise_value_storage_array);
	if (values_storage &&
		(size = get_Value_storage_size(value_type, time_sequence)) &&
		(0 < number_of_values))
	{
		return_code = 1;
		temp_values_storage = values_storage;
		if (time_sequence)
		{
			/* set pointers to NULL */
			switch (value_type)
			{
				case DOUBLE_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((double **)temp_values_storage) = (double *)NULL;
						temp_values_storage += size;
					}
				}	break;
				case FE_VALUE_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((FE_value **)temp_values_storage) = (FE_value *)NULL;
						temp_values_storage += size;
					}
				}	break;
				case FLT_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((float **)temp_values_storage) = (float *)NULL;
						temp_values_storage += size;
					}
				} break;
				case SHORT_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((short **)temp_values_storage) = (short *)NULL;
						temp_values_storage += size;
					}
				} break;
				case INT_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((int **)temp_values_storage) = (int *)NULL;
						temp_values_storage += size;
					}
				}	break;
				case UNSIGNED_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((unsigned **)temp_values_storage) = (unsigned *)NULL;
						temp_values_storage += size;
					}
				}	break;
				default:
				{
					display_message(ERROR_MESSAGE, "initialise_value_storage_array.  "
						"Value type does not support time_sequences");
					return_code = 0;
				} break;
			}
		}
		else
		{
			/* set values to zero */
			switch (value_type)
			{
				case DOUBLE_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((double *)temp_values_storage) = 0.0;
						temp_values_storage += size;
					}
				}	break;
				case ELEMENT_XI_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						new_value = temp_values_storage;
						*((struct FE_element **)new_value) = (struct FE_element *)NULL;
						new_value += sizeof(struct FE_element *);
						for (j = 0; j < MAXIMUM_ELEMENT_XI_DIMENSIONS; j++)
						{
							*((FE_value *)new_value) = 0;
							new_value += sizeof(FE_value);
						}
						temp_values_storage += size;
					}
				}	break;
				case FE_VALUE_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((FE_value *)temp_values_storage) = 0.0;
						temp_values_storage += size;
					}
				}	break;
				case FLT_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((float *)temp_values_storage) = 0.0;
						temp_values_storage += size;
					}
				} break;
				case SHORT_VALUE:
				{
					display_message(ERROR_MESSAGE," initialise_value_storage_array."
						"SHORT_VALUE. Code not written yet. Beware alignment problems ");
					return_code = 0;
				} break;
				case INT_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((int *)temp_values_storage) = 0;
						temp_values_storage += size;
					}
				}	break;
				case UNSIGNED_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((unsigned *)temp_values_storage) = 0;
						temp_values_storage += size;
					}
				}	break;
				case STRING_VALUE:
				{
					for (i = number_of_values; 0 < i; i--)
					{
						*((char **)temp_values_storage) = (char *)NULL;
						temp_values_storage += size;
					}
				} break;
				/* set number of array values to 0, array pointers to NULL */
				case DOUBLE_ARRAY_VALUE:
				{
					double **array_address;

					for (i = number_of_values; 0 < i; i--)
					{
						/* copy the number of array values (0!) to temp_values_storage */
						*((int *)temp_values_storage) = 0;
						/* copy the pointer to the array values (currently NULL), to
							 temp_values_storage*/
						array_address = (double **)(temp_values_storage + sizeof(int));
						*array_address = (double *)NULL;
						temp_values_storage += size;
					}
				} break;
				case FE_VALUE_ARRAY_VALUE:
				{
					FE_value **array_address;

					for (i = number_of_values; 0 < i; i--)
					{
						*((int *)temp_values_storage) = 0;
						array_address = (FE_value **)(temp_values_storage + sizeof(int));
						*array_address = (FE_value *)NULL;
						temp_values_storage += size;
					}
				} break;
				case FLT_ARRAY_VALUE:
				{
					float **array_address;

					for (i = number_of_values; 0 < i; i--)
					{
						*((int *)temp_values_storage) = 0;
						array_address = (float **)(temp_values_storage + sizeof(int));
						*array_address = (float *)NULL;
						temp_values_storage += size;
					}
				} break;
				case SHORT_ARRAY_VALUE:
				{
					short **array_address;

					for (i = number_of_values; 0 < i; i--)
					{
						*((int *)temp_values_storage) = 0;
						array_address = (short **)(temp_values_storage + sizeof(int));
						*array_address = (short *)NULL;
						temp_values_storage += size;
					}
				} break;
				case INT_ARRAY_VALUE:
				{
					int **array_address;

					for (i = number_of_values; 0 < i; i--)
					{
						*((int *)temp_values_storage) = 0;
						array_address = (int **)(temp_values_storage + sizeof(int));
						*array_address = (int *)NULL;
						temp_values_storage += size;
					}
				} break;
				case UNSIGNED_ARRAY_VALUE:
				{
					unsigned **array_address;

					for (i = number_of_values; 0 < i; i--)
					{
						*((int *)temp_values_storage) = 0;
						array_address = (unsigned **)(temp_values_storage + sizeof(int));
						*array_address = (unsigned *)NULL;
						temp_values_storage += size;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"initialise_value_storage_array.  Unknown value_type");
					return_code = 0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"initialise_value_storage_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* initialise_value_storage_array */

static Value_storage *make_value_storage_array(enum Value_type value_type,
	struct FE_time_sequence *time_sequence, int number_of_values)
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Allocates space large enough to contain <number_of_values> of the given
<value_type>.
For non-array types, the contents of field->values_storage is:
   | data type (eg FE_value) | x number_of_values
For array types, the contents of field->values_storage is:
   ( | int (number of array values) | pointer to array (eg double *) |
   x number_of_values )
For time types where the <time_sequence> is nonNULL then then values_storage is:
   ( | pointer to array (eg double *) | x number_of_values )
Initialises the contents to be zero for values, NULL for pointers.
==============================================================================*/
{
	int size, values_storage_size;
	Value_storage *values_storage;

	ENTER(make_value_storage_array);
	values_storage = (Value_storage *)NULL;
	if ((size = get_Value_storage_size(value_type ,time_sequence)) &&
		(0 < number_of_values))
	{
		values_storage_size = size*number_of_values;
		ADJUST_VALUE_STORAGE_SIZE(values_storage_size);
		if (ALLOCATE(values_storage, Value_storage, values_storage_size))
		{
			if (!initialise_value_storage_array(values_storage, value_type,
				time_sequence, number_of_values))
			{
				DEALLOCATE(values_storage);
				values_storage = (Value_storage *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"make_value_storage_array.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_value_storage_array.  Invalid argument(s)");
	}
	LEAVE;

	return (values_storage);
} /* make_value_storage_array */

#if defined (DEBUG_CODE)
int show_FE_nodal_FE_values(struct FE_node *node)
/************************************************************************
LAST MODIFIED : 19 April 1999

DESCRIPTION
A debug function to print a node's value storage (to stdout)
=======================================================================*/
{
	int return_code = 0,limit,i;
	FE_value *values;
	ENTER(show_FE_nodal_FE_values);
	if (node)
	{
		if (node->values_storage)
		{
			limit = get_FE_node_number_of_values(node);
			return_code = 1;
			values = (FE_value*)(node->values_storage);
			for (i=0;i<limit;i++)
			{
				printf("Nodal value[%d], = %f \n",i,*values);
				values++;
			}
		}
		else
		{
			printf("Node->values_storage = NULL \n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"show_FE_nodal_values."
			"Invalid arguments");
	}
	LEAVE;
	return (return_code);
} /* show_FE_nodal_field_FE_values */

static int show_node_field(struct FE_node_field *node_field,void *count)
/************************************************************************
LAST MODIFIED : 19 April 1999

DESCRIPTION
A debug function to print a node fields (to stdout)
=======================================================================*/
{
	int return_code,i;
	struct FE_node_field_component *component;

	int *the_count;

	ENTER(show_node_field);
	the_count = (int *)count;
	printf("count = %d\n",*the_count);
	if (node_field)
	{
		printf("node_field = %p \n",node_field);
		if (node_field->field)
		{
			printf("   node_field->field = %p \n",node_field->field);
			printf("   name = %s \n",node_field->field->name);
			printf("   number of values = %d \n",node_field->field->number_of_values);
			printf("   number of components = %d \n",node_field->field->number_of_components);
			for (i=0;i<node_field->field->number_of_components;i++)
			{
				printf("      component %d: \n",i);
				component = &(node_field->components[i]);
				printf("      number of versions = %d \n",component->number_of_versions);
				printf("      number of derivatives = %d \n",component->number_of_derivatives);
			}
		}
		else
		{
			printf("node_field->field = NULL \n");
		}
		return_code =1;
	}
	else
	{
		printf("node_field = NULL \n");
		return_code = 1;
	}
	(*the_count)++;
	printf("\n");
	LEAVE;
	return (return_code);
}

int show_FE_nodal_node_fields(struct FE_node *node)
/************************************************************************
LAST MODIFIED : 19 April 1999

DESCRIPTION
A debug function to print all a node's node fields (to stdout)
=======================================================================*/
{
	int return_code,count;
	ENTER(show_FE_nodal_node_fields);
	if (node)
	{
		if (node->fields)
		{
			count =0;
			FOR_EACH_OBJECT_IN_LIST(FE_node_field)
				(show_node_field,(void *)(&count),node->fields->node_field_list);
		}
		else
		{
			printf("Node->fields = NULL \n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"show_FE_nodal_values."
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* show_FE_nodal_node_fields */
#endif /* defined (DEBUG_CODE) */

static int FE_node_fields_match(struct FE_node_field *node_field_1,
	struct FE_node_field *node_field_2, int ignore_field_and_time_sequence,
	int ignore_component_value)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
Returns true if <node_field_1> and <node_field_2> are equivalent.
Note that the component value pointers do not have to match since the position
of the data in the values_storage can be different.
No comparison of field and time_sequence members will be made if
<ignore_field_and_time_sequence> is set.
No comparison of the component value, the offset into the nodal values_storage
where the data is kept it <ignore_component_value> is set.
Use with caution!
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type_1, *nodal_value_type_2;
	int i, j, number_of_components, return_code;
	struct FE_node_field_component *component_1, *component_2;

	ENTER(FE_node_fields_match);
	return_code=0;
	if (node_field_1 && node_field_2)
	{
		if (ignore_field_and_time_sequence ||
			((node_field_1->field == node_field_2->field) &&
				(node_field_1->time_sequence == node_field_2->time_sequence)))
		{
			if (((number_of_components =
				get_FE_field_number_of_components(node_field_1->field)) ==
				get_FE_field_number_of_components(node_field_2->field)) &&
				(component_1 = node_field_1->components) &&
				(component_2 = node_field_2->components))
			{
				return_code = 1;
				for (i = number_of_components; (return_code) && (0 < i); i--)
				{
					if ((ignore_component_value ||
						(component_1->value == component_2->value)) &&
						(component_1->number_of_derivatives ==
							component_2->number_of_derivatives) &&
						(component_1->number_of_versions ==
							component_2->number_of_versions))
					{
						nodal_value_type_1 = component_1->nodal_value_types;
						nodal_value_type_2 = component_2->nodal_value_types;
						if (nodal_value_type_1 && nodal_value_type_2)
						{
							j = 1 + component_1->number_of_derivatives;
							while (return_code && (j > 0))
							{
								if (*nodal_value_type_1 != *nodal_value_type_2)
								{
									return_code = 0;
								}
								nodal_value_type_1++;
								nodal_value_type_2++;
								j--;
							}
						}
						else if (nodal_value_type_1 || nodal_value_type_2)
						{
							return_code = 0;
						}
					}
					else
					{
						return_code = 0;
					}
					component_1++;
					component_2++;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_node_fields_match.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_node_fields_match */

static int FE_node_field_is_in_list(struct FE_node_field *node_field,
	void *node_field_list_void)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
Returns true if a node field exactly matching <node_field> is found in
<node_field_list>.
==============================================================================*/
{
	int return_code;
	struct FE_node_field *node_field_2;
	struct LIST(FE_node_field) *node_field_list;

	ENTER(FE_node_field_is_in_list);
	return_code = 0;
	if (node_field && (node_field->field) &&
		(node_field_list = (struct LIST(FE_node_field) *)node_field_list_void))
	{
		if ((node_field_2 = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
			node_field->field, node_field_list)) &&
			FE_node_fields_match(node_field, node_field_2,
				/*ignore_field_and_time_sequence*/0, /*ignore_component_value*/0))
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_is_in_list.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_is_in_list */

static int FE_node_field_get_number_of_values(struct FE_node_field *node_field)
/*******************************************************************************
LAST MODIFIED : 14 September 2000

DESCRIPTION :
Returns the number of values sum of (1+number_of_derivatives)*number_of_versions
for all components.
==============================================================================*/
{
	int i,number_of_values;
	struct FE_node_field_component *component;

	ENTER(FE_node_field_get_number_of_values);
	number_of_values=0;
	if (node_field)
	{
		component = node_field->components;
		for (i=node_field->field->number_of_components;0<i;i--)
		{
			number_of_values +=
				(1+component->number_of_derivatives) * component->number_of_versions;
			component++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_get_number_of_values.  Invalid argument");
	}
	LEAVE;

	return (number_of_values);
} /* FE_node_field_get_number_of_values */

static int FE_node_field_set_FE_time_sequence(struct FE_node_field *node_field,
	struct FE_time_sequence *time_sequence)
/*******************************************************************************
LAST MODIFIED : 13 December 2005

DESCRIPTION :
Sets the fe_time_sequence for this object.  If this FE_node_field is being
accessed more than once it will fail as there will be other Node_field_infos
and therefore nodes that would then have mismatched node_fields and values_storage.
Should only be doing this if the Node_field_info that this belongs to is only
being used by one node otherwise you would have to update the values_storage
for all nodes using the Node_field_info.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_field_set_FE_time_sequence);
	if (node_field && (node_field->access_count < 2))
	{
		REACCESS(FE_time_sequence)(&(node_field->time_sequence), time_sequence);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_set_FE_time_sequence.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_set_FE_time_sequence */

static int FE_node_field_add_values_storage_size(
	struct FE_node_field *node_field, void *values_storage_size_void)
/*******************************************************************************
LAST MODIFIED : 14 September 2000

DESCRIPTION :
If <node_field> represents a GENERAL_FE_FIELD then its required values_storage
in the node, adjusted for word alignment, is added on to <*values_storage_size>.
==============================================================================*/
{
	int return_code,this_values_storage_size,*values_storage_size;
	struct FE_field *field;

	ENTER(FE_node_field_add_values_storage_size);
	if (node_field&&(field=node_field->field)&&
		(values_storage_size = (int *)values_storage_size_void))
	{
		if (GENERAL_FE_FIELD==field->fe_field_type)
		{
			this_values_storage_size =
				FE_node_field_get_number_of_values(node_field) *
				get_Value_storage_size(field->value_type, node_field->time_sequence);
			ADJUST_VALUE_STORAGE_SIZE(this_values_storage_size);
			(*values_storage_size) += this_values_storage_size;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_add_values_storage_size.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_add_values_storage_size */

static int FE_node_field_free_values_storage_arrays(
	struct FE_node_field *node_field, void *start_of_values_storage_void)
/*******************************************************************************
LAST MODIFIED: 9 March 2005

DESCRIPTION:
Frees accesses and dynamically allocated memory in <start_of_values_storage>
for the FE_node_field.
The <start_of_the_values_storage> address is passed so that the this function
can be called from an interator, the <node> is not passed so this function can
be used to deallocate parts of any values_storage.  The <values_storage> can
be NULL if there are no GENERAL_FE_FIELDs defined on the node.
Only certain value types, eg. arrays, strings, element_xi require this.
==============================================================================*/
{
	enum Value_type value_type;
	int i,number_of_components,number_of_values,return_code;
	struct FE_node_field_component *component;
	Value_storage *start_of_values_storage, *values_storage;

	ENTER(FE_node_field_free_values_storage_arrays);
	if (node_field&&node_field->field)
	{
		return_code = 1;
		/* only general fields have node_field_components and can have
			 values_storage at the node */
		if (GENERAL_FE_FIELD==node_field->field->fe_field_type)
		{
			value_type = node_field->field->value_type;
			number_of_components = node_field->field->number_of_components;
			component = node_field->components;
			for (i=0;i<number_of_components;i++)
			{
				if (NULL != (start_of_values_storage=(Value_storage *)start_of_values_storage_void))
				{
					values_storage = start_of_values_storage + component->value;
					number_of_values=
						(1+component->number_of_derivatives)*component->number_of_versions;
					free_value_storage_array(values_storage,value_type,
						node_field->time_sequence,number_of_values);
					component++;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_node_field_free_values_storage_arrays. Invalid values storage");
					return_code = 0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_free_values_storage_arrays. Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_free_values_storage_arrays */

struct FE_node_field_merge_values_storage_data
{
	Value_storage *new_values_storage;
	struct LIST(FE_node_field) *old_node_field_list;
	Value_storage *old_values_storage;
	struct LIST(FE_node_field) *add_node_field_list;
	Value_storage *add_values_storage;
	int optimised_merge;
}; /* FE_node_field_merge_values_storage_data */

static int FE_node_field_merge_values_storage(
	struct FE_node_field *new_node_field,	void *copy_data_void)
/*******************************************************************************
LAST MODIFIED: 4 March 2005

DESCRIPTION:
If <new_node_field> uses values storage then:

... when <add_node_field_list> and <add_values_storage> provided:
Finds the equivalent node field in the <old_node_field_list> or
<add_node_field_list>, and copies values giving precedence to the latter.
If the node fields have times, the time arrays are allocated once, then the
old values are copied followed by the add values to correctly merge the times.

... when <add_node_field_list> and <add_values_storage> not provided:
Copies the values for <new_node_field> into <new_values_storage> from the
<old_values_storage> with the equivalent node field in <old_node_field_list>.

... when <new_values_storage> is not provided then the values described by
<add_node_field_list> are copied from the <add_values_storage> into the
corresponding places in the <old_values_storage>.

Notes:
Assumes <new_values_storage> is already allocated to the appropriate size.
Assumes the only differences between equivalent node fields are in time version;
no checks on this are made here.
Assumes component nodal values are consecutive and start at first component.
<copy_data_void> points at a struct FE_node_field_merge_values_storage_data.
==============================================================================*/
{
	enum Value_type value_type;
	int number_of_values, return_code;
	struct FE_field *field;
	struct FE_node_field *add_node_field, *old_node_field;
	struct FE_node_field_merge_values_storage_data *copy_data;
	Value_storage *destination, *source;

	ENTER(FE_node_field_merge_values_storage);
	if (new_node_field && (field = new_node_field->field) && (copy_data =
		(struct FE_node_field_merge_values_storage_data *)copy_data_void))
	{
		return_code = 1;
		/* only GENERAL_FE_FIELD has values stored with node */
		if (GENERAL_FE_FIELD == field->fe_field_type)
		{
			old_node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
				field, copy_data->old_node_field_list);
			if (copy_data->add_node_field_list)
			{
				add_node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
					field, copy_data->add_node_field_list);
			}
			else
			{
				add_node_field = (struct FE_node_field *)NULL;
			}
			if (copy_data->new_values_storage)
			{
				/* Merging into a new values storage */
				if (old_node_field || add_node_field)
				{
					/* destination in new_values_storage according to new_node_field */
					destination =
						copy_data->new_values_storage + new_node_field->components->value;
					value_type = field->value_type;
					number_of_values = FE_node_field_get_number_of_values(new_node_field);
					if ((!add_node_field) ||
						(old_node_field && new_node_field->time_sequence))
					{
						/* source in old_values_storage according to old_node_field */
						if (copy_data->old_values_storage && old_node_field->components)
						{
							source = copy_data->old_values_storage +
								old_node_field->components->value;
							return_code = copy_value_storage_array(destination, value_type,
								new_node_field->time_sequence, old_node_field->time_sequence,
								number_of_values, source, copy_data->optimised_merge);
						}
						else
						{
							return_code = 0;
						}
					}
					if (return_code && add_node_field)
					{
						/* source in add_values_storage according to add_node_field */
						if (copy_data->add_values_storage && add_node_field->components)
						{
							source = copy_data->add_values_storage +
								add_node_field->components->value;
							if (old_node_field && new_node_field->time_sequence)
							{
								return_code = copy_time_sequence_values_storage_arrays(
									destination, value_type, new_node_field->time_sequence,
									add_node_field->time_sequence, number_of_values, source);
							}
							else
							{
								return_code = copy_value_storage_array(destination, value_type,
									new_node_field->time_sequence, add_node_field->time_sequence,
									number_of_values, source, copy_data->optimised_merge);
							}
						}
						else
						{
							return_code = 0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "FE_node_field_merge_values_storage.  "
						"Could not find equivalent existing node field");
					return_code = 0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"FE_node_field_merge_values_storage.  Unable to copy values");
					return_code = 0;
				}
			}
			else
			{
				/* Replacing values in the existing values storage by copying
					them from the add_node_storage to the old_node_storage.
					(old_node_field and new_node_field should be the same) */
				if (old_node_field && (old_node_field == new_node_field))
				{
					if (add_node_field)
					{
						destination =
							copy_data->old_values_storage + old_node_field->components->value;
						value_type = field->value_type;
						number_of_values = FE_node_field_get_number_of_values(new_node_field);
						if (copy_data->add_values_storage && add_node_field->components)
						{
							source = copy_data->add_values_storage +
								add_node_field->components->value;
							if (old_node_field->time_sequence)
							{
								return_code = copy_time_sequence_values_storage_arrays(
									destination, value_type, old_node_field->time_sequence,
									add_node_field->time_sequence, number_of_values, source);
							}
							else
							{
								/* Release the storage of the old values.  The pointer is from the
									start of the values storage to make it work as a node_field iterator. */
								FE_node_field_free_values_storage_arrays(old_node_field,
									(void *)copy_data->old_values_storage);
								return_code = copy_value_storage_array(destination, value_type,
									old_node_field->time_sequence, add_node_field->time_sequence,
									number_of_values, source, copy_data->optimised_merge);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"FE_node_field_merge_values_storage.  Unable to merge values");
							return_code = 0;
						}
					}
					/* else do nothing as we are leaving this field alone */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_node_field_merge_values_storage.  "
						"Unable to find corresponding node fields when updating values.");
					return_code = 0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_merge_values_storage.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_merge_values_storage */

static int merge_FE_node_values_storage(struct FE_node *node,
	Value_storage *values_storage,
	struct LIST(FE_node_field) *new_node_field_list,
	struct FE_node *add_node, int optimised_merge)
/*******************************************************************************
LAST MODIFIED: 4 March 2005

DESCRIPTION:
For each field in <new_node_field_list> requiring values storage, finds the
equivalent node field in either <node>, <add_node> or both. If only one of
<node> or <add_node> contains an equivalent node field, those values are
copied. If there is an equivalent node field in both, behaviour depends on
whether the node fields have times:
* if the node fields have no times, values are taken from <add_node>.
* if the node fields have times, the times arrays are allocated, then the
values at times in <node> are copied, followed by those in <add_node>.
Hence, the values in <add_node> take preference over those in <node>.
If <values_storage> is not provided then the fields described in the
<new_node_field_list> are looked for in the <add_node> and if found the
values are copied into the existing values storage in <node>.
Notes:
* there must be an equivalent node field at either <node> or <add_node>;
* <add_node> is optional and used only by merge_FE_node. If NULL then a node
field must be found in <node>;
* Values_storage, when provided, must already be allocated to the appropriate
size but is not assumed to contain any information prior to being filled here;
* Any objects or arrays referenced in the values_storage are accessed or
allocated in the new <values_storage> so <node> and <add_node> are unchanged.
* It is up to the calling function to have checked that the node fields in
<node>, <add_node> and <new_node_field_list> are compatible.
==============================================================================*/
{
	int return_code;
	struct FE_node_field_merge_values_storage_data copy_data;

	ENTER(merge_FE_node_values_storage);
	if (node && node->fields && new_node_field_list &&
		((!add_node) || add_node->fields))
	{
		copy_data.new_values_storage = values_storage;
		copy_data.old_node_field_list = node->fields->node_field_list;
		copy_data.old_values_storage = node->values_storage;
		if (add_node)
		{
			copy_data.add_node_field_list = add_node->fields->node_field_list;
			copy_data.add_values_storage = add_node->values_storage;
		}
		else
		{
			copy_data.add_node_field_list = (struct LIST(FE_node_field) *)NULL;
			copy_data.add_values_storage = (Value_storage *)NULL;
		}
		copy_data.optimised_merge = optimised_merge;
		return_code = FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
			FE_node_field_merge_values_storage, (void *)(&copy_data),
			new_node_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"merge_FE_node_values_storage.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* merge_FE_node_values_storage */

static int get_FE_node_field_list_values_storage_size(
	struct LIST(FE_node_field) *node_field_list)
/*******************************************************************************
LAST MODIFIED : 13 September 2000

DESCRIPTION :
Returns the size, in bytes, of the data in the nodal values storage owned
by the all the node_fields in node_field_list.
==============================================================================*/
{
	int values_storage_size;

	ENTER(get_FE_node_field_list_values_storage_size);
	values_storage_size=0;
	if (node_field_list)
	{
		FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
			FE_node_field_add_values_storage_size,
			(void *)&values_storage_size,node_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_field_list_values_storage_size.  Invalid argument");
	}
	LEAVE;

	return (values_storage_size);
} /* get_FE_node_field_list_values_storage_size */

static int allocate_and_copy_FE_node_values_storage(struct FE_node *node,
	Value_storage **values_storage)
/******************************************************************************
LAST MODIFIED: 1 November 2002

DESCRIPTION:
Allocates values_storage to the same size as node->values_storage.
Copies the node->values_storage to values_storage. Also allocates and copies
any arrays in node->values_storage.

Note that values_storage contains no information about the value_type(s) or the
number of values of the data in it. You must refer to the FE_node/FE_field to
get this.

The the calling function is responsible for deallocating values_storage,
and any arrays in values_storage.
==============================================================================*/
{
	int return_code,size;
	Value_storage *dest_values_storage;

	ENTER(allocate_and_copy_FE_node_values_storage);
	if (node)
	{
		return_code = 1;

		if (node->fields)
		{
			size = get_FE_node_field_list_values_storage_size(
				node->fields->node_field_list);
			if (size)
			{
				if (ALLOCATE(dest_values_storage,Value_storage,size))
				{
					return_code = merge_FE_node_values_storage(node, dest_values_storage,
						node->fields->node_field_list, (struct FE_node *)NULL,
						/*optimised_merge*/0);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"allocate_and_copy_FE_node_values_storage.  Not enough memory");
					dest_values_storage = (Value_storage *)NULL;
					return_code = 0;
				}
			}
			else /* no fields, nothing to copy */
			{
				dest_values_storage = (Value_storage *)NULL;
			}
		}
		else /* no fields, nothing to copy */
		{
		 dest_values_storage = (Value_storage *)NULL;
		}
		*values_storage = dest_values_storage;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"allocate_and_copy_FE_node_values_storage.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* allocate_and_copy_FE_node_values_storage */

/***************************************************************************//**
 * If node_field is for a coordinate field, set it at supplied address if
 * currently none or name is alphabetically less than current field.
 * @param coordinate_field_address_void  struct FE_field **.
 */
static int FE_node_field_get_first_coordinate_field(struct FE_node_field
	*node_field, void *coordinate_field_address_void)
{
	int return_code;
	struct FE_field **coordinate_field_address, *field;

	ENTER(FE_node_field_get_first_coordinate_field);
	coordinate_field_address = (struct FE_field **)coordinate_field_address_void;
	if (node_field && coordinate_field_address)
	{
		return_code = 1;
		field = node_field->field;
		if (FE_field_is_coordinate_field(field, (void *)NULL))
		{
			if ((NULL == *coordinate_field_address) ||
				(strcmp(field->name, (*coordinate_field_address)->name) < 0))
			{
				*coordinate_field_address = field;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_get_first_coordinate_field.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static int FE_node_field_has_time(struct FE_node_field *node_field,void *dummy)
/*******************************************************************************
LAST MODIFIED: 16 June 1999

DESCRIPTION:
returns true if <node_field> has time information defined
==============================================================================*/
{
	int return_code;
	ENTER(FE_node_field_has_time);

	if (node_field&&!dummy)
	{
		return_code = (node_field->field->number_of_times > 0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_has_time. Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}/* FE_node_field_has_time */

static int assign_FE_node_field_component(
	struct FE_node_field_component *component,
	int value,int number_of_derivatives,int number_of_versions,
	enum FE_nodal_value_type *nodal_value_types)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Assigns the fields for the component
???DB.  Do nodal_value_types better ?
==============================================================================*/
{
	int i,return_code;

	ENTER(assign_FE_node_field_component);

	return_code = 1;
	if (component && (0 <= value) && (0 <= number_of_derivatives) &&
		(1 <= number_of_versions) && nodal_value_types)
	{
		if (!(component->nodal_value_types))
		{
			i=number_of_derivatives+1;
			if (ALLOCATE(component->nodal_value_types,enum FE_nodal_value_type,i))
			{
				while (i>0)
				{
					i--;
					(component->nodal_value_types)[i]=nodal_value_types[i];
				}
				component->value = value;
				component->number_of_derivatives=number_of_derivatives;
				component->number_of_versions=number_of_versions;
			}
			else
			{
				display_message(ERROR_MESSAGE, "assign_FE_node_field_component.  "
					"Could not allocate memory for nodal value types");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"assign_FE_node_field_component.  Component already assigned");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"assign_FE_node_field_component.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* assign_FE_node_field_component */

/***************************************************************************//**
 * Sorting function for FE_node_field and FE_element_field lists.
 * Returns:
 * -1 if field_1 < field_2
 *  0 if field_1 = field_2
 *  1 if field_1 > field_2
 * Note: formerly used strcmp(field_1->name,field_2->name) to order by field
 * name alphabetically, but this made it nearly impossible to rename FE_fields.
 */
static int compare_FE_field(struct FE_field *field_1,struct FE_field *field_2)
{
	if (field_1 < field_2)
	{
		return -1;
	}
	else if (field_1 > field_2)
	{
		return 1;
	}
	return 0;
}

static struct FE_node_field *CREATE(FE_node_field)(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 19 January 1998

DESCRIPTION :
Allocates memory and assigns fields for a node field.  The components are
allocated and set to "zero".
???DB.  Not allocating any nodal value names.
==============================================================================*/
{
	int number_of_components;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component;

	ENTER(CREATE(FE_node_field));
	if (field)
	{
		number_of_components=field->number_of_components;
		if (ALLOCATE(node_field,struct FE_node_field,1)&&
			ALLOCATE(component,struct FE_node_field_component,number_of_components))
		{
			node_field->field=ACCESS(FE_field)(field);
			node_field->time_sequence=(struct FE_time_sequence *)NULL;
			node_field->components=component;
			while (number_of_components>0)
			{
				component->value=0;
				component->number_of_derivatives=0;
				component->number_of_versions=0;
				component->nodal_value_types=(enum FE_nodal_value_type *)NULL;
				component++;
				number_of_components--;
			}
			node_field->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_node_field).  Could not allocate memory for node field");
			DEALLOCATE(node_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_node_field).  Invalid argument(s)");
		node_field=(struct FE_node_field *)NULL;
	}
	LEAVE;

	return (node_field);
} /* CREATE(FE_node_field) */

static int DESTROY(FE_node_field)(struct FE_node_field **node_field_address)
/*******************************************************************************
LAST MODIFIED : 19 January 1998

DESCRIPTION :
Frees the memory for the node field and sets <*node_field_address> to NULL.
==============================================================================*/
{
	int i,return_code;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component;

	ENTER(DESTROY(FE_node_field));
	if ((node_field_address)&&(node_field= *node_field_address))
	{
		if (0==node_field->access_count)
		{
			component=node_field->components;
			for (i=node_field->field->number_of_components;i>0;i--)
			{
				DEALLOCATE(component->nodal_value_types);
				component++;
			}
			/* free the components */
			DEALLOCATE(node_field->components);
			if (node_field->time_sequence)
			{
				DEACCESS(FE_time_sequence)(&(node_field->time_sequence));
			}
			(void)DEACCESS(FE_field)(&(node_field->field));
			DEALLOCATE(*node_field_address);
		}
		else
		{
			*node_field_address=(struct FE_node_field *)NULL;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_node_field) */

DECLARE_OBJECT_FUNCTIONS(FE_node_field)

static struct FE_node_field *copy_create_FE_node_field_with_offset(
	struct FE_node_field *source_node_field,int value_offset)
/*******************************************************************************
LAST MODIFIED : 14 September 2000

DESCRIPTION :
Creates an FE_node_field that is identical to <source_node_field> except that
the <value_offset> is added to the value member of the components (which is
an offset into the values_storage array at the node). For non-GENERAL_FE_FIELD
types, the value is not changed.
==============================================================================*/
{
	int i,number_of_components;
	struct FE_field *field;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component,*source_component;

	ENTER(copy_create_FE_node_field_with_offset);
	if (source_node_field&&(field=source_node_field->field))
	{
		if (NULL != (node_field=CREATE(FE_node_field)(field)))
		{
			if (source_node_field->time_sequence)
			{
				node_field->time_sequence = ACCESS(FE_time_sequence)
					(source_node_field->time_sequence);
			}
			if (GENERAL_FE_FIELD != field->fe_field_type)
			{
				/* though component->value is currently irrelevant for these fields,
					 keep it unchanged for future use */
				value_offset=0;
			}
			number_of_components=get_FE_field_number_of_components(field);
			component=node_field->components;
			source_component=source_node_field->components;
			for (i=number_of_components;(0<i)&&node_field;i--)
			{
				if (!assign_FE_node_field_component(component,
					source_component->value+value_offset,
					source_component->number_of_derivatives,
					source_component->number_of_versions,
					source_component->nodal_value_types))
				{
					display_message(ERROR_MESSAGE,
						"copy_create_FE_node_field_with_offset.  "
						"Could not assign node field component");
					DESTROY(FE_node_field)(&node_field);
				}
				component++;
				source_component++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"copy_create_FE_node_field_with_offset.  Could not create node field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"copy_create_FE_node_field_with_offset.  Invalid argument(s)");
		node_field=(struct FE_node_field *)NULL;
	}
	LEAVE;

	return (node_field);
} /* copy_create_FE_node_field_with_offset */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(FE_node_field, field, \
	struct FE_field *, compare_FE_field)

DECLARE_INDEXED_LIST_FUNCTIONS(FE_node_field)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(FE_node_field, field, \
	struct FE_field *, compare_FE_field)

struct FE_node_field_copy_with_equivalent_field_data
/*******************************************************************************
LAST MODIFIED : 27 November 2002

DESCRIPTION :
Data for passing to FE_node_field_copy_with_equivalent_field.
==============================================================================*/
{
	struct FE_time_sequence_package *fe_time;
	struct LIST(FE_field) *fe_field_list;
	struct LIST(FE_node_field) *node_field_list;
};

static int FE_node_field_copy_with_equivalent_field(
	struct FE_node_field *node_field, void *data_void)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Creates a copy of <node_field> using the same named field in <fe_field_list>
and adds it to <node_field_list>. Checks fields are equivalent.
Also creates an equivalent FE_time_sequence in the new <fe_time> for this
node field.
<data_void> points at a struct FE_node_field_copy_with_equivalent_field_data.
==============================================================================*/
{
	int i, number_of_components, return_code;
	struct FE_field *equivalent_field;
	struct FE_node_field *copy_node_field;
	struct FE_node_field_copy_with_equivalent_field_data *data;
	struct FE_node_field_component *component, *copy_component;

	ENTER(FE_node_field_copy_with_equivalent_field);
	if (node_field && node_field->field &&
		(data = (struct FE_node_field_copy_with_equivalent_field_data *)data_void))
	{
		return_code = 1;
		if (NULL != (equivalent_field = FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(
			node_field->field->name, data->fe_field_list)))
		{
			if (FE_fields_match_exact(node_field->field, equivalent_field))
			{
				if (NULL != (copy_node_field = CREATE(FE_node_field)(equivalent_field)))
				{
					if (node_field->time_sequence)
					{
						if (!(copy_node_field->time_sequence = ACCESS(FE_time_sequence)(
							get_FE_time_sequence_matching_FE_time_sequence(data->fe_time,
								node_field->time_sequence))))
						{
							return_code = 0;
						}
					}
					if (GENERAL_FE_FIELD == equivalent_field->fe_field_type)
					{
						number_of_components =
							get_FE_field_number_of_components(equivalent_field);
						component = node_field->components;
						copy_component = copy_node_field->components;
						for (i = number_of_components; (0 < i) && return_code; i--)
						{
							if (!assign_FE_node_field_component(copy_component,
								component->value,
								component->number_of_derivatives,
								component->number_of_versions,
								component->nodal_value_types))
							{
								return_code = 0;
							}
							component++;
							copy_component++;
						}
					}
					if (return_code)
					{
						if (!ADD_OBJECT_TO_LIST(FE_node_field)(copy_node_field,
							data->node_field_list))
						{
							return_code = 0;
						}
					}
					if (!return_code)
					{
						display_message(ERROR_MESSAGE,
							"FE_node_field_copy_with_equivalent_field.  "
							"Could not copy node field component");
						DESTROY(FE_node_field)(&copy_node_field);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_node_field_copy_with_equivalent_field.  "
						"Could not create node field");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_field_copy_with_equivalent_field.  Fields not equivalent");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_field_copy_with_equivalent_field.  No equivalent field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_copy_with_equivalent_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_copy_with_equivalent_field */

struct LIST(FE_node_field) *
	FE_node_field_list_clone_with_FE_field_list(
		struct LIST(FE_node_field) *node_field_list,
		struct LIST(FE_field) *fe_field_list, struct FE_time_sequence_package *fe_time)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
Returns a new FE_node_field list that is identical to <node_field_list>
except that it references equivalent same-name fields from <fe_field_list> and
uses FE_time_sequences in <fe_time>.
It is an error if an equivalent FE_field is not found.
==============================================================================*/
{
	struct LIST(FE_node_field) *return_node_field_list;
	struct FE_node_field_copy_with_equivalent_field_data data;

	ENTER(FE_node_field_list_clone_with_FE_field_list);
	return_node_field_list = (struct LIST(FE_node_field) *)NULL;
	if (node_field_list && fe_field_list && fe_time)
	{
		data.fe_time = fe_time;
		data.fe_field_list = fe_field_list;
		data.node_field_list = CREATE(LIST(FE_node_field))();
		if (FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
			FE_node_field_copy_with_equivalent_field, (void *)&data,
			node_field_list))
		{
			return_node_field_list = data.node_field_list;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_field_list_clone_with_FE_field_list.  Failed");
			DESTROY(LIST(FE_node_field))(&data.node_field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_list_clone_with_FE_field_list.  Invalid argument(s)");
	}
	LEAVE;

	return (return_node_field_list);
} /* FE_node_field_list_clone_with_FE_field_list */

struct FE_node_field_info *CREATE(FE_node_field_info)(
	FE_nodeset *fe_nodeset, struct LIST(FE_node_field) *fe_node_field_list,
	int number_of_values)
{
	struct FE_node_field_info *fe_node_field_info = 0;
	if (fe_nodeset)
	{
		if (ALLOCATE(fe_node_field_info, struct FE_node_field_info, 1))
		{
			/*???RC not convinced number_of_values needs to be in this structure */
			fe_node_field_info->number_of_values = number_of_values;
			fe_node_field_info->values_storage_size = 0;
			fe_node_field_info->node_field_list = CREATE_LIST(FE_node_field)();
			/* maintain pointer to the the FE_nodeset this information belongs to.
				 It is not ACCESSed since FE_nodeset is the owning object and it
				 would prevent the FE_nodeset from being destroyed. */
			fe_node_field_info->fe_nodeset = fe_nodeset;
			fe_node_field_info->access_count = 1;
			if (fe_node_field_info->node_field_list && ((!fe_node_field_list) ||
					COPY_LIST(FE_node_field)(fe_node_field_info->node_field_list,
							fe_node_field_list)))
			{
				fe_node_field_info->values_storage_size =
					get_FE_node_field_list_values_storage_size(
						fe_node_field_info->node_field_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(FE_node_field_info).  Unable to build node field list");
				DEACCESS(FE_node_field_info)(&fe_node_field_info);
				fe_node_field_info = (struct FE_node_field_info *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_node_field_info).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_node_field_info).  Invalid argument(s)");
	}
	return (fe_node_field_info);
}

/**
 * Destroys the FE_node_field_info at *<node_field_info_address>. Frees the
 * memory for the information and sets <*node_field_info_address> to NULL.
 */
int DESTROY(FE_node_field_info)(
	struct FE_node_field_info **node_field_info_address)
{
	int return_code;
	struct FE_node_field_info *node_field_info;
	if (node_field_info_address && (node_field_info = *node_field_info_address))
	{
		if (0 == node_field_info->access_count)
		{
			DESTROY(LIST(FE_node_field))(&(node_field_info->node_field_list));
			DEALLOCATE(*node_field_info_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_node_field_info).  Non-zero access count");
			return_code = 0;
		}
		*node_field_info_address = (struct FE_node_field_info *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_node_field_info).  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

DECLARE_ACCESS_OBJECT_FUNCTION(FE_node_field_info)

/**
 * Special version of DEACCESS which if access_count reaches 1 removes the
 * info from the list held by its fe_nodeset member.
 */
PROTOTYPE_DEACCESS_OBJECT_FUNCTION(FE_node_field_info)
{
	int return_code;
	struct FE_node_field_info *object;
	if (object_address && (object = *object_address))
	{
		(object->access_count)--;
		return_code = 1;
		if (object->access_count <= 1)
		{
			if (1 == object->access_count)
			{
				if (object->fe_nodeset)
				{
					return_code = object->fe_nodeset->remove_FE_node_field_info(object);
				}
			}
			else
			{
				return_code = DESTROY(FE_node_field_info)(object_address);
			}
		}
		*object_address = (struct FE_node_field_info *)NULL;
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

PROTOTYPE_REACCESS_OBJECT_FUNCTION(FE_node_field_info)
{
	if (new_object)
		++(new_object->access_count);
	if (object_address)
	{
		if (*object_address)
			DEACCESS(FE_node_field_info)(object_address);
		*object_address = new_object;
		return 1;
	}
	return 0;
}

DECLARE_LIST_FUNCTIONS(FE_node_field_info)

int FE_node_field_info_clear_FE_nodeset(
	struct FE_node_field_info *node_field_info, void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	if (node_field_info)
	{
		node_field_info->fe_nodeset = 0;
		return 1;
	}
	return 0;
}

int FE_node_field_info_has_FE_field(
	struct FE_node_field_info *node_field_info, void *fe_field_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Returns true if <node_field_info> has an node field for <fe_field>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_field_info_has_FE_field);
	if (FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
		(struct FE_field *)fe_field_void, node_field_info->node_field_list))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_info_has_FE_field */

int FE_node_field_info_has_empty_FE_node_field_list(
	struct FE_node_field_info *node_field_info, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 October 2002

DESCRIPTION :
Returns true if <node_field_info> has no node fields.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_field_info_has_empty_FE_node_field_list);
	USE_PARAMETER(dummy_void);
	if (node_field_info)
	{
		if (0 == NUMBER_IN_LIST(FE_node_field)(node_field_info->node_field_list))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_info_has_empty_FE_node_field_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_info_has_empty_FE_node_field_list */

int FE_node_field_info_has_FE_field_with_multiple_times(
	struct FE_node_field_info *node_field_info, void *fe_field_void)
/*******************************************************************************
LAST MODIFIED: 26 February 2003

DESCRIPTION:
Returns true if <node_field_info> has a node_field that references
<fe_field_void> and that node_field has multiple times.
==============================================================================*/
{
	int return_code;
	struct FE_field *field;
	struct FE_node_field *node_field;

	ENTER(FE_node_field_info_has_FE_field_with_multiple_times);
	if (node_field_info && (field = (struct FE_field *)fe_field_void))
	{
		if (NULL != (node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field, field)(field,
			node_field_info->node_field_list)))
		{
			if (node_field->time_sequence)
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_info_has_FE_field_with_multiple_times.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}/* FE_node_field_info_has_FE_field_with_multiple_times */

int FE_node_field_info_has_matching_FE_node_field_list(
	struct FE_node_field_info *node_field_info, void *node_field_list_void)
/*******************************************************************************
LAST MODIFIED : 14 November 2002

DESCRIPTION :
Returns true if <node_field_info> has a FE_node_field_list containing all the
same FE_node_fields as <node_field_list>.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_node_field) *node_field_list;

	ENTER(FE_node_field_info_has_matching_FE_node_field_list);
	return_code = 0;
	if (node_field_info &&
		(node_field_list = (struct LIST(FE_node_field) *)node_field_list_void))
	{
		if ((NUMBER_IN_LIST(FE_node_field)(node_field_list) ==
			NUMBER_IN_LIST(FE_node_field)(node_field_info->node_field_list)))
		{
			if (FOR_EACH_OBJECT_IN_LIST(FE_node_field)(FE_node_field_is_in_list,
				(void *)(node_field_info->node_field_list), node_field_list))
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_info_has_matching_FE_node_field_list.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_info_has_matching_FE_node_field_list */

struct LIST(FE_node_field) *FE_node_field_info_get_node_field_list(
	struct FE_node_field_info *fe_node_field_info)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
Returns the node field list contained in the <node_field_info>.
==============================================================================*/
{
	struct LIST(FE_node_field) *node_field_list;

	ENTER(FE_node_field_info_get_node_field_list);
	if (fe_node_field_info)
	{
		node_field_list = fe_node_field_info->node_field_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_info_get_node_field_list.  Invalid argument(s)");
		node_field_list = (struct LIST(FE_node_field) *)NULL;
	}
	LEAVE;

	return (node_field_list);
} /* FE_node_field_info_get_node_field_list */

int FE_node_field_info_get_number_of_values(
	struct FE_node_field_info *fe_node_field_info)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
Returns the number of values expected for the <node_field_info>.
==============================================================================*/
{
	int number_of_values;

	ENTER(FE_node_field_info_get_number_of_values);
	if (fe_node_field_info)
	{
		number_of_values = fe_node_field_info->number_of_values;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_info_get_number_of_values.  Invalid argument(s)");
		number_of_values = 0;
	}
	LEAVE;

	return (number_of_values);
} /* FE_node_field_info_get_number_of_values */

int FE_node_field_info_add_node_field(
	struct FE_node_field_info *fe_node_field_info,
	struct FE_node_field *new_node_field, int new_number_of_values)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Adds the <new_node_field> to the list in the <fe_node_field_info> and updates the
<new_number_of_values>.  This should only be done if object requesting the change
is known to be the only object using this field info.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_field_info_add_node_field);
	if (fe_node_field_info)
	{
		if (ADD_OBJECT_TO_LIST(FE_node_field)(new_node_field,
				fe_node_field_info->node_field_list))
		{
			fe_node_field_info->number_of_values = new_number_of_values;
			FE_node_field_add_values_storage_size(
				new_node_field, (void *)&fe_node_field_info->values_storage_size);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_field_info_add_node_field.  Unable to add field to list");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_info_add_node_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_info_add_node_field */

int FE_node_field_info_used_only_once(
	struct FE_node_field_info *fe_node_field_info)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Returns 1 if the <node_field_info> access count indicates that it is being used
by only one external object.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_field_info_used_only_once);
	if (fe_node_field_info)
	{
		/* Once plus the access by the FE_region into its list == 2 */
		if (fe_node_field_info->access_count <= 2)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_info_used_only_once.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_info_used_only_once */

int FE_node_field_log_FE_field_change(
	struct FE_node_field *node_field, void *fe_field_change_log_void)
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
Logs the field in <node_field> as RELATED_OBJECT_CHANGED
in <fe_field_change_log>.
==============================================================================*/
{
	int return_code;
	struct CHANGE_LOG(FE_field) *fe_field_change_log;

	ENTER(FE_node_field_log_FE_field_change);
	if (node_field && (fe_field_change_log =
		(struct CHANGE_LOG(FE_field) *)fe_field_change_log_void))
	{
		return_code = CHANGE_LOG_OBJECT_CHANGE(FE_field)(fe_field_change_log,
			node_field->field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_log_FE_field_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_log_FE_field_change */

int FE_node_field_info_log_FE_field_changes(
	struct FE_node_field_info *fe_node_field_info,
	struct CHANGE_LOG(FE_field) *fe_field_change_log)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Marks each FE_field in <fe_node_field_info> as RELATED_OBJECT_CHANGED
in <fe_field_change_log>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_field_info_log_FE_field_changes);
	if (fe_node_field_info && fe_field_change_log)
	{
		return_code = FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
			FE_node_field_log_FE_field_change, (void *)fe_field_change_log,
			fe_node_field_info->node_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_info_log_FE_field_changes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_info_log_FE_field_changes */

DECLARE_CHANGE_LOG_MODULE_FUNCTIONS(FE_node)

/**
 * Compare functions for listing struct FE_element_type_node_sequence in order:
 * - number_of_nodes changing slowest;
 * - node numbers in numerical order (nodes must be in ascending order).
 * Returns values like strcmp:
 * -1 = identifier 1 < identifier 2
 * 0 = identifier 1 = identifier 2
 * 1 = identifier 1 > identifier 2
 */
static int compare_FE_element_type_node_sequence_identifier(
	FE_element_type_node_sequence_identifier &identifier1,
	FE_element_type_node_sequence_identifier &identifier2)
{
	/* 1. compare number_of_nodes */
	int number_of_nodes = identifier1.number_of_nodes;
	if (number_of_nodes < identifier2.number_of_nodes)
	{
		return -1;
	}
	else if (number_of_nodes > identifier2.number_of_nodes)
	{
		return 1;
	}
	else
	{
		/* 2. compare node_numbers - assumed in ascending order */
		for (int i = 0; i < number_of_nodes; ++i)
		{
			if (identifier1.node_numbers[i] < identifier2.node_numbers[i])
			{
				return -1;
			}
			else if (identifier1.node_numbers[i] > identifier2.node_numbers[i])
			{
				return 1;
			}
		}
	}
	return 0;
}

struct FE_element_type_node_sequence *CREATE(FE_element_type_node_sequence)(
	struct FE_element *element, int face_number)
{
	int i,node_number,*node_numbers,number_of_nodes,placed,j,k;
	struct FE_element_type_node_sequence *element_type_node_sequence;
	struct FE_node **nodes_in_element;

	ENTER(CREATE(FE_element_type_node_sequence));
	if (element)
	{
		/* get list of nodes used by default coordinate field in element/face */
		if (calculate_FE_element_field_nodes(element, face_number,
			(struct FE_field *)NULL, &number_of_nodes, &nodes_in_element,
			/*top_level_element*/(struct FE_element *)NULL) && (0 < number_of_nodes))
		{
			if (ALLOCATE(element_type_node_sequence, struct FE_element_type_node_sequence, 1) &&
				ALLOCATE(node_numbers, int, number_of_nodes))
			{
				element_type_node_sequence->identifier.number_of_nodes = number_of_nodes;
				element_type_node_sequence->identifier.node_numbers = node_numbers;
				element_type_node_sequence->element=ACCESS(FE_element)(element);
				element_type_node_sequence->dimension = element->getDimension();
				if (face_number >= 0)
					--(element_type_node_sequence->dimension);
				element_type_node_sequence->access_count=0;
				/* put the nodes in the identifier in ascending order */
				for (i=0;element_type_node_sequence&&(i<number_of_nodes);i++)
				{
					node_number=(nodes_in_element[i])->cm_node_identifier;
					node_numbers[i]=node_number;
					/* SAB Reenabled the matching of differently ordered faces as
						detecting the continuity correctly is more important than the
						problems with lines matching to different nodes when
						inheriting from different parents.
						OLDCOMMENT
						We do not want to sort these node numbers as the order of the nodes
						determines which ordering the faces are in and if these do not match
						we will get the lines matched to different pairs of the nodes. */
					placed=0;
					for (j=0;(!placed)&&(j<i);j++)
					{
						if (node_number<node_numbers[j])
						{
							/* make space for the new number */
							for (k=i;j<k;k--)
							{
								node_numbers[k]=node_numbers[k-1];
							}
							node_numbers[j]=node_number;
							placed=1;
						}
					}
					if (!placed)
					{
						node_numbers[i]=node_number;
					}
				}
#if defined (DEBUG_CODE)
				/*???debug*/
				printf("FE_element_type_node_sequence  %d-D element %d has nodes: ",
					element->getDimension(), element->get_identifier());
				for (i=0;i<number_of_nodes;i++)
				{
					printf(" %d",node_numbers[i]);
				}
				printf("\n");
#endif /* defined (DEBUG_CODE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(FE_element_type_node_sequence).  Not enough memory");
				DEALLOCATE(element_type_node_sequence);
			}
			for (i=0;i<number_of_nodes;i++)
			{
				DEACCESS(FE_node)(nodes_in_element+i);
			}
			DEALLOCATE(nodes_in_element);
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(FE_element_type_node_sequence).  "
				"Could not get nodes in element");
			element_type_node_sequence=(struct FE_element_type_node_sequence *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_element_type_node_sequence).  Invalid argument(s)");
		element_type_node_sequence=(struct FE_element_type_node_sequence *)NULL;
	}
	LEAVE;

	return (element_type_node_sequence);
} /* CREATE(FE_element_type_node_sequence) */

int DESTROY(FE_element_type_node_sequence)(
	struct FE_element_type_node_sequence **element_type_node_sequence_address)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Cleans up memory used by the FE_element_type_node_sequence.
==============================================================================*/
{
	int return_code;
	struct FE_element_type_node_sequence *element_type_node_sequence;

	ENTER(DESTROY(FE_element_type_node_sequence));
	if (element_type_node_sequence_address&&
		(element_type_node_sequence= *element_type_node_sequence_address))
	{
		if (0==element_type_node_sequence->access_count)
		{
			/* must deaccess element */
			DEACCESS(FE_element)(&(element_type_node_sequence->element));
			DEALLOCATE(element_type_node_sequence->identifier.node_numbers);
			DEALLOCATE(*element_type_node_sequence_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_element_type_node_sequence).  Non-zero access count of %d",
				element_type_node_sequence->access_count);
			*element_type_node_sequence_address=
				(struct FE_element_type_node_sequence *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_element_type_node_sequence).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_element_type_node_sequence) */

DECLARE_OBJECT_FUNCTIONS(FE_element_type_node_sequence)

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(FE_element_type_node_sequence, \
	identifier, FE_element_type_node_sequence_identifier&, \
	compare_FE_element_type_node_sequence_identifier)

DECLARE_INDEXED_LIST_FUNCTIONS(FE_element_type_node_sequence)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION( \
	FE_element_type_node_sequence,identifier, \
	FE_element_type_node_sequence_identifier&, \
	compare_FE_element_type_node_sequence_identifier)

int FE_element_type_node_sequence_is_collapsed(
	struct FE_element_type_node_sequence *element_type_node_sequence)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns true if the <element_type_node_sequence> represents an element that
has collapsed, ie, is a face with <= 2 unique nodes, or a line with 1 unique
node.
???RC Note that repeated nodes in the face/line are not put in the node_numbers
array twice, facilitating the simple logic in this function. If they were, then
this function would have to be modified extensively.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_type_node_sequence_is_collapsed);
	if (element_type_node_sequence)
	{
		return_code=((2 == element_type_node_sequence->dimension)&&
			(2 >= element_type_node_sequence->identifier.number_of_nodes))||
			((1 == element_type_node_sequence->dimension)&&
				(1 == element_type_node_sequence->identifier.number_of_nodes));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_type_node_sequence_is_collapsed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_type_node_sequence_is_collapsed */

struct FE_element *FE_element_type_node_sequence_get_FE_element(
	struct FE_element_type_node_sequence *element_type_node_sequence)
{
	if (element_type_node_sequence)
		return element_type_node_sequence->element;
	return 0;
}

void FE_element_type_node_sequence_set_FE_element(
	struct FE_element_type_node_sequence *element_type_node_sequence,
	struct FE_element *element)
{
	if (element_type_node_sequence)
		REACCESS(FE_element)(&element_type_node_sequence->element, element);
}

FE_element_type_node_sequence *FE_element_type_node_sequence_list_find_match(
	struct LIST(FE_element_type_node_sequence) *element_type_node_sequence_list,
	FE_element_type_node_sequence *element_type_node_sequence)
{
	if (element_type_node_sequence_list && element_type_node_sequence)
		return FIND_BY_IDENTIFIER_IN_LIST(FE_element_type_node_sequence, identifier)(
			element_type_node_sequence->identifier, element_type_node_sequence_list);
	return 0;
}

DECLARE_CHANGE_LOG_MODULE_FUNCTIONS(FE_field)

static int count_nodal_values(struct FE_node_field *node_field,
	void *number_of_nodal_values)
/*******************************************************************************
LAST MODIFIED : 12 April 1996

DESCRIPTION :
Increases the <number_of_nodal_values> by the number of values for <node_field>.
==============================================================================*/
{
	int i,*number_of_values,return_code;
	struct FE_node_field_component *component;

	ENTER(count_nodal_values);
	if (node_field&&(node_field->field)&&(component=node_field->components)&&
		(number_of_values=(int *)number_of_nodal_values))
	{
		i=node_field->field->number_of_components;
		while (i>0)
		{
			*number_of_values += (component->number_of_versions)*
				((component->number_of_derivatives)+1);
			component++;
			i--;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"count_nodal_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* count_nodal_values */

static int count_nodal_size(struct FE_node_field *node_field,
	void *number_of_nodal_values)
/*******************************************************************************
LAST MODIFIED : 26 October 1999

DESCRIPTION :
Increases the <number_of_nodal_values> by the number of values for <node_field>
times the size of <node_field->field->value_type> .
==============================================================================*/
{
	int i,*number_of_values,return_code,size,this_values_storage_size;
	struct FE_node_field_component *component;

	ENTER(count_nodal_values);
	if (node_field&&(node_field->field)&&(component=node_field->components)&&
		(number_of_values=(int *)number_of_nodal_values))
	{
		size=get_Value_storage_size(node_field->field->value_type,
			node_field->time_sequence);
		this_values_storage_size=0;
		for (i=node_field->field->number_of_components;0<i;i--)
		{
			this_values_storage_size += ((component->number_of_versions)*
				((component->number_of_derivatives)+1))*size;
			component++;
		}
		ADJUST_VALUE_STORAGE_SIZE(this_values_storage_size);
		*number_of_values += this_values_storage_size;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"count_nodal_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* count_nodal_size */

struct Merge_FE_node_field_into_list_data
{
	int requires_merged_storage;
	int values_storage_size;
	int number_of_values;
	struct LIST(FE_node_field) *list;
}; /* struct Merge_FE_node_field_into_list_data */

static int merge_FE_node_field_into_list(struct FE_node_field *node_field,
	void *merge_FE_node_field_into_list_data)
/*******************************************************************************
LAST MODIFIED : 23 October 2002

DESCRIPTION :
Merges the <node_field> into the list.
If there is already a node_field for the field in <node_field>, checks the two
are compatible and adds any new times that may be in the new field.
If <node_field> introduces a new field to the list, it is added to the list
with value offset to the <value_size> which it subsequently increases to fit
the new data added for this node field. <number_of_values> is also increased
appropriately.
==============================================================================*/
{
	int i, j, new_values_storage_size, return_code, size, this_number_of_values;
	struct FE_field *field;
	struct FE_node_field *existing_node_field, *new_node_field;
	struct FE_node_field_component *component, *existing_component,
		*new_component;
	struct FE_time_sequence *merged_time_sequence;
	struct Merge_FE_node_field_into_list_data *merge_data;

	ENTER(merge_FE_node_field_into_list);
	if (node_field&&(field=node_field->field) &&
		(merge_data = (struct Merge_FE_node_field_into_list_data *)
		merge_FE_node_field_into_list_data))
	{
		/* check if the node field is in the list */
		if (NULL != (existing_node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
			field, merge_data->list)))
		{
			/* the node field is already in the list */
			/* check that the components agree */
			/*???DB.  Check names ? */
			if ((component = node_field->components) &&
				(existing_component = existing_node_field->components))
			{
				i = field->number_of_components;
				return_code = 1;
				while (return_code && (i > 0))
				{
					if ((component->number_of_derivatives ==
						existing_component->number_of_derivatives) &&
						(component->number_of_versions ==
							existing_component->number_of_versions))
					{
						component++;
						existing_component++;
						i--;
					}
					else
					{
						return_code = 0;
					}
				}
			}
			else
			{
				return_code = 0;
			}
			if (return_code)
			{
				/* if the new node_field is a time based node field merge it in */
				if (node_field->time_sequence)
				{
					if (existing_node_field->time_sequence)
					{
						/* Merging two time fields.  Make a node_field with the
							 combined list */
						if (NULL != (merged_time_sequence =
							FE_region_get_FE_time_sequence_merging_two_time_series(
								FE_field_get_FE_region(field), node_field->time_sequence,
								existing_node_field->time_sequence)))
						{
							if (compare_FE_time_sequence(merged_time_sequence,
									existing_node_field->time_sequence))
							{
								/* Time sequences are different */
								merge_data->requires_merged_storage = 1;
								new_node_field = copy_create_FE_node_field_with_offset(
									existing_node_field, /*offset*/0);
								REACCESS(FE_time_sequence)(&(new_node_field->time_sequence),
									merged_time_sequence);
								if (REMOVE_OBJECT_FROM_LIST(FE_node_field)(
									existing_node_field, merge_data->list) &&
									ADD_OBJECT_TO_LIST(FE_node_field)(new_node_field,
									merge_data->list))
								{
									/* Finished */
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"merge_FE_node_field_into_list.  "
										"Unable to replace node_field in merged list.");
									return_code = 0;
								}
							}
							else
							{
								/* Can continue with the existing time_sequence */
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"merge_FE_node_field_into_list.  Unable to merge time arrays.");
							return_code = 0;
						}
					}
					else
					{
						/* Overwriting a single valued field with a time based
							 field, the values storage size will change, need to
							 write the code */
						display_message(ERROR_MESSAGE,
							"merge_FE_node_field_into_list.  "
							"Need to write code to overwrite non time field with "
							"time based field");
						return_code = 0;
					}
				}
				else
				{
					if (existing_node_field->time_sequence)
					{
						/* Overwriting a time based field with a single valued
							 field, the values storage size will change, need to
							 write the code */
						display_message(ERROR_MESSAGE,
							"merge_FE_node_field_into_list.  "
							"Need to write code to overwrite time field with "
							"non time based field");
						return_code = 0;
					}
				}
			}
		}
		else
		{
			/* the node field is not already in the list */
			/* create a new node field with modified offsets */
			merge_data->requires_merged_storage = 1;
			if ((component = node_field->components) &&
				(new_node_field = CREATE(FE_node_field)(field)))
			{
				FE_node_field_set_FE_time_sequence(new_node_field,
					node_field->time_sequence);
				size = get_Value_storage_size(field->value_type,
					node_field->time_sequence);
				new_component = new_node_field->components;
				i = field->number_of_components;
				return_code = 1;
				new_values_storage_size = 0;
				while (return_code && (i > 0))
				{
					new_component->value =
						merge_data->values_storage_size + new_values_storage_size;
					new_component->number_of_derivatives =
						component->number_of_derivatives;
					new_component->number_of_versions =
						component->number_of_versions;
					this_number_of_values = ((component->number_of_versions)*
						((component->number_of_derivatives) + 1));
					new_values_storage_size += this_number_of_values*size;
					merge_data->number_of_values += this_number_of_values;
					if (component->nodal_value_types)
					{
						/* assign nodal value names */
						if (ALLOCATE(new_component->nodal_value_types,
							enum FE_nodal_value_type,
							new_component->number_of_derivatives + 1))
						{
							for (j = new_component->number_of_derivatives; j >= 0; j--)
							{
								(new_component->nodal_value_types)[j] =
									(component->nodal_value_types)[j];
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"merge_FE_node_field_into_list.  "
								"Could not allocate memory for nodal value types");
						}
					}
					component++;
					new_component++;
					i--;
				}
				if (return_code)
				{
					ADJUST_VALUE_STORAGE_SIZE(new_values_storage_size);
					merge_data->values_storage_size += new_values_storage_size;
					/* add the new node field to the list */
					if (!ADD_OBJECT_TO_LIST(FE_node_field)(new_node_field,
						merge_data->list))
					{
						DESTROY(FE_node_field)(&new_node_field);
						return_code = 0;
					}
				}
				else
				{
					DESTROY(FE_node_field)(&new_node_field);
				}
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"merge_FE_node_field_into_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* merge_FE_node_field_into_list */

#if !defined (WINDOWS_DEV_FLAG)
/***************************************************************************//**
 * Outputs details of how field is defined at node.
 */
static int list_FE_node_field(struct FE_node *node, struct FE_field *field,
	void *dummy_void)
{
	char *component_name, *string_value;
	const char *type_string;
	enum FE_nodal_value_type *type;
	FE_value time;
	int i,version,k,number_of_components,number_of_times,number_of_versions,
		return_code,time_index,xi_dimension,xi_index;
	struct FE_element *embedding_element;
	struct FE_node_field *node_field;
	struct FE_node_field_component *node_field_component;
	Value_storage *values_storage, *value;

	ENTER(list_FE_node_field);
	USE_PARAMETER(dummy_void);
	if (node && field)
	{
		node_field = (struct FE_node_field *)NULL;
		if (node->fields)
		{
			node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
				field, node->fields->node_field_list);
		}
		if (node_field && node_field->components)
		{
			return_code=1;
			display_message(INFORMATION_MESSAGE,"  %s",field->name);
			if (NULL != (type_string=ENUMERATOR_STRING(CM_field_type)(field->cm_field_type)))
			{
				display_message(INFORMATION_MESSAGE,", %s",type_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"list_FE_node_field.  Invalid CM field type");
				return_code=0;
			}
			if (NULL != (type_string=ENUMERATOR_STRING(Coordinate_system_type)(
				field->coordinate_system.type)))
			{
				display_message(INFORMATION_MESSAGE,", %s",type_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"list_FE_node_field.  Invalid field coordinate system");
				return_code=0;
			}
			number_of_components=field->number_of_components;
			display_message(INFORMATION_MESSAGE,", #Components=%d\n",
				number_of_components);
			if (node_field->time_sequence)
			{
				number_of_times = FE_time_sequence_get_number_of_times(
					node_field->time_sequence);
			}
			else
			{
				number_of_times = 1;
			}
			for (time_index = 0 ; return_code && (time_index < number_of_times) ;
				  time_index++)
			{
				if (node_field->time_sequence)
				{
					FE_time_sequence_get_time_for_index(node_field->time_sequence,
						time_index, &time);
					display_message(INFORMATION_MESSAGE,"   Time: %g\n", time);
				}
				i=0;
				node_field_component = node_field->components;
				while (return_code&&(i<number_of_components))
				{
					if (NULL != (component_name=get_FE_field_component_name(field,i)))
					{
						display_message(INFORMATION_MESSAGE,"    %s",component_name);
						DEALLOCATE(component_name);
					}
					number_of_versions=node_field_component->number_of_versions;
					if (1<number_of_versions)
					{
						display_message(INFORMATION_MESSAGE,", #Versions=%d",
							number_of_versions);
					}
					display_message(INFORMATION_MESSAGE,".  ");
					/* display field based information */
					if (field->number_of_values)
					{
						int count;

						display_message(INFORMATION_MESSAGE,"field based values: ");
						switch (field->value_type)
						{
							case FE_VALUE_VALUE:
							{
								display_message(INFORMATION_MESSAGE,"\n");
								display_message(INFORMATION_MESSAGE,"    ");
								/* output in columns if FE_VALUE_MAX_OUTPUT_COLUMNS > 0 */
								for (count=0;count<field->number_of_values;count++)
								{
									display_message(INFORMATION_MESSAGE," %" FE_VALUE_STRING,
										*((FE_value *)(field->values_storage+
											  count*sizeof(FE_value))));
									if ((0<FE_VALUE_MAX_OUTPUT_COLUMNS)&&
										(0==((count+1)%FE_VALUE_MAX_OUTPUT_COLUMNS)))
									{
										display_message(INFORMATION_MESSAGE,"\n");
									}
								}
								display_message(INFORMATION_MESSAGE,"\n");
							} break;
							case STRING_VALUE:
							{
								display_message(INFORMATION_MESSAGE, "\n");
								display_message(INFORMATION_MESSAGE, "    ");
								for (count = 0; count < field->number_of_values; count++)
								{
									if (get_FE_field_string_value(field, count, &string_value))
									{
										make_valid_token(&string_value);
										display_message(INFORMATION_MESSAGE, " %s", string_value);
										DEALLOCATE(string_value);
									}
								}
								display_message(INFORMATION_MESSAGE, "\n");
							} break;
							default:
							{
								display_message(INFORMATION_MESSAGE, "list_FE_node_field.  "
									"Can't display that field value_type yet.  Write the code!");
							} break;
						}
					}
					/* display node based information*/
					if ((values_storage=node->values_storage)&&(type=node_field_component->
							 nodal_value_types))
					{
						values_storage += node_field_component->value;
						version=0;
						while (return_code&&(version<number_of_versions))
						{
							if (1<number_of_versions)
							{
								display_message(INFORMATION_MESSAGE,"\n      Version %d.  ",version+1);
								type=node_field_component->nodal_value_types;
							}
							k=1+(node_field_component->number_of_derivatives);
							while (return_code&&(k>0))
							{
								display_message(INFORMATION_MESSAGE,"%s=",
									ENUMERATOR_STRING(FE_nodal_value_type)(*type));
								/* display node based field information */
								if (field->number_of_times)
								{
									/* for the moment don't display the (generally massive) time
										based */
									/* field information */
									display_message(INFORMATION_MESSAGE," Time based field ");
								}
								else
								{
									if (node_field->time_sequence)
									{
										switch (field->value_type)
										{
											case FE_VALUE_VALUE:
											{
												display_message(INFORMATION_MESSAGE,"%g",
													*(*((FE_value**)values_storage) + time_index));
											} break;
											case INT_VALUE:
											{
												display_message(INFORMATION_MESSAGE,"%d",
													*(*((int **)values_storage) + time_index));
											} break;
											default:
											{
												display_message(INFORMATION_MESSAGE,"list_FE_node_field: "
													"Can't display times for this value type");
											} break;
										}/* switch*/
									}
									else /* (node_field->time_sequence) */
									{
										switch (field->value_type)
										{
											case FE_VALUE_VALUE:
											{
												display_message(INFORMATION_MESSAGE,"%g",
													*((FE_value*)values_storage));
											} break;
											case ELEMENT_XI_VALUE:
											{
												embedding_element = *((struct FE_element **)values_storage);
												if (embedding_element)
												{
													xi_dimension = embedding_element->getDimension();
													display_message(INFORMATION_MESSAGE,"%d-D %d xi",
														xi_dimension, embedding_element->get_identifier());
													value=values_storage+sizeof(struct FE_element *);
													for (xi_index=0;xi_index<xi_dimension;xi_index++)
													{
														display_message(INFORMATION_MESSAGE," %g",
															*((FE_value *)value));
														value += sizeof(FE_value);
													}
												}
												else
												{
													display_message(INFORMATION_MESSAGE,"UNDEFINED");
												}
											} break;
											case INT_VALUE:
											{
												display_message(INFORMATION_MESSAGE,"%d",
													*((int *)values_storage));
											} break;
											case STRING_VALUE:
											{
												char *name;

												if (get_FE_nodal_string_value(node,field,
														 /*component_number*/i,version,*type,&name))
												{
													display_message(INFORMATION_MESSAGE,name);
													DEALLOCATE(name);
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"list_FE_node_field.  Could not get string value");
													return_code=0;
												}
											} break;
											default:
											{
												display_message(INFORMATION_MESSAGE,"list_FE_node_field: "
													"Can't display that Value_type yet. Write the code!");
											} break;
										}/* switch*/
									} /* (node_field->time_sequence) */
								} /* if (field->number_of_times) */
								values_storage += get_Value_storage_size(field->value_type,
									node_field->time_sequence);
								type++;
								k--;
								if (k>0)
								{
									display_message(INFORMATION_MESSAGE,", ");
								}
							}
							version++;
						} /* while (return_code&&(version<number_of_versions)) */
						display_message(INFORMATION_MESSAGE,"\n");
					}
					else
					{
						/* missing nodal values only an error if no field based values
							either */
						if (!(field->number_of_values))
						{
							display_message(ERROR_MESSAGE,
								"list_FE_node_field.  Missing nodal values");
							return_code=0;
						}
					}
					node_field_component++;
					i++;
				}
			} /* time_index */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"list_FE_node_field.  Field %s is not defined at node %d",
				field->name, node->cm_node_identifier);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_FE_node_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_FE_node_field */
#endif /* !defined (WINDOWS_DEV_FLAG) */

struct Match_FE_element_shape_data
{
	int dimension;
	const int *type;
}; /* struct Match_FE_element_shape_data */

static int match_FE_element_shape(struct FE_element_shape *shape,
	void *match_FE_element_shape_data)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Checks if the <match_FE_element_shape_data> matchs the <shape>.
Note a NULL shape type means an unspecified shape of that dimension.
==============================================================================*/
{
	const int *find_type;
	int i, return_code, *shape_type;
	struct Match_FE_element_shape_data *match_data;

	ENTER(match_FE_element_shape);
	if (shape && (match_data =
		(struct Match_FE_element_shape_data *)match_FE_element_shape_data))
	{
		if (match_data->dimension == shape->dimension)
		{
			find_type = match_data->type;
			shape_type = shape->type;
			if (find_type && shape_type)
			{
				i = (match_data->dimension)*((match_data->dimension)+1)/2;
				while ((i>0)&&(*find_type== *shape_type))
				{
					find_type++;
					shape_type++;
					i--;
				}
				if (i>0)
				{
					return_code=0;
				}
				else
				{
					return_code=1;
				}
			}
			else if ((!find_type) && (!shape_type))
			{
				/* no type array: unspecified shape */
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"match_FE_element_shape.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* match_FE_element_shape */

static struct FE_element_shape *find_FE_element_shape_in_list(int dimension,
	const int *type,struct LIST(FE_element_shape) *list)
/*******************************************************************************
LAST MODIFIED : 18 November 2002

DESCRIPTION :
Searchs the <list> for the element shape with the specified <dimension> and
<type> and returns the address of the element_shape.
A NULL <type> means an unspecified shape of <dimension>.
==============================================================================*/
{
	struct Match_FE_element_shape_data match_data;
	struct FE_element_shape *shape;

	ENTER(find_FE_element_shape_in_list);
	if ((dimension > 0) && list)
	{
		match_data.dimension = dimension;
		match_data.type = type;
		shape = FIRST_OBJECT_IN_LIST_THAT(FE_element_shape)(match_FE_element_shape,
			(void *)(&match_data),list);
	}
	else
	{
		shape = (struct FE_element_shape *)NULL;
	}
	LEAVE;

	return (shape);
} /* find_FE_element_shape_in_list */

namespace {

/**
 * Cache to make ElementDOFMap::evaluate more efficient.
 */
class ElementDOFMapEvaluationCache
{
public:
	FE_element *element;
	FE_field *field;
	int componentNumber;
	cmzn_mesh_scale_factor_set *scaleFactorSet;
	FE_value time;
	// cache node_field_info/component as next node is likely to re-use
	FE_node_field_info *nodeFieldInfo;
	FE_node_field_component *nodeFieldComponent;
	// cache time sequence and indexes as requires binary search to find
	FE_time_sequence *timeSequence;
	int timeIndex1, timeIndex2;
	FE_value timeXi;
	// cache element node scale information to save lookups
	FE_node **nodes;
	int numberOfElementNodes;
	FE_value *scaleFactors;
	int numberOfScaleFactors;

	ElementDOFMapEvaluationCache(FE_element *element,
			FE_field *field, FE_value componentNumber = 0, cmzn_mesh_scale_factor_set *scaleFactorSet = 0, FE_value time = 0.0) :
		element(element),
		field(field),
		componentNumber(componentNumber),
		scaleFactorSet(scaleFactorSet),
		time(time),
		nodeFieldInfo(0),
		nodeFieldComponent(0),
		timeSequence(0),
		scaleFactors(0),
		numberOfScaleFactors(0)
	{
		FE_element_node_scale_field_info *info = element->information;
		if (info)
		{
			this->nodes = info->nodes;
			this->numberOfElementNodes = info->number_of_nodes;
			// get scale factors for the current scale factor set
			if (scaleFactorSet)
			{
				this->scaleFactors = info->getScaleFactorsForSet(scaleFactorSet, this->numberOfScaleFactors);
			}
		}
		else
		{
			this->nodes = 0;
			this->numberOfElementNodes = 0;
		}
	}

	inline void setTimesequence(FE_time_sequence *timeSequenceIn)
	{
		if (timeSequenceIn != this->timeSequence)
		{
			this->timeSequence = timeSequenceIn;
			if (this->timeSequence)
			{
				FE_time_sequence_get_interpolation_for_time(this->timeSequence,
					this->time, &this->timeIndex1, &this->timeIndex2, &this->timeXi);
			}
		}
	}

};

/**
 * Cache to make ElementDOFMap::matchesWithInfo more efficient.
 */
class ElementDOFMapMatchCache
{
public:
	cmzn_mesh_scale_factor_set *scaleFactorSet;
	FE_element_node_scale_field_info *info1;
	FE_element_node_scale_field_info *info2;
	FE_node **nodes1;
	int numberOfElementNodes1;
	FE_node **nodes2;
	int numberOfElementNodes2;
	int numberOfScaleFactors1;
	FE_value *scaleFactors1;
	int numberOfScaleFactors2;
	FE_value *scaleFactors2;

	/**
	 * @param scaleFactorSet  The mesh scale factor set.
	 * @param thisInfo  Node scale information supplying nodes and scale factors
	 * for map1 (this).
	 * @param info2  Node scale information supplying nodes and scale factors
	 * for map2 (other).
	 * @param globalNodes1  Optional list of global nodes. If supplied then the
	 * node in this list of the same identifier as the node referenced by this map
	 * with info1 is used. Used in "can be merged" code when non-global nodes are
	 * in the element.
	 */
	ElementDOFMapMatchCache(cmzn_mesh_scale_factor_set *scaleFactorSet,
			FE_element_node_scale_field_info *info1,
			FE_element_node_scale_field_info *info2) :
		scaleFactorSet(scaleFactorSet),
		info1(info1),
		info2(info2),
		numberOfScaleFactors1(0),
		scaleFactors1(0),
		numberOfScaleFactors2(0),
		scaleFactors2(0)
	{
		if (this->info1)
		{
			this->nodes1 = this->info1->nodes;
			this->numberOfElementNodes1 = this->info1->number_of_nodes;
			if (this->scaleFactorSet)
			{
				this->scaleFactors1 = this->info1->getScaleFactorsForSet(scaleFactorSet, this->numberOfScaleFactors1);
			}
		}
		else
		{
			this->nodes1 = 0;
			this->numberOfElementNodes1 = 0;
		}
		if (this->info2)
		{
			this->nodes2 = this->info2->nodes;
			this->numberOfElementNodes2 = this->info2->number_of_nodes;
			if (this->scaleFactorSet)
			{
				this->scaleFactors2 = this->info2->getScaleFactorsForSet(scaleFactorSet, this->numberOfScaleFactors2);
			}
		}
		else
		{
			this->nodes2 = 0;
			this->numberOfElementNodes2 = 0;
		}
	}
};

/**
 * DOF mapping taking a single node value by value type and version, and
 * optionally multiplying it by a single scale factor.
 */
class NodeToElementDOFMap : public ElementDOFMap
{
	int nodeIndex;
	FE_nodal_value_type valueType;
	int version;
	int scaleFactorIndex;

public:
	NodeToElementDOFMap(int nodeIndex) :
		nodeIndex(nodeIndex),
		valueType(FE_NODAL_VALUE),
		version(0),
		scaleFactorIndex(-1)
	{
	}

	virtual ElementDOFMap *clone()
	{
		return new NodeToElementDOFMap(*this);
	}

	virtual ElementDOFMap *cloneWithNewNodeIndices(
		FE_element_node_scale_field_info *mergeInfo,
		FE_element_node_scale_field_info *sourceInfo)
	{
		NodeToElementDOFMap *newNodeMap = 0;
		if (sourceInfo && sourceInfo->nodes && mergeInfo && mergeInfo->nodes)
		{
			if ((this->nodeIndex >= 0) && (this->nodeIndex < sourceInfo->number_of_nodes))
			{
				newNodeMap = new NodeToElementDOFMap(*this);
				FE_node *node = sourceInfo->nodes[this->nodeIndex];
				if (node)
				{
					FE_node **mergeNode = mergeInfo->nodes;
					int mergeNodeIndex = 0;
					for (int i = 0; i < mergeInfo->number_of_nodes; ++i)
					{
						if (node == mergeNode[i])
						{
							newNodeMap->nodeIndex = mergeNodeIndex;
							break;
						}
						++mergeNodeIndex;
					}
				}
				else
				{
					// Note sure if this is relevant here:
					// [since using this function in define_FE_field_at_element,
					// have to handle case of	NULL nodes by using existing node_index]
					newNodeMap->nodeIndex = this->nodeIndex;
				}
			}
		}
		return newNodeMap;
	}

	virtual bool evaluate(ElementDOFMapEvaluationCache& cache, FE_value& value)
	{
		if ((!cache.nodes) || (this->nodeIndex >= cache.numberOfElementNodes))
		{
			display_message(ERROR_MESSAGE, "NodeToElementDOFMap::evaluate.  "
				"Element %d field %s component %d: local node index %d out of range %d",
				cache.element->get_identifier(), cache.field->name, cache.componentNumber + 1,
				this->nodeIndex + 1, cache.numberOfElementNodes);
			return false;
		}
		FE_node *node = cache.nodes[this->nodeIndex];
		if (node->fields != cache.nodeFieldInfo)
		{
			FE_node_field *node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
				cache.field, node->fields->node_field_list);
			if (!node_field)
			{
				display_message(ERROR_MESSAGE, "NodeToElementDOFMap::evaluate.  "
					"Element %d field %s component %d: Field not defined on global node %d at local node index %d",
					cache.element->get_identifier(), cache.field->name, cache.componentNumber + 1,
					node->cm_node_identifier, this->nodeIndex + 1);
				return false;
			}
			cache.nodeFieldComponent = node_field->components + cache.componentNumber;
			cache.nodeFieldInfo = node->fields;
			cache.setTimesequence(node_field->time_sequence);
		}
		FE_nodal_value_type *valueTypes = cache.nodeFieldComponent->nodal_value_types;
		if (!valueTypes)
		{
			display_message(ERROR_MESSAGE, "NodeToElementDOFMap::evaluate.  "
				"Element %d field %s component %d: Global node %d has no value/derivative types",
				cache.element->get_identifier(), cache.field->name, cache.componentNumber + 1,
				node->cm_node_identifier);
			return false;
		}
		int numberOfValueTypes = cache.nodeFieldComponent->number_of_derivatives + 1;
		int i = 0;
		while ((i < numberOfValueTypes) && (valueTypes[i] != this->valueType))
		{
			++i;
		}
		if ((i == numberOfValueTypes) || (this->version >= cache.nodeFieldComponent->number_of_versions))
		{
			display_message(ERROR_MESSAGE, "NodeToElementDOFMap::evaluate.  "
				"Element %d field %s component %d: Global node %d has no %s version %d",
				cache.element->get_identifier(), cache.field->name, cache.componentNumber + 1,
				node->cm_node_identifier, ENUMERATOR_STRING(FE_nodal_value_type)(this->valueType),
				this->version + 1);
			return false;
		}
		void *nodeValues = node->values_storage + cache.nodeFieldComponent->value;
		int valueIndex = i;
		if (this->version)
		{
			valueIndex += this->version * (cache.nodeFieldComponent->number_of_derivatives + 1);
		}
		FE_value scaleFactor = 1.0;
		if (this->scaleFactorIndex >= 0)
		{
			if (this->scaleFactorIndex >= cache.numberOfScaleFactors)
			{
				display_message(ERROR_MESSAGE, "NodeToElementDOFMap::evaluate.  "
					"Element %d field %s component %d: Scale factor index %d is out of range %d",
					cache.element->get_identifier(), cache.field->name, cache.componentNumber + 1,
					this->scaleFactorIndex + 1, cache.numberOfScaleFactors);
				return false;
			}
			scaleFactor *= cache.scaleFactors[this->scaleFactorIndex];
		}
		if (cache.timeSequence)
		{
			FE_value *array = *((static_cast<FE_value **>(nodeValues) + valueIndex));
			value = scaleFactor*(
				(1.0 - cache.timeXi)*array[cache.timeIndex1] +
				(      cache.timeXi)*array[cache.timeIndex2]);
		}
		else
		{
			value = scaleFactor*(static_cast<FE_value *>(nodeValues)[valueIndex]);
		}
		return true;
	}

	virtual bool evaluateNode(ElementDOFMapEvaluationCache& cache, FE_node*& node)
	{
		if ((!cache.nodes) || (this->nodeIndex >= cache.numberOfElementNodes))
		{
			display_message(ERROR_MESSAGE, "NodeToElementDOFMap::evaluateNode.  "
				"Element %d field %s component %d: local node index %d out of range %d",
				cache.element->get_identifier(), cache.field->name, cache.componentNumber + 1,
				this->nodeIndex + 1, cache.numberOfElementNodes);
			return false;
		}
		node = cache.nodes[this->nodeIndex];
		return true;
	}

	virtual bool matches(ElementDOFMap *otherMap)
	{
		NodeToElementDOFMap *nodeMap2 = dynamic_cast<NodeToElementDOFMap *>(otherMap);
		if (nodeMap2)
		{
			if ((this->nodeIndex == nodeMap2->nodeIndex) &&
				(this->valueType == nodeMap2->valueType) &&
				(this->version == nodeMap2->version) &&
				(this->scaleFactorIndex == nodeMap2->scaleFactorIndex))
			{
				return true;
			}
		}
		return false;
	}

	virtual bool matchesWithInfo(ElementDOFMap *otherMap, ElementDOFMapMatchCache& cache)
	{
		NodeToElementDOFMap *nodeMap2 = dynamic_cast<NodeToElementDOFMap *>(otherMap);
		if (nodeMap2)
		{
			FE_node *node1 = (this->nodeIndex < cache.numberOfElementNodes1) ? cache.nodes1[this->nodeIndex] : 0;
			FE_node *node2 = (this->nodeIndex < cache.numberOfElementNodes2) ? cache.nodes2[this->nodeIndex] : 0;
			if (node1 && node2 && (node1 == node2) &&
				(this->valueType == nodeMap2->valueType) &&
				(this->version == nodeMap2->version) &&
				(this->scaleFactorIndex == nodeMap2->scaleFactorIndex))
			{
				return true;
			}
		}
		return false;
	}

	virtual void setLocalNodeInUse(int numberOfLocalNodes, int *localNodeInUse)
	{
		if (this->nodeIndex < numberOfLocalNodes)
		{
			localNodeInUse[this->nodeIndex] = 1;
		}
	}
};

/**
 * DOF mapping which sums values from 0 or more DOF maps.
 * For building general linear maps, hanging nodes etc.
 * Note: maps passed to this object belong to it and are destroyed with it.
 */
class SumElementDOFMap : public ElementDOFMap
{
	int numberOfMaps;
	ElementDOFMap **maps;

public:
	SumElementDOFMap() :
		numberOfMaps(0),
		maps(0)
	{
	}

	SumElementDOFMap(SumElementDOFMap& source) :
		numberOfMaps(source.numberOfMaps),
		maps(0)
	{
		ALLOCATE(this->maps, ElementDOFMap*, this->numberOfMaps);
		for (int i = 0; i < this->numberOfMaps; ++i)
		{
			this->maps[i] = source.maps[i]->clone();
		}
	}

	virtual ~SumElementDOFMap()
	{
		for (int i = 0; i < this->numberOfMaps; ++i)
		{
			delete this->maps[i];
		}
		DEALLOCATE(maps);
	}

	virtual ElementDOFMap *clone()
	{
		return new SumElementDOFMap(*this);
	}

	virtual ElementDOFMap *cloneWithNewNodeIndices(
		FE_element_node_scale_field_info *mergeInfo,
		FE_element_node_scale_field_info *sourceInfo)
	{
		SumElementDOFMap *newSum = new SumElementDOFMap();
		for (int i = 0; i < this->numberOfMaps; ++i)
		{
			newSum->addMap(this->maps[i]->cloneWithNewNodeIndices(mergeInfo, sourceInfo));
		}
		return newSum;
	}

	/**
	 * Add map to the list of maps in the sum. This object assumes ownership of the
	 * map and will be responsible for destroying it.
	 * @return  true on success, false on failure
	 */
	bool addMap(ElementDOFMap *map)
	{
		if (map)
		{
			ElementDOFMap **temp;
			if (REALLOCATE(temp, this->maps, ElementDOFMap*, this->numberOfMaps + 1))
			{
				this->maps = temp;
				this->maps[this->numberOfMaps] = map;
				++(this->numberOfMaps);
				return true;
			}
		}
		return false;
	}

	virtual bool evaluate(ElementDOFMapEvaluationCache& cache, FE_value& value)
	{
		value = 0.0;
		FE_value term;
		for (int i = 0; i < this->numberOfMaps; ++i)
		{
			if (!this->maps[i]->evaluate(cache, term))
			{
				display_message(ERROR_MESSAGE, "SumElementDOFMap::evaluate.  "
					"Element %d field %s component %d:  Could not evaluate map %d of sum",
					cache.element->get_identifier(), cache.field->name, cache.componentNumber + 1,
					i + 1);
				return false;
			}
			value += term;
		}
		return true;
	}

	/** WARNING: returns only the first node from which DOFs are mapped, if any */
	virtual bool evaluateNode(ElementDOFMapEvaluationCache& cache, FE_node*& node)
	{
		node = 0;
		for (int i = 0; i < this->numberOfMaps; ++i)
		{
			if (!this->maps[i]->evaluateNode(cache, node))
			{
				display_message(ERROR_MESSAGE, "SumElementDOFMap::evaluateNode.  "
					"Element %d field %s component %d:  Could not evaluate map %d of sum",
					cache.element->get_identifier(), cache.field->name, cache.componentNumber + 1,
					i + 1);
				return false;
			}
			if (node)
			{
				return true;
			}
		}
		return true;
	}

	virtual bool matches(ElementDOFMap *otherMap)
	{
		SumElementDOFMap *otherSum = dynamic_cast<SumElementDOFMap *>(otherMap);
		if (!otherSum || (this->numberOfMaps != otherSum->numberOfMaps))
			return false;
		for (int i = 0; i < this->numberOfMaps; ++i)
		{
			if (!this->maps[i]->matches(otherSum->maps[i]))
				return false;
		}		
		return true;
	}

	virtual bool matchesWithInfo(ElementDOFMap *otherMap, ElementDOFMapMatchCache& cache)
	{
		SumElementDOFMap *otherSum = dynamic_cast<SumElementDOFMap *>(otherMap);
		if (!otherSum || (this->numberOfMaps != otherSum->numberOfMaps))
			return false;
		for (int i = 0; i < this->numberOfMaps; ++i)
		{
			if (!this->maps[i]->matchesWithInfo(otherSum->maps[i], cache))
				return false;
		}		
		return true;
	}

	virtual void setLocalNodeInUse(int numberOfLocalNodes, int *localNodeInUse)
	{
		for (int i = 0; i < this->numberOfMaps; ++i)
		{
			this->maps[i]->setLocalNodeInUse(numberOfLocalNodes, localNodeInUse);
		}
	}
};

} // namespace { }

/**
 * The standard function for calculating the <element> <values> for the given
 * <component_number> of <element_field>. Calculates the <number_of_values> and
 * the <values> for the component.  The storage for the <values> is allocated by
 * the function.
 * Uses relative offsets into nodal values array in standard and general node to
 * element maps. Absolute offset for start of field component is obtained from
 * the node_field_component for the field at the node.
 */
static int global_to_element_map_values(struct FE_element *element,
	struct FE_element_field *element_field, FE_value time, int component_number,
	int *number_of_values,FE_value **values)
{
	FE_field *field;
	FE_element_field_component *component;
	FE_value *element_values = 0;
	int number_of_element_values = 0;
	int return_code = 1;

	if (element&&element_field&&(field=element_field->field)&&number_of_values&&
		values&&(0<=component_number)&&
		(component_number<field->number_of_components)&&
		(component=element_field->components[component_number]))
	{
		FE_basis *basis = component->basis;
		/* retrieve the element values */
		switch (component->type)
		{
			case STANDARD_NODE_TO_ELEMENT_MAP:
				/* global values are associated with nodes */
			{
				FE_value *fe_value_array, *element_value, *scale_factors = 0;
				int number_of_element_nodes, number_of_map_values = 0,
					number_of_scale_factors,*scale_factor_index,scale_index,
					value_index;
				short *short_array;
				struct FE_node *node = NULL,**nodes;
				struct FE_node_field *node_field;
				struct FE_node_field_component *node_field_component;
				struct FE_node_field_info *node_field_info;
				struct Standard_node_to_element_map *standard_node_map,
					**standard_node_map_address;
				void *global_values;
				/* check information */
				if ((element->information)&&(nodes=element->information->nodes)&&
					((number_of_element_nodes=element->information->number_of_nodes)>0)&&
					((0==(number_of_scale_factors=
						  element->information->number_of_scale_factors))||
						((0<number_of_scale_factors)&&
							(scale_factors=element->information->scale_factors))))
				{
					/* calculate the number of element values by summing the numbers of
						values retrieved from each node */
					number_of_element_values=0;
					standard_node_map_address=
						component->map.standard_node_based.node_to_element_maps;
					int j = component->map.standard_node_based.number_of_nodes;
					return_code=1;
					while (return_code&&(j>0))
					{
						if ((standard_node_map= *standard_node_map_address)&&
							((number_of_map_values=
								standard_node_map->number_of_nodal_values)>0)&&
							(0<=standard_node_map->node_index)&&
							(standard_node_map->node_index<number_of_element_nodes)&&
							(node=nodes[standard_node_map->node_index])&&
							(node->values_storage)&&(node->fields))
						{
							number_of_element_values += number_of_map_values;
							standard_node_map_address++;
							j--;
						}
						else
						{
							return_code = 0;
							display_message(ERROR_MESSAGE,"global_to_element_map_values.  "
								"Invalid standard node to element map");
							if (standard_node_map)
							{
								if (number_of_map_values>0)
								{
									if ((0<=standard_node_map->node_index)&&
										(standard_node_map->node_index<number_of_element_nodes))
									{
										if (node)
										{
											if (node->values_storage)
											{
											}
											else
											{
												display_message(ERROR_MESSAGE,"global_to_element_map_values.  "
													"Node %d used by field %s in element %d has no field parameters.",
													node->cm_node_identifier, field->name, element->get_identifier());
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,"global_to_element_map_values.  "
												"Node reference missing for field %s at element %d.",
												field->name, element->get_identifier());
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,"global_to_element_map_values.  "
											"Node element map for field %s specifies a node index out of range for element %d.",
											field->name, element->get_identifier());
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"global_to_element_map_values.  "
										"No map values for element %d.",
										element->get_identifier());
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"global_to_element_map_values.  "
									"No standard node map for element %d.",
									element->get_identifier());
							}
						}
					}
					if (return_code)
					{
						/* allocate storage for storing the element values */
						if ((number_of_element_values>0)&&
							(ALLOCATE(element_values,FE_value,number_of_element_values)))
						{
							element_value=element_values;
							/* for each node retrieve the scaled nodal values */
							standard_node_map_address=
								component->map.standard_node_based.node_to_element_maps;
							j=component->map.standard_node_based.number_of_nodes;
							/* Need node_field_component to get absolute offset into nodal
								values array. Also store node_field_info so we don't have to
								get node_field_component each time if it is not changing */
							node_field_info=(struct FE_node_field_info *)NULL;
							node_field_component=(struct FE_node_field_component *)NULL;
							FE_time_sequence *time_sequence = 0;
							int time_index_one, time_index_two;
							FE_value time_xi;
							while (return_code&&(j>0))
							{
								/* retrieve the scaled nodal values */
								standard_node_map= *standard_node_map_address;
								node=nodes[standard_node_map->node_index];
								scale_factor_index=standard_node_map->scale_factor_indices;
								/* get node_field_component for absolute offsets into nodes */
								if (node_field_info != node->fields)
								{
									if (node->fields && (node_field =
										FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field, node->fields->node_field_list)) &&
										node_field->components)
									{
										node_field_component = node_field->components + component_number;
										node_field_info = node->fields;
										if ((node_field->time_sequence != time_sequence) &&
											(time_sequence = node_field->time_sequence))
										{
											FE_time_sequence_get_interpolation_for_time(time_sequence,
												time, &time_index_one, &time_index_two, &time_xi);
										}
									}
									else
									{
										/* field not defined at this node */
										node_field_component=(struct FE_node_field_component *)NULL;
									}
								}
								if (node_field_component)
								{
									/* add absolute value offset from node_field_component and
										cast address into FE_value type */
									global_values = node->values_storage + node_field_component->value;
									FE_nodal_value_type *nodal_value_type_address = standard_node_map->nodal_value_types;
									int *nodal_version_address = standard_node_map->nodal_versions;
									int number_of_node_value_types = node_field_component->number_of_derivatives + 1;
									int number_of_node_versions = node_field_component->number_of_versions;
									int k = standard_node_map->number_of_nodal_values;
									while (0 < k)
									{
										FE_nodal_value_type nodal_value_type = *nodal_value_type_address;
										int version = *nodal_version_address;
										if (FE_NODAL_UNKNOWN == nodal_value_type)
										{
											// special case of zero DOF
											*element_value = 0;
										}
										else
										{
											int i = 0;
											while ((node_field_component->nodal_value_types[i] != nodal_value_type) &&
												(i < number_of_node_value_types))
											{
												++i;
											}
											if (i >= number_of_node_value_types)
											{
												display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
													"Parameter '%s' not found for field %s at node %d, used from element %d",
													ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type), field->name,
													node->cm_node_identifier, element->get_identifier());
												return_code = 0;
												break;
											}
											if ((version < 0) || (version > number_of_node_versions))
											{
												display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
													"Parameter '%s' version %d is out of range (%d) for field %s at node %d, used from element %d",
													ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_type),
													version + 1, number_of_node_versions, field->name,
													node->cm_node_identifier, element->get_identifier());
												return_code = 0;
												break;
											}
											// GRC future: versions per derivative change
											value_index = version*number_of_node_value_types + i;
											switch (field->value_type)
											{
												case FE_VALUE_VALUE:
												{
													if (time_sequence)
													{
														fe_value_array = *((static_cast<FE_value **>(global_values) + value_index));
														*element_value = (1.0 - time_xi)*fe_value_array[time_index_one]
															+ time_xi*fe_value_array[time_index_two];
													}
													else
													{
														*element_value = static_cast<FE_value *>(global_values)[value_index];
													}
												} break;
												case SHORT_VALUE:
												{
													if (time_sequence)
													{
														short_array = *((static_cast<short **>(global_values)+value_index));
														*element_value = (1.0 - time_xi)*(FE_value)short_array[time_index_one]
															+ time_xi*static_cast<FE_value>(short_array[time_index_two]);
													}
													else
													{
														*element_value = static_cast<FE_value>(static_cast<short *>(global_values)[value_index]);
													}
												} break;
												default:
												{
													display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
														"Unsupported value type %s in finite element field",
														Value_type_string(field->value_type));
													return_code = 0;
												} break;
											}
											if ((0 <= (scale_index = *scale_factor_index)) &&
												(scale_index < number_of_scale_factors))
											{
												(*element_value) *= scale_factors[scale_index];
											}
										}
										++element_value;
										++nodal_value_type_address;
										++nodal_version_address;
										++scale_factor_index;
										--k;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"global_to_element_map_values.  Cannot evaluate field %s "
										"in element %d because it is not defined at node %d",
										field->name,element->get_identifier(),node->cm_node_identifier);
									return_code=0;
								}
								standard_node_map_address++;
								j--;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"global_to_element_map_values.  Could not allocate memory for values");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"global_to_element_map_values.  Missing element information");
					return_code=0;
				}
			} break;
			case GENERAL_ELEMENT_MAP:
			{
				ElementDOFMap **maps = component->map.general_map_based.maps;
				number_of_element_values = component->map.general_map_based.number_of_maps;
				ALLOCATE(element_values, FE_value, number_of_element_values);
				if (element_values)
				{
					ElementDOFMapEvaluationCache cache(element, field, component_number,
						component->get_scale_factor_set(), time);
					for (int j = 0; j < number_of_element_values; ++j)
					{
						if (!maps[j]->evaluate(cache, element_values[j]))
						{
							return_code = 0;
							break;
						}
					}
				}
				else
				{
					return_code = 0;
				}
			} break;
		case ELEMENT_GRID_MAP:
			{
				display_message(ERROR_MESSAGE,
					"global_to_element_map_values.  Not valid for grid");
				return_code = 0;
			} break;
		}
		if (return_code)
		{
			/* if necessary, modify the calculated element values */
			if (component->modify)
			{
				return_code=(component->modify)(component,element,field,
					time,number_of_element_values,element_values);
			}
			if (return_code)
			{
				if (FE_basis_get_number_of_functions(basis) == number_of_element_values)
				{
					int number_of_blended_element_values = FE_basis_get_number_of_blended_functions(basis);
					if (number_of_blended_element_values > 0)
					{
						FE_value *blended_element_values = FE_basis_get_blended_element_values(basis, element_values);
						*values=blended_element_values;
						*number_of_values=number_of_blended_element_values;
						DEALLOCATE(element_values);
						if (!blended_element_values)
						{
							display_message(ERROR_MESSAGE,
								"global_to_element_map_values.  Could not allocate memory for blended values");
							return_code = 0;
						}
					}
					else
					{
						*values = element_values;
						*number_of_values = number_of_element_values;
					}
				}
				else
				{
					char *basis_string = FE_basis_get_description_string(basis);
					display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
						"Incorrect number of element values (%d) for basis (%s)",
						number_of_element_values, basis_string);
					DEALLOCATE(basis_string);
					DEALLOCATE(element_values);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"global_to_element_map_values.  Error modifying element values");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"global_to_element_map_values.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

/**
 * The standard function for calculating the nodes used for a <component>.
 * Calculates the <number_of_values> and the node used to calculate each element
 * value for a <component>.  The storage for the nodes array
 * (<*element_values_address>) is allocated by the function.
 * Limitation: Only returns the first node contributing to the DOF for general
 * maps from multiple nodes.
 */
static int global_to_element_map_nodes(
	struct FE_element_field_component *component,struct FE_element *element,
	struct FE_field *field,int *number_of_values,
	struct FE_node ***element_values_address)
{
	int number_of_element_values = 0;
	FE_node **element_values = 0;
	int return_code = 1;
	if (component&&element&&field&&number_of_values&&element_values_address)
	{
		/* retrieve the element values */
		switch (component->type)
		{
			case STANDARD_NODE_TO_ELEMENT_MAP:
			{
				int j,k,number_of_element_nodes,number_of_map_values;
				struct FE_node **element_value,*node,**nodes;
				/* check information */
				if ((element->information)&&(nodes=element->information->nodes)&&
					((number_of_element_nodes=element->information->number_of_nodes)>0))
				{
					/* calculate the number of element values by summing the numbers
						of values retrieved from each node */
					number_of_element_values=0;
					Standard_node_to_element_map *standard_node_map,
						**standard_node_map_address;
					standard_node_map_address=
						component->map.standard_node_based.node_to_element_maps;
					j=component->map.standard_node_based.number_of_nodes;
					while (return_code&&(j>0))
					{
						if ((standard_node_map= *standard_node_map_address)&&
							((number_of_map_values=
							standard_node_map->number_of_nodal_values)>0)&&
							(0<=standard_node_map->node_index)&&
							(standard_node_map->node_index<number_of_element_nodes)&&
							(node=nodes[standard_node_map->node_index])&&
							(node->values_storage)&&(node->fields))
						{
							number_of_element_values += number_of_map_values;
							standard_node_map_address++;
							j--;
						}
						else
						{
							display_message(ERROR_MESSAGE,"global_to_element_map_nodes.  "
								"Invalid standard node to element map");
							return_code=0;
						}
					}
					if (return_code)
					{
						/* allocate storage for storing the element values */
						if ((number_of_element_values>0)&&(ALLOCATE(element_values,
							struct FE_node *,number_of_element_values)))
						{
							element_value=element_values;
							/* for each node retrieve the scaled nodal values */
							standard_node_map_address=
								component->map.standard_node_based.node_to_element_maps;
							for (j=component->map.standard_node_based.number_of_nodes;j>0;
								j--)
							{
								/* retrieve the scaled nodal values */
								standard_node_map= *standard_node_map_address;
								node=nodes[standard_node_map->node_index];
								for (k=standard_node_map->number_of_nodal_values;k>0;k--)
								{
									*element_value=node;
									element_value++;
								}
								standard_node_map_address++;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
					"global_to_element_map_nodes.  Could not allocate memory for values");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"global_to_element_map_nodes.  Missing element information");
					return_code=0;
				}
			} break;
			case GENERAL_ELEMENT_MAP:
			{
				// Warning: only returns first node contributing to DOF
				// Not adequate for general maps from multiple nodes
				ElementDOFMap **maps = component->map.general_map_based.maps;
				number_of_element_values = component->map.general_map_based.number_of_maps;
				ALLOCATE(element_values, struct FE_node *, number_of_element_values);
				if (element_values)
				{
					ElementDOFMapEvaluationCache cache(element, field);
					for (int j = 0; j < number_of_element_values; ++j)
					{
						if (!maps[j]->evaluateNode(cache, element_values[j]))
						{
							return_code = 0;
							break;
						}
					}
				}
				else
				{
					return_code = 0;
				}
			} break;
			case ELEMENT_GRID_MAP:
			{
				display_message(ERROR_MESSAGE,
					"global_to_element_map_nodes.  Not valid for grid");
				return_code=0;
			} break;
		}
		if (return_code)
		{
			*element_values_address=element_values;
			*number_of_values=number_of_element_values;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"global_to_element_map_nodes.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

struct Check_element_grid_map_values_storage_data
{
	int check_sum,values_storage_size;
}; /* struct Check_element_grid_map_values_storage_data */

static int check_element_grid_map_values_storage(
	struct FE_element_field *element_field,
	void *check_element_grid_map_values_storage_data_void)
/*******************************************************************************
LAST MODIFIED : 14 December 2000

DESCRIPTION :
If the <element_field> is grid based, check that the index range is within the
values_storage array and add the values_storage size it requires to the
check_sum.
==============================================================================*/
{
	int i,j,*number_in_xi,number_of_values,return_code,size,value_index,
		values_storage_size;
	struct Check_element_grid_map_values_storage_data *check_grid_data;
	struct FE_element_field_component **component;

	ENTER(check_element_grid_map_values_storage);
	return_code=0;
	if (element_field&&element_field->field&&(check_grid_data=
		(struct Check_element_grid_map_values_storage_data *)
		check_element_grid_map_values_storage_data_void))
	{
		return_code=1;
		/* only GENERAL_FE_FIELD has components and can be grid-based */
		if (GENERAL_FE_FIELD==element_field->field->fe_field_type)
		{
			size=get_Value_storage_size(element_field->field->value_type,
				(struct FE_time_sequence *)NULL);
			component=element_field->components;
			for (i=element_field->field->number_of_components;(0<i)&&return_code;i--)
			{
				if (ELEMENT_GRID_MAP==(*component)->type)
				{
					number_in_xi=((*component)->map).element_grid_based.number_in_xi;
					number_of_values=1;
					int number_of_xi_coordinates = 0;
					FE_basis_get_dimension((*component)->basis, &number_of_xi_coordinates);
					for (j = number_of_xi_coordinates; j > 0; j--)
					{
						number_of_values *= (*number_in_xi)+1;
						number_in_xi++;
					}
					value_index=((*component)->map).element_grid_based.value_index;
					values_storage_size = number_of_values*size;
					/* make sure values storage is word aligned for machine */
					ADJUST_VALUE_STORAGE_SIZE(values_storage_size);
					if ((value_index<check_grid_data->values_storage_size)&&
						(value_index+values_storage_size <=
							check_grid_data->values_storage_size))
					{
						check_grid_data->check_sum += values_storage_size;
					}
					else
					{
						return_code=0;
					}
				}
				component++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"check_element_grid_map_values_storage.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* check_element_grid_map_values_storage */

static int for_FE_field_at_node_iterator(struct FE_node_field *node_field,
	void *iterator_and_data_void)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
FE_node_field iterator for for_each_FE_field_at_node.
==============================================================================*/
{
	int return_code;
	struct FE_node_field_iterator_and_data *iterator_and_data;

	ENTER(for_FE_field_at_node_iterator);
	if (node_field&&(iterator_and_data=
		(struct FE_node_field_iterator_and_data *)iterator_and_data_void)&&
		iterator_and_data->iterator)
	{
		return_code=(iterator_and_data->iterator)(iterator_and_data->node,
			node_field->field,iterator_and_data->user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_FE_field_at_node_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* for_FE_field_at_node_iterator */

static struct FE_node_field *FE_node_get_FE_node_field(struct FE_node *node,
	struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Returns the FE_node_field structure describing how <fe_field> is defined at
<node>. Returns NULL with no error if <fe_field> is not defined at <node>.
==============================================================================*/
{
	struct FE_node_field *node_field;

	ENTER(FE_node_get_FE_node_field);
	if (node && node->fields && fe_field)
	{
		node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(fe_field,
			node->fields->node_field_list);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"FE_node_get_FE_node_field.  Invalid argument(s)");
		node_field = (struct FE_node_field *)NULL;
	}
	LEAVE;

	return (node_field);
} /* FE_node_get_FE_node_field */

static int find_FE_nodal_values_storage_dest(struct FE_node *node,
	struct FE_field *field,int component_number,int version,
	enum FE_nodal_value_type type,enum Value_type value_type,
	Value_storage **values_storage, struct FE_time_sequence **time_sequence)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Returns a pointer to the nodal values_storage and the matching time_sequence
for the given node, component, version, type and value_type.
Returns 0 with no error in cases where the version or type is not stored, hence
can use this function to determing if either are defined.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type;
	int i,length,return_code,size;
	struct FE_node_field *node_field;
	struct FE_node_field_component *node_field_component;
	Value_storage *the_values_storage = (Value_storage *)NULL;

	ENTER(find_FE_nodal_values_storage_dest);
	return_code=0;
	if (node&&field&&(0<=component_number)&&
		(component_number<field->number_of_components)&&(0<=version))
	{
		if (NULL != (node_field = FE_node_get_FE_node_field(node, field)))
		{
			if (NULL != (node_field_component=node_field->components))
			{
				if (node_field->field->value_type == value_type)
				{
					node_field_component += component_number;
					if (version < node_field_component->number_of_versions)
					{
						if (NULL != (nodal_value_type=node_field_component->nodal_value_types))
						{
							length=1+(node_field_component->number_of_derivatives);
							i=0;
							while ((i<length) && (type != nodal_value_type[i]))
							{
								i++;
							}
							if (i<length)
							{
								size = get_Value_storage_size(value_type,
									node_field->time_sequence);
								the_values_storage = node->values_storage +
									(node_field_component->value) + ((version*length+i)*size);
								*values_storage = the_values_storage;
								*time_sequence = node_field->time_sequence;
								return_code=1;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"find_FE_nodal_values_storage_dest.  "
								"Missing nodal_value_type array");
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"find_FE_nodal_values_storage_dest.  value_type mismatch");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"find_FE_nodal_values_storage_dest.  Invalid node/field");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"find_FE_nodal_values_storage_dest.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* find_FE_nodal_values_storage_dest */

static char *get_automatic_component_name(char **component_names,
	int component_no)
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Used internally by some FE_field functions. Returns an allocated string
containing the component name for <component_no> (starting at 0 for the first).
If <component_names> or component_names[component_no] are NULL a name consisting
of the value component_no+1 is created and returned.
If <component_names> is not NULL, the function assumes that component_no is
less than the number of names in this array.
It is up to the calling function to deallocate the returned string.
==============================================================================*/
{
	char *component_name,*source_name,temp_string[20];

	ENTER(get_automatic_component_name);
	if (0<=component_no)
	{
		if (component_names&&component_names[component_no])
		{
			source_name=component_names[component_no];
		}
		else
		{
			sprintf(temp_string,"%i",component_no+1);
			source_name=temp_string;
		}
		component_name = duplicate_string(source_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_automatic_component_name.  Invalid component_no");
		component_name=(char *)NULL;
	}
	LEAVE;

	return (component_name);
} /* get_automatic_component_name */

static int node_on_axis(struct FE_node *node,struct FE_field *field,
	FE_value time, enum Coordinate_system_type coordinate_system_type)
/*******************************************************************************
LAST MODIFIED : 26 December 2000

DESCRIPTION :
Returns non-zero if the <node> is on the axis for the <field>/
<coordinate_system_type> and zero otherwise.
==============================================================================*/
{
	FE_value node_value;
	int return_code;

	ENTER(node_on_axis);
	return_code=0;
	switch (coordinate_system_type)
	{
		case CYLINDRICAL_POLAR:
		{
			calculate_FE_field(field,0,node,(struct FE_element *)NULL,
				(FE_value *)NULL,time,&node_value);
			if (0==node_value)
			{
				return_code=1;
			}
		} break;
		case PROLATE_SPHEROIDAL:
		case OBLATE_SPHEROIDAL:
		{
			calculate_FE_field(field,1,node,(struct FE_element *)NULL,
				(FE_value *)NULL,time,&node_value);
			if ((0==node_value)||(PI==node_value))
			{
				return_code=1;
			}
		} break;
		case SPHERICAL_POLAR:
		{
			calculate_FE_field(field,2,node,(struct FE_element *)NULL,
				(FE_value *)NULL,time,&node_value);
			if ((-PI/2==node_value)||(PI/2==node_value))
			{
				return_code=1;
			}
		} break;
		default:
		{
			// nothing to do; return 0
		} break;
	}
	LEAVE;

	return (return_code);
} /* node_on_axis */

static int face_calculate_xi_normal(struct FE_element_shape *shape,
	int face_number,FE_value *normal)
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Calculates the <normal>, in xi space, of the specified face.

Cannot guarantee that <normal> is inward.
==============================================================================*/
{
	double determinant,*matrix_double,norm;
	FE_value *face_to_element,sign;
	int i,j,k,*pivot_index,return_code;

	ENTER(face_calculate_xi_normal);
	const int dimension = get_FE_element_shape_dimension(shape);
	if ((0 < dimension) && (0<=face_number) && (face_number < shape->number_of_faces) && normal)
	{
		return_code = 1;
		if (1==dimension)
		{
			normal[0]=(FE_value)1;
		}
		else
		{
			/* working storage */
			ALLOCATE(matrix_double,double,(dimension-1)*(dimension-1));
			ALLOCATE(pivot_index,int,dimension-1);
			if (matrix_double&&pivot_index)
			{
				sign=(FE_value)1;
				face_to_element=(shape->face_to_element)+
					(face_number*dimension*dimension);
				i=0;
				while (return_code&&(i<dimension))
				{
					/* calculate the determinant of face_to_element excluding column 1 and
						row i+1 */
					for (j=0;j<i;j++)
					{
						for (k=1;k<dimension;k++)
						{
							matrix_double[j*(dimension-1)+(k-1)]=
								(double)face_to_element[j*dimension+k];
						}
					}
					for (j=i+1;j<dimension;j++)
					{
						for (k=1;k<dimension;k++)
						{
							matrix_double[(j-1)*(dimension-1)+(k-1)]=
								(double)face_to_element[j*dimension+k];
						}
					}
					if (LU_decompose(dimension-1,matrix_double,pivot_index,
						&determinant,/*singular_tolerance*/1.0e-12))
					{
						for (j=0;j<dimension-1;j++)
						{
							determinant *= matrix_double[j*(dimension-1)+j];
						}
						normal[i]=sign*(FE_value)determinant;
						sign= -sign;
					}
					else
					{
						normal[i] = 0.0;
						sign = -sign;
					}
					i++;
				}
				if (return_code)
				{
					/* normalize face_normal */
					norm=(double)0;
					for (i=0;i<dimension;i++)
					{
						norm += (double)(normal[i])*(double)(normal[i]);
					}
					if (0<norm)
					{
						norm=sqrt(norm);
						for (i=0;i<dimension;i++)
						{
							normal[i] /= (FE_value)norm;
						}
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"face_calculate_xi_normal.  "
					"Could not allocate matrix_double (%p) or pivot_index (%p)",
					matrix_double,pivot_index);
				return_code=0;
			}
			DEALLOCATE(pivot_index);
			DEALLOCATE(matrix_double);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"face_calculate_xi_normal.  "
			"Invalid argument(s).  %p %d %d %p",shape,dimension,face_number,normal);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* face_calculate_xi_normal */

/**
 * Adds the <increment> to <xi>.  If this moves <xi> outside of the shape, then
 * the step is limited to take <xi> to the boundary, <face_number> is set to be
 * the limiting face, <fraction> is updated with the fraction of the <increment>
 * actually used, the <increment> is updated to contain the part not used,
 * the <xi_face> are calculated for that face and the <xi> are changed to be
 * on the boundary of the shape.
 */
int FE_element_shape_xi_increment(struct FE_element_shape *shape,
	FE_value *xi,FE_value *increment, FE_value *step_size,
	int *face_number_address, FE_value *xi_face)
{
	double determinant,step_size_local;
	FE_value *face_to_element;
	int dimension,face_number,i,j,k,return_code;
	/* While we are calculating this in doubles all the xi locations are
		stored in floats and so when we change_element and are numerically just
		outside it can be a value as large as this */
	// ???DB.  Assumes that <xi> is inside
	// ???DB.  Needs reordering inside finite_element
#define SMALL_STEP (1e-4)

	ENTER(FE_element_shape_xi_increment);
	return_code=0;
	if ((shape) && (0 < (dimension = shape->dimension)) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS) &&
		xi && increment && face_number_address && xi_face)
	{
		/* working space */
		double face_matrix[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS];
		double face_rhs[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		int pivot_index[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		return_code=1;
		/* check if it is in the shape */
		face_number= -1;
		*step_size=(FE_value)1;
		face_to_element=shape->face_to_element;
		for (i=0;i<shape->number_of_faces;i++)
		{
			for (j=0;j<dimension;j++)
			{
				face_matrix[j*dimension]=(double)(-increment[j]);
				face_rhs[j]=(double)(xi[j]-(*face_to_element));
				face_to_element++;
				for (k=1;k<dimension;k++)
				{
					face_matrix[j*dimension+k]=(double)(*face_to_element);
					face_to_element++;
				}
			}
			/* Don't terminate if the LU_decompose fails as the matrix is
				probably singular, just implying that we are travelling parallel
				to the face */
			if (LU_decompose(dimension,face_matrix,pivot_index,
				&determinant,/*singular_tolerance*/1.0e-12)&&
				LU_backsubstitute(dimension,face_matrix,
				pivot_index,face_rhs))
			{
				step_size_local=face_rhs[0];
				if ((step_size_local > -SMALL_STEP) && (step_size_local < SMALL_STEP) && ((FE_value)step_size_local<*step_size))
				{
					determinant = 0.0;
					for (j = 0 ; j < dimension ; j++)
					{
						determinant += shape->face_normals[i * dimension + j] *
							increment[j];
					}
					if (determinant > -SMALL_STEP)
					{
						/* We are stepping out of the element so this is a boundary */
						for (j=1;j<dimension;j++)
						{
							xi_face[j-1]=(FE_value)(face_rhs[j]);
						}
						face_number=i;
						*step_size=(FE_value)step_size_local; /* or set to zero exactly?? */
					}
				}
				else
				{
					if ((0 < step_size_local) && ((FE_value)step_size_local<*step_size))
					{
						for (j=1;j<dimension;j++)
						{
							xi_face[j-1]=(FE_value)(face_rhs[j]);
						}
						face_number=i;
						*step_size=(FE_value)step_size_local;
					}
				}
			}
		}
		*face_number_address=face_number;
		if (-1==face_number)
		{
			/* inside */
			for (i=0;i<dimension;i++)
			{
				xi[i] += increment[i];
				increment[i]=(FE_value)0;
			}
		}
		else
		{
			/* not inside */
			for (i=0;i<dimension;i++)
			{
				/* SAB Could use use face_to_element and face_xi to calculate
					updated xi location instead */
				xi[i] = xi[i] + *step_size * increment[i];
				increment[i] *= (1. - *step_size);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_shape_xi_increment.  "
			"Invalid argument(s).  %p %p %p %p dimension %d",shape,xi,increment,
			face_number_address, get_FE_element_shape_dimension(shape));
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_shape_xi_increment */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(CM_field_type)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(CM_field_type));
	switch (enumerator_value)
	{
		case CM_ANATOMICAL_FIELD:
		{
			enumerator_string = "anatomical";
		} break;
		case CM_COORDINATE_FIELD:
		{
			enumerator_string = "coordinate";
		} break;
		case CM_GENERAL_FIELD:
		{
			enumerator_string = "field";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(CM_field_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(CM_field_type)

struct FE_field_info *CREATE(FE_field_info)(struct FE_region *fe_region)
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
{
	struct FE_field_info *fe_field_info;

	ENTER(CREATE(FE_field_info));
	fe_field_info = (struct FE_field_info *)NULL;
	if (fe_region)
	{
		if (ALLOCATE(fe_field_info, struct FE_field_info, 1))
		{
			/* maintain pointer to the the FE_region this information belongs to.
				 It is not ACCESSed since FE_region is the owning object and it
				 would prevent the FE_region from being destroyed. */
			fe_field_info->fe_region = fe_region;
			fe_field_info->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_field_info).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_field_info).  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field_info);
} /* CREATE(FE_field_info) */

int DESTROY(FE_field_info)(
	struct FE_field_info **fe_field_info_address)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Destroys the FE_field_info at *<field_info_address>. Frees the
memory for the information and sets <*field_info_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_field_info *fe_field_info;

	ENTER(DESTROY(FE_field_info));
	if ((fe_field_info_address) &&
		(fe_field_info = *fe_field_info_address))
	{
		if (0 == fe_field_info->access_count)
		{
			DEALLOCATE(*fe_field_info_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_field_info).  Non-zero access count");
			return_code = 0;
		}
		*fe_field_info_address = (struct FE_field_info *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_field_info).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_field_info) */

DECLARE_OBJECT_FUNCTIONS(FE_field_info)

int FE_field_info_clear_FE_region(struct FE_field_info *field_info)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Clears the pointer to FE_region in <field_info>.
Private function only to be called by destroy_FE_region.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_info_clear_FE_region);
	if (field_info)
	{
		field_info->fe_region = (struct FE_region *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_info_clear_FE_region.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_info_clear_FE_region */

int get_FE_field_external_information(struct FE_field *field,
	struct FE_field_external_information **external_information)
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
Creates a copy of the <external_information> of the <field>.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_field_external_information);
	return_code=0;
	if (field&&external_information)
	{
		if (field->external)
		{
			if (field->external->duplicate)
			{
				*external_information=(field->external->duplicate)(field->external);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"get_FE_field_external_information.  "
					"Invalid external field information");
			}
		}
		else
		{
			*external_information=(struct FE_field_external_information *)NULL;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_external_information.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_external_information */

int set_FE_field_external_information(struct FE_field *field,
	struct FE_field_external_information *external_information)
/*******************************************************************************
LAST MODIFIED : 3 September 2001

DESCRIPTION :
Copies the <external_information> into the <field>.

Should only call this function for unmanaged fields.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_external_information);
	return_code=0;
	if (field)
	{
		return_code=1;
		if (field->external)
		{
			if (field->external->destroy)
			{
				(field->external->destroy)(&(field->external));
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_FE_field_external_information.  "
					"Invalid external field information");
				return_code=0;
			}
		}
		if (external_information)
		{
			if (external_information->duplicate)
			{
				field->external=(external_information->duplicate)(external_information);
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_FE_field_external_information.  "
					"Invalid external_information");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_external_information.  Invalid argument");
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_external_information */

struct FE_field *CREATE(FE_field)(const char *name, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Creates and returns a struct FE_field of <name> belonging to the ultimate
master FE_region of <fe_region>. The new field has no name/identifier, zero
components, field_type FIELD, NOT_APPLICABLE coordinate system, no field values.
???RC Used to pass <fe_time> in here and store in FE_field; can now get it from
FE_region.
==============================================================================*/
{
	int return_code;
	struct FE_field *field;

	ENTER(CREATE(FE_field));
	field = (struct FE_field *)NULL;
	if (name && fe_region)
	{
		if (ALLOCATE(field, struct FE_field, 1))
		{
			return_code = 1;
			if (!(field->name = duplicate_string(name)))
			{
				return_code = 0;
			}
			/* get and ACCESS FE_field_info relating this field back to fe_region */
			if (!(field->info =
				ACCESS(FE_field_info)(FE_region_get_FE_field_info(fe_region))))
			{
				return_code = 0;
			}
			field->fe_field_type = GENERAL_FE_FIELD;
			field->indexer_field = (struct FE_field *)NULL;
			field->number_of_indexed_values = 0;
			field->cm_field_type = CM_GENERAL_FIELD;
			field->external = (struct FE_field_external_information *)NULL;
			field->number_of_components = 0;
			/* don't allocate component names until we have custom names */
			field->component_names = (char **)NULL;
			field->coordinate_system.type = NOT_APPLICABLE;
			field->number_of_values = 0;
			field->values_storage = (Value_storage *)NULL;
			field->value_type = UNKNOWN_VALUE;
			field->element_xi_mesh_dimension = 0;
			field->number_of_times = 0;
			field->time_value_type = UNKNOWN_VALUE;
			field->times = (Value_storage *)NULL;
			field->number_of_wrappers = 0;
			field->access_count = 0;
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"CREATE(FE_field).  Could not construct contents");
				DEALLOCATE(field);
				field = (struct FE_field *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(FE_field).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "CREATE(FE_field).  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* CREATE(FE_field) */

int DESTROY(FE_field)(struct FE_field **field_address)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Frees the memory for the field and sets <*field_address> to NULL.
==============================================================================*/
{
	char **component_name;
	int i,return_code;
	struct FE_field *field;

	ENTER(DESTROY(FE_field));
	if ((field_address)&&(field= *field_address))
	{
		if (0==field->access_count)
		{
			/* free the field name */
			if (field->name)
			{
				DEALLOCATE(field->name);
			}
			DEACCESS(FE_field_info)(&field->info);
			REACCESS(FE_field)(&(field->indexer_field),(struct FE_field *)NULL);
			if (field->values_storage)
			{
				/* free any arrays pointed to by field->values_storage */
				free_value_storage_array(field->values_storage,field->value_type,
					(struct FE_time_sequence *)NULL,field->number_of_values);
				/* free the global values */
				DEALLOCATE(field->values_storage);
			}

			/* free the component names */
			if (NULL != (component_name=field->component_names))
			{
				for (i=field->number_of_components;i>0;i--)
				{
					DEALLOCATE(*component_name);
					component_name++;
				}
				DEALLOCATE(field->component_names);
			}
			DEALLOCATE(*field_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_field).  Non-zero access_count (%d)",field->access_count);
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_field) */

int list_FE_field(struct FE_field *field,void *dummy)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
Outputs the information contained in <field>.
==============================================================================*/
{
	char *component_name;
	int i, number_of_components, return_code;

	ENTER(list_FE_field);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code=1;
		/* write the identifier */
		display_message(INFORMATION_MESSAGE, "field : %s\n", field->name);
		display_message(INFORMATION_MESSAGE,
			"  access count = %d\n", field->access_count);
		display_message(INFORMATION_MESSAGE,"  type = %s",
			ENUMERATOR_STRING(CM_field_type)(field->cm_field_type));
		display_message(INFORMATION_MESSAGE,"  coordinate system = %s",
			ENUMERATOR_STRING(Coordinate_system_type)(field->coordinate_system.type));
		number_of_components=field->number_of_components;
		display_message(INFORMATION_MESSAGE,", #Components = %d\n",
			number_of_components);
		i=0;
		while (return_code&&(i<number_of_components))
		{
			if (NULL != (component_name = get_FE_field_component_name(field, i)))
			{
				display_message(INFORMATION_MESSAGE,"    %s", component_name);
				DEALLOCATE(component_name);
			}
			/* display field based information*/
			if (field->number_of_values)
			{
				int count;

				display_message(INFORMATION_MESSAGE,"field based values: ");
				switch (field->value_type)
				{
					case FE_VALUE_VALUE:
					{
						display_message(INFORMATION_MESSAGE,"\n");
						/* output in columns if FE_VALUE_MAX_OUTPUT_COLUMNS > 0 */
						for (count=0;count<field->number_of_values;count++)
						{
							display_message(INFORMATION_MESSAGE," %"FE_VALUE_STRING,
								*((FE_value*)(field->values_storage + count*sizeof(FE_value))));
							if ((0<FE_VALUE_MAX_OUTPUT_COLUMNS)&&
								(0==((count+1) % FE_VALUE_MAX_OUTPUT_COLUMNS)))
							{
								display_message(INFORMATION_MESSAGE,"\n");
							}
						}
					} break;
					default:
					{
						display_message(INFORMATION_MESSAGE,"list_FE_field: "
							"Can't display that field value_type yet. Write the code!");
					} break;
				}	/* switch () */
			}
			display_message(INFORMATION_MESSAGE,"\n");
			i++;
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,"list_FE_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_FE_field */

DECLARE_OBJECT_FUNCTIONS(FE_field)

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(FE_field)

DECLARE_INDEXED_LIST_STL_FUNCTIONS(FE_field)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(FE_field,name,const char *)
DECLARE_INDEXED_LIST_STL_IDENTIFIER_CHANGE_FUNCTIONS(FE_field,name)

DECLARE_CHANGE_LOG_FUNCTIONS(FE_field)

int FE_field_copy_without_identifier(struct FE_field *destination,
	struct FE_field *source)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Copies the contents but not the name identifier of <source> to <destination>.
Function prototype should be in finite_element_private.h, so not public.
???RC Change to macro so identifier member can vary?
?COPY_WITHOUT_IDENTIFIER object_type,identifier
==============================================================================*/
{
	char **component_names;
	int i, return_code;
	Value_storage *times, *values_storage;

	ENTER(FE_field_copy_without_identifier);
	if (destination && source)
	{
		return_code=1;
		component_names=(char **)NULL;
		values_storage=(Value_storage *)NULL;
		times=(Value_storage *)NULL;
		if (source->component_names)
		{
			if (ALLOCATE(component_names,char *,source->number_of_components))
			{
				for (i=0;i<source->number_of_components;i++)
				{
					component_names[i]=(char *)NULL;
				}
				/* copy the old names, clear any new ones */
				for (i=0;i<(source->number_of_components)&&return_code;i++)
				{
					if (source->component_names[i])
					{
						if (ALLOCATE(component_names[i],char,
							strlen(source->component_names[i])+1))
						{
							strcpy(component_names[i],source->component_names[i]);
						}
						else
						{
							return_code=0;
						}
					}
				}
			}
			else
			{
				return_code=0;
			}
		}
		if (0<source->number_of_values)
		{
			if (!((values_storage=make_value_storage_array(source->value_type,
				(struct FE_time_sequence *)NULL,source->number_of_values))&&
				copy_value_storage_array(values_storage,source->value_type,
					(struct FE_time_sequence *)NULL,(struct FE_time_sequence *)NULL,
					source->number_of_values,source->values_storage, /*optimised_merge*/0)))
			{
				return_code=0;
			}
		}
		if (0<source->number_of_times)
		{
			if (!((times=make_value_storage_array(source->time_value_type,
				(struct FE_time_sequence *)NULL,source->number_of_times))&&
				copy_value_storage_array(times,source->time_value_type,
					(struct FE_time_sequence *)NULL,(struct FE_time_sequence *)NULL,
					source->number_of_times,source->times, /*optimised_merge*/0)))
			{
				return_code=0;
			}
		}
		if (return_code)
		{
			REACCESS(FE_field_info)(&(destination->info), source->info);
			if (destination->cm_field_type != source->cm_field_type)
			{
				display_message(WARNING_MESSAGE, "Changing field %s CM type from %s to %s",
					source->name, ENUMERATOR_STRING(CM_field_type)(destination->cm_field_type),
					ENUMERATOR_STRING(CM_field_type)(source->cm_field_type));
				destination->cm_field_type = source->cm_field_type;
			}
			if (destination->external)
			{
				if (destination->external->destroy)
				{
					(destination->external->destroy)(&(destination->external));
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_field_copy_without_identifier.  "
						"Invalid destination->external");
				}
			}
			if (source->external)
			{
				if (source->external->duplicate)
				{
					destination->external=(source->external->duplicate)(
						source->external);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_field_copy_without_identifier.  "
						"Invalid source->external");
				}
			}
			destination->fe_field_type=source->fe_field_type;
			REACCESS(FE_field)(&(destination->indexer_field),
				source->indexer_field);
			destination->number_of_indexed_values=
				source->number_of_indexed_values;
			if (destination->component_names)
			{
				for (i = 0; i < destination->number_of_components; i++)
				{
					if (destination->component_names[i])
					{
						DEALLOCATE(destination->component_names[i]);
					}
				}
				DEALLOCATE(destination->component_names);
			}
			destination->number_of_components=source->number_of_components;
			destination->component_names=component_names;
			COPY(Coordinate_system)(&(destination->coordinate_system),
				&(source->coordinate_system));
			destination->value_type=source->value_type;
			destination->time_value_type=source->time_value_type;
			/* replace old values_storage with new */
			if (0<destination->number_of_values)
			{
				free_value_storage_array(destination->values_storage,
					destination->value_type,(struct FE_time_sequence *)NULL,
					destination->number_of_values);
				DEALLOCATE(destination->values_storage);
			}
			destination->number_of_values=source->number_of_values;
			destination->values_storage=values_storage;
			/* replace old times with new */
			if (0<destination->number_of_times)
			{
				free_value_storage_array(destination->times,
					destination->time_value_type,(struct FE_time_sequence *)NULL,
					destination->number_of_times);
				DEALLOCATE(destination->times);
			}
			destination->number_of_times=source->number_of_times;
			destination->times=times;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_field_copy_without_identifier.  "
				"Could not copy dynamic contents");
		}
		if (!return_code)
		{
			if (component_names)
			{
				for (i=0;i<source->number_of_components;i++)
				{
					if (component_names[i])
					{
						DEALLOCATE(component_names[i]);
					}
				}
				DEALLOCATE(component_names);
			}
			if (values_storage)
			{
				free_value_storage_array(values_storage,source->value_type,
					(struct FE_time_sequence *)NULL,source->number_of_values);
				DEALLOCATE(values_storage);
			}
			if (times)
			{
				free_value_storage_array(times,source->time_value_type,
					(struct FE_time_sequence *)NULL,source->number_of_times);
				DEALLOCATE(times);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_copy_without_identifier.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_copy_without_identifier */

int FE_field_matches_description(struct FE_field *field, const char *name,
	enum FE_field_type fe_field_type,struct FE_field *indexer_field,
	int number_of_indexed_values,enum CM_field_type cm_field_type,
	struct Coordinate_system *coordinate_system,enum Value_type value_type,
	int number_of_components,char **component_names,
	int number_of_times,enum Value_type time_value_type,
	struct FE_field_external_information *external)
/*******************************************************************************
LAST MODIFIED : 31 August 2001

DESCRIPTION :
Returns true if <field> has exactly the same <name>, <field_info>... etc. as
those given in the parameters.
==============================================================================*/
{
	char *component_name,*field_component_name;
	int i,return_code;

	ENTER(FE_field_matches_description);
	/* does not match until proven so */
	return_code=0;
	if (field&&name&&coordinate_system&&(0<=number_of_times))
	{
		if (field->name&&(0==strcmp(field->name,name))&&
			(fe_field_type==field->fe_field_type)&&
			((INDEXED_FE_FIELD != fe_field_type)||
				((indexer_field==field->indexer_field)&&
					(number_of_indexed_values==field->number_of_indexed_values)))&&
			(cm_field_type==field->cm_field_type)&&
			Coordinate_systems_match(&(field->coordinate_system),coordinate_system)&&
			(value_type == field->value_type)&&
			(number_of_components==field->number_of_components)&&
			(number_of_times==field->number_of_times)&&
			(time_value_type == field->time_value_type))
		{
			/* matches until disproven */
			return_code=1;
			/* check external */
			if (external)
			{
				if (field->external)
				{
					if ((external->compare)&&
						(external->compare==field->external->compare))
					{
						if ((external->compare)(external,field->external))
						{
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				if (field->external)
				{
					return_code=0;
				}
			}
			/* check the component names match */
			i=number_of_components;
			while ((i>0)&&return_code)
			{
				i--;
				if ((field_component_name=
					get_automatic_component_name(field->component_names,i))&&
					(component_name=get_automatic_component_name(component_names,i)))
				{
					if (strcmp(component_name,field_component_name))
					{
						return_code=0;
					}
					DEALLOCATE(component_name);
				}
				else
				{
					return_code=0;
				}
				DEALLOCATE(field_component_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_matches_description.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_field_matches_description */

bool FE_fields_match_fundamental(struct FE_field *field1,
	struct FE_field *field2)
{
	if (field1 && field2)
	{
		return
			(field1->value_type == field2->value_type) &&
			(field1->fe_field_type == field2->fe_field_type) &&
			(field1->number_of_components == field2->number_of_components) &&
			(0 != Coordinate_systems_match(&(field1->coordinate_system),
				&(field2->coordinate_system)));
	}
	display_message(ERROR_MESSAGE,
		"FE_fields_match_fundamental.  Missing field(s)");
	return false;
}

bool FE_fields_match_exact(struct FE_field *field1, struct FE_field *field2)
{
	if (field1 && field2)
	{
		// does not match until proven so
		if ((0 == strcmp(field1->name, field2->name)) &&
			(field1->fe_field_type == field2->fe_field_type) &&
			((INDEXED_FE_FIELD != field1->fe_field_type) ||
				(field1->indexer_field && field2->indexer_field &&
					(0 == strcmp(field1->indexer_field->name,
						field2->indexer_field->name)) &&
					FE_fields_match_fundamental(field1->indexer_field,
						field2->indexer_field) &&
					(field1->number_of_indexed_values ==
						field2->number_of_indexed_values))) &&
			(field1->cm_field_type == field2->cm_field_type) &&
			Coordinate_systems_match(&(field1->coordinate_system),
				&(field2->coordinate_system)) &&
			(field1->value_type == field2->value_type) &&
			(field1->number_of_components == field2->number_of_components) &&
			(field1->number_of_times == field2->number_of_times) &&
			(field1->time_value_type == field2->time_value_type))
		{
			// matches until disproven
			if (field1->external)
			{
				if ((field2->external) && (field1->external->compare) &&
					(field1->external->compare == field2->external->compare))
				{
					if ((field1->external->compare)(field1->external, field2->external))
						return false;
				}
				else
					return false;
			}
			else if (field2->external)
				return false;
			// check component names match
			for (int i = field1->number_of_components; i <= 0; --i)
			{
				char *component_name1 = get_automatic_component_name(field1->component_names, i);
				char *component_name2 = get_automatic_component_name(field2->component_names, i);
				bool matching = (component_name1) && (component_name2) &&
					(0 == strcmp(component_name1, component_name2));
				DEALLOCATE(component_name2);
				DEALLOCATE(component_name1);
				if (!matching)
					return false;
			}
			return true;
		}
	}
	else
		display_message(ERROR_MESSAGE, "FE_fields_match_exact.  Missing field(s)");
	return false;
}

/**
 * List iterator function which fetches a field with the same name as <field>
 * from <field_list>. Returns 1 (true) if there is either no such field in the
 * list or the two fields return true for FE_fields_match_fundamental(),
 * otherwise returns 0 (false).
 */
int FE_field_can_be_merged_into_list(struct FE_field *field, void *field_list_void)
{
	struct LIST(FE_field) *field_list = reinterpret_cast<struct LIST(FE_field) *>(field_list_void);
	if (field && field_list)
	{
		FE_field *other_field = FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(field->name, field_list);
		if ((!(other_field)) || FE_fields_match_fundamental(field, other_field))
			return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_can_be_merged_into_list.  Invalid argument(s)");
	}
	return 0;
}

// forward declarations
static void Standard_node_to_element_map_clear_node_value_labels(Standard_node_to_element_map *map);
static bool Standard_node_to_element_map_determine_or_check_node_value_labels(
	Standard_node_to_element_map *map, FE_field *field, int componentIndex,
	FE_element *element, FE_nodeset *target_fe_nodeset, FE_field *target_field);

int FE_element_field_info_check_field_node_value_labels(
	struct FE_element_field_info *element_field_info, FE_field *field,
	struct FE_region *target_fe_region)
{
	if (element_field_info && field)
	{
		if (field->fe_field_type != GENERAL_FE_FIELD)
			return 1; // only GENERAL field has components array
		FE_element_field *element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
			field, element_field_info->element_field_list);
		if (!element_field)
			return 1; // field not defined so can ignore
		bool needLabels = false;
		// run loops to end to check have valid standard node maps
		for (int i = 0; i < field->number_of_components; ++i)
		{
			FE_element_field_component *component = element_field->components[i];
			if (component->type != STANDARD_NODE_TO_ELEMENT_MAP)
				continue;
			const int nodeCount = component->map.standard_node_based.number_of_nodes;
			for (int n = 0; n < nodeCount; ++n)
			{
				Standard_node_to_element_map *map = component->map.standard_node_based.node_to_element_maps[n];
				if (!map)
					return 0;
				if (map->nodal_value_indices)
					needLabels = true;
			}
		}
		if (needLabels)
		{
			// get node value types and versions from first element using this element_field
			// and check all other elements are using it with the same value types and versions.
			// following is expensive, but necessary to ensure node value types consistently used
			FE_region *fe_region = field->info->fe_region;
			int minDimension = 1;
			bool success = true;
			FE_nodeset *target_fe_nodeset = 0;
			FE_field *target_field = 0;
			if (target_fe_region)
			{
				target_fe_nodeset = FE_region_find_FE_nodeset_by_field_domain_type(
					target_fe_region, CMZN_FIELD_DOMAIN_TYPE_NODES);
				target_field = FE_region_get_FE_field_from_name(target_fe_region, field->name);
			}
			for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (minDimension <= dimension) && success; --dimension)
			{
				FE_mesh *fe_mesh = FE_region_find_FE_mesh_by_dimension(fe_region, dimension);
				cmzn_elementiterator *elemIter = fe_mesh->createElementiterator();
				cmzn_element *element;
				while ((element = cmzn_elementiterator_next_non_access(elemIter)) && success)
				{
					// note element_field can be used in multiple element_field_info
					if ((element->fields == element_field_info) || (element_field ==
						FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(field, element->fields->element_field_list)))
					{
						minDimension = dimension; // don't expect element_field is used for multiple dimensions
						for (int i = 0; (i < field->number_of_components) && success; ++i)
						{
							FE_element_field_component *component = element_field->components[i];
							if (component->type != STANDARD_NODE_TO_ELEMENT_MAP)
								continue;
							const int nodeCount = component->map.standard_node_based.number_of_nodes;
							for (int n = 0; n < nodeCount; ++n)
							{
								Standard_node_to_element_map *map = component->map.standard_node_based.node_to_element_maps[n];
								if (!Standard_node_to_element_map_determine_or_check_node_value_labels(
									map, field, i, element, target_fe_nodeset, target_field))
								{
									success = false;
									break;
								}
							}
						}
					}
				}
				cmzn_elementiterator_destroy(&elemIter);
			}
			if (!success)
			{
				// ensure value_type and version arrays in maps are cleared since not consistent
				for (int i = 0; i < field->number_of_components; ++i)
				{
					FE_element_field_component *component = element_field->components[i];
					if (component->type != STANDARD_NODE_TO_ELEMENT_MAP)
						continue;
					const int nodeCount = component->map.standard_node_based.number_of_nodes;
					for (int n = 0; n < nodeCount; ++n)
					{
						Standard_node_to_element_map *map = component->map.standard_node_based.node_to_element_maps[n];
						Standard_node_to_element_map_clear_node_value_labels(map);
					}
				}
				return 0;
			}
		}
		return 1;
	}
	return 0;
}

int FE_field_has_multiple_times(struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns true if any node_fields corresponding to <field> have time_sequences.
This will be improved when regionalised, so that hopefully the node field
list we will be looking at will not be global but will belong to the region.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_has_multiple_times);
	if (fe_field)
	{
		return_code = FE_region_FE_field_has_multiple_times(
			FE_field_get_FE_region(fe_field), fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_has_multiple_times.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_has_multiple_times */

static int FE_element_field_uses_non_linear_basis(
	struct FE_element_field *element_field)
{
	if (element_field)
	{
		int i;
		for (i = 0; i < element_field->field->number_of_components; i++)
		{
			if (FE_basis_is_non_linear(element_field->components[i]->basis))
			{
				return 1;
			}
		}
	}
	return 0;
}

static int FE_element_field_info_FE_field_uses_non_linear_basis(
	struct FE_element_field_info *field_info, void *fe_field_void)
{
	struct FE_field *fe_field;
	fe_field = (struct FE_field *)fe_field_void;
	if (field_info && fe_field)
	{
		struct FE_element_field *element_field;
		element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
			fe_field, field_info->element_field_list);
		if (element_field && FE_element_field_uses_non_linear_basis(element_field))
		{
			return 1;
		}
	}
	return 0;
}

int FE_field_uses_non_linear_basis(struct FE_field *fe_field)
{
	if (fe_field && fe_field->info && fe_field->info->fe_region)
	{
		for (int dimension = 1; dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dimension)
		{
			FE_mesh *fe_mesh = FE_region_find_FE_mesh_by_dimension(fe_field->info->fe_region, dimension);
			if (FIRST_OBJECT_IN_LIST_THAT(FE_element_field_info)(
				FE_element_field_info_FE_field_uses_non_linear_basis, (void *)fe_field,
				fe_mesh->get_FE_element_field_info_list_private()))
			{
				return 1;
			}
		}
	}
	return 0;
}

int ensure_FE_field_is_in_list(struct FE_field *field, void *field_list_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Iterator function for adding <field> to <field_list> if not currently in it.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_field) *field_list;

	ENTER(ensure_FE_field_is_in_list);
	if (field&&(field_list=(struct LIST(FE_field) *)field_list_void))
	{
		if (!IS_OBJECT_IN_LIST(FE_field)(field,field_list))
		{
			return_code=ADD_OBJECT_TO_LIST(FE_field)(field,field_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_FE_field_is_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* ensure_FE_field_is_in_list */

struct FE_region *FE_field_get_FE_region(struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns the FE_region that <fe_field> belongs to.
==============================================================================*/
{
	struct FE_region *fe_region;

	ENTER(FE_field_get_FE_region);
	if (fe_field && fe_field->info)
	{
		fe_region = fe_field->info->fe_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_get_FE_region.  Invalid argument(s)");
		fe_region = (struct FE_region *)NULL;
	}
	LEAVE;

	return (fe_region);
} /* FE_field_get_FE_region */

int FE_field_add_wrapper(struct FE_field *field)
{
	if (field)
		return (++(field->number_of_wrappers));
	return 0;
}

int FE_field_remove_wrapper(struct FE_field *field)
{
	if (field)
		return (--(field->number_of_wrappers));
	return 0;
}

int FE_field_set_FE_field_info(struct FE_field *fe_field,
	struct FE_field_info *fe_field_info)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Changes the FE_field_info at <fe_field> to <fe_field_info>.
Private function only to be called by FE_region when merging FE_regions!
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_set_FE_field_info);
	if (fe_field && fe_field_info)
	{
		return_code =
			REACCESS(FE_field_info)(&(fe_field->info), fe_field_info);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_set_FE_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_set_FE_field_info */

int FE_field_get_access_count(struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 11 March 2003

DESCRIPTION :
Returns the FE_region that <fe_field> belongs to.
==============================================================================*/
{
	int access_count;

	ENTER(FE_field_get_access_count);
	if (fe_field)
	{
		access_count = fe_field->access_count;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_get_access_count.  Invalid argument(s)");
		access_count = 0;
	}
	LEAVE;

	return (access_count);
} /* FE_field_get_access_count */

int FE_node_get_access_count(struct FE_node *fe_node)
{
	if (fe_node)
	{
		return fe_node->access_count;
	}
	return 0;
}

int FE_element_get_access_count(struct FE_element *fe_element)
{
	if (fe_element)
	{
		return fe_element->access_count;
	}
	return 0;
}

char *get_FE_field_component_name(struct FE_field *field,int component_no)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns the name of component <component_no> of <field>. If no name is stored
for the component, a string comprising the value component_no+1 is returned.
Up to calling function to DEALLOCATE the returned string.
==============================================================================*/
{
	char *component_name;

	ENTER(get_FE_field_component_name);
	if (field&&(0<=component_no)&&(component_no<field->number_of_components))
	{
		component_name=
			get_automatic_component_name(field->component_names,component_no);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_component_name.  Invalid argument(s)");
		component_name=(char *)NULL;
	}
	LEAVE;

	return (component_name);
} /* get_FE_field_component_name */

int set_FE_field_component_name(struct FE_field *field,int component_no,
	const char *component_name)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Sets the name of component <component_no> of <field>. Only sets name if it is
different from that already returned for field to preserve default names if can.
==============================================================================*/
{
	char *temp_component_name;
	int different_name,i,return_code;

	ENTER(set_FE_field_component_name);
	if (field&&(0<=component_no)&&(component_no<field->number_of_components)&&
		component_name)
	{
		if (NULL != (temp_component_name=get_FE_field_component_name(field,component_no)))
		{
			different_name=strcmp(temp_component_name,component_name);
			DEALLOCATE(temp_component_name);
		}
		else
		{
			different_name=1;
		}
		if (different_name)
		{
			if (ALLOCATE(temp_component_name,char,strlen(component_name)+1))
			{
				strcpy(temp_component_name,component_name);
				/* component_names array may be non-existent if default names used */
				if (field->component_names)
				{
					if (field->component_names[component_no])
					{
						DEALLOCATE(field->component_names[component_no]);
					}
				}
				else
				{
					if (ALLOCATE(field->component_names,char *,
						field->number_of_components))
					{
						/* clear the pointers to names */
						for (i=0;i<field->number_of_components;i++)
						{
							field->component_names[i]=(char *)NULL;
						}
					}
				}
				if (field->component_names)
				{
					field->component_names[component_no]=temp_component_name;
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"set_FE_field_component_name.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_component_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_component_name */

struct Coordinate_system *get_FE_field_coordinate_system(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Returns a pointer to the coordinate system for the <field>.
???RC Should not be returning pointer to internal structure; change to
fill struct Coordinate_system at address passed to this function.
==============================================================================*/
{
	struct Coordinate_system *coordinate_system;

	ENTER(get_FE_field_coordinate_system);
	if (field)
	{
		coordinate_system = &field->coordinate_system;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_coordinate_system.  Invalid field");
		coordinate_system = (struct Coordinate_system *)NULL;
	}
	LEAVE;

	return (coordinate_system);
} /* get_FE_field_coordinate_system */

int set_FE_field_coordinate_system(struct FE_field *field,
	struct Coordinate_system *coordinate_system)
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Sets the coordinate system of the <field>.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_coordinate_system);
	if (field&&coordinate_system)
	{
		return_code=
			COPY(Coordinate_system)(&field->coordinate_system,coordinate_system);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_coordinate_system.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_coordinate_system */

int get_FE_field_number_of_components(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Returns the number of components for the <field>.
==============================================================================*/
{
	int number_of_components;

	ENTER(get_FE_field_number_of_components);
	if (field)
	{
		number_of_components=field->number_of_components;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_number_of_components.  Missing field");
		number_of_components=0;
	}
	LEAVE;

	return (number_of_components);
} /* get_FE_field_number_of_components */

int set_FE_field_number_of_components(struct FE_field *field,
	int number_of_components)
/*******************************************************************************
LAST MODIFIED : 16 January 2002

DESCRIPTION :
Sets the number of components in the <field>. Automatically assumes names for
any new components. Clears/reallocates the values_storage for FE_field_types
that use them, eg. CONSTANT_FE_FIELD and INDEXED_FE_FIELD - but only if number
of components changes. If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
ELEMENT_XI_VALUE, STRING_VALUE and URL_VALUE fields may only have 1 component.
==============================================================================*/
{
	char **component_names;
	int i,number_of_values,return_code;
	Value_storage *values_storage;

	ENTER(set_FE_field_number_of_components);
	if (field && (0<number_of_components) && ((1 == number_of_components) ||
		((field->value_type != ELEMENT_XI_VALUE) && (field->value_type != STRING_VALUE) &&
			(field->value_type != URL_VALUE))))
	{
		return_code=1;
		if (number_of_components != field->number_of_components)
		{
			/* 1. make dynamic allocations for number_of_components-specific data */
			component_names=(char **)NULL;
			if (field->component_names)
			{
				if (ALLOCATE(component_names,char *,number_of_components))
				{
					/* copy the old names, clear any new ones */
					for (i=0;i<number_of_components;i++)
					{
						if (i<field->number_of_components)
						{
							component_names[i]=field->component_names[i];
						}
						else
						{
							component_names[i]=(char *)NULL;
						}
					}
				}
				else
				{
					return_code=0;
				}
			}
			values_storage=(Value_storage *)NULL;
			number_of_values=0;
			switch (field->fe_field_type)
			{
				case CONSTANT_FE_FIELD:
				{
					number_of_values=number_of_components;
				} break;
				case GENERAL_FE_FIELD:
				{
					number_of_values=0;
				} break;
				case INDEXED_FE_FIELD:
				{
					number_of_values=field->number_of_indexed_values*number_of_components;
				} break;
				default:
				{
					return_code=0;
				} break;
			}
			if (number_of_values != field->number_of_values)
			{
				if (!(values_storage=make_value_storage_array(field->value_type,
					(struct FE_time_sequence *)NULL,number_of_values)))
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				/* 2. free current number_of_components-specific data */
				if (field->component_names)
				{
					/* free component_names no longer used */
					for (i = number_of_components; i < field->number_of_components; i++)
					{
						if (field->component_names[i])
						{
							DEALLOCATE(field->component_names[i]);
						}
					}
					DEALLOCATE(field->component_names);
				}
				if (field->values_storage)
				{
					free_value_storage_array(field->values_storage,field->value_type,
						(struct FE_time_sequence *)NULL,field->number_of_values);
					DEALLOCATE(field->values_storage);
				}
				/* 3. establish the new number_of_components and associated data */
				field->number_of_components=number_of_components;
				field->component_names=component_names;
				field->values_storage=values_storage;
				field->number_of_values=number_of_values;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_FE_field_number_of_components.  Not enough memory");
				DEALLOCATE(component_names);
				DEALLOCATE(values_storage);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_number_of_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_number_of_components */

int get_FE_field_number_of_values(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Returns the number of global values for the <field>.
==============================================================================*/
{
	int number_of_values;

	ENTER(get_FE_field_number_of_values);
	if (field)
	{
		number_of_values=field->number_of_values;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_number_of_values.  Invalid field");
		number_of_values=0;
	}
	LEAVE;

	return (number_of_values);
} /* get_FE_field_number_of_values */

int get_FE_field_number_of_times(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Returns the number of global times for the <field>.
==============================================================================*/
{
	int number_of_times;

	ENTER(get_FE_field_number_of_times);
	if (field)
	{
		number_of_times=field->number_of_times;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_number_of_times.  Invalid field");
		number_of_times=0;
	}
	LEAVE;

	return (number_of_times);
} /* get_FE_field_number_of_times */

int set_FE_field_number_of_times(struct FE_field *field,
	int number_of_times)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Sets the number of times stored with the <field>
REALLOCATES the requires memory in field->value_storage, based upon the
field->time_value_type.

For non-array types, the contents of field->times_storage is:
   | data type (eg FE_value) | x number_of_times

For array types, the contents of field->times is:
   ( | int (number of array values) | pointer to array (eg double *) | x number_of_times )

Sets data in this memory to 0, pointers to NULL.

MUST have called set_FE_field_time_value_type before calling this function.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int i,j, return_code,size;
	Value_storage *new_value, *times;

	ENTER(set_FE_field_number_of_times);
	if (field&&(0<=number_of_times))
	{
		return_code=1;
		if (number_of_times != 0)
		{
			field->number_of_times=number_of_times;
			size = get_Value_storage_size(field->time_value_type,
				(struct FE_time_sequence *)NULL);

			if (REALLOCATE(times,field->times,Value_storage,
				size*number_of_times))
			{
				field->times = times;
				for (i=0;i<number_of_times;i++)
				{
					switch (field->time_value_type)
					{
						/* set values to zero*/
						case DOUBLE_VALUE:
						{
							*((double *)times) = 0;
						}	break;
						case ELEMENT_XI_VALUE:
						{
							new_value = times;
							*((struct FE_element **)new_value) = (struct FE_element *)NULL;
							new_value+=sizeof(struct FE_element *);
							for (j = 0 ; j < MAXIMUM_ELEMENT_XI_DIMENSIONS ; j++)
							{
								*((FE_value *)new_value) = 0;
								new_value+=sizeof(FE_value);
							}
						}	break;
						case FE_VALUE_VALUE:
						{
							*((FE_value *)times) = 0;
						}	break;
						case FLT_VALUE:
						{
							*((float *)times) = 0;
						} break;
						case SHORT_VALUE:
						{
							display_message(ERROR_MESSAGE," set_FE_field_number_of_times."
								"SHORT_VALUE. Code not written yet. Beware alignment problems ");
							return_code =0;
						} break;
						case INT_VALUE:
						{
							*((int *)times) = 0;
						}	break;
						case UNSIGNED_VALUE:
						{
							*((unsigned *)times) = 0;
						}	break;
						/* set number of array values to 0, array pointers to NULL*/
						case DOUBLE_ARRAY_VALUE:
						{
							double **array_address;
							/* copy the number of array values (0!) to times*/
							*((int *)times) = 0;
							/* copy the pointer to the array values (currently NULL), to times*/
							array_address = (double **)(times+sizeof(int));
							*array_address = (double *)NULL;
						} break;
						case FE_VALUE_ARRAY_VALUE:
						{
							FE_value **array_address;
							*((int *)times) = 0;
							array_address = (FE_value **)(times+sizeof(int));
							*array_address = (FE_value *)NULL;
						} break;
						case FLT_ARRAY_VALUE:
						{
							float **array_address;
							*((int *)times) = 0;
							array_address = (float **)(times+sizeof(int));
							*array_address = (float *)NULL;
						} break;
						case SHORT_ARRAY_VALUE:
						{
							short **array_address;
							*((int *)times) = 0;
							array_address = (short **)(times+sizeof(int));
							*array_address = (short *)NULL;
						} break;
						case INT_ARRAY_VALUE:
						{
							int **array_address;
							*((int *)times) = 0;
							array_address = (int **)(times+sizeof(int));
							*array_address = (int *)NULL;
						} break;
						case UNSIGNED_ARRAY_VALUE:
						{
							unsigned **array_address;
							*((int *)times) = 0;
							array_address = (unsigned **)(times+sizeof(int));
							*array_address = (unsigned *)NULL;
						} break;
						case STRING_VALUE:
						{
							char **str_address;
							str_address = (char **)(times);
							*str_address = (char *)NULL;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE," set_FE_field_number_of_times."
								"  Unsupported value_type");
							return_code =0;
						} break;
					}	/*	switch (field->time_value_type) */
					times += size;
				}/* (i=0;i<number_of_times;i++) */
			}/* if (REALLOCATE */
			else
			{
				display_message(ERROR_MESSAGE,"set_FE_field_number_of_times."
					" Not enough memory");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_number_of_times.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_number_of_times */

enum CM_field_type get_FE_field_CM_field_type(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 30 August 2001

DESCRIPTION :
Returns the CM_field_type for the <field>.
==============================================================================*/
{
	enum CM_field_type type;

	ENTER(get_FE_field_CM_field_type);
	if (field)
	{
		type=field->cm_field_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_CM_field_type.  Invalid field");
		type=CM_GENERAL_FIELD;
	}
	LEAVE;

	return (type);
} /* get_FE_field_CM_field_type */

int set_FE_field_CM_field_type(struct FE_field *field,
	enum CM_field_type cm_field_type)
/*******************************************************************************
LAST MODIFIED : 30 August 2001

DESCRIPTION :
Sets the CM_field_type of the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_CM_field_type);
	if (field)
	{
		field->cm_field_type=cm_field_type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_CM_field_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_CM_field_type */

enum FE_field_type get_FE_field_FE_field_type(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns the FE_field_type for the <field>.
==============================================================================*/
{
	enum FE_field_type fe_field_type;

	ENTER(get_FE_field_FE_field_type);
	if (field)
	{
		fe_field_type=field->fe_field_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_FE_field_type.  Invalid field");
		fe_field_type=UNKNOWN_FE_FIELD;
	}
	LEAVE;

	return (fe_field_type);
} /* get_FE_field_FE_field_type */

int set_FE_field_type_constant(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type CONSTANT_FE_FIELD.
Allocates and clears the values_storage of the field to fit
field->number_of_components of the current value_type.
If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int number_of_values,return_code;
	Value_storage *values_storage;

	ENTER(set_FE_field_type_constant);
	return_code=0;
	if (field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_values=field->number_of_components;
		if (NULL != (values_storage=make_value_storage_array(field->value_type,
			(struct FE_time_sequence *)NULL,number_of_values)))
		{
			/* 2. free current type-specific data */
			if (field->values_storage)
			{
				free_value_storage_array(field->values_storage,field->value_type,
					(struct FE_time_sequence *)NULL,field->number_of_values);
				DEALLOCATE(field->values_storage);
			}
			REACCESS(FE_field)(&(field->indexer_field),NULL);
			field->number_of_indexed_values=0;
			/* 3. establish the new type */
			field->fe_field_type=CONSTANT_FE_FIELD;
			field->values_storage=values_storage;
			field->number_of_values=number_of_values;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_field_type_constant.  Could not allocate values_storage");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_type_constant.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_type_constant */

int set_FE_field_type_general(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type GENERAL_FE_FIELD.
Frees any values_storage currently in use by the field.
If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_type_general);
	return_code=0;
	if (field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		/* none */
		/* 2. free current type-specific data */
		if (field->values_storage)
		{
			free_value_storage_array(field->values_storage,field->value_type,
				(struct FE_time_sequence *)NULL,field->number_of_values);
			DEALLOCATE(field->values_storage);
		}
		REACCESS(FE_field)(&(field->indexer_field),NULL);
		field->number_of_indexed_values=0;
		/* 3. establish the new type */
		field->fe_field_type=GENERAL_FE_FIELD;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_type_general.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_type_general */

int get_FE_field_type_indexed(struct FE_field *field,
	struct FE_field **indexer_field,int *number_of_indexed_values)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
If the field is of type INDEXED_FE_FIELD, the indexer_field and
number_of_indexed_values it uses are returned - otherwise an error is reported.
Use function get_FE_field_FE_field_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_field_type_indexed);
	if (field&&(INDEXED_FE_FIELD==field->fe_field_type)&&indexer_field&&
		number_of_indexed_values)
	{
		*indexer_field=field->indexer_field;
		*number_of_indexed_values=field->number_of_indexed_values;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_type_indexed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_type_indexed */

int set_FE_field_type_indexed(struct FE_field *field,
	struct FE_field *indexer_field,int number_of_indexed_values)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Converts the <field> to type INDEXED_FE_FIELD, indexed by the given
<indexer_field> and with the given <number_of_indexed_values>. The indexer_field
must return a single integer value to be valid.
Allocates and clears the values_storage of the field to fit
field->number_of_components x number_of_indexed_values of the current
value_type. If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int number_of_values,return_code;
	Value_storage *values_storage;

	ENTER(set_FE_field_type_indexed);
	return_code=0;
	if (field&&indexer_field&&(0<number_of_indexed_values)&&
		(1==get_FE_field_number_of_components(indexer_field))&&
		(INT_VALUE==get_FE_field_value_type(indexer_field))&&
		/* and to avoid possible endless loops... */
		(INDEXED_FE_FIELD != get_FE_field_FE_field_type(indexer_field)))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_values=field->number_of_components*number_of_indexed_values;
		if (NULL != (values_storage=make_value_storage_array(field->value_type,
			(struct FE_time_sequence *)NULL,number_of_values)))
		{
			/* 2. free current type-specific data */
			if (field->values_storage)
			{
				free_value_storage_array(field->values_storage,field->value_type,
					(struct FE_time_sequence *)NULL,field->number_of_values);
				DEALLOCATE(field->values_storage);
			}
			/* 3. establish the new type */
			field->fe_field_type=INDEXED_FE_FIELD;
			REACCESS(FE_field)(&(field->indexer_field),indexer_field);
			field->number_of_indexed_values=number_of_indexed_values;
			field->values_storage=values_storage;
			field->number_of_values=number_of_values;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_field_type_indexed.  Could not allocate values_storage");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_type_indexed.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_type_indexed */

int FE_field_set_indexer_field(struct FE_field *fe_field,
	struct FE_field *indexer_fe_field)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
If <fe_field> is already indexed, substitutes <indexer_fe_field>.
Does not change any of the values currently stored in <fe_field>
Used to merge indexed fields into different FE_regions; should not be used for
any other purpose.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_set_indexer_field);
	return_code = 0;
	if (fe_field && (INDEXED_FE_FIELD == fe_field->fe_field_type) &&
		indexer_fe_field &&
		(1 == get_FE_field_number_of_components(indexer_fe_field)) &&
		(INT_VALUE == get_FE_field_value_type(indexer_fe_field)) &&
		/* and to avoid possible endless loops... */
		(INDEXED_FE_FIELD != get_FE_field_FE_field_type(indexer_fe_field)))
	{
		REACCESS(FE_field)(&(fe_field->indexer_field), indexer_fe_field);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_set_indexer_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_field_set_indexer_field */

int FE_field_log_FE_field_change(struct FE_field *fe_field,
	void *fe_field_change_log_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION :
Logs the field in <fe_field> as RELATED_OBJECT_CHANGED in the
struct CHANGE_LOG(FE_field) pointed to by <fe_field_change_log_void>.
???RC Later may wish to allow more than just RELATED_OBJECT_CHANGED, or have
separate functions for each type.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_log_FE_field_change);
	/*???RC try to make this as efficient as possible so no argument checking */
	return_code = CHANGE_LOG_OBJECT_CHANGE(FE_field)(
		(struct CHANGE_LOG(FE_field) *)fe_field_change_log_void,
		fe_field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
	LEAVE;

	return (return_code);
} /* FE_field_log_FE_field_change */

enum Value_type get_FE_field_value_type(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns the value_type of the <field>.
==============================================================================*/
{
	enum Value_type value_type;

	ENTER(get_FE_field_value_type);
	if (field)
	{
		value_type=field->value_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_value_type.  Invalid field");
		value_type=UNKNOWN_VALUE;
	}
	LEAVE;

	return (value_type);
} /* get_FE_field_value_type */

int set_FE_field_value_type(struct FE_field *field,enum Value_type value_type)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Sets the value_type of the <field>. Clears/reallocates the values_storage for
FE_field_types that use them, eg. CONSTANT_FE_FIELD and INDEXED_FE_FIELD - but
only if the value_type changes. If function fails the field is left exactly as
it was. Should only call this function for unmanaged fields.
ELEMENT_XI_VALUE, STRING_VALUE and URL_VALUE fields may only have 1 component.
==============================================================================*/
{
	int number_of_values,return_code;
	Value_storage *values_storage;

	ENTER(set_FE_field_value_type);
	if (field && ((1 >= field->number_of_components) ||
		((value_type != ELEMENT_XI_VALUE) && (value_type != STRING_VALUE) &&
			(value_type != URL_VALUE))))
	{
		return_code=1;
		if (value_type != field->value_type)
		{
			/* 1. make dynamic allocations for value_type-specific data */
			values_storage=(Value_storage *)NULL;
			number_of_values=field->number_of_values;
			if (0!=number_of_values)
			{
				if (!(values_storage=make_value_storage_array(value_type,
					(struct FE_time_sequence *)NULL,number_of_values)))
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				/* 2. free current value_type-specific data */
				if (field->values_storage)
				{
					free_value_storage_array(field->values_storage,field->value_type,
						(struct FE_time_sequence *)NULL,field->number_of_values);
					DEALLOCATE(field->values_storage);
				}
				/* 3. establish the new value_type and associated data */
				field->value_type=value_type;
				field->values_storage=values_storage;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_FE_field_value_type.  Not enough memory");
				DEALLOCATE(values_storage);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_value_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}/* set_FE_field_value_type */

int FE_field_get_element_xi_mesh_dimension(struct FE_field *field)
{
	int element_xi_mesh_dimension;
	if (field && (field->value_type == ELEMENT_XI_VALUE))
	{
		element_xi_mesh_dimension = field->element_xi_mesh_dimension;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_value_type.  Invalid argument(s)");
		element_xi_mesh_dimension = 0;
	}
	return element_xi_mesh_dimension;
}

int FE_field_set_element_xi_mesh_dimension(struct FE_field *field,
	int mesh_dimension)
{
	int return_code;
	if (field && (field->value_type == ELEMENT_XI_VALUE) &&
		(0 <= mesh_dimension) && (mesh_dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		field->element_xi_mesh_dimension = mesh_dimension;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_value_type.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

struct FE_node_field_info_get_highest_node_derivative_and_version_data
{
	FE_field *field;
	int highest_derivative;
	int highest_version;
};

int FE_node_field_info_get_highest_node_derivative_and_version(FE_node_field_info *node_field_info, void *data_void)
{
	FE_node_field_info_get_highest_node_derivative_and_version_data *data =
		static_cast<FE_node_field_info_get_highest_node_derivative_and_version_data*>(data_void);
	FE_node_field *node_field =
		FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(data->field, node_field_info->node_field_list);
	if (node_field)
	{
		for (int c = 0; c < data->field->number_of_components; ++c)
		{
			FE_node_field_component *node_field_component = node_field->components + c;
			if (!node_field_component->nodal_value_types)
			{
				display_message(ERROR_MESSAGE, "FE_node_field_info_get_highest_node_derivative_version.  Missing nodal value types");
				return 0;
			}
			int number_of_values = node_field_component->number_of_derivatives + 1;
			for (int v = 0; v < number_of_values; ++v)
			{
				int derivative = (node_field_component->nodal_value_types[v] - FE_NODAL_VALUE) + 1;
				if (derivative > data->highest_derivative)
					data->highest_derivative = derivative;
			}
			if (node_field_component->number_of_versions > data->highest_version)
				data->highest_version = node_field_component->number_of_versions;
		}
	}
	return 1;
}

int FE_field_get_highest_node_derivative_and_version(FE_field *field,
	int& highest_derivative, int& highest_version)
{
	FE_region *fe_region = FE_field_get_FE_region(field);
	FE_nodeset *fe_nodeset = FE_region_find_FE_nodeset_by_field_domain_type(fe_region, CMZN_FIELD_DOMAIN_TYPE_NODES);
	if (!fe_nodeset)
		return CMZN_ERROR_ARGUMENT;
	LIST(FE_node_field_info) *node_field_info_list = fe_nodeset->get_FE_node_field_info_list_private();
	FE_node_field_info_get_highest_node_derivative_and_version_data data = { field, 0, 0 };
	if (!FOR_EACH_OBJECT_IN_LIST(FE_node_field_info)(
		FE_node_field_info_get_highest_node_derivative_and_version, &data, node_field_info_list))
	{
		return CMZN_ERROR_GENERAL;
	}
	highest_derivative = data.highest_derivative;
	highest_version = data.highest_version;
	return CMZN_OK;
}

enum Value_type get_FE_field_time_value_type(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Returns the time_value_type of the <field>.
==============================================================================*/
{
	enum Value_type time_value_type;

	ENTER(get_FE_field_time_value_type);
	if (field)
	{
		time_value_type=field->time_value_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_time_value_type.  Invalid field");
		time_value_type=UNKNOWN_VALUE;
	}
	LEAVE;

	return (time_value_type);
} /*get_FE_field_time_value_type */

int set_FE_field_time_value_type(struct FE_field *field,enum Value_type time_value_type)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Sets the time_value_type of the <field>.
Should only call this function for unmanaged fields.
=========================================================================*/
{
	int return_code;

	ENTER(set_FE_field_time_value_type);
	if (field)
	{
		field->time_value_type=time_value_type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_field_time_value_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}/*set_FE_field_time_value_type */

int get_FE_field_max_array_size(struct FE_field *field,int *max_number_of_array_values,
	enum Value_type *value_type)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Given the field, search vaules_storage  for the largest array, and return it in
max_number_of_array_values. Return the field value_type.
====================================================================================*/
{
 int return_code,size,i,number_of_array_values;

	Value_storage *values_storage;

	ENTER(get_FE_field_max_array_size);
	if (field)
	{
		if (field->number_of_values)
		{
			return_code=1;
			*value_type = field->value_type;
			switch (field->value_type)
			{
				case DOUBLE_ARRAY_VALUE:
				case FE_VALUE_ARRAY_VALUE:
				case FLT_ARRAY_VALUE:
				case SHORT_ARRAY_VALUE:
				case INT_ARRAY_VALUE:
				case UNSIGNED_ARRAY_VALUE:
				case STRING_VALUE:
				{
					*max_number_of_array_values = 0;
					size = get_Value_storage_size(*value_type,
						(struct FE_time_sequence *)NULL);
					values_storage = field->values_storage;
					for (i=0;i<field->number_of_values;i++)
					{
						if (field->value_type == STRING_VALUE)
						{
							char *the_string,**str_address;
							/* get the string's length*/
							str_address = (char **)(values_storage);
							the_string = *str_address;
							number_of_array_values = static_cast<int>(strlen(the_string)) + 1;/* +1 for null termination*/
							if (number_of_array_values > *max_number_of_array_values)
							{
								*max_number_of_array_values = number_of_array_values;
							}
						}
						else
						{
							/* get the number of array values  for the specified array in vaules_storage */
							number_of_array_values = *((int *)values_storage);
							if (number_of_array_values > *max_number_of_array_values)
							{
								*max_number_of_array_values = number_of_array_values;
							}
						}
						values_storage+=(i*size);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE," get_FE_field_max_array_size. Not an array type)");
					number_of_array_values = 0;
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE," get_FE_field_max_array_size. No values at field");

			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE," get_FE_field_max_array_size. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* get_FE_field_max_array_size */

int get_FE_field_array_attributes(struct FE_field *field, int value_number,
 int *number_of_array_values, enum Value_type *value_type)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Get the value_type and the number of array values for the array in field->values_storage
specified by value_number.
Give an error if field->values_storage isn't storing array types.
====================================================================================*/
{

  int return_code,size;

	Value_storage *values_storage;

	ENTER(get_FE_field_array_attributes);
	if (field&&(0<=value_number)&&(value_number<=field->number_of_values))
	{
		if (field->number_of_values)
		{
			return_code=1;
			*value_type = field->value_type;
			switch (field->value_type)
			{
				case DOUBLE_ARRAY_VALUE:
				case FE_VALUE_ARRAY_VALUE:
				case FLT_ARRAY_VALUE:
				case SHORT_ARRAY_VALUE:
				case INT_ARRAY_VALUE:
				case UNSIGNED_ARRAY_VALUE:
				{
					/* get the correct offset*/
					size = get_Value_storage_size(*value_type,
						(struct FE_time_sequence *)NULL);
					values_storage = field->values_storage+(value_number*size);
					/* get the number of array values  for the specified array in vaules_storage */
					*number_of_array_values = *((int *)values_storage);
				} break;
				case STRING_VALUE:
				{
					char *the_string,**str_address;
					/* get the correct offset*/
					size = get_Value_storage_size(*value_type,
						(struct FE_time_sequence *)NULL);
					values_storage = field->values_storage+(value_number*size);
					/* get the string*/
					str_address = (char **)(values_storage);
					the_string = *str_address;
					*number_of_array_values = static_cast<int>(strlen(the_string)) + 1;/* +1 for null termination*/
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"get_FE_field_array_attributes. Not an array type)");
					number_of_array_values = 0;
					return_code=0;
				} break;
			}
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,"get_FE_field_array_attributes. No values at the field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_FE_field_array_attributes. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* get_FE_field_array_attributes */

int get_FE_field_double_array_value(struct FE_field *field, int value_number,
	double *array, int number_of_array_values)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Get the double array in field->values_storage specified by value_number, of
of length number_of_array_values. If number_of_array_values > the stored arrays
max length, gets the max length.
MUST allocate space for the array before calling this function.

Use get_FE_field_array_attributes() or get_FE_field_max_array_size()
to get the size of an array.
==============================================================================*/
{

	int return_code,size,the_array_number_of_values,array_size;


	Value_storage *values_storage;
	double *the_array,**array_address;

	ENTER(get_FE_field_double_array_value);
	if (field&&array&&(0<=value_number)&&(value_number<=field->number_of_values))
	{
		if (field->number_of_values)
		{
			return_code=1;
			size = get_Value_storage_size(DOUBLE_ARRAY_VALUE,
				(struct FE_time_sequence *)NULL);
			/* get the correct offset*/
			values_storage = field->values_storage+(value_number*size);
			/* get the number of array values  for the specified array in vaules_storage */
			the_array_number_of_values = *((int *)values_storage);
			if (number_of_array_values>the_array_number_of_values)
			{
				number_of_array_values=the_array_number_of_values;
			}
			array_size = number_of_array_values*sizeof(double);

			/* get the address to copy from*/
			array_address = (double **)(values_storage+sizeof(int));
			the_array = *array_address;
			/*copy the data to the passed array */
			memcpy(array,the_array,array_size);
		}
		else
		{
			display_message(ERROR_MESSAGE,"get_FE_field_double_array_value. No values at field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_FE_field_double_array_value. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
}/* get_FE_field_double_array_value */

int set_FE_field_double_array_value(struct FE_field *field, int value_number,
	double *array, int number_of_array_values)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Finds any existing double array at the place specified by  value_number in
field->values_storage.
Frees it.
Allocates a new array, according to number_of_array_values.
Copies the contents of the passed array to this allocated one.
Copies number of array values, and the pointer to the allocated array to the
specified  place in the field->values_storage.

Therefore, should free the passed array, after passing it to this function.

The <field> must be of the correct FE_field_type to have such values and
<number> must be within the range valid for that type.
==============================================================================*/
{

	int return_code,size,array_size;

	double *pointer,*the_array,**array_address;

	Value_storage *values_storage;
	ENTER(set_FE_field_double_array_value);
	if (field&&array&&(0<=value_number)&&(value_number<=field->number_of_values))
	{
		return_code=1;

		if (field->value_type!=DOUBLE_ARRAY_VALUE)
		{
			display_message(ERROR_MESSAGE,"set_FE_field_double_array_value. "
				" value type doesn't match");
			return_code=0;
		}

		size = get_Value_storage_size(DOUBLE_ARRAY_VALUE,
			(struct FE_time_sequence *)NULL);

		/* get the correct offset*/
		values_storage = field->values_storage+(value_number*size);
		/* get the pointer to stored the array, free any existing one */
		array_address = (double **)(values_storage+sizeof(int));
		pointer = *array_address;
		if (pointer!=NULL)
			DEALLOCATE(pointer);
		/* copy the number of array values into field->values_storage*/
		*((int *)values_storage) = number_of_array_values;
		/* Allocate the space for the array, and copy the data in */
		array_size = number_of_array_values*sizeof(double);
		if (ALLOCATE(the_array,double,array_size))
		{
			memcpy(the_array,array,array_size);
			/*copy the pointer to the array into field->values_storage  */
			*array_address = the_array;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_FE_field_double_array_value. Out of Memory )");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_field_double_array_value. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
}/* set_FE_field_double_array_value */

int get_FE_field_string_value(struct FE_field *field,int value_number,
	char **string)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Returns a copy of the string stored at <value_number> in the <field>.
Up to the calling function to DEALLOCATE the returned string.
Returned <*string> may be a valid NULL if that is what is in the field.
==============================================================================*/
{
	int return_code,size;
	char *the_string,**string_address;

	ENTER(get_FE_field_string_value);
	return_code=0;
	if (field&&(0<=value_number)&&(value_number<field->number_of_values)&&string)
	{
		/* get the pointer to the stored string */
		size = get_Value_storage_size(STRING_VALUE,(struct FE_time_sequence *)NULL);
		string_address = (char **)(field->values_storage+value_number*size);
		if (NULL != (the_string = *string_address))
		{
			if (ALLOCATE(*string,char,strlen(the_string)+1))
			{
				strcpy(*string,the_string);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_field_string_value.  Not enough memory");
			}
		}
		else
		{
			/* no string, so successfully return NULL */
			*string = (char *)NULL;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_string_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_string_value */

int set_FE_field_string_value(struct FE_field *field,int value_number,
	char *string)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Copies and sets the <string> stored at <value_number> in the <field>.
<string> may be NULL.
==============================================================================*/
{
	int return_code,size;
	char *the_string,**string_address;

	ENTER(set_FE_field_string_value);
	return_code=0;
	if (field&&(0<=value_number)&&(value_number<field->number_of_values))
	{
		/* get the pointer to the stored string */
		size = get_Value_storage_size(STRING_VALUE,(struct FE_time_sequence *)NULL);
		string_address = (char **)(field->values_storage+value_number*size);
		if (string)
		{
			/* reallocate the string currently there */
			if (REALLOCATE(the_string,*string_address,char,strlen(string)+1))
			{
				strcpy(the_string,string);
				*string_address=the_string;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_FE_field_string_value.  Not enough memory");
			}
		}
		else
		{
			/* NULL string; free the existing string */
			if (*string_address)
			{
				DEALLOCATE(*string_address);
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_string_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_string_value */

int get_FE_field_element_xi_value(struct FE_field *field,int number,
	struct FE_element **element, FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Gets the specified global value for the <field>.
==============================================================================*/
{
	int i, number_of_xi_dimensions, return_code;
	Value_storage *values_storage;

	ENTER(get_FE_field_element_xi_value);
	if (field&&(0<=number)&&(number<field->number_of_values)
		&&(field->value_type==ELEMENT_XI_VALUE))
	{
		if (field->number_of_values)
		{
			return_code=1;

			/* get the correct offset*/
			values_storage = field->values_storage + (number*(sizeof(struct FE_element *) +
				MAXIMUM_ELEMENT_XI_DIMENSIONS * sizeof(FE_value)));

			/* copy the element and xi out */
			*element = *((struct FE_element **)values_storage);
			values_storage += sizeof(struct FE_element *);
			number_of_xi_dimensions = get_FE_element_dimension(*element);
			if (number_of_xi_dimensions <= MAXIMUM_ELEMENT_XI_DIMENSIONS)
			{
				/* Extract the xi values */
				for (i = 0 ; i < number_of_xi_dimensions ; i++)
				{
					xi[i] = *((FE_value *)values_storage);
					values_storage += sizeof(FE_value);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_field_element_xi_value.  Number of xi dimensions of element exceeds maximum");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_field_element_xi_value. no values at field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_element_xi_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}/* get_FE_field_element_xi_value */

int set_FE_field_element_xi_value(struct FE_field *field,int number,
	struct FE_element *element, FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Sets the specified global value for the <field>, to the passed Element and xi.
The <field> must be of the correct FE_field_type to have such values and
<number> must be within the range valid for that type.
==============================================================================*/
{
	int i, number_of_xi_dimensions, return_code;
	Value_storage  *values_storage;

	ENTER(set_FE_field_element_xi_value);
	if (field&&(0<=number)&&(number<=field->number_of_values)
		&&(field->value_type==ELEMENT_XI_VALUE) && element && xi)
	{
		return_code=1;
		number_of_xi_dimensions = get_FE_element_dimension(element);
		if (number_of_xi_dimensions <= MAXIMUM_ELEMENT_XI_DIMENSIONS)
		{

			/* get the correct offset*/
			values_storage = field->values_storage + (number*(sizeof(struct FE_element *) +
				MAXIMUM_ELEMENT_XI_DIMENSIONS * sizeof(FE_value)));

			/* copy the element in ensuring correct accessing */
			REACCESS(FE_element)(((struct FE_element **)values_storage), element);
			values_storage += sizeof(struct Element *);
			/* Write in the xi values */
			for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
			{
				if (i<number_of_xi_dimensions)
				{
					*((FE_value *)values_storage) = xi[i];
				}
				else
				{
					/* set spare xi values to 0 */
					*((FE_value *)values_storage) = 0.0;
				}
				values_storage += sizeof(FE_value);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_field_element_xi_value.  Number of xi dimensions of element exceeds maximum");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE," set_FE_field_element_xi_value. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* set_FE_field_element_xi_value */

int get_FE_field_FE_value_value(struct FE_field *field,int number,
	FE_value *value)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Gets the specified global FE_value <value> from <field>.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_field_FE_value_value);
	if (field&&(FE_VALUE_VALUE==field->value_type)&&field->values_storage&&
		(0<=number)&&(number<=field->number_of_values)&&value)
	{
		*value = *((FE_value *)(field->values_storage+(number*sizeof(FE_value))));
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_FE_value_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_FE_value_value */

int set_FE_field_FE_value_value(struct FE_field *field,int number,
	FE_value value)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Sets the specified global FE_value <value> in <field>.
The <field> must be of type FE_VALUE_VALUE to have such values and
<number> must be within the range from get_FE_field_number_of_values.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_FE_value_value);
	if (field&&(FE_VALUE_VALUE==field->value_type)&&field->values_storage&&
		(0<=number)&&(number<=field->number_of_values))
	{
		*((FE_value *)(field->values_storage+(number*sizeof(FE_value)))) = value;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_FE_value_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_FE_value_value */

int get_FE_field_int_value(struct FE_field *field,int number,int *value)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Gets the specified global int <value> from <field>.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_field_int_value);
	if (field&&(INT_VALUE==field->value_type)&&field->values_storage&&
		(0<=number)&&(number<=field->number_of_values)&&value)
	{
		*value = *((int *)(field->values_storage+(number*sizeof(int))));
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_int_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_int_value */

int set_FE_field_int_value(struct FE_field *field,int number,int value)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Sets the specified global int <value> in <field>.
The <field> must be of type INT_VALUE to have such values and
<number> must be within the range from get_FE_field_number_of_values.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_int_value);
	if (field&&(INT_VALUE==field->value_type)&&field->values_storage&&
		(0<=number)&&(number<=field->number_of_values))
	{
		*((int *)(field->values_storage+(number*sizeof(int)))) = value;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_int_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_int_value */

int get_FE_field_time_FE_value(struct FE_field *field,int number,FE_value *value)
/*******************************************************************************
LAST MODIFIED : 10 June 1999

DESCRIPTION :
Gets the specified global time value for the <field>.
==============================================================================*/
{
	int return_code;
	Value_storage *times;

	ENTER(get_FE_field_time_FE_value);
	if (field&&(0<=number)&&(number<field->number_of_times))
	{
		if (field->number_of_times)
		{
			return_code=1;
			/* get the correct offset*/
			times = field->times+(number*sizeof(FE_value));
			/* copy the value in*/
			*value = *((FE_value *)times);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_field_time_FE_value.  no times at field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_time_FE_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}/*get_FE_field_time_FE_value */

int set_FE_field_time_FE_value(struct FE_field *field,int number,FE_value value)
/*******************************************************************************
LAST MODIFIED : l0 June 1999

DESCRIPTION :
Sets the specified global time value for the <field>, to the passed FE_value
The field value MUST have been previously allocated with set_FE_field_number_of_times
==============================================================================*/
{

	int return_code;
	FE_value *times;

	ENTER( set_FE_field_time_FE_value);
	if (field&&(0<=number)&&(number<=field->number_of_times))
	{
		return_code=1;
		if (field->time_value_type!=FE_VALUE_VALUE)
		{
			display_message(ERROR_MESSAGE," set_FE_field_time_FE_value. "
				" value type doesn't match");
			return_code=0;
		}
		/* get the correct offset*/
		times = (FE_value *)(field->times+(number*sizeof(FE_value)));
		/* copy the value in*/
		*times = value;
	}
	else
	{
		display_message(ERROR_MESSAGE," set_FE_field_time_FE_value. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* set_FE_field_time_FE_value */

const char *get_FE_field_name(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Returns a pointer to the name for the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	const char *name;

	ENTER(get_FE_field_name);
	if (field)
	{
		name = field->name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_name.  Invalid argument(s)");
		name = (const char *)NULL;
	}
	LEAVE;

	return (name);
}/*get_FE_field_name */

int set_FE_field_name(struct FE_field *field, const char *name)
{
	char *temp;
	int return_code;

	ENTER(set_FE_field_name);
	if (field&&name)
	{
		temp = duplicate_string(name);
		if (temp)
		{
			DEALLOCATE(field->name);
			field->name = temp;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_FE_field_name.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_field_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_name */

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_field_component)
/*****************************************************************************
LAST MODIFIED : 2 February 1999

DESCRIPTION :
Returns the FE_field_component component name.
Up to the calling function to deallocate the returned char string.
============================================================================*/
{
	int return_code;

	ENTER(GET_NAME(FE_field_component));
	if (object&&name_ptr)
	{
		if (NULL != (*name_ptr=get_FE_field_component_name(object->field,object->number)))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GET_NAME(FE_field_component).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GET_NAME(FE_field_component) */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(cmzn_element_face_type)
{
	switch (enumerator_value)
	{
		case CMZN_ELEMENT_FACE_TYPE_ALL:
			return "all";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI1_0:
			return "xi1_0";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI1_1:
			return "xi1_1";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI2_0:
			return "xi2_0";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI2_1:
			return "xi2_1";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI3_0:
			return "xi3_0";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI3_1:
			return "xi3_1";
			break;
		case CMZN_ELEMENT_FACE_TYPE_INVALID:
			break;
	}
	return 0;
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(cmzn_element_point_sampling_mode)
{
	switch (enumerator_value)
	{
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CENTRES:
			return "cell_centres";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS:
			return "cell_corners";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_POISSON:
			return "cell_poisson";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION:
			return "set_location";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_GAUSSIAN_QUADRATURE:
			return "gaussian_quadrature";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_INVALID:
			break;
	}
	return 0;
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(cmzn_element_point_sampling_mode)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(cmzn_element_quadrature_rule)
{
	switch (enumerator_value)
	{
		case CMZN_ELEMENT_QUADRATURE_RULE_GAUSSIAN:
			return "gaussian_quadrature";
			break;
		case CMZN_ELEMENT_QUADRATURE_RULE_MIDPOINT:
			return "midpoint_quadrature";
			break;
		case CMZN_ELEMENT_QUADRATURE_RULE_INVALID:
			break;
	}
	return 0;
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(cmzn_element_quadrature_rule)

/** Important: check other enumerator functions work when adding new values.
 * They assume enums are powers of 2 */
PROTOTYPE_ENUMERATOR_STRING_FUNCTION(cmzn_field_domain_type)
{
	switch (enumerator_value)
	{
	case CMZN_FIELD_DOMAIN_TYPE_POINT:
		return "domain_point";
		break;
	case CMZN_FIELD_DOMAIN_TYPE_NODES:
		return "domain_nodes";
		break;
	case CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS:
		return "domain_datapoints";
		break;
	case CMZN_FIELD_DOMAIN_TYPE_MESH1D:
		return "domain_mesh1d";
		break;
	case CMZN_FIELD_DOMAIN_TYPE_MESH2D:
		return "domain_mesh2d";
		break;
	case CMZN_FIELD_DOMAIN_TYPE_MESH3D:
		return "domain_mesh3d";
		break;
	case CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION:
		return "domain_mesh_highest_dimension";
		break;
	case CMZN_FIELD_DOMAIN_TYPE_INVALID:
		break;
	}
	return 0;
}


/** Note: assumes valid enums are powers of 2, starting at 1 */
int STRING_TO_ENUMERATOR(cmzn_field_domain_type)(const char *enumerator_string,
	enum cmzn_field_domain_type *enumerator_value_address)
{
	if (enumerator_string && enumerator_value_address)
	{
		enum cmzn_field_domain_type value = static_cast<cmzn_field_domain_type>(1);
		const char *valid_string;
		while (0 != (valid_string = ENUMERATOR_STRING(cmzn_field_domain_type)(value)))
		{
			if (fuzzy_string_compare_same_length(enumerator_string, valid_string))
			{
				*enumerator_value_address = value;
				return 1;
			}
			value = static_cast<cmzn_field_domain_type>(2*value);
		}
	}
	return 0;
}


/** Note: assumes valid enums are powers of 2, starting at 1 */
const char **ENUMERATOR_GET_VALID_STRINGS(cmzn_field_domain_type)(
	int *number_of_valid_strings,
	ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_field_domain_type) conditional_function,
	void *user_data)
{
	*number_of_valid_strings = 0;
	const char **valid_strings;

	ALLOCATE(valid_strings, const char *, 64); // bits in a 64-bit integer, to be safe
	enum cmzn_field_domain_type value = static_cast<cmzn_field_domain_type>(1);
	const char *valid_string;
	while (0 != (valid_string = ENUMERATOR_STRING(cmzn_field_domain_type)(value)))
	{
		if ((0 == conditional_function) || (conditional_function(value, user_data)))
		{
			valid_strings[*number_of_valid_strings] = valid_string;
			++(*number_of_valid_strings);
		}
		value = static_cast<cmzn_field_domain_type>(2*value);
	}
	return valid_strings;
}

struct FE_node_field_creator *CREATE(FE_node_field_creator)(
	int number_of_components)
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
An object for defining the components, number_of_versions,
number_of_derivatives and their types at a node.
By default each component has 1 version and no derivatives.
==============================================================================*/
{
	int i;
	struct FE_node_field_creator *node_field_creator;

	ENTER(CREATE(FE_node_field_creator));
	if (number_of_components)
	{
		if ((ALLOCATE(node_field_creator, struct FE_node_field_creator, 1))&&
			(ALLOCATE(node_field_creator->numbers_of_versions, int,
			number_of_components)) &&
			(ALLOCATE(node_field_creator->numbers_of_derivatives, int,
			number_of_components))&&
			(ALLOCATE(node_field_creator->nodal_value_types,
			enum FE_nodal_value_type *, number_of_components)))
		{
			node_field_creator->number_of_components = number_of_components;
			for (i = 0 ; node_field_creator && (i < number_of_components) ; i++)
			{
				node_field_creator->numbers_of_versions[i] = 1;
				node_field_creator->numbers_of_derivatives[i] = 0;
				if (ALLOCATE(node_field_creator->nodal_value_types[i],
					enum FE_nodal_value_type, 1))
				{
					*(node_field_creator->nodal_value_types[i]) = FE_NODAL_VALUE;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(FE_node_field_creator).  Unable to allocate arrays");
					DEALLOCATE(node_field_creator);
					node_field_creator = (struct FE_node_field_creator *)NULL;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_node_field_creator).  Unable to allocate arrays");
			node_field_creator = (struct FE_node_field_creator *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_node_field_creator).  Invalid argument(s)");
		node_field_creator = (struct FE_node_field_creator *)NULL;
	}
	LEAVE;

	return (node_field_creator);
} /* CREATE(FE_node_field_creator) */

struct FE_node_field_creator *create_FE_node_field_creator_from_node_field(
	struct FE_node *node, struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 4 February 2001

DESCRIPTION :
Creates an FE_node_field_creator from <node>,<field>
==============================================================================*/
{
	enum FE_nodal_value_type nodal_value_type;
	int i,j,number_of_components,number_of_derivatives,number_of_versions;
	struct FE_node_field_component *node_field_components;
	struct FE_node_field_creator *node_field_creator;
	struct FE_node_field *node_field;

	ENTER(create_FE_node_field_creator_from_node_field);
	node_field_creator = (struct FE_node_field_creator *)NULL;
	if (node&&field&&node->fields)
	{
		if((node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
			node->fields->node_field_list))&&
			(node_field_components=node_field->components))
		{
			number_of_components=field->number_of_components;
			if (NULL != (node_field_creator=CREATE(FE_node_field_creator)(number_of_components)))
			{
				bool success = true;
				for(i=0;(i<number_of_components)&&success;i++)
				{
					number_of_versions=node_field_components[i].number_of_versions;
					if (CMZN_OK != FE_node_field_creator_define_versions(node_field_creator, i, number_of_versions))
						success = false;
					number_of_derivatives=node_field_components[i].number_of_derivatives;
					for(j=1;(j<=number_of_derivatives)&&success;j++)
					{
						nodal_value_type=node_field_components[i].nodal_value_types[j];
						if (CMZN_OK != FE_node_field_creator_define_derivative(node_field_creator, i, nodal_value_type))
							success = false;
					}
				}
				if (!success)
				{
					DESTROY(FE_node_field_creator)(&node_field_creator);
					display_message(ERROR_MESSAGE, "create_FE_node_field_creator_from_node_field.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_FE_node_field_creator_from_node_field.  Unable to allocate");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_FE_node_field_creator_from_node_field. field not defined at node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_FE_node_field_creator_from_node_field.  Invalid argument(s)");
	}
	LEAVE;

	return (node_field_creator);
} /* create_FE_node_field_creator_from_node_field */

int DESTROY(FE_node_field_creator)(
	struct FE_node_field_creator **node_field_creator_address)
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Frees the memory for the node field creator and sets
<*node_field_creator_address> to NULL.
==============================================================================*/
{
	int i,return_code;
	struct FE_node_field_creator *node_field_creator;

	ENTER(DESTROY(FE_node_field_creator));
	if ((node_field_creator_address)&&(node_field_creator =
		*node_field_creator_address))
	{
		for (i = 0 ; i < node_field_creator->number_of_components ; i++)
		{
			DEALLOCATE(node_field_creator->nodal_value_types[i]);
		}
		DEALLOCATE(node_field_creator->nodal_value_types);
		DEALLOCATE(node_field_creator->numbers_of_derivatives);
		DEALLOCATE(node_field_creator->numbers_of_versions);
		DEALLOCATE(*node_field_creator_address);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_node_field_creator) */

int FE_node_field_creator_define_derivative(
	struct FE_node_field_creator *node_field_creator, int component_number,
	enum FE_nodal_value_type derivative_type)
{
	if (node_field_creator && (component_number >= 0) &&
		(component_number < node_field_creator->number_of_components))
	{
		int number_of_derivatives = node_field_creator->numbers_of_derivatives[component_number];
		FE_nodal_value_type *nodal_value_types = node_field_creator->nodal_value_types[component_number];
		for (int i = 0; i <= number_of_derivatives; ++i)
		{
			if (nodal_value_types[i] == derivative_type)
				return CMZN_ERROR_ALREADY_EXISTS;
		}
		// for compatibility with EX reader, must add new derivative last
		FE_nodal_value_type *new_nodal_value_types;
		if (REALLOCATE(new_nodal_value_types, nodal_value_types, enum FE_nodal_value_type, number_of_derivatives + 2))
		{
			new_nodal_value_types[number_of_derivatives + 1] = derivative_type;
			node_field_creator->nodal_value_types[component_number] = new_nodal_value_types;
			++(node_field_creator->numbers_of_derivatives[component_number]);
			return CMZN_OK;
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

void FE_node_field_creator_undefine_derivative(
	struct FE_node_field_creator *node_field_creator, int component_number,
	enum FE_nodal_value_type derivative_type)
{
	if (node_field_creator && (component_number >= 0) &&
		 (component_number < node_field_creator->number_of_components))
	{
		int number_of_derivatives = node_field_creator->numbers_of_derivatives[component_number];
		enum FE_nodal_value_type *nodal_value_types = node_field_creator->nodal_value_types[component_number];
		// start at 1 so can't underfine FE_NODAL_VALUE
		for (int j = 1; j <= number_of_derivatives; ++j)
		{
			if (nodal_value_types[j] == derivative_type)
			{
				while (j < number_of_derivatives)
				{
					nodal_value_types[j] = nodal_value_types[j + 1];
					++j;
				}
				--(node_field_creator->numbers_of_derivatives[component_number]);
				return;
			}
		}
	}
}

int FE_node_field_creator_get_number_of_derivatives(
	struct FE_node_field_creator *node_field_creator, int component_number)
{
	if (node_field_creator && (0 <= component_number) &&
		(component_number < node_field_creator->number_of_components))
	{
		return node_field_creator->numbers_of_derivatives[component_number];
	}
	return 0;
}

int FE_node_field_creator_define_versions(
	struct FE_node_field_creator *node_field_creator, int component_number,
	int number_of_versions)
{
	if (node_field_creator && (component_number >= 0) &&
		 (component_number < node_field_creator->number_of_components))
	{
		node_field_creator->numbers_of_versions[component_number] =
			number_of_versions;
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int FE_node_field_creator_get_number_of_versions(
	struct FE_node_field_creator *node_field_creator, int component_number)
{
	int number_of_versions = 0;
	if (node_field_creator && (component_number < node_field_creator->number_of_components))
	{
		if (component_number >= 0)
		{
			number_of_versions = node_field_creator->numbers_of_versions[component_number];
		}
		else
		{
			for (int i = 0; i < node_field_creator->number_of_components; ++i)
			{
				if (node_field_creator->numbers_of_versions[i] > number_of_versions)
				{
					number_of_versions = node_field_creator->numbers_of_versions[i];
				}
			}
		}
	}
	return number_of_versions;
}

int FE_node_field_creator_has_derivative(
	struct FE_node_field_creator *node_field_creator, int component_number,
	enum FE_nodal_value_type derivative_type)
{
	if (node_field_creator && (component_number < node_field_creator->number_of_components))
	{
		int start = 0;
		int limit = node_field_creator->number_of_components;
		if (component_number >= 0)
		{
			start = component_number;
			limit = component_number + 1;
		}
		for (int i = start; i < limit; ++i)
		{
			int number_of_derivatives = node_field_creator->numbers_of_derivatives[i];
			enum FE_nodal_value_type *nodal_value_types = node_field_creator->nodal_value_types[i];
			for (int j = 0; j <= number_of_derivatives; ++j)
			{
				if (nodal_value_types[j] == derivative_type)
					return 1;
			}
		}
	}
	return 0;
}

struct FE_node *CREATE(FE_node)(int cm_node_identifier,
	FE_nodeset *fe_nodeset, struct FE_node *template_node)
{
	int return_code;
	struct FE_node *node;

	ENTER(CREATE(FE_node));
	node = (struct FE_node *)NULL;
	if ((0 <= cm_node_identifier) &&
		(((!fe_nodeset) && template_node) || (fe_nodeset && (!template_node))))
	{
		if (ALLOCATE(node, struct FE_node, 1))
		{
			return_code = 1;
			/* clear the new node so we can destroy it if anything fails */
			node->cm_node_identifier = cm_node_identifier;
			node->fields = (struct FE_node_field_info *)NULL;
			node->values_storage = (Value_storage *)NULL;
			node->access_count = 0;
			if (template_node)
			{
				if (!(node->fields =
					ACCESS(FE_node_field_info)(template_node->fields)))
				{
					display_message(ERROR_MESSAGE, "CREATE(FE_node).  "
						"Could not set field info from template node");
					return_code = 0;
				}
				if (template_node->values_storage)
				{
					if (!allocate_and_copy_FE_node_values_storage(template_node,
						&node->values_storage))
					{
						display_message(ERROR_MESSAGE,
							"CREATE(FE_node).  Could not copy values from template node");
						/* values_storage may be corrupt, so clear it */
						node->values_storage = (Value_storage *)NULL;
						return_code = 0;
					}
				}
			}
			else
			{
				node->fields = fe_nodeset->get_FE_node_field_info(/*number_of_values*/0, (struct LIST(FE_node_field) *)0);
				if (0 == node->fields)
				{
					display_message(ERROR_MESSAGE,
						"CREATE(FE_node).  FE_nodeset could not supply node field info");
					return_code = 0;
				}
			}
			if (!return_code)
			{
				DESTROY(FE_node)(&node);
				node = (struct FE_node *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_node).  Could not allocate memory for node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "CREATE(FE_node).  Invalid argument(s)");
	}
	LEAVE;

	return (node);
} /* CREATE(FE_node) */

int DESTROY(FE_node)(struct FE_node **node_address)
/*******************************************************************************
LAST MODIFIED : 11 October 2002

DESCRIPTION :
Frees the memory for the node, sets <*node_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_node *node;

	ENTER(DESTROY(FE_node));
	if ((node_address) && (node = *node_address))
	{
		if (0 == node->access_count)
		{
			/* free the node values_storage */
			//  following is also done by FE_node_invalidate:
			if (node->fields)
			{
				FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
					FE_node_field_free_values_storage_arrays,
					(void *)node->values_storage,node->fields->node_field_list);
				DEACCESS(FE_node_field_info)(&(node->fields));
			}
			DEALLOCATE(node->values_storage);

			/* free the memory associated with the node */
			DEALLOCATE(*node_address);
		}
		else
		{
			*node_address = (struct FE_node *)NULL;
		}
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_node) */

void FE_node_invalidate(struct FE_node *node)
{
	if (node)
	{
		if (node->fields)
		{
			FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
				FE_node_field_free_values_storage_arrays,
				(void *)node->values_storage,node->fields->node_field_list);
			DEACCESS(FE_node_field_info)(&(node->fields));
		}
		DEALLOCATE(node->values_storage);
	}
}

DECLARE_OBJECT_FUNCTIONS(FE_node)

PROTOTYPE_COPY_OBJECT_FUNCTION(FE_node)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Creates an EXACT copy of the node.
==============================================================================*/
{
	int return_code;
	struct FE_node_field_info *node_field_info;

	ENTER(COPY(FE_node));
	return_code=0;
	/* check the arguments */
	if (source&&destination)
	{
		DEACCESS(FE_node_field_info)(&(destination->fields));
		/* free the node values_storage */
		if (destination->fields)
		{
			FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
				FE_node_field_free_values_storage_arrays,
				(void *)destination->values_storage,destination->fields->node_field_list);
		}
		DEALLOCATE(destination->values_storage);

		/* copy the new */
		node_field_info=source->fields;
		if ( allocate_and_copy_FE_node_values_storage(source,&(destination->values_storage)))
		{
			destination->fields=ACCESS(FE_node_field_info)(node_field_info);
			destination->cm_node_identifier=source->cm_node_identifier;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"COPY(FE_node).  Could not do copy_FE_node_values_storage for node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"COPY(FE_node).  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* COPY(FE_node) */

struct FE_node_field_copy_with_FE_field_list_data
{
	int number_of_values, values_storage_size;
	struct LIST(FE_field) *fe_field_list;
	struct LIST(FE_node_field) *node_field_list;
};

static int FE_node_field_copy_with_FE_field_list(
	struct FE_node_field *node_field, void *copy_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
If the FE_field in <node_field> is in <fe_field_list>, makes a copy of it in
the <node_field_list>. It is is a GENERAL_FE_FIELD, the value pointer for
the copy is set to the current <value_storage_size>, and both it and the
<number_of_values> are increased by the appropriate size for <node_field>.
==============================================================================*/
{
	int number_of_values, return_code, values_storage_size;
	struct FE_node_field *offset_node_field;
	struct FE_node_field_copy_with_FE_field_list_data *copy_data;

	ENTER(FE_node_field_copy_with_FE_field_list);
	if (node_field && (copy_data =
		(struct FE_node_field_copy_with_FE_field_list_data *)copy_data_void))
	{
		return_code = 1;
		if (IS_OBJECT_IN_LIST(FE_field)(node_field->field,
			copy_data->fe_field_list))
		{
			if (GENERAL_FE_FIELD == node_field->field->fe_field_type)
			{
				number_of_values = FE_node_field_get_number_of_values(node_field);
				values_storage_size = number_of_values *
					get_Value_storage_size(node_field->field->value_type,
						node_field->time_sequence);
				/* adjust size for proper word alignment in memory */
				ADJUST_VALUE_STORAGE_SIZE(values_storage_size);

				if ((offset_node_field =
					copy_create_FE_node_field_with_offset(node_field,
						copy_data->values_storage_size - node_field->components->value)) &&
					ADD_OBJECT_TO_LIST(FE_node_field)(offset_node_field,
						copy_data->node_field_list))
				{
					copy_data->number_of_values += number_of_values;
					copy_data->values_storage_size += values_storage_size;
				}
				else
				{
					DESTROY(FE_node_field)(&offset_node_field);
					return_code = 0;
				}
			}
			else
			{
				/* non-GENERAL FE_node_fields can be shared */
				return_code = ADD_OBJECT_TO_LIST(FE_node_field)(node_field,
					copy_data->node_field_list);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_node_field_copy_with_FE_field_list.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_copy_with_FE_field_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_copy_with_FE_field_list */

struct FE_node *FE_node_copy_with_FE_field_list(struct FE_node *node,
	struct LIST(FE_field) *fe_field_list)
{
	struct FE_node *copy_node;
	struct FE_node_field_copy_with_FE_field_list_data copy_data;
	FE_nodeset *fe_nodeset;
	Value_storage *values_storage;

	ENTER(FE_node_copy_with_FE_field_list);
	copy_node = (struct FE_node *)NULL;
	if (node && node->fields && (fe_nodeset = node->fields->fe_nodeset) &&
		fe_field_list)
	{
		copy_data.number_of_values = 0;
		copy_data.values_storage_size = 0;
		copy_data.fe_field_list = fe_field_list;
		copy_data.node_field_list = CREATE(LIST(FE_node_field))();
		if (FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
			FE_node_field_copy_with_FE_field_list, (void *)&copy_data,
				node->fields->node_field_list))
		{
			values_storage = (Value_storage *)NULL;
			if ((0 == copy_data.values_storage_size) ||
				(ALLOCATE(values_storage, Value_storage,
					copy_data.values_storage_size) &&
					merge_FE_node_values_storage(node, values_storage,
						copy_data.node_field_list, (struct FE_node *)NULL,
						/*optimised_merge*/0)))
			{
				/* create a node field info for the combined list */
				struct FE_node_field_info *fe_node_field_info = fe_nodeset->get_FE_node_field_info(
					copy_data.number_of_values, copy_data.node_field_list);
				if (0 != fe_node_field_info)
				{
					copy_node = CREATE(FE_node)(node->cm_node_identifier, fe_nodeset,
						(struct FE_node *)NULL);
					if (NULL != copy_node)
					{
						/* fill in the fields and values storage */
						REACCESS(FE_node_field_info)(&(copy_node->fields), fe_node_field_info);
						copy_node->values_storage = values_storage;
					}
					DEACCESS(FE_node_field_info)(&fe_node_field_info);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_node_copy_with_FE_field_list.  Could not get node field info");
					/* do not bother to clean up dynamic contents of values_storage */
					DEALLOCATE(values_storage);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_copy_with_FE_field_list.  Could copy values_storage");
				/* cannot clean up dynamic contents of values_storage */
				DEALLOCATE(values_storage);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_copy_with_FE_field_list.  Error cloning node field list");
		}
		DESTROY(LIST(FE_node_field))(&(copy_data.node_field_list));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_copy_with_FE_field_list.  Invalid argument(s)");
	}
	LEAVE;

	return (copy_node);
} /* FE_node_copy_with_FE_field_list */

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_node)
/*****************************************************************************
LAST MODIFIED : 5 November 1997

DESCRIPTION :
Returns the FE_node->cm_node_identifier identifier as a string.
Up to the calling routine to deallocate the returned char string!
============================================================================*/
{
	char temp_string[20];
	int return_code;

	ENTER(GET_NAME(FE_node));
	if (object&&name_ptr)
	{
		sprintf(temp_string,"%i",object->cm_node_identifier);
		if (ALLOCATE(*name_ptr,char,strlen(temp_string)+1))
		{
			strcpy(*name_ptr,temp_string);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GET_NAME(FE_node).  Could not allocate space for name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GET_NAME(FE_node).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GET_NAME(FE_node) */

/**
 * Updates the pointer to <node_field_info_address> to point to a node_field info
 * which appends to the fields in <node_field_info_address> one <new_node_field>.
 * The node_field_info returned in <node_field_info_address> will be for the
 * <new_number_of_values>.
 * The <fe_nodeset> maintains an internal list of these structures so they can be
 * shared between nodes.  This function allows a fast path when adding a single
 * field.  If the node_field passed in is only referenced by one external object
 * then it is assumed that this function can modify it rather than copying.  If
 * there are more references then the object is copied and then modified.
 * This function handles the access and deaccess of the <node_field_info_address>
 * as if it is just updating then there is nothing to do.
 */
int FE_nodeset_get_FE_node_field_info_adding_new_times(
	FE_nodeset *fe_nodeset, struct FE_node_field_info **node_field_info_address,
	struct FE_node_field *new_node_field)
{
	int return_code;
	struct FE_node_field *node_field, *node_field_copy;
	struct FE_node_field_info *existing_node_field_info;
	struct LIST(FE_node_field) *node_field_list;

	if (fe_nodeset && node_field_info_address &&
		(existing_node_field_info = *node_field_info_address))
	{
		return_code = 1;
		if (NULL != (node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(new_node_field->field,
			existing_node_field_info->node_field_list)))
		{
			if (FE_node_field_info_used_only_once(existing_node_field_info))
			{
				if (node_field->access_count > 1)
				{
					/* Need to copy this node_field */
					node_field_copy = copy_create_FE_node_field_with_offset(node_field,
						/*value_offset*/0);
					REMOVE_OBJECT_FROM_LIST(FE_node_field)(node_field,
						existing_node_field_info->node_field_list);
					/* Update the time sequence */
					FE_node_field_set_FE_time_sequence(node_field_copy, new_node_field->time_sequence);
					ADD_OBJECT_TO_LIST(FE_node_field)(node_field_copy,
						existing_node_field_info->node_field_list);
				}
				else
				{
					FE_node_field_set_FE_time_sequence(node_field, new_node_field->time_sequence);
				}
				/* Should check there isn't a node_field equivalent to this modified one
					already in the list, and if there is use that instead */
			}
			else
			{
				/* Need to copy after all */
				node_field_list = CREATE_LIST(FE_node_field)();
				if (COPY_LIST(FE_node_field)(node_field_list, existing_node_field_info->node_field_list))
				{
					/* Find the correct node field in the new list */
					if (NULL != (node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
						new_node_field->field, node_field_list)))
					{
						node_field_copy = copy_create_FE_node_field_with_offset(node_field,
							/*value_offset*/0);
						REMOVE_OBJECT_FROM_LIST(FE_node_field)(node_field, node_field_list);
						/* Update the time sequence */
						FE_node_field_set_FE_time_sequence(node_field_copy, new_node_field->time_sequence);
						ADD_OBJECT_TO_LIST(FE_node_field)(node_field_copy, node_field_list);
						/* create the new node information, number_of_values has not changed */
						struct FE_node_field_info *new_node_field_info = fe_nodeset->get_FE_node_field_info(
							existing_node_field_info->number_of_values, node_field_list);
						if (0 != new_node_field_info)
						{
							if (*node_field_info_address)
								DEACCESS(FE_node_field_info)(node_field_info_address);
							*node_field_info_address = new_node_field_info;
						}
					}
				}
				DESTROY(LIST(FE_node_field))(&node_field_list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_nodeset_get_FE_node_field_info_adding_new_times.  Field not already defined.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodeset_get_FE_node_field_info_adding_new_times.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int define_FE_field_at_node(struct FE_node *node, struct FE_field *field,
	struct FE_time_sequence *fe_time_sequence,
	struct FE_node_field_creator *fe_node_field_creator)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Defines a field at a node (does not assign values)
==============================================================================*/
{
	enum FE_nodal_value_type **component_nodal_value_types;
	enum FE_time_sequence_mapping time_sequence_mapping;
	enum Value_type value_type;
	int *component_number_of_derivatives,*component_number_of_versions,
		existing_values_storage_size, i, j, new_number_of_times, new_values_storage_size,
		number_of_values, number_of_values_in_component, previous_number_of_times,
		return_code,size,total_number_of_values,value_size;
	struct FE_node_field *existing_node_field, *merged_node_field, *new_node_field,
		*node_field;
	struct FE_node_field_component *component;
	struct FE_node_field_info *node_field_info;
	struct FE_region *fe_region;
	struct FE_time_sequence *existing_time_sequence;
	struct LIST(FE_node_field) *new_node_field_list;
	struct Merge_FE_node_field_into_list_data merge_data;
	Value_storage *new_value, *storage;

	ENTER(define_FE_field_at_node);
	return_code = 0;
	if (node && field && (fe_region = FE_field_get_FE_region(field)) &&
		(node_field_info = node->fields) &&
		(node_field_info->fe_nodeset) &&
		(node_field_info->fe_nodeset->get_FE_region() == fe_region) &&
		(component_number_of_derivatives =
			fe_node_field_creator->numbers_of_derivatives) &&
		(component_number_of_versions =
			fe_node_field_creator->numbers_of_versions) &&
		(component_nodal_value_types =
			fe_node_field_creator->nodal_value_types) &&
		(field->number_of_components == fe_node_field_creator->number_of_components))
	{
		new_values_storage_size = 0;
		value_type = field->value_type;
		size = get_Value_storage_size(value_type, fe_time_sequence);
		return_code = 1;

		/* create the node field */
		if (NULL != (node_field = CREATE(FE_node_field)(field)))
		{
			/* access now, deaccess at end to clean up if fails */
			ACCESS(FE_node_field)(node_field);
			if (fe_time_sequence)
			{
				FE_node_field_set_FE_time_sequence(node_field,
					fe_time_sequence);
			}
			number_of_values = 0;
			if (GENERAL_FE_FIELD == field->fe_field_type)
			{
				i = field->number_of_components;
				component = node_field->components;
				while (return_code && (i > 0))
				{
					/*???DB.  Inline assign_FE_node_field_component and get rid of
					  it ? */
					return_code = assign_FE_node_field_component(component,
						node_field_info->values_storage_size +
						new_values_storage_size,
						*component_number_of_derivatives,
						*component_number_of_versions,
						*component_nodal_value_types);
					number_of_values_in_component = (*component_number_of_versions) *
						(1+(*component_number_of_derivatives));
					number_of_values += number_of_values_in_component;
					new_values_storage_size += number_of_values_in_component*size;
					component_number_of_derivatives++;
					component_number_of_versions++;
					component_nodal_value_types++;
					component++;
					i--;
				}
			}
			if (return_code)
			{
				if (NULL != (existing_node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
					field,node_field_info->node_field_list)))
				{
					if (existing_node_field->time_sequence)
					{
						existing_time_sequence = ACCESS(FE_time_sequence)(existing_node_field->time_sequence);
					}
					else
					{
						existing_time_sequence = (struct FE_time_sequence *)NULL;
					}

					/* Check they are consistent or we are only adding times */
					/* Need to copy node field list in case it is modified  */
					new_node_field_list = CREATE_LIST(FE_node_field)();
					if (COPY_LIST(FE_node_field)(new_node_field_list,
							node_field_info->node_field_list))
					{
						merge_data.requires_merged_storage = 0;
						merge_data.values_storage_size = 0;
						merge_data.number_of_values = node_field_info->number_of_values;
						merge_data.list = new_node_field_list;
						if (merge_FE_node_field_into_list(node_field, (void *)(&merge_data)))
						{
							if (merge_data.requires_merged_storage)
							{
								/* Time sequences are different or it would have failed */
								/* get the node_field_info for the new list */
								merged_node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
									field, new_node_field_list);
								FE_nodeset_get_FE_node_field_info_adding_new_times(
									node_field_info->fe_nodeset, &node->fields,
									merged_node_field);
								new_node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
									field, node->fields->node_field_list);
								/* Update values storage, similar to copy_value_storage_array but
								   we are updating existing storage and need to initialise the new values */
								/* Offsets must not have changed so we can use the existing_node_field */
								storage = node->values_storage + existing_node_field->components->value;
								value_size=get_Value_storage_size(value_type, new_node_field->time_sequence);
								time_sequence_mapping =
									FE_time_sequences_mapping(existing_time_sequence, new_node_field->time_sequence);
								switch (time_sequence_mapping)
								{
									case FE_TIME_SEQUENCE_MAPPING_IDENTICAL:
									{
										/* Do nothing, in fact we shouldn't get here as the merge_data.requires_merged_storage
											should be false in this case */
									} break;
									case FE_TIME_SEQUENCE_MAPPING_APPEND:
									{
										previous_number_of_times = FE_time_sequence_get_number_of_times(existing_time_sequence);
										new_number_of_times = FE_time_sequence_get_number_of_times(new_node_field->time_sequence);
										for (i=0;(i<number_of_values)&&return_code;i++)
										{
											reallocate_time_values_storage_array(value_type,
												new_number_of_times, storage, storage,
												/*initialise_storage*/1, previous_number_of_times);
											storage += value_size;
										}
									} break;
									default:
									{
										/* Need a temporary pointer as we will be allocating the new
											memory, copying the existing values and then replacing the
											pointer with the new value */
										void *temp_storage;

										/* Fallback default implementation */
										for (i=0;(i<number_of_values)&&return_code;i++)
										{
											if (!(allocate_time_values_storage_array(value_type,
														new_node_field->time_sequence,(Value_storage *)&temp_storage,/*initialise_storage*/1)&&
													copy_time_sequence_values_storage_array(storage,value_type,
														existing_time_sequence,new_node_field->time_sequence,(Value_storage *)&temp_storage)))
											{
												display_message(ERROR_MESSAGE,
													"define_FE_field_at_node.  Failed to copy array");
												return_code = 0;
											}
											/* Must free the src array now otherwise we will lose any reference to it */
											free_value_storage_array(storage,value_type,existing_time_sequence,1);
											/* Update the value storage with the new pointer */
											*(void **)storage = temp_storage;
											storage += value_size;
										}
									} break;
								}
							}
							/* else existing time sequence includes new times so do nothing */
						}
						else
						{
							display_message(ERROR_MESSAGE, "define_FE_field_at_node.  "
								"Field already defined incompatibly at node.");
						}
					}
					if (existing_time_sequence)
					{
						DEACCESS(FE_time_sequence)(&existing_time_sequence);
					}
				}
				else
				{
					existing_values_storage_size = node_field_info->values_storage_size;
					total_number_of_values = node_field_info->number_of_values + number_of_values;
					if (node_field_info->fe_nodeset->get_FE_node_field_info_adding_new_field(
						&node_field_info, node_field, total_number_of_values))
					{
						if (GENERAL_FE_FIELD == field->fe_field_type)
						{
							ADJUST_VALUE_STORAGE_SIZE(new_values_storage_size);
							if (REALLOCATE(new_value, node->values_storage, Value_storage,
									node_field_info->values_storage_size +
									new_values_storage_size))
							{
								node->values_storage = new_value;
								/* initialize new values */
								new_value += existing_values_storage_size;
								for (i = number_of_values ; i > 0 ; i--)
								{
									if (fe_time_sequence)
									{
										allocate_time_values_storage_array(value_type,
											fe_time_sequence, new_value, /*initialise_storage*/1);
										new_value += size;
									}
									else
									{
										switch (value_type)
										{
											case ELEMENT_XI_VALUE:
											{
												*((struct FE_element **)new_value) =
													(struct FE_element *)NULL;
												new_value += sizeof(struct FE_element *);
												for (j = 0; j < MAXIMUM_ELEMENT_XI_DIMENSIONS; j++)
												{
													*((FE_value *)new_value) = FE_VALUE_INITIALIZER;
													new_value += sizeof(FE_value);
												}
											} break;
											case FE_VALUE_VALUE:
											{
												*((FE_value *)new_value) = FE_VALUE_INITIALIZER;
												new_value += size;
											} break;
											case UNSIGNED_VALUE:
											{
												*((unsigned *)new_value) = 0;
												new_value += size;
											} break;
											case INT_VALUE:
											{
												*((int *)new_value) = 0;
												new_value += size;
											} break;
											case DOUBLE_VALUE:
											{
												*((double *)new_value) = 0;
												new_value += size;
											} break;
											case FLT_VALUE:
											{
												*((float *)new_value) = 0;
												new_value += size;
											} break;
											case SHORT_VALUE:
											{
												display_message(ERROR_MESSAGE,
													"define_FE_field_at_node.  SHORT_VALUE: Code not "
													"written yet. Beware alignmemt problems!");
												return_code = 0;
											} break;
											case DOUBLE_ARRAY_VALUE:
											{
												double *array = (double *)NULL;
												double **array_address;
												int zero = 0;
												/* copy number of array values to values_storage*/
												*((int *)new_value) = zero;
												/* copy pointer to array values to values_storage */
												array_address =
													(double **)(new_value + sizeof(int));
												*array_address = array;
												new_value += size;
											} break;
											case FE_VALUE_ARRAY_VALUE:
											{
												FE_value *array = (FE_value *)NULL;
												FE_value **array_address;
												int zero = 0;
												*((int *)new_value) = zero;
												array_address =
													(FE_value **)(new_value + sizeof(int));
												*array_address = array;
												new_value += size;
											} break;
											case FLT_ARRAY_VALUE:
											{
												float *array = 0;
												float **array_address;
												int zero = 0;
												*((int *)new_value) = zero;
												array_address = (float **)(new_value + sizeof(int));
												*array_address = array;
												new_value += size;
											} break;
											case SHORT_ARRAY_VALUE:
											{
												short *array = (short *)NULL;
												short **array_address;
												int zero = 0;
												*((int *)new_value) = zero;
												array_address = (short **)(new_value + sizeof(int));
												*array_address = array;
												new_value += size;
											} break;
											case  INT_ARRAY_VALUE:
											{
												int *array = (int *)NULL;
												int **array_address;
												int zero = 0;
												*((int *)new_value) = zero;
												array_address = (int **)(new_value + sizeof(int));
												*array_address = array;
												new_value += size;
											} break;
											case  UNSIGNED_ARRAY_VALUE:
											{
												unsigned *array = (unsigned *)NULL;
												unsigned **array_address;
												int zero = 0;
												*((int *)new_value) = zero;
												array_address =
													(unsigned **)(new_value + sizeof(int));
												*array_address = array;
												new_value += size;
											} break;
											case STRING_VALUE:
											{
												char **string_address;

												string_address = (char **)(new_value);
												*string_address = (char *)NULL;
												new_value += size;
											} break;
											default:
											{
												display_message(ERROR_MESSAGE,
													"define_FE_field_at_node.  Unsupported value_type");
												return_code = 0;
											} break;
										}
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE, "define_FE_field_at_node.  "
									"Could not reallocate nodal values");
								return_code = 0;
							}
						}
						if (return_code)
						{
							/* already handled the accessing in FE_nodeset::get_FE_node_field_info_adding_new_field */
							node->fields = node_field_info;
						}
						else
						{
							DEACCESS(FE_node_field_info)(&node_field_info);
						}
					}
				}
			}
			DEACCESS(FE_node_field)(&node_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_FE_field_at_node.  Could not create node_field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_FE_field_at_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* define_FE_field_at_node */

struct FE_node_field_add_to_list_with_exclusion_data
{
	int value_exclusion_length, value_exclusion_start;
	struct FE_node_field *excluded_node_field;
	struct LIST(FE_node_field) *node_field_list;
};

static int FE_node_field_add_to_list_with_exclusion(
	struct FE_node_field *node_field,void *exclusion_data_void)
/*******************************************************************************
LAST MODIFIED : 14 September 2000

DESCRIPTION :
If <node_field> is before the excluded_node_field, it is added to the list.
If <node_field> is the excluded_node_field, it is ignored.
If <node_field> is after the excluded_node_field, a copy of it is made with
a new value reduced by the <value_exclusion_length>.
==============================================================================*/
{
	int return_code;
	struct FE_node_field *offset_node_field;
	struct FE_node_field_add_to_list_with_exclusion_data *exclusion_data;

	ENTER(FE_node_field_add_to_list_with_exclusion);
	if (node_field && (exclusion_data=
		(struct FE_node_field_add_to_list_with_exclusion_data *)
		exclusion_data_void))
	{
		return_code=1;
		if (node_field != exclusion_data->excluded_node_field)
		{
			if ((GENERAL_FE_FIELD == node_field->field->fe_field_type) &&
				(node_field->components->value > exclusion_data->value_exclusion_start))
			{
				/* create copy of node_field with component->value reduced
					 by value_exclusion_length */
				if (NULL != (offset_node_field=copy_create_FE_node_field_with_offset(
					node_field,-exclusion_data->value_exclusion_length)))
				{
					if (!ADD_OBJECT_TO_LIST(FE_node_field)(offset_node_field,
						exclusion_data->node_field_list))
					{
						DESTROY(FE_node_field)(&offset_node_field);
						return_code=0;
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				return_code=ADD_OBJECT_TO_LIST(FE_node_field)(node_field,
					exclusion_data->node_field_list);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_node_field_add_to_list_with_exclusion.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_add_to_list_with_exclusion.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_add_to_list_with_exclusion */

int FE_element_ensure_FE_field_nodes_are_not_in_list(
	struct FE_element *element,void *node_list_field_data_void)
/*******************************************************************************
LAST MODIFIED : 28 April 2003

DESCRIPTION :
Iterator function which, if <element> has nodes and the listed <fe_field>
defined on it, ensures those nodes contributing to <fe_field> are not in the
<fe_node_list>.
==============================================================================*/
{
	int i, number_of_element_field_nodes, number_of_nodes, return_code;
	struct Node_list_field_data *node_list_field_data;
	struct FE_node **element_field_nodes;

	ENTER(FE_element_ensure_FE_field_nodes_are_not_in_list);
	if (element && (node_list_field_data =
		(struct Node_list_field_data *)node_list_field_data_void))
	{
		return_code = 1;
		/* only elements with nodes are considered */
		if (get_FE_element_number_of_nodes(element, &number_of_nodes))
		{
			if ((0 < number_of_nodes) &&
				FE_field_is_defined_in_element(node_list_field_data->fe_field, element))
			{
				/* get the nodes used by this element field */
				if (calculate_FE_element_field_nodes(element, /*face_number*/-1, node_list_field_data->fe_field,
					&number_of_element_field_nodes, &element_field_nodes,
					/*top_level_element*/(struct FE_element *)NULL))
				{
					for (i = 0; (i < number_of_element_field_nodes) && return_code; i++)
					{
						if (element_field_nodes[i] &&
							IS_OBJECT_IN_LIST(FE_node)(element_field_nodes[i],
								node_list_field_data->fe_node_list))
						{
							return_code = REMOVE_OBJECT_FROM_LIST(FE_node)(
								element_field_nodes[i], node_list_field_data->fe_node_list);
						}
					}
					if (0 < number_of_element_field_nodes)
					{
						for (i = 0; i < number_of_element_field_nodes; i++)
						{
							if (element_field_nodes[i])
							{
								DEACCESS(FE_node)(&(element_field_nodes[i]));
							}
						}
						DEALLOCATE(element_field_nodes);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_element_ensure_FE_field_nodes_are_not_in_list.  "
						"Could not get element field nodes");
					return_code = 0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_ensure_FE_field_nodes_are_not_in_list.  "
				"Could not get number of nodes");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_ensure_FE_field_nodes_are_not_in_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_ensure_FE_field_nodes_are_not_in_list */

int undefine_FE_field_at_node(struct FE_node *node, struct FE_field *field)
{
	int bytes_to_copy,field_number_of_values,return_code;
	struct FE_node_field *node_field;
	struct FE_node_field_add_to_list_with_exclusion_data exclusion_data;
	struct FE_node_field_info *existing_node_field_info;
	struct FE_region *fe_region;
	Value_storage *values_storage;

	if (node && field && (fe_region = FE_field_get_FE_region(field)) &&
		(existing_node_field_info = node->fields) &&
		(existing_node_field_info->fe_nodeset) &&
		(existing_node_field_info->fe_nodeset->get_FE_region() == fe_region))
	{
		/* check if the field is already defined at the node */
		if (NULL != (node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
			existing_node_field_info->node_field_list)))
		{
			field_number_of_values = FE_node_field_get_number_of_values(node_field);
			if (GENERAL_FE_FIELD == field->fe_field_type)
			{
				exclusion_data.value_exclusion_start = node_field->components->value;
				exclusion_data.value_exclusion_length = field_number_of_values *
					get_Value_storage_size(node_field->field->value_type,
						node_field->time_sequence);
				/* adjust size for proper word alignment in memory */
				ADJUST_VALUE_STORAGE_SIZE(exclusion_data.value_exclusion_length);
			}
			else
			{
				exclusion_data.value_exclusion_start =
					existing_node_field_info->values_storage_size;
				exclusion_data.value_exclusion_length = 0;
			}
			exclusion_data.excluded_node_field = node_field;
			exclusion_data.node_field_list = CREATE(LIST(FE_node_field))();
			if (FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
				FE_node_field_add_to_list_with_exclusion, (void *)&exclusion_data,
				existing_node_field_info->node_field_list))
			{
				struct FE_node_field_info *new_node_field_info =
					existing_node_field_info->fe_nodeset->get_FE_node_field_info(
						existing_node_field_info->number_of_values - field_number_of_values,
						exclusion_data.node_field_list);
				if (0 != new_node_field_info)
				{
					if (0 < exclusion_data.value_exclusion_length)
					{
						/* free arrays, embedded locations */
						FE_node_field_free_values_storage_arrays(node_field, (void*)node->values_storage);
						/* copy values_storage after the removed field */
						bytes_to_copy = existing_node_field_info->values_storage_size -
							(exclusion_data.value_exclusion_start +
								exclusion_data.value_exclusion_length);
						if (0<bytes_to_copy)
						{
							/* use memmove instead of memcpy as memory blocks overlap */
							memmove(node->values_storage+exclusion_data.value_exclusion_start,
								node->values_storage+exclusion_data.value_exclusion_start+
								exclusion_data.value_exclusion_length,bytes_to_copy);
						}
						if (REALLOCATE(values_storage,node->values_storage,Value_storage,
							new_node_field_info->values_storage_size))
						{
							node->values_storage=values_storage;
						}
					}
					DEACCESS(FE_node_field_info)(&(node->fields));
					node->fields = new_node_field_info;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"undefine_FE_field_at_node.  Could not create node field info");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"undefine_FE_field_at_node.  Could not copy node field list");
				return_code=0;
			}
			DESTROY(LIST(FE_node_field))(&exclusion_data.node_field_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"undefine_FE_field_at_node.  Field %s is not defined at node %d",
				field->name,node->cm_node_identifier);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"undefine_FE_field_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int define_FE_field_at_node_simple(struct FE_node *node, struct FE_field *field,
	int number_of_derivatives, enum FE_nodal_value_type *derivative_value_types)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Defines <field> at <node> using the same <number_of_derivatives>
and <nodal_value_types> for each component, and only 1 version.
==============================================================================*/
{
	int j, n, number_of_components, return_code;
	struct FE_node_field_creator *node_field_creator;

	ENTER(define_FE_field_at_node_simple);
	if (node && field &&
		(0 < (number_of_components = get_FE_field_number_of_components(field))) &&
		((0 == number_of_derivatives) || ((0 < number_of_derivatives) && derivative_value_types)))
	{
		return_code = 1;
		if (NULL != (node_field_creator = CREATE(FE_node_field_creator)(number_of_components)))
		{
			for (n = 0; n < number_of_components; n++)
			{
				for (j = 0 ; j < number_of_derivatives ; j++)
				{
					int result = FE_node_field_creator_define_derivative(node_field_creator,
						/*component_number*/n, derivative_value_types[j]);
					if (CMZN_OK != result)
					{
						display_message(ERROR_MESSAGE, "define_FE_field_at_node_simple.   Can't define derivative");
						return_code = 0;
					}
				}
			}
			if (return_code)
			{
				if (!define_FE_field_at_node(node, field, (struct FE_time_sequence *)NULL,
					node_field_creator))
				{
					display_message(ERROR_MESSAGE, "define_FE_field_at_node_simple.   Could not define field at node");
					return_code = 0;
				}
			}
			DESTROY(FE_node_field_creator)(&(node_field_creator));
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_FE_field_at_node_simple.  ");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_FE_field_at_node_simple.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_FE_field_at_node_simple */

int for_FE_field_at_node(struct FE_field *field,
	FE_node_field_iterator_function *iterator,void *user_data,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
If an <iterator> is supplied and the <field> is defined at the <node> then the
result of the <iterator> is returned.  Otherwise, if an <iterator> is not
supplied and the <field> is defined at the <node> then a non-zero is returned.
Otherwise, zero is returned.
???DB.  Multiple behaviour dangerous ?
==============================================================================*/
{
	int return_code;
	struct FE_node_field *node_field;
	struct FE_node_field_iterator_and_data iterator_and_data;

	ENTER(for_FE_field_at_node);
	return_code=0;
	if (field&&node&&(node->fields))
	{
		node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
			node->fields->node_field_list);

		if (node_field)
		{
			if (iterator)
			{
				iterator_and_data.iterator=iterator;
				iterator_and_data.user_data=user_data;
				iterator_and_data.node=node;
				return_code=for_FE_field_at_node_iterator(node_field,
					&iterator_and_data);
			}
			else
			{
				return_code=1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"for_FE_field_at_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* for_FE_field_at_node */

int for_each_FE_field_at_node(FE_node_field_iterator_function *iterator,
	void *user_data,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
Calls the <iterator> for each field defined at the <node> until the <iterator>
returns 0 or it runs out of fields.  Returns the result of the last <iterator>
called.
==============================================================================*/
{
	int return_code;
	struct FE_node_field_iterator_and_data iterator_and_data;

	ENTER(for_each_FE_field_at_node);
	return_code=0;
	if (iterator&&node&&(node->fields))
	{
		iterator_and_data.iterator=iterator;
		iterator_and_data.user_data=user_data;
		iterator_and_data.node=node;
		return_code=FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
			for_FE_field_at_node_iterator,&iterator_and_data,
			node->fields->node_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_FE_field_at_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* for_each_FE_field_at_node */

int for_each_FE_field_at_node_alphabetical_indexer_priority(
	FE_node_field_iterator_function *iterator,void *user_data,
	struct FE_node *node)
{
	int i, number_of_fields, return_code;
	struct FE_field *field;
	struct FE_field_order_info *field_order_info;
	struct FE_node_field *node_field;

	ENTER(for_each_FE_field_at_node_alphabetical_indexer_priority);
	return_code = 0;
	if (iterator && node && node->fields)
	{
		// get list of all fields in default alphabetical order
		field_order_info = CREATE(FE_field_order_info)();
		FE_region *fe_region = node->fields->fe_nodeset->get_FE_region();
		return_code = FE_region_for_each_FE_field(fe_region,
			FE_field_add_to_FE_field_order_info, (void *)field_order_info);
		FE_field_order_info_prioritise_indexer_fields(field_order_info);
		number_of_fields = get_FE_field_order_info_number_of_fields(field_order_info);
		for (i = 0; i < number_of_fields; i++)
		{
			field = get_FE_field_order_info_field(field_order_info, i);
			node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
				node->fields->node_field_list);
			if (node_field)
			{
				return_code = (iterator)(node, field, user_data);
			}
		}
		DESTROY(FE_field_order_info)(&field_order_info);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_FE_field_at_node_alphabetical_indexer_priority.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

static int FE_node_field_has_FE_field_values(
	struct FE_node_field *node_field,void *dummy)
/*******************************************************************************
LAST MODIFIED: 19 October 1999

DESCRIPTION:
Returns true if <node_field> has a field with values_storage.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_field_has_FE_field_values);
	USE_PARAMETER(dummy);
	if (node_field&&node_field->field)
	{
		return_code=(0<node_field->field->number_of_values);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_has_FE_field_values.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_has_FE_field_values */

int FE_node_has_FE_field_values(struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 24 September 1999

DESCRIPTION :
Returns true if any single field defined at <node> has values stored with
the field.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_has_FE_field_values);
	if (node&&node->fields)
	{
		return_code=((struct FE_node_field *)NULL !=
			FIRST_OBJECT_IN_LIST_THAT(FE_node_field)(
				FE_node_field_has_FE_field_values,(void *)NULL,
				node->fields->node_field_list));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_has_FE_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_has_FE_field_values */

struct FE_node_field_info *FE_node_get_FE_node_field_info(struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Returns the FE_node_field_info from <node>. Must not be modified!
==============================================================================*/
{
	struct FE_node_field_info *fe_node_field_info;

	ENTER(FE_node_get_FE_node_field_info);
	if (node)
	{
		fe_node_field_info = node->fields;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_get_FE_node_field_info.  Invalid argument(s)");
		fe_node_field_info = (struct FE_node_field_info *)NULL;
	}
	LEAVE;

	return (fe_node_field_info);
} /* FE_node_get_FE_node_field_info */

int FE_node_set_FE_node_field_info(struct FE_node *node,
	struct FE_node_field_info *fe_node_field_info)
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
Changes the FE_node_field_info at <node> to <fe_node_field_info>.
Note it is very important that the old and the new FE_node_field_info structures
describe the same data layout in the nodal values_storage!
Private function only to be called by FE_region when merging FE_regions!
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_set_FE_node_field_info);
	if (node && fe_node_field_info)
	{
		return_code =
			REACCESS(FE_node_field_info)(&(node->fields), fe_node_field_info);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_set_FE_node_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_set_FE_node_field_info */

FE_nodeset *FE_node_get_FE_nodeset(struct FE_node *node)
{
	if (node && node->fields)
		return node->fields->fe_nodeset;
	return 0;
}

int FE_node_to_node_string(struct FE_node *node, char **string_address)
/*****************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Returns an allocated <string> of the identifier of <node>.
============================================================================*/
{
	char tmp_string[50];
	int return_code;

	ENTER(FE_node_to_node_string);
	if (node && string_address)
	{
		sprintf(tmp_string, "%d", get_FE_node_identifier(node));
		*string_address = duplicate_string(tmp_string);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_to_node_string.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_to_node_string */

int equivalent_FE_field_at_nodes(struct FE_field *field,struct FE_node *node_1,
	struct FE_node *node_2)
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns non-zero if the <field> is defined in the same way at the two nodes.
Note this will also return true if <field> is not defined at both nodes.
==============================================================================*/
{
	int return_code;
	struct FE_node_field *node_field_1, *node_field_2;

	ENTER(equivalent_FE_field_at_nodes);
	return_code = 0;
	if (field && node_1 && node_2)
	{
		if (node_1->fields == node_2->fields)
		{
			return_code = 1;
		}
		else
		{
			node_field_1 = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
				node_1->fields->node_field_list);
			node_field_2 = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
				node_2->fields->node_field_list);
			return_code = ((!node_field_1) && (!node_field_2)) || (
				node_field_1 && node_field_2 &&
				FE_node_fields_match(node_field_1, node_field_2,
					/*ignore_field_and_time_sequence*/0, /*ignore_component_value*/1));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"equivalent_FE_field_at_nodes.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* equivalent_FE_field_at_nodes */

int equivalent_FE_fields_at_nodes(struct FE_node *node_1,
	struct FE_node *node_2)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns true if all fields are defined in the same way at the two nodes.
==============================================================================*/
{
	int return_code;

	ENTER(equivalent_FE_fields_at_nodes);
	return_code=0;
	if (node_1&&(node_1->fields)&&node_2&&(node_2->fields)&&
		(node_1->fields==node_2->fields))
	{
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* equivalent_FE_fields_at_nodes */

int FE_nodal_value_version_exists(struct FE_node *node,
	struct FE_field *field, int component_number, int version,
	enum FE_nodal_value_type type)
/*******************************************************************************
LAST MODIFIED : 23 June 1999

DESCRIPTION :
Returns 1 if the <field>, <component_number>, <version> and <type> are stored at the
node and valid at <time>.
???DB.  May need speeding up
==============================================================================*/
{
	int return_code;
	struct FE_time_sequence *time_sequence;
	Value_storage *values_storage = NULL;

	ENTER(FE_nodal_value_version_exists);
	return_code=0;
	if (node && field && (0<=component_number)&&
		(component_number < field->number_of_components) && (0<=version))
	{
		if (find_FE_nodal_values_storage_dest(node,field,component_number,
			version,type,get_FE_field_value_type(field),
			&values_storage,&time_sequence))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodal_value_version_exists.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_nodal_value_version_exists */

int get_FE_nodal_value_as_string(struct FE_node *node,
	struct FE_field *field,int component_number,int version,
	enum FE_nodal_value_type type, FE_value time, char **string)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Returns as a string the value for the (<version>, <type>) for the <field>
<component_number> at the <node>.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/
{
	char temp_string[40];
	int return_code;

	ENTER(get_FE_nodal_value_as_string);
	return_code=0;
	(*string)=(char *)NULL;
	if (node&&field&&(0<=component_number)&&
		(component_number<field->number_of_components)&&(0<=version))
	{
		switch (field->value_type)
		{
			case ELEMENT_XI_VALUE:
			{
				FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
				int dimension, error,i;
				struct FE_element *element;

				if (get_FE_nodal_element_xi_value(node,field,
					component_number,version,type,&element,xi) && 
					(0 < (dimension = get_FE_element_dimension(element))))
				{
					error=0;
					if (dimension == 1)
						append_string(string,"L",&error);
					else if (dimension == 2)
						append_string(string,"F",&error);
					else
						append_string(string,"E",&error);
					sprintf(temp_string," %d",element->get_identifier());
					append_string(string,temp_string,&error);
					for (i=0;i<dimension;i++)
					{
						sprintf(temp_string," %g",xi[i]);
						append_string(string,temp_string,&error);
					}
					return_code = !error;
				}
			} break;
			case FE_VALUE_VALUE:
			{
				FE_value value;

				if (get_FE_nodal_FE_value_value(node,field,component_number,
					version,type,time,&value))
				{
					sprintf(temp_string,"%g",value);
					return_code=append_string(string,temp_string,&return_code);
				}
			} break;
			case INT_VALUE:
			{
				int value;

				if (get_FE_nodal_int_value(node,field,component_number,
					version,type,time,&value))
				{
					sprintf(temp_string,"%d",value);
					return_code=append_string(string,temp_string,&return_code);
				}
			} break;
			case STRING_VALUE:
			{
				return_code=get_FE_nodal_string_value(node,field,component_number,
					version,type,string);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"get_FE_nodal_value_as_string.  Unknown value type %s",
					Value_type_string(field->value_type));
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_nodal_value_as_string.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* get_FE_nodal_value_as_string */

#define INSTANTIATE_GET_FE_NODAL_VALUE_FUNCTION( value_type, value_enum ) \
int get_FE_nodal_ ## value_type ## _value(struct FE_node *node, \
	struct FE_field *field, int component_number, int version, \
	enum FE_nodal_value_type type, FE_value time, value_type *value) \
/******************************************************************************* \
LAST MODIFIED : 21 April 2005 \
 \
DESCRIPTION : \
Gets a particular value (<version>, <type>) for the <field> \
and <component_number> at the <node> and <time>. \
???DB.  May need speeding up \
==============================================================================*/ \
{ \
   value_type *array; \
	FE_value xi; \
	int return_code, time_index_one, time_index_two; \
	struct FE_time_sequence *time_sequence; \
	Value_storage *values_storage = NULL; \
 \
	ENTER(get_FE_nodal_ ## value_type ## _value); \
	return_code=0; \
	if (node && field && (0 <= component_number) && \
		(component_number < field->number_of_components) && (0<=version) && value) \
	{ \
		switch (field->fe_field_type) \
		{ \
			case CONSTANT_FE_FIELD: \
			{ \
				*value = *((value_type *)(field->values_storage)+component_number); \
				return_code=1; \
			} break; \
			case GENERAL_FE_FIELD: \
			{ \
				if (find_FE_nodal_values_storage_dest(node,field,component_number, \
					version,type, value_enum ,&values_storage,&time_sequence)) \
				{ \
					if (time_sequence) \
					{ \
						FE_time_sequence_get_interpolation_for_time(time_sequence, \
							time, &time_index_one, &time_index_two, &xi); \
				   array = *((value_type **)values_storage); \
			   *value = (value_type)(array[time_index_one] * (1.0 - xi) + \
						  array[time_index_two] * xi); \
					} \
					else \
					{ \
						*value = *((value_type *)values_storage); \
					} \
					return_code=1; \
				} \
			} break; \
			case INDEXED_FE_FIELD: \
			{ \
				int index; \
\
				if (get_FE_nodal_int_value(node,field->indexer_field, \
			   /*component_number*/0,/*version*/0,	\
					FE_NODAL_VALUE,time,&index)) \
				{ \
					/* index numbers start at 1 */ \
					if ((1<=index)&&(index<=field->number_of_indexed_values)) \
					{ \
						*value = *((value_type *)(field->values_storage)+ \
							field->number_of_indexed_values*component_number+index-1); \
						return_code=1;	 \
					} \
					else \
					{	 \
						display_message(ERROR_MESSAGE,"get_FE_nodal_" #value_type "_value.  " \
							"Index field %s gave out-of-range index %d in field %s", \
							field->indexer_field->name,index,field->name); \
					} \
				} \
			} break; \
			default: \
			{ \
				display_message(ERROR_MESSAGE, \
					"get_FE_nodal_" #value_type "_value.  Unknown FE_field_type"); \
			} break; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"get_FE_nodal_" #value_type "_value.  Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* get_FE_nodal_ ## value_type ## _value */

#define INSTANTIATE_SET_FE_NODAL_VALUE_FUNCTION( value_type, value_enum ) \
int set_FE_nodal_ ## value_type ## _value(struct FE_node *node, \
	struct FE_field *field, int component_number, int version, \
	enum FE_nodal_value_type type, FE_value time, value_type value) \
/******************************************************************************* \
LAST MODIFIED : 21 April 2005 \
 \
DESCRIPTION : \
Sets a particular value (<version>, <type>) for the <field> \
and <component_number> at the <node>. \
==============================================================================*/ \
{ \
   value_type *array; \
	int return_code, time_index;  \
	struct FE_time_sequence *time_sequence; \
	Value_storage *values_storage = NULL; \
 \
	ENTER(set_FE_nodal_ ## value_type ## _value); \
	return_code=0; \
	/* check arguments */ \
	if (node && field && (0<=component_number) && \
		(component_number < field->number_of_components) && (0<=version)) \
	{ \
		/* get the values storage */ \
		if (find_FE_nodal_values_storage_dest(node,field,component_number, \
		 version,type, value_enum,&values_storage,&time_sequence)) \
		{ \
			if (time_sequence) \
			{ \
				if (FE_time_sequence_get_index_for_time(time_sequence, time, &time_index)) \
				{ \
					array = *((value_type **)values_storage); \
					array[time_index] = value; \
					return_code = 1; \
				} \
			   else \
				{ \
					display_message(ERROR_MESSAGE,"set_FE_nodal_" #value_type "_value.  " \
						"Time value for time %g not defined at this node.", time); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				/* copy in the value */ \
				*((value_type *)values_storage) = value; \
				return_code=1; \
			} \
			/* avoid notifying changes to non-managed nodes */ \
			if (return_code && node->fields->fe_nodeset->containsNode(node)) \
				node->fields->fe_nodeset->nodeFieldChange(node, field); \
		} \
		else \
		{	 \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"set_FE_nodal_" #value_type "_value.  Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* set_FE_nodal_ ## value_type ## _value */

#define INSTANTIATE_GET_FE_NODAL_VALUE_STORAGE_FUNCTION( value_type, value_enum ) \
int get_FE_nodal_ ## value_type ## _storage(struct FE_node *node, \
	struct FE_field *field, int component_number, int version, \
	enum FE_nodal_value_type type, FE_value time, value_type **value) \
/******************************************************************************* \
LAST MODIFIED : 8 May 2007 \
 \
DESCRIPTION : \
Returns a pointer to the memory which contains the values storage for this  \
degree of freedom.  This pointer will be invalid if the node is modified so \
it should only be used temporarily. \
==============================================================================*/ \
{ \
   value_type *array; \
	int return_code, time_index;  \
	struct FE_time_sequence *time_sequence; \
	Value_storage *values_storage = NULL; \
 \
	ENTER(get_FE_nodal_ ## value_type ## _storage); \
	return_code=0; \
	/* check arguments */ \
	if (node && field && (0<=component_number) && \
		(component_number < field->number_of_components) && (0<=version)) \
	{ \
		/* get the values storage */ \
		if (find_FE_nodal_values_storage_dest(node,field,component_number, \
		 version,type, value_enum,&values_storage,&time_sequence)) \
		{ \
			if (time_sequence) \
			{ \
				if (FE_time_sequence_get_index_for_time(time_sequence, time, &time_index)) \
				{ \
					array = *((value_type **)values_storage); \
					*value = &(array[time_index]);		\
					return_code = 1; \
				} \
			   else \
				{ \
					display_message(ERROR_MESSAGE,"get_FE_nodal_" #value_type "_storage.  " \
						"Time value for time %g not defined at this node.", time); \
					return_code=0; \
				} \
			} \
			else \
			{ \
				/* copy in the value */ \
				*value = (value_type *)values_storage; \
				return_code=1; \
			} \
			/* avoid notifying changes to non-managed nodes */ \
			if (return_code && node->fields->fe_nodeset->containsNode(node)) \
			{ \
				/* Notify the clients that this node is changed, even though it hasn't \
					changed yet.  If users of this function have used begin_cache and \
					end cache around their routines then the correct notifications will happen \
					once the end cache is done */ \
				node->fields->fe_nodeset->nodeFieldChange(node, field); \
			} \
		} \
		else \
		{	 \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"get_FE_nodal_" #value_type "_storage.  Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* set_FE_nodal_ ## value_type ## _value */

#define INSTANTIATE_FE_NODAL_VALUE_FUNCTIONS( value_type , value_enum ) \
INSTANTIATE_GET_FE_NODAL_VALUE_FUNCTION(value_type,value_enum) \
INSTANTIATE_SET_FE_NODAL_VALUE_FUNCTION(value_type,value_enum) \
INSTANTIATE_GET_FE_NODAL_VALUE_STORAGE_FUNCTION(value_type,value_enum)

INSTANTIATE_FE_NODAL_VALUE_FUNCTIONS( FE_value , FE_VALUE_VALUE )
INSTANTIATE_FE_NODAL_VALUE_FUNCTIONS( double , DOUBLE_VALUE )
INSTANTIATE_FE_NODAL_VALUE_FUNCTIONS( float , FLT_VALUE )
INSTANTIATE_FE_NODAL_VALUE_FUNCTIONS( int , INT_VALUE )
INSTANTIATE_FE_NODAL_VALUE_FUNCTIONS( short , SHORT_VALUE )

int get_FE_nodal_element_xi_value(struct FE_node *node,
	struct FE_field *field, int component_number, int version,
	enum FE_nodal_value_type type, struct FE_element **element, FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
Gets a particular element_xi_value (<version>, <type>) for the field <component>
at the <node>.
???DB.  May need speeding up
==============================================================================*/
{
	int i,return_code;
	struct FE_time_sequence *time_sequence;
	Value_storage *values_storage = NULL;

	ENTER(get_FE_nodal_element_xi_value);
	return_code=0;
	if (node&&field&&(0<=component_number)&&
		(component_number<field->number_of_components)&&(0<=version)&&
		element&&xi&&(field->value_type==ELEMENT_XI_VALUE))
	{
		values_storage=(Value_storage *)NULL;
		switch (field->fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				values_storage=field->values_storage +
					get_Value_storage_size(ELEMENT_XI_VALUE,
						(struct FE_time_sequence *)NULL)*component_number;
				return_code=1;
			} break;
			case GENERAL_FE_FIELD:
			{
				if (find_FE_nodal_values_storage_dest(node,field,component_number,
					version,type,ELEMENT_XI_VALUE,&values_storage,&time_sequence))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"get_FE_nodal_element_xi_value.  "
						"find_FE_nodal_values_storage_dest failed");
				}
			} break;
			case INDEXED_FE_FIELD:
			{
				int index;

				if (get_FE_nodal_int_value(node,field->indexer_field,
					/*component_number*/0,/*version*/0,
					FE_NODAL_VALUE,/*time*/0,&index))
				{
					/* index numbers start at 1 */
					if ((1<=index)&&(index<=field->number_of_indexed_values))
					{
						values_storage=field->values_storage+
							get_Value_storage_size(ELEMENT_XI_VALUE,
							(struct FE_time_sequence *)NULL)*
							(field->number_of_indexed_values*component_number+index-1);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"get_FE_nodal_element_xi_value.  "
							"Index field %s gave out-of-range index %d in field %s",
							field->indexer_field->name,index,field->name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"get_FE_nodal_element_xi_value.  "
						"Field %s, indexed by %s not defined at node %",
						field->name,field->indexer_field->name,node->cm_node_identifier);
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"get_FE_nodal_element_xi_value.  Unknown FE_field_type");
			} break;
		}
		if (return_code&&values_storage)
		{
			*element = *((struct FE_element **)values_storage);
			values_storage += sizeof(struct FE_element *);
			for (i = 0 ; i < MAXIMUM_ELEMENT_XI_DIMENSIONS ; i++)
			{
				xi[i] = *((FE_value *)values_storage);
				values_storage += sizeof(FE_value);
			}
		}
		else
		{
			if (return_code)
			{
				display_message(ERROR_MESSAGE,
					"get_FE_nodal_element_xi_value.  No values storage");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_nodal_element_xi_value.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* get_FE_nodal_element_xi_value */

int set_FE_nodal_element_xi_value(struct FE_node *node,
	struct FE_field *field, int component_number, int version,
	enum FE_nodal_value_type type,struct FE_element *element, const FE_value *xi)
{
	int return_code = 0;
	int dimension = 0;
	if (node && field && (field->value_type == ELEMENT_XI_VALUE) &&
		(0 <= component_number) && (component_number < field->number_of_components) &&
		(0 <= version) && ((!element) || ((0 < (dimension = get_FE_element_dimension(element))) && xi)))
	{
		// GRC maintain fixed dimension here:
		if ((!element) || (0 == field->element_xi_mesh_dimension) ||
			(dimension == field->element_xi_mesh_dimension))
		{
			Value_storage *values_storage = 0;
			FE_time_sequence *time_sequence = 0;
			/* get the values storage */
			if (find_FE_nodal_values_storage_dest(node,field,component_number,
				version,type,ELEMENT_XI_VALUE,&values_storage,&time_sequence))
			{
				/* copy in the element_xi_value */
				REACCESS(FE_element)((struct FE_element **)values_storage, element);
				values_storage += sizeof(struct FE_element *);
				for (int i = 0 ; i < MAXIMUM_ELEMENT_XI_DIMENSIONS ; i++)
				{
					if (i < dimension)
					{
						*((FE_value *)values_storage) = xi[i];
					}
					else
					{
						/* set spare xi values to 0 */
						*((FE_value *)values_storage) = 0.0;
					}
					values_storage += sizeof(FE_value);
				}
				/* avoid notifying changes to non-managed nodes */
				if (node->fields->fe_nodeset->containsNode(node))
					node->fields->fe_nodeset->nodeFieldChange(node, field);
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "set_FE_nodal_element_xi_value.  "
				"Field %s is restricted to mesh dimension %d; cannot set location in %d-D element number %d.",
				field->name, field->element_xi_mesh_dimension, dimension, element->get_identifier());
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_FE_nodal_element_xi_value.  Invalid argument(s)");
	}
	return (return_code);
}

int FE_node_is_in_Multi_range(struct FE_node *node,void *multi_range_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Conditional function returning true if <node> identifier is in the
<multi_range>.
==============================================================================*/
{
	int return_code;
	struct Multi_range *multi_range;

	ENTER(FE_node_is_in_Multi_range);
	if (node&&(multi_range=(struct Multi_range *)multi_range_void))
	{
		return_code=
			Multi_range_is_value_in_range(multi_range,node->cm_node_identifier);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_is_in_Multi_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_is_in_Multi_range */

int FE_node_is_not_in_Multi_range(struct FE_node *node,void *multi_range_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Conditional function returning true if <node> identifier is NOT in the
<multi_range>.
==============================================================================*/
{
	int return_code;
	struct Multi_range *multi_range;

	ENTER(FE_node_is_not_in_Multi_range);
	if (node&&(multi_range=(struct Multi_range *)multi_range_void))
	{
		return_code=
			!Multi_range_is_value_in_range(multi_range,node->cm_node_identifier);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_is_not_in_Multi_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_is_not_in_Multi_range */

int add_FE_node_number_to_Multi_range(struct FE_node *node,
	void *multi_range_void)
/*******************************************************************************
LAST MODIFIED : 20 February 2000

DESCRIPTION :
Iterator function for adding the number of <node> to <multi_range>.
==============================================================================*/
{
	int node_number,return_code;
	struct Multi_range *multi_range;

	ENTER(add_FE_node_number_to_Multi_range);
	if (node&&(multi_range=(struct Multi_range *)multi_range_void))
	{
		node_number=get_FE_node_identifier(node);
		return_code=Multi_range_add_range(multi_range,node_number,node_number);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_FE_node_number_to_Multi_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_FE_node_number_to_Multi_range */

int add_FE_node_to_list(struct FE_node *node, void *node_list_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Iterator function for adding <node> to <node_list>. Does not expect <node> to
already be in list so more efficient than ensure_FE_node_is_in_list
==============================================================================*/
{
	int return_code;

	ENTER(add_FE_node_to_list);
	return_code =
		ADD_OBJECT_TO_LIST(FE_node)(node, (struct LIST(FE_node) *)node_list_void);
	LEAVE;

	return (return_code);
} /* add_FE_node_to_list */

int ensure_FE_node_is_in_list(struct FE_node *node,void *node_list_void)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Iterator function for adding <node> to <node_list> if not currently in it.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_node) *node_list;

	ENTER(ensure_FE_node_is_in_list);
	if (node&&(node_list=(struct LIST(FE_node) *)node_list_void))
	{
		if (!IS_OBJECT_IN_LIST(FE_node)(node,node_list))
		{
			return_code=ADD_OBJECT_TO_LIST(FE_node)(node,node_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_FE_node_is_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* ensure_FE_node_is_in_list */

int ensure_FE_node_is_in_list_conditional(struct FE_node *node,
	void *list_conditional_data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Iterator function for adding <node> to a list - if not already in it - if a
conditional function with user_data is true.
The node_list, conditional function and user_data are passed in a
struct FE_node_list_conditional_data * in the second argument.
Warning: Must not be iterating over the list being added to!
==============================================================================*/
{
	int return_code;
	struct FE_node_list_conditional_data *list_conditional_data;

	ENTER(ensure_FE_node_is_in_list_conditional);
	if (node&&(list_conditional_data=
		(struct FE_node_list_conditional_data *)list_conditional_data_void)&&
		list_conditional_data->node_list&&list_conditional_data->function)
	{
		if ((list_conditional_data->function)(node,
			list_conditional_data->user_data))
		{
			if (!IS_OBJECT_IN_LIST(FE_node)(node,list_conditional_data->node_list))
			{
				return_code=
					ADD_OBJECT_TO_LIST(FE_node)(node,list_conditional_data->node_list);
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_FE_node_is_in_list_conditional.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* ensure_FE_node_is_in_list_conditional */

int ensure_FE_node_is_not_in_list(struct FE_node *node,void *node_list_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Iterator function for removing <node> from <node_list> if currently in it.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_node) *node_list;

	ENTER(ensure_FE_node_is_not_in_list);
	if (node&&(node_list=(struct LIST(FE_node) *)node_list_void))
	{
		if (IS_OBJECT_IN_LIST(FE_node)(node,node_list))
		{
			return_code=REMOVE_OBJECT_FROM_LIST(FE_node)(node,node_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_FE_node_is_not_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* ensure_FE_node_is_not_in_list */

int toggle_FE_node_in_list(struct FE_node *node,void *node_list_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
If <node> is in <node_list> it is taken out, otherwise it is added.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_node) *node_list;

	ENTER(toggle_FE_node_in_list);
	if (node&&(node_list=(struct LIST(FE_node) *)node_list_void))
	{
		if (IS_OBJECT_IN_LIST(FE_node)(node,node_list))
		{
			return_code=REMOVE_OBJECT_FROM_LIST(FE_node)(node,node_list);
		}
		else
		{
			return_code=ADD_OBJECT_TO_LIST(FE_node)(node,node_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"toggle_FE_node_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* toggle_FE_node_in_list */

int FE_nodeset_clear_embedded_locations(FE_nodeset *fe_nodeset,
	struct LIST(FE_field) *field_list)
{
	if (!field_list || !fe_nodeset)
		return 0;
	cmzn_set_FE_field *fields = reinterpret_cast<cmzn_set_FE_field*>(field_list);
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] = { 0.0, 0.0, 0.0 };
	for (cmzn_set_FE_field::iterator field_iter = fields->begin(); field_iter != fields->end(); ++field_iter)
	{
		FE_field *field = *field_iter;
		if ((ELEMENT_XI_VALUE == field->value_type) &&
			(GENERAL_FE_FIELD == field->fe_field_type))
		{
			cmzn_nodeiterator *node_iter = fe_nodeset->createNodeiterator();
			cmzn_node_id node = 0;
			while (0 != (node = node_iter->next_non_access()))
			{
				// don't clear embedded locations on nodes now owned by a different nodeset
				// as happens when merging from a separate region
				if (node->fields->fe_nodeset == fe_nodeset)
					set_FE_nodal_element_xi_value(node, field, /*component_number*/0,
						/*version*/0, FE_NODAL_VALUE, (struct FE_element *)0, xi);
			}
			cmzn_nodeiterator_destroy(&node_iter);
		}
	}
	return 1;
}

static int FE_node_field_has_field_with_name(
	struct FE_node_field *node_field, void *field_name_void)
/*******************************************************************************
LAST MODIFIED : 14 November 2002

DESCRIPTION :
Returns true if the name of the field in <node_field> matches <field_name>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_field_has_field_with_name);
	return_code = 0;
	if (node_field && node_field->field && field_name_void)
	{
		if (0 == strcmp(node_field->field->name, (char *)field_name_void))
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_has_field_with_name.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_has_field_with_name */

static int FE_node_field_can_be_merged(struct FE_node_field *node_field,
	void *node_field_list_void)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
Fetches a node_field with field of the same name as that in <node_field> from
<node_field_list>. Returns true if there is either no such node_field in the
list or the two node_fields are identically defined apart from the field itself.
Checks first that the FE_fields match.
==============================================================================*/
{
	int return_code;
	struct FE_node_field *other_node_field;
	struct LIST(FE_node_field) *node_field_list;

	ENTER(FE_node_field_can_be_merged);
	return_code = 0;
	if (node_field && node_field->field &&
		(node_field_list = (struct LIST(FE_node_field) *)node_field_list_void))
	{
		if (NULL != (other_node_field = FIRST_OBJECT_IN_LIST_THAT(FE_node_field)(
			FE_node_field_has_field_with_name, (void *)node_field->field->name,
			node_field_list)))
		{
			if (FE_fields_match_exact(node_field->field, other_node_field->field))
			{
				if (FE_node_fields_match(node_field, other_node_field,
					/*ignore_field_and_time_sequence*/1, /*ignore_component_value*/1))
				{
					return_code = 1;
				}
			}
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_can_be_merged.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_can_be_merged */

int FE_node_can_be_merged(struct FE_node *node, void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 November 2002

DESCRIPTION :
Fetches a node with the same identifier as <node> from <data>->node_list.
Returns true if there is either no such node in the list or the two nodes have
the same node field definitions for all fields of the same name.
Note that the actual field may be different, but it is assumed that the same
name fields are already proven to be compatible. <data_void> should point at a
properly initialised struct FE_node_can_be_merged_data.
After using the function, deallocate data->compatible_node_field_info!
==============================================================================*/
{
	int i, return_code;
	struct FE_node *other_node;
	struct FE_node_field_info **node_field_info;
	struct FE_node_can_be_merged_data *data;

	ENTER(FE_node_can_be_merged);
	return_code = 0;
	if (node && (data = (struct FE_node_can_be_merged_data *)data_void) &&
		data->node_list)
	{
		if (NULL != (other_node = FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(
			node->cm_node_identifier, data->node_list)))
		{
			/* check if the node_field_info have already been proved compatible */
			node_field_info = data->compatible_node_field_info;
			for (i = 0; (i < data->number_of_compatible_node_field_info) &&
				(!return_code); i++)
			{
				if ((*node_field_info == node->fields) &&
					(*(node_field_info + 1) == other_node->fields))
				{
					return_code = 1;
				}
				node_field_info += 2;
			}
			if (!return_code)
			{
				/* slow path: loop through node fields */
				if (FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
					FE_node_field_can_be_merged,
					(void *)other_node->fields->node_field_list,
					node->fields->node_field_list))
				{
					/* store combination of node field info in compatible list */
					if (REALLOCATE(node_field_info, data->compatible_node_field_info,
						struct FE_node_field_info *,
						2*(data->number_of_compatible_node_field_info + 1)))
					{
						node_field_info[data->number_of_compatible_node_field_info*2] =
							node->fields;
						node_field_info[data->number_of_compatible_node_field_info*2 + 1] =
							other_node->fields;
						data->compatible_node_field_info = node_field_info;
						data->number_of_compatible_node_field_info++;
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE, "FE_node_can_be_merged.  "
							"Could not reallocate compatible_node_field_info");
					}
				}
			}
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_can_be_merged.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_node_can_be_merged */

int FE_node_has_FE_field_and_string_data(struct FE_node *node,void *data_void)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Returns true(1) if the <data_void>->fe_field is define at the <node> AND
the nodal string at <node>,<data_void>->fe_field is equal to <data_void>->string.
Otherwise returns false (0)
==============================================================================*/
{
	int return_code;
	struct FE_field_and_string_data *field_and_string_data;
	struct FE_field *fe_field;
	char *required_string,*field_string;

	ENTER(FE_node_has_FE_field_and_string_data);
	field_and_string_data=(struct FE_field_and_string_data *)NULL;
	fe_field=(struct FE_field *)NULL;
	required_string=(char *)NULL;
	field_string=(char *)NULL;
	return_code=0;
	if (node&&(field_and_string_data=(struct FE_field_and_string_data *)data_void))
	{
		fe_field=field_and_string_data->fe_field;
		required_string=field_and_string_data->string;
		if (FE_field_is_defined_at_node(fe_field,node))
		{
			if (get_FE_nodal_string_value(node,fe_field,0,0,FE_NODAL_VALUE,&field_string))
			{
				if (!strcmp(required_string,field_string))
				{
					return_code=1;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_node_has_FE_field_and_string_data.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /*FE_node_has_FE_field_and_string_data */

int FE_node_is_in_list(struct FE_node *node,void *node_list_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns true if <node> is in <node_list>.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_node) *node_list;

	ENTER(FE_node_is_in_list);
	if (node&&(node_list=(struct LIST(FE_node) *)node_list_void))
	{
		return_code = IS_OBJECT_IN_LIST(FE_node)(node,node_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_is_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_is_in_list */

int FE_node_is_not_in_list(struct FE_node *node,void *node_list_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns true if <node> is not in <node_list>.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_node) *node_list;

	ENTER(FE_node_is_not_in_list);
	if (node&&(node_list=(struct LIST(FE_node) *)node_list_void))
	{
		return_code = !IS_OBJECT_IN_LIST(FE_node)(node,node_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_is_not_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_is_not_in_list */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(FE_nodal_value_type)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(FE_nodal_value_type));
	switch (enumerator_value)
	{
		case FE_NODAL_VALUE:
		{
			enumerator_string = "value";
		} break;
		case FE_NODAL_D_DS1:
		{
			enumerator_string = "d/ds1";
		} break;
		case FE_NODAL_D_DS2:
		{
			enumerator_string = "d/ds2";
		} break;
		case FE_NODAL_D2_DS1DS2:
		{
			enumerator_string = "d2/ds1ds2";
		} break;
		case FE_NODAL_D_DS3:
		{
			enumerator_string = "d/ds3";
		} break;
		case FE_NODAL_D2_DS1DS3:
		{
			enumerator_string = "d2/ds1ds3";
		} break;
		case FE_NODAL_D2_DS2DS3:
		{
			enumerator_string = "d2/ds2ds3";
		} break;
		case FE_NODAL_D3_DS1DS2DS3:
		{
			enumerator_string = "d3/ds1ds2ds3";
		} break;
		case FE_NODAL_UNKNOWN:
		{
			enumerator_string = "unknown";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(FE_nodal_value_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(FE_nodal_value_type)

int get_FE_field_time_array_index_at_FE_value_time(struct FE_field *field,
	FE_value time,FE_value *the_time_high,FE_value *the_time_low,
	int *the_array_index,int *the_index_high,int *the_index_low)
/*******************************************************************************
LAST MODIFIED : 1 August 2000

DESCRIPTION
Given a <field> and <time>, checks that <field> has times defined and returns:
<the_array_index>, the array index of <field> times closest to <time>.
<the_index_high>, <the_index_low> the upper and lower limits for <the_array_index>
(ideally <the_index_high>==<the_index_low>==<the_array_index>).
<the_time_low> the time corresponding to <the_index_low>.
<the_time_high> the time corresponding to <the_index_high>.

All this information (rather than just <the_array_index> ) is returned so can
perform interpolation, etc.
==============================================================================*/
{
	int array_index,done,index_high,index_low,number_of_times,return_code,step;
	FE_value first_time = 0,last_time = 0,this_time,fe_value_index,time_high,time_low;

	ENTER(get_FE_field_time_array_index_at_FE_value_time);
	return_code=0;
	if (field&&the_time_high&&the_time_low&&the_array_index&&the_index_high&&
		the_index_low&&(number_of_times=get_FE_field_number_of_times(field)))
	{
		get_FE_field_time_FE_value(field,0,&first_time);
		get_FE_field_time_FE_value(field,number_of_times-1,&last_time);
		/*Initial est. of the array index, assuming times evenly spaced, no gaps */
		/*This assumption and hence estimate is true for most signal files. */
		fe_value_index=((time-first_time)/(last_time-first_time))*(number_of_times-1);
		fe_value_index+=0.5;/*round float to nearest int */
		array_index=(int)floor(fe_value_index);
		time_low=0;
		time_high=0;
		done=0;
		index_low=0;
		index_high=number_of_times-1;
		/* do binary search for <time>'s array index. Also look at time of */
		/* adjacent array element, as index estimate may be slightly off due to*/
		/* rounding error. This avoids unnecessarily long search from end of array */
		while(!done)
		{
			if (get_FE_field_time_FE_value(field,array_index,&this_time))
			{
				if (this_time>=time)
				{
					index_high=array_index;
					if (array_index>0)
					{
						/* get adjacent array element*/
						get_FE_field_time_FE_value(field,array_index-1,&time_low);
						/* are we between elements?*/
						if (time_low<time)
						{
							index_low=array_index-1;
							return_code=1;
							done=1;
						}
						else
						{
							time_low=0;
						}
					}
					else
					{
						/* can't get lower adjacent array element when array_index=0. Finished*/
						get_FE_field_time_FE_value(field,array_index,&time_low);
						index_low=array_index;
						return_code=1;
						done=1;
					}
				}
				else /* (this_time<time) */
				{
					index_low=array_index;
					if (array_index<(number_of_times-1))
					{
						/* get adjacent array element*/
						get_FE_field_time_FE_value(field,array_index+1,&time_high);
						/* are we between elements?*/
						if (time_high>time)
						{
							index_high=array_index+1;
							return_code=1;
							done=1;
						}
						else
						{
							time_high=0;
						}
					}
					else
					{
						/* can't get higher adjacent array element when */
						/*array_index=(number_of_times-1). Finished*/
						get_FE_field_time_FE_value(field,array_index,&time_high);
						index_high=array_index;
						return_code=1;
						done=1;
					}
				}
				if (!done)
				{
					step=(index_high-index_low)/2;
					/* No exact match, can't subdivide further, must do interpolation.*/
					if (step==0)
					{
						done=1;
						return_code=1;
					}
					else
					{
						array_index=index_low+step;
					}

				}/* if (!done)	*/
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_field_time_array_index_at_FE_value_time time out of range");
			}
		}	/* while(!done)	*/
		/* index_low and index_high should now be adjacent */
		if (!time_low)
		{
			get_FE_field_time_FE_value(field,index_low,&time_low);
		}
		if (!time_high)
		{
			get_FE_field_time_FE_value(field,index_high,&time_high);
		}
		*the_time_high=time_high;
		*the_time_low=time_low;
		*the_array_index=array_index;
		*the_index_high=index_high;
		*the_index_low=index_low;
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"get_FE_field_time_array_index_at_FE_value_time. Invalid arguments time out of range");
	}
	LEAVE;
	return (return_code);
}/*get_FE_field_time_array_index_at_FE_value_time*/

int get_FE_nodal_string_value(struct FE_node *node,
	struct FE_field *field,int component_number,int version,
	enum FE_nodal_value_type type,char **string)
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
Returns a copy of the string for <version>, <type> of <field><component_number>
at the <node>. Up to the calling function to DEALLOCATE the returned string.
Returned <*string> may be a valid NULL if that is what is in the node.
==============================================================================*/
{
	char *the_string;
	int return_code;
	struct FE_time_sequence *time_sequence;
	Value_storage *values_storage = NULL;

	ENTER(get_FE_nodal_string_value);
	return_code=0;
	if (node&&field&&(0<=component_number)&&
		(component_number<field->number_of_components)&&(0<=version)&&string)
	{
		values_storage=(Value_storage *)NULL;
		switch (field->fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				values_storage=field->values_storage +
					get_Value_storage_size(STRING_VALUE,(struct FE_time_sequence *)NULL)
					*component_number;
				return_code=1;
			} break;
			case GENERAL_FE_FIELD:
			{
				if (find_FE_nodal_values_storage_dest(node,field,component_number,
					version,type,STRING_VALUE,&values_storage,&time_sequence))
				{
					return_code=1;
				}
			} break;
			case INDEXED_FE_FIELD:
			{
				int index;

				if (get_FE_nodal_int_value(node,field->indexer_field,
					/*component_number*/0,/*version*/0,
					FE_NODAL_VALUE,/*time*/0,&index))
				{
					/* index numbers start at 1 */
					if ((1<=index)&&(index<=field->number_of_indexed_values))
					{
						values_storage=field->values_storage+
							get_Value_storage_size(STRING_VALUE,
							(struct FE_time_sequence *)NULL)*
							(field->number_of_indexed_values*component_number+index-1);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"get_FE_nodal_string_value.  "
							"Index field %s gave out-of-range index %d in field %s",
							field->indexer_field->name,index,field->name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"get_FE_nodal_string_value.  "
						"Field %s, indexed by %s not defined at node %",
						field->name,field->indexer_field->name,node->cm_node_identifier);
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"get_FE_nodal_string_value.  Unknown FE_field_type");
			} break;
		}
		if (return_code&&values_storage)
		{
			if (NULL != (the_string = *((char **)values_storage)))
			{
				if (ALLOCATE(*string,char,strlen(the_string)+1))
				{
					strcpy(*string,the_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_FE_nodal_string_value.  Not enough memory");
					return_code=0;
				}
			}
			else
			{
				/* no string, so successfully return NULL */
				*string = (char *)NULL;
			}
		}
		else
		{
			if (return_code)
			{
				display_message(ERROR_MESSAGE,
					"get_FE_nodal_element_xi_value.  No values storage");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_nodal_string_value.Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* get_FE_nodal_string_value */

int set_FE_nodal_string_value(struct FE_node *node,
	struct FE_field *field,int component_number,int version,
	enum FE_nodal_value_type type,const char *string)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Copies and sets the <string> for <version>, <type> of <field><component_number>
at the <node>. <string> may be NULL.
==============================================================================*/
{
	char *the_string,**string_address;
	int return_code;
	struct FE_time_sequence *time_sequence;
	Value_storage *values_storage = NULL;

	ENTER(set_FE_nodal_string_value);
	return_code=0;
	if (node&&field&&(0<=component_number)&&
		(component_number<field->number_of_components)&&(0<=version))
	{
		/* get the values storage */
		if (find_FE_nodal_values_storage_dest(node,field,component_number,
			version,type,STRING_VALUE,&values_storage,&time_sequence))
		{
			/* get the pointer to the stored string */
			string_address = (char **)(values_storage);
			if (string)
			{
				/* reallocate the string currently there */
				if (REALLOCATE(the_string,*string_address,char,strlen(string)+1))
				{
					strcpy(the_string,string);
					*string_address=the_string;
					/* avoid notifying changes to non-managed nodes */
					if (node->fields->fe_nodeset->containsNode(node))
						node->fields->fe_nodeset->nodeFieldChange(node, field);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_FE_nodal_string_value.  Not enough memory");
				}
			}
			else
			{
				/* NULL string; free the existing string */
				if (*string_address)
				{
					DEALLOCATE(*string_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_nodal_string_value.  find_FE_nodal_values_storage_dest failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_nodal_string_value.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_nodal_string_value */

int set_FE_nodal_field_double_values(struct FE_field *field,
	struct FE_node *node,double *values, int *number_of_values)
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets the node field's values storage (at node->values_storage, NOT
field->values_storage) with the doubles in values.
Returns the number of doubles copied in number_of_values.
Assumes that values is set up with the correct number of doubles.
Assumes that the node->values_storage has been allocated with enough
memory to hold all the values.
Assumes that the nodal fields have been set up, with information to
place the values.
==============================================================================*/
{
	int return_code,number_of_components,number_of_versions,number_of_derivatives,
		length,i,j,the_number_of_values=0;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component;
	double *dest,*source;

	ENTER(set_FE_nodal_field_double_values);
	return_code=0;
	if (field&&node&&values&&(node->values_storage))
	{
		if (field->value_type==DOUBLE_VALUE)
		{
			if (NULL != (node_field =FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
				node->fields->node_field_list)))
			{
				return_code=1;
				number_of_components = node_field->field->number_of_components;
				source = values;
				for (i=0;i<number_of_components;i++)
				{
					j=0;
					component = &(node_field->components[i]);
					dest = (double *)(node->values_storage + component->value);
					number_of_versions = component->number_of_versions;
					number_of_derivatives = component->number_of_derivatives;
					length = (1+number_of_derivatives)*number_of_versions;
					while (j<length)
					{
						*dest = *source;
						j++;
						dest++;
						source++;
					}
					the_number_of_values += length;
				}
				*number_of_values = the_number_of_values;
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_FE_nodal_field_double_values.  "
					"Can't find field in node");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_FE_nodal_field_double_values.  "
				"field->value_type != DOUBLE_VALUE");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_nodal_field_double_values.  "
			"Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* set_FE_nodal_field_double_values */

int get_FE_nodal_field_number_of_values(struct FE_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Returns the total number of values stored for that field at the node, equals
sum of <1+num_derivatives>*num_versions for each component.
==============================================================================*/
{
	int number_of_values;
	struct FE_node_field *node_field;

	ENTER(get_FE_nodal_field_number_of_values);
	if (field&&node&&node->fields)
	{
		if (NULL != (node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
			node->fields->node_field_list)))
		{
			number_of_values=FE_node_field_get_number_of_values(node_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_nodal_field_number_of_values.  Can't find field %s at node %d",
				field->name,node->cm_node_identifier);
			number_of_values=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_nodal_field_number_of_values.  Invalid argument(s)");
		number_of_values=0;
	}
	LEAVE;

	return (number_of_values);
} /* get_FE_nodal_field_number_of_values */

int FE_field_get_node_parameter_labels(FE_field *field, FE_node *node, FE_value time,
	FE_node *lastNode, int *componentParameterCounts, int **componentDerivatives, 
	int **componentVersions, bool &isHomogeneous)
{
	if (!(field && node))
		return CMZN_ERROR_ARGUMENT;
	if (lastNode && (node->fields == lastNode->fields))
		return CMZN_OK;
	if (!((field->value_type == FE_VALUE_VALUE) &&
			componentParameterCounts && componentDerivatives && componentVersions))
		return CMZN_ERROR_ARGUMENT;
	FE_node_field *node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
		field, node->fields->node_field_list);
	if (!node_field)
		return CMZN_ERROR_NOT_FOUND;
	const int componentCount = field->number_of_components;
	isHomogeneous = componentCount > 1;
	for (int c = 0; c < field->number_of_components; ++c)
	{
		FE_node_field_component *node_field_component = node_field->components + c;
		const int derivativesCount = node_field_component->number_of_derivatives + 1; // value wasn't counted so add it
		componentParameterCounts[c] = derivativesCount*node_field_component->number_of_versions;
		int *derivatives = componentDerivatives[c];
		int *versions = componentVersions[c];
		for (int v = 0; v < node_field_component->number_of_versions; ++v)
			for (int d = 0; d < derivativesCount; ++d)
			{
				*(derivatives++) = node_field_component->nodal_value_types[d] + 1;
				*(versions++) = v + 1;
			}
		if ((c != 0) && isHomogeneous)
		{
			if (componentParameterCounts[c] != componentParameterCounts[c - 1])
				isHomogeneous = false;
			else
			{
				derivatives = componentDerivatives[c];
				versions = componentVersions[c];
				int *lastDerivatives = componentDerivatives[c - 1];
				int *lastVersions = componentVersions[c - 1];
				for (int i = 0; i < componentParameterCounts[c];
					++i, ++derivatives, ++versions, ++lastDerivatives, ++ lastVersions)
				{
					if ((*derivatives != *lastDerivatives) || (*versions != *lastVersions))
					{
						isHomogeneous = false;
						break;
					}
				}
			}
		}
	}
	return CMZN_OK;
}

int get_FE_nodal_field_FE_value_values(struct FE_field *field,
	struct FE_node *node,int *number_of_values,FE_value time, FE_value **values)
{
	int i,j,length,number_of_derivatives,number_of_versions,return_code,
		time_index_one, time_index_two, size;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component;
	FE_value *dest,*source, xi;
	struct FE_time_sequence *time_sequence;
	Value_storage *the_value_storage;

	return_code=0;
	time_sequence = (struct FE_time_sequence *)NULL;
	if (field&&node&&number_of_values&&values&&node->values_storage)
	{
		if (field->value_type==FE_VALUE_VALUE)
		{
			if ((node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
				node->fields->node_field_list))&&node_field->components)
			{
				*number_of_values = FE_node_field_get_number_of_values(node_field);
				if (ALLOCATE(*values,FE_value,*number_of_values))
				{
					time_sequence = get_FE_node_field_FE_time_sequence(node,field);
					if (time_sequence)
					{
						FE_time_sequence_get_interpolation_for_time(time_sequence,time,&time_index_one,
							&time_index_two,&xi);
					}
					dest= *values;
					for (i=0;i<field->number_of_components;i++)
					{
						component = &(node_field->components[i]);
						if (time_sequence)
						{
							source = 0;
							the_value_storage = (node->values_storage + component->value);
							size = get_Value_storage_size(FE_VALUE_VALUE, time_sequence);
						}
						else
						{
							source = (FE_value *)(node->values_storage + component->value);
							the_value_storage = 0;
							size = 0;
						}
						number_of_versions = component->number_of_versions;
						number_of_derivatives = component->number_of_derivatives;
						length=(1+number_of_derivatives)*number_of_versions;
						for (j=length;0<j;j--)
						{
							if (time_sequence)
							{
								source = *(FE_value **)(the_value_storage);
								if (xi != 0 && time_index_one != time_index_two)
								{
									*dest = source[time_index_one]*(1.0 - xi) + source[time_index_two]*xi;
								}
								else
								{
									*dest = source[time_index_one];
								}
								dest++;
								the_value_storage += size;
							}
							else
							{
								*dest = *source;
								dest++;
								source++;
							}
						}
					}
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_FE_nodal_field_FE_value_values.  Not enough memory");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_nodal_field_FE_value_values.  Can't find field %s at node %d",
					field->name,node->cm_node_identifier);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"get_FE_nodal_field_FE_value_values.  "
				"value_type not FE_VALUE_VALUE");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_FE_nodal_field_FE_value_values.  "
			"Invalid arguments");
	}
	return (return_code);
}

int set_FE_nodal_field_FE_value_values(struct FE_field *field,
	struct FE_node *node, FE_value *values, int *number_of_values, FE_value time)
{
	if (!(field && node && values && (node->values_storage)))
	{
		display_message(ERROR_MESSAGE,
			"set_FE_nodal_field_FE_value_values.  Invalid arguments.  %p %p %p", field, node, values);
		return 0;
	}
	if (field->value_type != FE_VALUE_VALUE)
	{
		display_message(ERROR_MESSAGE,
			"set_FE_nodal_field_FE_value_values.  Field %s is not FE_value type", field->name);
		return 0;
	}
	FE_node_field *node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
		field, node->fields->node_field_list);
	if (!node_field)
	{
		display_message(ERROR_MESSAGE,
			"set_FE_nodal_field_FE_value_values.  Field %s is not define at node %d", field->name, node->cm_node_identifier);
		return 0;
	}
	int time_index;
	if (node_field->time_sequence)
	{
		if (!FE_time_sequence_get_index_for_time(node_field->time_sequence, time, &time_index))
		{
			display_message(ERROR_MESSAGE,
				"set_FE_nodal_field_FE_value_values.  "
				"Field %s does not store parameters at time %g", field->name, time);
			return 0;
		}
	}
	const int number_of_components = node_field->field->number_of_components;
	int the_number_of_values = 0;
	FE_value *source = values;
	for (int i = 0; i < number_of_components; ++i)
	{
		FE_node_field_component &component = node_field->components[i];
		const int number_of_versions = component.number_of_versions;
		const int number_of_derivatives = component.number_of_derivatives;
		const int length = (1 + number_of_derivatives)*number_of_versions;
		if (node_field->time_sequence)
		{
			FE_value **destArray = (FE_value **)(node->values_storage + component.value);
			for (int j = 0; j < length; ++j)
			{
				(*destArray)[time_index] = *source;
				++destArray;
				++source;
			}
		}
		else
		{
			FE_value *dest = (FE_value *)(node->values_storage + component.value);
			for (int j = 0; j < length; ++j)
			{
				*dest = *source;
				++dest;
				++source;
			}
		}
		the_number_of_values += length;
	}
	*number_of_values = the_number_of_values;
	return 1;
}

int FE_field_assign_node_parameters_sparse_FE_value(FE_field *field, FE_node *node,
	int arraySize, FE_value *values, int *valueExists, int valuesCount,
	int componentsSize, int componentsOffset,
	int derivativesSize, int derivativesOffset,
	int versionsSize, int versionsOffset)
{
	if (!(field && node && (FE_VALUE_VALUE == field->value_type) &&
		(0 < arraySize) && values && valueExists && (0 < valuesCount) &&
		(componentsSize == field->number_of_components) &&
		(componentsSize*derivativesSize*versionsSize == arraySize)))
	{
		display_message(ERROR_MESSAGE, "FE_node_assign_FE_value_parameters_sparse.  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
	FE_node_field *node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field, node->fields->node_field_list);
	if (!node_field)
	{
		display_message(ERROR_MESSAGE, "FE_node_assign_FE_value_parameters_sparse.  Field %s is not defined at node %d",
			field->name, node->cm_node_identifier);
		return CMZN_ERROR_NOT_FOUND;
	}
	if (node_field->time_sequence)
	{
		display_message(ERROR_MESSAGE, "FE_node_assign_FE_value_parameters_sparse.  Field %s at node %d is time-varying; case is not implemented",
			field->name, node->cm_node_identifier);
		return CMZN_ERROR_NOT_IMPLEMENTED;
	}
	int numberAssigned = 0;
	for (int c = 0; c < componentsSize; ++c)
	{
		FE_node_field_component *component = node_field->components + c;
		if (!component->nodal_value_types)
		{
			display_message(ERROR_MESSAGE, "FE_node_assign_FE_value_parameters_sparse.  Field %s at node %d has no nodal value types",
				field->name, node->cm_node_identifier);
			return CMZN_ERROR_ARGUMENT;
		}
		const int number_of_versions = component->number_of_versions;
		const int number_of_derivatives = component->number_of_derivatives + 1;
		FE_value *source = values + c*componentsOffset;
		int *sourceExists = valueExists + c*componentsOffset;
		FE_value *target = reinterpret_cast<FE_value *>(node->values_storage + component->value);
		for (int v = 0; v < number_of_versions; ++v)
		{
			for (int d = 0; d < number_of_derivatives; ++d)
			{
				const int od = derivativesOffset*(component->nodal_value_types[d] - FE_NODAL_VALUE);
				if (sourceExists[od])
				{
					*target = source[od];
					++numberAssigned;
				}
				else
					*target = 0.0; // workaround for redundant parameters
				++target;
			}
			source += versionsOffset;
			sourceExists += versionsOffset;
		}
	}
	if (numberAssigned != valuesCount)
	{
		display_message(ERROR_MESSAGE, "FE_node_assign_FE_value_parameters_sparse.  Field %s at node %d configuration cannot take all parameters supplied",
			field->name, node->cm_node_identifier);
		return CMZN_ERROR_INCOMPATIBLE_DATA;
	}
	return CMZN_OK;
}

int set_FE_nodal_field_float_values(struct FE_field *field,
	struct FE_node *node,float *values, int *number_of_values)
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets the node field's values storage (at node->values_storage, NOT
field->values_storage) with the floats in values.
Returns the number of floats copied in number_of_values.
Assumes that values is set up with the correct number of floats.
Assumes that the node->values_storage has been allocated with enough
memory to hold all the values.
Assumes that the nodal fields have been set up, with information to
place the values.
==============================================================================*/
{
	int return_code,number_of_components,number_of_versions,number_of_derivatives,
		length,i,j,the_number_of_values=0;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component;
	float *dest,*source;

	ENTER(set_FE_nodal_field_float_values);
	return_code=0;
	if (field&&node&&values&&(node->values_storage))
	{
		if (field->value_type==FLT_VALUE)
		{
			if (NULL != (node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
				node->fields->node_field_list)))
			{
				return_code=1;
				number_of_components = node_field->field->number_of_components;
				source = values;
				for (i=0;i<number_of_components;i++)
				{
					j=0;
					component = &(node_field->components[i]);
					dest = (float *)(node->values_storage + component->value);
					number_of_versions = component->number_of_versions;
					number_of_derivatives = component->number_of_derivatives;
					length =(1+number_of_derivatives)*number_of_versions;
					while (j<length)
					{
						*dest = *source;
						j++;
						dest++;
						source++;
					}
					the_number_of_values += length;
				}
				*number_of_values = the_number_of_values;
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_FE_nodal_field_float_values.  "
					"Can't find field in node");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_FE_nodal_field_float_values.  "
				"field->value_type != FLT_VALUE");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_nodal_field_float_values.  "
			"Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* set_FE_nodal_field_float_values */

int get_FE_nodal_field_int_values(struct FE_field *field,
	struct FE_node *node, int *number_of_values, FE_value time, int **values)
{
	int i,j,length,number_of_derivatives,number_of_versions,return_code,
		time_index,time_index2,size;
	FE_value xi;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component;
	int *dest,*source;
	struct FE_time_sequence *time_sequence;
	Value_storage *the_value_storage;

	return_code=0;
	if (field&&node&&number_of_values&&values&&node->values_storage)
	{
		if (field->value_type==INT_VALUE)
		{
			if ((node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
				node->fields->node_field_list))&&node_field->components)
			{
				*number_of_values=FE_node_field_get_number_of_values(node_field);
				if (ALLOCATE(*values,int,*number_of_values))
				{
					time_sequence = get_FE_node_field_FE_time_sequence(node,field);
					if (time_sequence)
					{
							FE_time_sequence_get_interpolation_for_time(time_sequence,time,&time_index,
								&time_index2,&xi);
					}
					return_code=1;
					dest= *values;
					for (i=0;i<field->number_of_components;i++)
					{
						component = &(node_field->components[i]);
						if (time_sequence)
						{
							the_value_storage = (node->values_storage + component->value);
							size = get_Value_storage_size(INT_VALUE, time_sequence);
						}
						else
						{
							the_value_storage = 0;
							size = 0;
						}
						source=(int *)(node->values_storage + component->value);
						number_of_versions = component->number_of_versions;
						number_of_derivatives = component->number_of_derivatives;
						length=(1+number_of_derivatives)*number_of_versions;
						for (j=length;0<j;j--)
						{
							if (time_sequence)
							{
								source = *(int **)(the_value_storage);
								*dest = source[time_index];
								dest++;
								the_value_storage += size;
							}
							else
							{
								*dest = *source;
								dest++;
								source++;
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_FE_nodal_field_int_values.  Not enough memory");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_nodal_field_int_values.  Can't find field %s at node %d",
					field->name,node->cm_node_identifier);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"get_FE_nodal_field_int_values.  "
				"value_type not FE_VALUE_VALUE");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_FE_nodal_field_int_values.  "
			"Invalid arguments");
	}
	return (return_code);
}

int set_FE_nodal_field_int_values(struct FE_field *field,
	struct FE_node *node,int *values, int *number_of_values)
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Sets the node field's values storage (at node->values_storage, NOT
field->values_storage) with the integers in values.
Returns the number of integers copied in number_of_values.
Assumes that values is set up with the correct number of ints.
Assumes that the node->values_storage has been allocated with enough
memory to hold all the values.
Assumes that the nodal fields have been set up, with information to
place the values.
==============================================================================*/
{
	int return_code,number_of_components,number_of_versions,
		number_of_derivatives,length,i,j,the_number_of_values=0;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component;
	int *dest,*source;

	ENTER(set_FE_nodal_field_int_values);
	return_code=0;
	if (field&&node&&values&&(node->values_storage))
	{
		if (field->value_type==INT_VALUE)
		{
			if (NULL != (node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
				field,node->fields->node_field_list)))
			{
				return_code=1;
				number_of_components = node_field->field->number_of_components;
				source = values;
				for (i=0;i<number_of_components;i++)
				{
					j=0;
					component = &(node_field->components[i]);
					dest = (int *)(node->values_storage + component->value);
					number_of_versions = component->number_of_versions;
					number_of_derivatives = component->number_of_derivatives;
					length =(1+number_of_derivatives)*number_of_versions;
					while (j<length)
					{
						*dest = *source;
						j++;
						dest++;
						source++;
					}
					the_number_of_values += length;
				}
				*number_of_values = the_number_of_values;
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_FE_nodal_field_int_values.  "
					"Can't find field in node");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_FE_nodal_field_int_values.  "
				"field->value_type != INT_VALUE");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_nodal_field_int_values.  "
			"Invalid arguments");
	}
	LEAVE;

	return (return_code);
} /* set_FE_nodal_field_int_values */

int get_FE_node_number_of_values(struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 5 November 1998

DESCRIPTION :
Returns the number of values stored at the <node>.
==============================================================================*/
{
	int number_of_values;

	ENTER(get_FE_node_number_of_values);
	if (node&&(node->fields))
	{
		number_of_values=node->fields->number_of_values;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_number_of_values.  Invalid node");
		number_of_values=0;
	}
	LEAVE;

	return (number_of_values);
} /* get_FE_node_number_of_values */

int get_FE_node_number_of_fields(struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 5 November 1998

DESCRIPTION :
Returns the number of fields stored at the <node>.
==============================================================================*/
{
	int number_of_fields;

	ENTER(get_FE_node_number_of_fields);
	if (node&&(node->fields))
	{
		number_of_fields=
			NUMBER_IN_LIST(FE_node_field)(node->fields->node_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_number_of_fields.  Invalid node");
		number_of_fields=0;
	}
	LEAVE;

	return (number_of_fields);
} /* get_FE_node_number_of_fields */

struct FE_time_sequence *get_FE_node_field_FE_time_sequence(struct FE_node *node,
	struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 15 November 2004

DESCRIPTION :
Returns the <fe_time_sequence> corresponding to the <node> and <field>.  If the
<node> and <field> have no time dependence then the function will return NULL.
==============================================================================*/
{
	struct FE_node_field *node_field;
	struct FE_time_sequence *time_sequence;

	ENTER(get_FE_node_field_FE_time_sequence);
	if (node&&field)
	{
		if (NULL != (node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
			field,node->fields->node_field_list)))
		{
			time_sequence = node_field->time_sequence;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_node_field_component_FE_time_sequence.  "
				"Field %s not defined at node %d",field->name,node->cm_node_identifier);
			time_sequence = (struct FE_time_sequence *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_field_FE_time_sequence.  Invalid argument(s)");
		time_sequence = (struct FE_time_sequence *)NULL;
	}
	LEAVE;

	return (time_sequence);
} /* get_FE_node_field_FE_time_sequence */

enum FE_nodal_value_type *get_FE_node_field_component_nodal_value_types(
	struct FE_node *node,struct FE_field *field,int component_number)
/*******************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Returns an array of the (1+number_of_derivatives) value types for the
node field component.
It is up to the calling function to DEALLOCATE the returned array.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_types;
	int i,number_of_derivatives;
	struct FE_node_field *node_field;
	struct FE_node_field_component *component;

	ENTER(get_FE_node_field_component_nodal_value_types);
	nodal_value_types=(enum FE_nodal_value_type *)NULL;
	if (node&&field&&(0<=component_number)&&
		(component_number<field->number_of_components))
	{
		if (NULL != (node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
			field,node->fields->node_field_list)))
		{
			component=&(node_field->components[component_number]);
			number_of_derivatives=component->number_of_derivatives;
			if (ALLOCATE(nodal_value_types,enum FE_nodal_value_type,
				1+number_of_derivatives))
			{
				for (i=0;i<=number_of_derivatives;i++)
				{
					/* non-GENERAL_FE_FIELD do not have nodal_value_types since
						 derivatives do not make sense */
					if (component->nodal_value_types)
					{
						nodal_value_types[i]=component->nodal_value_types[i];
					}
					else
					{
						nodal_value_types[i]=FE_NODAL_VALUE;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_node_field_component_nodal_value_types.  Not enough memory");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_node_field_component_nodal_value_types.  "
				"Field %s not defined at node %d",field->name,node->cm_node_identifier);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_field_component_nodal_value_types.  Invalid argument(s)");
	}
	LEAVE;

	return (nodal_value_types);
} /* get_FE_node_field_component_nodal_value_types */

int get_FE_node_field_component_number_of_derivatives(struct FE_node *node,
	struct FE_field *field,int component_number)
/*******************************************************************************
LAST MODIFIED : 23 September 1999

DESCRIPTION :
Returns the number of derivatives for the node field component.
==============================================================================*/
{
	int number_of_derivatives;
	struct FE_node_field *node_field;

	ENTER(get_FE_node_field_number_of_derivatives);
	if (node&&field&&(0<=component_number)&&
		(component_number<field->number_of_components))
	{
		if (NULL != (node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
			field,node->fields->node_field_list)))
		{
			number_of_derivatives =
				node_field->components[component_number].number_of_derivatives;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_node_field_component_number_of_derivatives.  "
				"Field %s not defined at node %d",field->name,node->cm_node_identifier);
			number_of_derivatives=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_field_number_of_derivatives.  Invalid argument(s)");
		number_of_derivatives=0;
	}
	LEAVE;

	return (number_of_derivatives);
} /* get_FE_node_field_number_of_derivatives */

int get_FE_node_field_component_number_of_versions(struct FE_node *node,
	struct FE_field *field,int component_number)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Returns the number of versions for the node field component.
==============================================================================*/
{
	int number_of_versions;
	struct FE_node_field *node_field;

	ENTER(get_FE_node_field_component_number_of_versions);
	if (node&&field&&(0<=component_number)&&
		(component_number<field->number_of_components))
	{
		if (NULL != (node_field=FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
			field,node->fields->node_field_list)))
		{
			number_of_versions =
				node_field->components[component_number].number_of_versions;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_node_field_component_number_of_versions.  "
				"Field %s not defined at node %d",field->name,node->cm_node_identifier);
			number_of_versions=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_field_component_number_of_versions.  Invalid argument(s)");
		number_of_versions=0;
	}
	LEAVE;

	return (number_of_versions);
} /* get_FE_node_field_component_number_of_versions */

int get_FE_node_identifier(struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Returns the integer identifier of the <node>.
==============================================================================*/
{
	int identifier;

	ENTER(get_FE_node_identifier);
	if (node)
	{
		identifier = node->cm_node_identifier;
	}
	else
	{
		display_message(ERROR_MESSAGE, "get_FE_node_identifier.  Invalid node");
		identifier = -1;
	}
	LEAVE;

	return (identifier);
} /* get_FE_node_identifier */

int set_FE_node_identifier(struct FE_node *node, int identifier)
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Changes the identifier of <node> to <identifier>.
Caution! Should only call for nodes that are NOT in indexed lists;
Must wrap in LIST_BEGIN_IDENTIFIER_CHANGE/LIST_END_IDENTIFIER_CHANGE to ensure
node is temporarily removed from all the indexed lists it is in and re-added
afterwards. FE_region should be the only object that needs to call this.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_node_identifier);
	if (node && (0 < identifier))
	{
		node->cm_node_identifier = identifier;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_node_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_identifier */

struct FE_field *get_FE_node_default_coordinate_field(struct FE_node *node)
{
	struct FE_field *default_coordinate_field;

	ENTER(get_FE_node_default_coordinate_field);
	default_coordinate_field = (struct FE_field *)NULL;
	if (node && node->fields)
	{
		FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
			FE_node_field_get_first_coordinate_field,
			(void *)&default_coordinate_field, node->fields->node_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_default_coordinate_field.  Invalid node");
	}
	LEAVE;

	return (default_coordinate_field);
}

int FE_node_find_default_coordinate_field_iterator(
	struct FE_node *node, void *fe_field_ptr_void)
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
An FE_node iterator that returns 1 when an appropriate default_coordinate
fe_field is found.  The fe_field found is returned as fe_field_void.
==============================================================================*/
{
	int return_code;
	struct FE_field *field;

	ENTER(FE_node_find_default_coordinate_field_iterator);
	if (node)
	{
		if (NULL != (field = get_FE_node_default_coordinate_field(node)))
		{
			*((struct FE_field **)fe_field_ptr_void) = field;
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_find_default_coordinate_field_iterator.  Missing element");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_find_default_coordinate_field_iterator */

int merge_FE_node(struct FE_node *destination, struct FE_node *source)
{
	int number_of_values, values_storage_size, return_code;
	struct FE_node_field_info *destination_fields, *source_fields;
	FE_nodeset *fe_nodeset;
	struct LIST(FE_node_field) *node_field_list;
	struct Merge_FE_node_field_into_list_data merge_data;
	Value_storage *values_storage;

	ENTER(merge_FE_node);
	if (destination && (destination_fields = destination->fields) &&
		(fe_nodeset = destination_fields->fe_nodeset) &&
		source && (source_fields = source->fields) &&
		(source_fields->fe_nodeset == fe_nodeset))
	{
		return_code = 1;
		/* construct a node field list containing the fields from destination */
		node_field_list = CREATE_LIST(FE_node_field)();
		if (COPY_LIST(FE_node_field)(node_field_list,
			destination_fields->node_field_list))
		{
			/* sum the values_storage_size and number_of_values */
			number_of_values = 0;
			values_storage_size = 0;
			if (FOR_EACH_OBJECT_IN_LIST(FE_node_field)(count_nodal_size,
				(void *)(&values_storage_size), node_field_list) &&
				FOR_EACH_OBJECT_IN_LIST(FE_node_field)(count_nodal_values,
					(void *)(&number_of_values), node_field_list))
			{
				/* include the new information */
				merge_data.requires_merged_storage = 0;
				merge_data.values_storage_size = values_storage_size;
				merge_data.number_of_values = number_of_values;
				/* node field list */
				merge_data.list = node_field_list;
				if (FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
					merge_FE_node_field_into_list, (void *)(&merge_data),
					source_fields->node_field_list))
				{
					if (!merge_data.requires_merged_storage)
					{
						/* Don't need to reallocate memory as we are only overwriting
							existing values */
						merge_FE_node_values_storage(destination, (Value_storage *)NULL,
							node_field_list, source, /*optimised_merge*/1);
					}
					else
					{
						number_of_values = merge_data.number_of_values;
						values_storage_size = merge_data.values_storage_size;
						values_storage = (Value_storage *)NULL;
						/* allocate the new values storage and fill it with values from the
							destination and the source, favouring the latter but merging all
							time arrays */
						if ((0 == values_storage_size) ||
							(ALLOCATE(values_storage, Value_storage, values_storage_size) &&
								merge_FE_node_values_storage(destination, values_storage,
									node_field_list, source, /*optimised_merge*/1)))
						{
							/* create a node field info for the combined list */
							struct FE_node_field_info *fe_node_field_info =
								fe_nodeset->get_FE_node_field_info(number_of_values, node_field_list);
							if (0 != fe_node_field_info)
							{
								/* clean up old destination values_storage */
								if (destination->values_storage)
								{
									if (destination_fields)
									{
										FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
											FE_node_field_free_values_storage_arrays,
											(void *)destination->values_storage,
											destination_fields->node_field_list);
									}
									DEALLOCATE(destination->values_storage);
								}
								/* insert new fields and values_storage */
								DEACCESS(FE_node_field_info)(&(destination->fields));
								destination->fields = fe_node_field_info;
								destination->values_storage = values_storage;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"merge_FE_node.  Could not get node field info");
								/* do not bother to clean up dynamic contents of values_storage */
								DEALLOCATE(values_storage);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"merge_FE_node.  Could copy values_storage");
							/* cannot clean up dynamic contents of values_storage */
							DEALLOCATE(values_storage);
							return_code = 0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"merge_FE_node.  Error merging node field list");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"merge_FE_node.  Error counting nodal values");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"merge_FE_node.  Could not copy node field list");
			return_code = 0;
		}
		DESTROY(LIST(FE_node_field))(&node_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE, "merge_FE_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* merge_FE_node */

#if !defined (WINDOWS_DEV_FLAG)
int list_FE_node(struct FE_node *node)
{
	int return_code;

	ENTER(list_FE_node);
	if (node)
	{
		return_code=1;
		/* write the number */
		display_message(INFORMATION_MESSAGE,"node : %d\n",node->cm_node_identifier);
		/* write the field information */
		if (node->fields)
		{
			for_each_FE_field_at_node_alphabetical_indexer_priority(
				list_FE_node_field, (void *)NULL, node);
		}
#if defined (DEBUG_CODE)
		/*???debug*/
		display_message(INFORMATION_MESSAGE,"  access count = %d\n",
			node->access_count);
#endif /* defined (DEBUG_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_FE_node.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}
#endif /* !defined (WINDOWS_DEV_FLAG) */

DECLARE_INDEXED_LIST_BTREE_FUNCTIONS(FE_node)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_BTREE_FUNCTION(FE_node,cm_node_identifier,int)
DECLARE_INDEXED_LIST_BTREE_IDENTIFIER_CHANGE_FUNCTIONS(FE_node,cm_node_identifier)
DECLARE_CREATE_INDEXED_LIST_BTREE_ITERATOR_FUNCTION(FE_node,cmzn_nodeiterator)

cmzn_nodeiterator_id cmzn_nodeiterator_access(cmzn_nodeiterator_id node_iterator)
{
	if (node_iterator)
		return node_iterator->access();
	return 0;
}

int cmzn_nodeiterator_destroy(cmzn_nodeiterator_id *node_iterator_address)
{
	if (node_iterator_address)
		return cmzn_nodeiterator::deaccess(*node_iterator_address);
	return 0;
}

cmzn_node_id cmzn_nodeiterator_next(cmzn_nodeiterator_id node_iterator)
{
	if (node_iterator)
		return node_iterator->next();
	return 0;
}

cmzn_node_id cmzn_nodeiterator_next_non_access(cmzn_nodeiterator_id node_iterator)
{
	if (node_iterator)
		return node_iterator->next_non_access();
	return 0;
}

void FE_node_list_write_btree_statistics(struct LIST(FE_node) *node_list)
{
	LIST_BTREE_STATISTICS(FE_node,node_list);
}

DECLARE_CHANGE_LOG_FUNCTIONS(FE_node)

Standard_node_to_element_map *Standard_node_to_element_map_create(
	int node_index, int number_of_nodal_values)
{
	if ((node_index < 0) || (number_of_nodal_values <= 0))
	{
		display_message(ERROR_MESSAGE,
			"Standard_node_to_element_map_create.  Invalid argument(s)");
		return 0;
	}
	struct Standard_node_to_element_map *map;
	ALLOCATE(map, struct Standard_node_to_element_map, 1);
	if (map)
	{
		map->nodal_value_indices = 0; // only used for reading legacy EX files
		ALLOCATE(map->nodal_value_types, FE_nodal_value_type, number_of_nodal_values);
		ALLOCATE(map->nodal_versions, int, number_of_nodal_values);
		ALLOCATE(map->scale_factor_indices, int, number_of_nodal_values);
		if ((map->scale_factor_indices) && (map->nodal_value_types) && (map->nodal_versions))
		{
			map->node_index = node_index;
			map->number_of_nodal_values = number_of_nodal_values;
			for (int i = 0; i < number_of_nodal_values; ++i)
			{
				map->nodal_value_types[i] = FE_NODAL_UNKNOWN; // gives a zero parameter
				map->nodal_versions[i] = 0; // first version; not used for zero parameter
				map->scale_factor_indices[i] = -1; // -1 means unit scale factor
			}
		}
		else
			Standard_node_to_element_map_destroy(&map);
	}
	if (!map)
	{
		display_message(ERROR_MESSAGE,
			"Standard_node_to_element_map_create.  Could not allocate memory for map");
	}
	return map;
}

Standard_node_to_element_map *Standard_node_to_element_map_create_legacy(
	int node_index, int number_of_nodal_values)
{
	Standard_node_to_element_map *map =
		Standard_node_to_element_map_create(node_index, number_of_nodal_values);
	if (map)
	{
		ALLOCATE(map->nodal_value_indices, int, number_of_nodal_values);
		if (map->nodal_value_indices)
		{
			for (int i = 0; i < number_of_nodal_values; ++i)
				map->nodal_value_indices[i] = -1; // -1 means zero value
			// if nodal_versions[0] is -1, need to find new labels
			map->nodal_versions[0] = -1;
		}
		else
			Standard_node_to_element_map_destroy(&map);
	}
	return map;
}

int Standard_node_to_element_map_destroy(
	struct Standard_node_to_element_map **map_address)
{
	int return_code;
	struct Standard_node_to_element_map *map;
	if ((map_address)&&(map= *map_address))
	{
		if (map->nodal_value_indices)
			DEALLOCATE(map->nodal_value_indices);
		DEALLOCATE(map->nodal_value_types);
		DEALLOCATE(map->nodal_versions);
		DEALLOCATE(map->scale_factor_indices);
		DEALLOCATE(*map_address);
		*map_address = 0;
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	return (return_code);
}

/**
 * @param info_1, info_2  Optional element node scale field info; both or
 * neither must be supplied. If supplied then actual nodes from these structures
 * are compared, otherwise local node indexes.
 */
static bool Standard_node_to_element_maps_match(
	Standard_node_to_element_map *standard_node_map_1, FE_element_node_scale_field_info *info_1,
	Standard_node_to_element_map *standard_node_map_2, FE_element_node_scale_field_info *info_2,
	int scale_factor_offset_2_to_1)
{
	if (standard_node_map_1 && standard_node_map_2 && ((info_1 && info_2) || (!info_1 && !info_2)))
	{
		if (info_1)
		{
			FE_node *node1 = (info_1->nodes)[standard_node_map_1->node_index];
			FE_node *node2 = (info_2->nodes)[standard_node_map_2->node_index];
			if (node1 != node2)
				return false;
		}
		else if (standard_node_map_1->node_index != standard_node_map_2->node_index)
			return false;
			
		int k = standard_node_map_1->number_of_nodal_values;
		if (standard_node_map_2->number_of_nodal_values != k)
			return false;
		int *value_index_1 = standard_node_map_1->nodal_value_indices;
		int *value_index_2 = standard_node_map_2->nodal_value_indices;
		if (((value_index_1) && (!value_index_2)) || ((!value_index_1) && (value_index_2)))
			return false;
		const bool checkValueIndices = (0 != value_index_1);
		int *scale_factor_index_1 = standard_node_map_1->scale_factor_indices;
		int *scale_factor_index_2 = standard_node_map_2->scale_factor_indices;
		FE_nodal_value_type *node_value_types_1 = standard_node_map_1->nodal_value_types;
		FE_nodal_value_type *node_value_types_2 = standard_node_map_2->nodal_value_types;
		int *node_versions_1 = standard_node_map_1->nodal_versions;
		int *node_versions_2 = standard_node_map_2->nodal_versions;
		while ((k > 0) && (*node_value_types_1 == *node_value_types_2) &&
			(*node_versions_1 == *node_versions_2) &&
			((*scale_factor_index_1) == ((*scale_factor_index_2) + scale_factor_offset_2_to_1)))
		{
			if (checkValueIndices)
			{
				if (*value_index_1 != *value_index_2)
					break;
				++value_index_1;
				++value_index_2;
			}
			++node_value_types_1;
			++node_value_types_2;
			++node_versions_1;
			++node_versions_2;
			++scale_factor_index_1;
			++scale_factor_index_2;
			--k;
		}
		if (k == 0)
			return true;
	}
	else
		display_message(ERROR_MESSAGE, "Standard_node_to_element_maps_match.  Invalid arguments");
	return false;
}

/**
 * Clears nodal value type and version arrays. Used only on failed conversion from nodal_value_indices.
 */
static void Standard_node_to_element_map_clear_node_value_labels(Standard_node_to_element_map *map)
{
	if (map)
	{
		for (int i = 0; i < map->number_of_nodal_values; ++i)
		{
			map->nodal_value_types[i] = FE_NODAL_UNKNOWN;
			map->nodal_versions[i] = 0;
		}
		// if nodal_versions[0] is -1, need to find new labels
		map->nodal_versions[0] = -1;
	}
}

/**
 * Determines the node value types and versions for each nodal value index in
 * the standard map by finding how the values in the node are labelled.
 * If there are no types/versions in the map these are added, otherwise the
 * new values are checked to ensure they are consistent.
 * @param map  The standard node to element map to update or check against.
 * @param field  The field to check.
 * @param component  The component numbers, starting at 0.
 * @param element  The element to get local nodes from.
 * @param target_fe_nodeset  Optional FE_nodeset to get nodes from if field
 * isn't defined on nodes in local region; used when merging input files.
 * @return  Boolean true on success, false on any failure.
 */
static bool Standard_node_to_element_map_determine_or_check_node_value_labels(
	Standard_node_to_element_map *map, FE_field *field, int componentIndex,
	FE_element *element, FE_nodeset *target_fe_nodeset, FE_field *target_field)
{
	FE_node *node = 0;
	if (!(get_FE_element_node(element, map->node_index, &node) && (node)))
	{
		display_message(ERROR_MESSAGE, "Standard_node_to_element_map_determine_or_check_node_value_labels.  "
			"Field %s in %d-D element %d map cannot find global node at local node index %d",
			field->name, get_FE_element_dimension(element), element->get_identifier(),
			map->node_index + 1);
		return false;
	}
	FE_node_field *node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
		field, node->fields->node_field_list);
	if (!node_field)
	{
		if (target_fe_nodeset && target_field)
		{
			FE_node *target_node = target_fe_nodeset->findNodeByIdentifier(node->cm_node_identifier);
			if (target_node)
				node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
					target_field, target_node->fields->node_field_list);
		}
		if (!node_field)
		{
			display_message(ERROR_MESSAGE, "Standard_node_to_element_map_determine_or_check_node_value_labels.  "
				"Field %s in %d-D element %d indexes global node %d (local node index %d) "
				"which has no parameters for that field.",
				field->name, get_FE_element_dimension(element), element->get_identifier(),
				node->cm_node_identifier, map->node_index + 1);
			return false;
		}
	}
	FE_node_field_component *node_field_component = node_field->components + componentIndex;
	if (!node_field_component->nodal_value_types)
	{
		display_message(ERROR_MESSAGE, "Standard_node_to_element_map_determine_or_check_node_value_labels.  "
			"Field %s in %d-D element %d indexes global node %d (local node index %d) "
			"which does not have value type (derivative) labels.",
			field->name, get_FE_element_dimension(element), element->get_identifier(),
			node->cm_node_identifier, map->node_index + 1);
		return false;
	}
	const int nodeValueTypesCount = 1 + node_field_component->number_of_derivatives;
	const int nodeVersionsCount = node_field_component->number_of_versions;
	const int totalNodeValuesCount = nodeValueTypesCount*nodeVersionsCount;
	const int valuesCount = map->number_of_nodal_values;
	// if nodal_versions[0] is -1, need to find new labels, otherwise compare
	const bool newLabels = (-1 == map->nodal_versions[0]);
	for (int v = 0; v < valuesCount; ++v)
	{
		const int nodeValueIndex = map->nodal_value_indices[v];
		if (nodeValueIndex > totalNodeValuesCount)
		{
			display_message(ERROR_MESSAGE, "Standard_node_to_element_map_determine_or_check_node_value_labels.  "
				"Field %s in %d-D element %d node value index is out of range for values "
				"stored in global node %d (local node index %d).",
				field->name, get_FE_element_dimension(element), element->get_identifier(),
				node->cm_node_identifier, map->node_index + 1);
			return false;
		}
		FE_nodal_value_type valueType;
		int version;
		if (nodeValueIndex < 0)
		{
			// encode special legacy case for zero parameter
			valueType = FE_NODAL_UNKNOWN;
			version = 0;
		}
		else
		{
			valueType = node_field_component->nodal_value_types[nodeValueIndex % nodeValueTypesCount];
			version = nodeValueIndex / nodeValueTypesCount;
			if (FE_NODAL_UNKNOWN == valueType)
			{
				display_message(ERROR_MESSAGE, "Standard_node_to_element_map_determine_or_check_node_value_labels.  "
					"Field %s in %d-D element %d addresses an 'unknown' value type in global node %d (local node %d).",
					field->name, get_FE_element_dimension(element), element->get_identifier(),
					node->cm_node_identifier, map->node_index + 1);
				return false;
			}
		}
		if (newLabels)
		{
			map->nodal_value_types[v] = valueType;
			map->nodal_versions[v] = version;
		}
		else
		{
			if ((valueType != map->nodal_value_types[v]) || (version != map->nodal_versions[v]))
			{
				display_message(ERROR_MESSAGE, "Standard_node_to_element_map_determine_or_check_node_value_labels.  "
					"Field %s in %d-D element %d uses different node value type or version labels to other elements "
					"using the same element field template. This unexpected case is not yet handled.",
					field->name, get_FE_element_dimension(element), element->get_identifier());
				return false;
			}
		}
	}
	return true;
}

/**
 * Creates and returns an exact copy of the struct Standard_node_to_element_map
 * <source>.
 */
static struct Standard_node_to_element_map *copy_create_Standard_node_to_element_map(
	struct Standard_node_to_element_map *source)
{
	struct Standard_node_to_element_map *map = 0;
	if (source)
	{
		const int node_index = source->node_index;
		const int number_of_nodal_values = source->number_of_nodal_values;
		if (source->nodal_value_indices)
			map = Standard_node_to_element_map_create_legacy(node_index, number_of_nodal_values);
		else
			map = Standard_node_to_element_map_create(node_index, number_of_nodal_values);
		if (map)
		{
			for (int i = 0; i < number_of_nodal_values; i++)
			{
				if (map->nodal_value_indices)
					map->nodal_value_indices[i] = source->nodal_value_indices[i];
				map->nodal_value_types[i] = source->nodal_value_types[i];
				map->nodal_versions[i] = source->nodal_versions[i];
				map->scale_factor_indices[i] = source->scale_factor_indices[i];
			}
		}
		else
			display_message(ERROR_MESSAGE, "copy_create_Standard_node_to_element_map.  Failed to create map");
	}
	else
		display_message(ERROR_MESSAGE, "copy_create_Standard_node_to_element_map.  Invalid argument");
	return(map);
}

/**
 * Offset scale factor indices to handle new absolute offsets in element.
 * Used in merge code.
 */
static int Standard_node_to_element_map_offset_scale_factor_indices(
	struct Standard_node_to_element_map *standard_node_map, int scale_factor_offset)
{
	if (!standard_node_map)
		return CMZN_ERROR_ARGUMENT;
	const int number_of_nodal_values = standard_node_map->number_of_nodal_values;
	int *scale_factor_index = standard_node_map->scale_factor_indices;
	for (int i = 0; i < number_of_nodal_values; i++)
	{
		if (0 <= *scale_factor_index) // since -1 == unit scaling
			*scale_factor_index += scale_factor_offset;
		++scale_factor_index;
	}
	return CMZN_OK;
}

int Standard_node_to_element_map_get_node_index(
	struct Standard_node_to_element_map *standard_node_map,
	int *node_index_address)
{
	int return_code;
	if (standard_node_map && node_index_address)
	{
		*node_index_address = standard_node_map->node_index;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Standard_node_to_element_map_get_node_index.  Invalid argument(s)");
		if (node_index_address)
		{
			*node_index_address = 0;
		}
		return_code = 0;
	}
	return(return_code);
}

/**
 * Use with care in merge code only.
 */
int Standard_node_to_element_map_set_node_index(
	Standard_node_to_element_map *standard_node_map, int node_index)
{
	if (!standard_node_map)
		return CMZN_ERROR_ARGUMENT;
	standard_node_map->node_index = node_index;
	return CMZN_OK;
}

int Standard_node_to_element_map_get_number_of_nodal_values(
	struct Standard_node_to_element_map *standard_node_map,
	int *number_of_nodal_values_address)
{
	int return_code;
	if (standard_node_map && number_of_nodal_values_address)
	{
		*number_of_nodal_values_address = standard_node_map->number_of_nodal_values;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Standard_node_to_element_map_get_number_of_nodal_values.  "
			"Invalid argument(s)");
		if (number_of_nodal_values_address)
		{
			*number_of_nodal_values_address = 0;
		}
		return_code = 0;
	}
	return(return_code);
}

int Standard_node_to_element_map_get_nodal_value_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int *nodal_value_index_address)
{
	int return_code;
	if (standard_node_map && standard_node_map->nodal_value_indices &&
		(0 <= nodal_value_number) &&
		(nodal_value_number < standard_node_map->number_of_nodal_values) &&
		nodal_value_index_address)
	{
		*nodal_value_index_address =
			standard_node_map->nodal_value_indices[nodal_value_number];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Standard_node_to_element_map_get_nodal_value_index.  "
			"Invalid argument(s)");
		if (nodal_value_index_address)
		{
			*nodal_value_index_address = 0;
		}
		return_code = 0;
	}
	return(return_code);
}

int Standard_node_to_element_map_set_nodal_value_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int nodal_value_index)
{
	int return_code;
	if (standard_node_map && standard_node_map->nodal_value_indices &&
		(0 <= nodal_value_number) &&
		(nodal_value_number < standard_node_map->number_of_nodal_values) &&
		(-1 == standard_node_map->nodal_value_indices[nodal_value_number]))
	{
		standard_node_map->nodal_value_indices[nodal_value_number] =
			nodal_value_index;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Standard_node_to_element_map_set_nodal_value_index.  Invalid argument(s)");
		return_code = 0;
	}
	return(return_code);
}

FE_nodal_value_type Standard_node_to_element_map_get_nodal_value_type(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number)
{
	if (standard_node_map && standard_node_map->nodal_value_types &&
		(0 <= nodal_value_number) &&
		(nodal_value_number < standard_node_map->number_of_nodal_values))
	{
		return standard_node_map->nodal_value_types[nodal_value_number];
	}
	return FE_NODAL_UNKNOWN;
}

int Standard_node_to_element_map_set_nodal_value_type(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, FE_nodal_value_type nodal_value_type)
{
	if (standard_node_map && standard_node_map->nodal_value_types &&
		(0 <= nodal_value_number) &&
		(nodal_value_number < standard_node_map->number_of_nodal_values) &&
		(FE_NODAL_VALUE <= nodal_value_type) &&
		(nodal_value_type <= FE_NODAL_D3_DS1DS2DS3))
	{
		standard_node_map->nodal_value_types[nodal_value_number] = nodal_value_type;
		return 1;
	}
	display_message(ERROR_MESSAGE, "Standard_node_to_element_map_set_nodal_value_type.  Invalid argument(s)");
	return 0;
}

int Standard_node_to_element_map_get_nodal_version(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number)
{
	if (standard_node_map && standard_node_map->nodal_versions &&
		(0 <= nodal_value_number) &&
		(nodal_value_number < standard_node_map->number_of_nodal_values))
	{
		return standard_node_map->nodal_versions[nodal_value_number] + 1;
	}
	return 0;
}

int Standard_node_to_element_map_set_nodal_version(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int nodal_version)
{
	if (standard_node_map && standard_node_map->nodal_versions &&
		(0 <= nodal_value_number) &&
		(nodal_value_number < standard_node_map->number_of_nodal_values) &&
		(0 < nodal_version))
	{
		standard_node_map->nodal_versions[nodal_value_number] = nodal_version - 1;
		return 1;
	}
	display_message(ERROR_MESSAGE, "Standard_node_to_element_map_set_nodal_version.  Invalid argument(s)");
	return 0;
}

int Standard_node_to_element_map_get_scale_factor_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number)
{
	if (standard_node_map && standard_node_map->scale_factor_indices &&
		(0 <= nodal_value_number) &&
		(nodal_value_number < standard_node_map->number_of_nodal_values))
	{
		return standard_node_map->scale_factor_indices[nodal_value_number];
	}
	display_message(ERROR_MESSAGE,
		"Standard_node_to_element_map_get_scale_factor_index.  Invalid argument(s)");
	return -1;
}

int Standard_node_to_element_map_set_scale_factor_index(
	struct Standard_node_to_element_map *standard_node_map,
	int nodal_value_number, int scale_factor_index)
{
	int return_code;
	if (standard_node_map && standard_node_map->scale_factor_indices &&
		(0 <= nodal_value_number) &&
		(nodal_value_number < standard_node_map->number_of_nodal_values) &&
		(-1 == standard_node_map->scale_factor_indices[nodal_value_number]))
	{
		standard_node_map->scale_factor_indices[nodal_value_number] =
			scale_factor_index;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Standard_node_to_element_map_set_scale_factor_index.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	return(return_code);
}

/**
 * Allocates memory and enters values for a component of a element field.
 * Allocates storage for the global to element maps and sets to NULL.
 */
struct FE_element_field_component *CREATE(FE_element_field_component)(
	enum Global_to_element_map_type type,int number_of_maps,
	struct FE_basis *basis,FE_element_field_component_modify modify)
{
	struct FE_element_field_component *component = 0;
	if ((number_of_maps>0)&&basis)
	{
		component = new FE_element_field_component();
		if (component)
		{
			switch (type)
			{
				case STANDARD_NODE_TO_ELEMENT_MAP:
				{
					if (ALLOCATE(component->map.standard_node_based.node_to_element_maps,
						struct Standard_node_to_element_map *,number_of_maps))
					{
						component->map.standard_node_based.number_of_nodes=number_of_maps;
						Standard_node_to_element_map **standard_node_to_element_map =
							component->map.standard_node_based.node_to_element_maps;
						for (int i=number_of_maps;i>0;i--)
						{
							*standard_node_to_element_map=
								(struct Standard_node_to_element_map *)NULL;
							standard_node_to_element_map++;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
		"CREATE(FE_element_field_component).  Could not allocate memory for maps");
						DEALLOCATE(component);
					}
				} break;
				case GENERAL_ELEMENT_MAP:
				{
					component->map.general_map_based.number_of_maps = number_of_maps;
					component->map.general_map_based.maps = new ElementDOFMap*[number_of_maps];
					for (int i = 0; i < number_of_maps; ++i)
					{
						component->map.general_map_based.maps[i] = 0;
					}
				} break;
				case ELEMENT_GRID_MAP:
				{
					int basis_dimension = 0;
					FE_basis_get_dimension(basis, &basis_dimension);
					if ((ALLOCATE(component->map.element_grid_based.number_in_xi,int,basis_dimension)))
					{
						int *number_in_xi = component->map.element_grid_based.number_in_xi;
						for (int i=basis_dimension;i>0;i--)
						{
							*number_in_xi=0;
							number_in_xi++;
						}
						component->map.element_grid_based.value_index=0;
					}
					else
					{
						display_message(ERROR_MESSAGE,
"CREATE(FE_element_field_component).  Could not allocate memory for number_in_xi");
						DEALLOCATE(component);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"CREATE(FE_element_field_component).  Invalid type");
					DEALLOCATE(component);
				} break;
			}
			if (component)
			{
				component->type=type;
				component->basis=ACCESS(FE_basis)(basis);
				component->modify=modify;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"CREATE(FE_element_field_component).  Could not allocate memory for component");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_element_field_component).  Invalid argument(s)");
	}
	return (component);
}

/**
 * Frees the memory for the component and sets <*component_address> to NULL.
 */
int DESTROY(FE_element_field_component)(
	struct FE_element_field_component **component_address)
{
	int return_code = 1;
	struct FE_element_field_component *component;
	if ((component_address)&&(component= *component_address))
	{
		switch (component->type)
		{
			case STANDARD_NODE_TO_ELEMENT_MAP:
			{
				Standard_node_to_element_map **standard_node_map =
					component->map.standard_node_based.node_to_element_maps;
				for (int i=component->map.standard_node_based.number_of_nodes;i>0;i--)
				{
					Standard_node_to_element_map_destroy(standard_node_map);
					standard_node_map++;
				}
				DEALLOCATE(component->map.standard_node_based.node_to_element_maps);
			} break;
			case GENERAL_ELEMENT_MAP:
			{
				for (int i = 0; i < component->map.general_map_based.number_of_maps; ++i)
				{
					delete component->map.general_map_based.maps[i];
				}
				delete[] component->map.general_map_based.maps;
			} break;
			case ELEMENT_GRID_MAP:
			{
				DEALLOCATE(component->map.element_grid_based.number_in_xi);
			} break;
		}
		delete *component_address;
		*component_address = 0;
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

/**
 * Creates and returns a copy of the supplied element field component. Due to
 * references to scale factor sets, the component is valid for use in the
 * current FE_region only.
 * @see FE_element_field_component_switch_FE_mesh
 */
struct FE_element_field_component *copy_create_FE_element_field_component(
	struct FE_element_field_component *source_component)
{
	struct FE_element_field_component *component = 0;
	if (source_component)
	{
		int i, number_of_maps = 0;
		switch(source_component->type)
		{
			case STANDARD_NODE_TO_ELEMENT_MAP:
			{
				number_of_maps=source_component->map.standard_node_based.number_of_nodes;
			}	break;
			case GENERAL_ELEMENT_MAP:
			{
				number_of_maps = source_component->map.general_map_based.number_of_maps;
			}	break;
			case ELEMENT_GRID_MAP:
			{
				number_of_maps=1;
			}	break;
		}/* switch(source_component->type) */
		/*create the component*/
		component=CREATE(FE_element_field_component)(source_component->type,number_of_maps,
			source_component->basis,source_component->modify);
		/* fill in the interior of component */
		if (component)
		{
			cmzn_mesh_scale_factor_set *scaleFactorSet = source_component->get_scale_factor_set();
			if (scaleFactorSet)
				component->set_scale_factor_set(scaleFactorSet);
			switch(source_component->type)
			{
				case STANDARD_NODE_TO_ELEMENT_MAP:
				{
					for(i=0;i<number_of_maps;i++)
					{
						component->map.standard_node_based.node_to_element_maps[i]=
							copy_create_Standard_node_to_element_map(
								source_component->map.standard_node_based.node_to_element_maps[i]);
					}
				}	break;
				case GENERAL_ELEMENT_MAP:
				{
					component->map.general_map_based.number_of_maps = number_of_maps;
					for(i=0;i<number_of_maps;i++)
					{
						component->map.general_map_based.maps[i] = 
							source_component->map.general_map_based.maps[i]->clone();
					}
				} break;
				case ELEMENT_GRID_MAP:
				{
					int number_of_xi_coordinates = 0;
					FE_basis_get_dimension(source_component->basis, &number_of_xi_coordinates);
					for(i = 0; i < number_of_xi_coordinates; i++)
					{
						component->map.element_grid_based.number_in_xi[i]=
							source_component->map.element_grid_based.number_in_xi[i];
					}
					component->map.element_grid_based.value_index=
						source_component->map.element_grid_based.value_index;
				}	break;
			} /* switch(source_component->type) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"copy_create_FE_element_field_component.  failed to create component");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"copy_create_FE_element_field_component.  Invalid argument");
	}
	return(component);
}

/**
 * Switches references to FE_region-specific objects, notably
 * scale factor sets, to the supplied fe_region.
 * @return  CMZN_OK on success, any other value on failure.
 */
static int FE_element_field_component_switch_FE_mesh(
	FE_element_field_component *component, FE_mesh *fe_mesh)
{
	if (!(component && fe_mesh))
		return CMZN_ERROR_ARGUMENT;
	cmzn_mesh_scale_factor_set *sourceScaleFactorSet = component->get_scale_factor_set();
	if (sourceScaleFactorSet)
	{
		cmzn_mesh_scale_factor_set *targetScaleFactorSet =
			fe_mesh->find_scale_factor_set_by_name(sourceScaleFactorSet->getName());
		if (!targetScaleFactorSet)
		{
			targetScaleFactorSet = fe_mesh->create_scale_factor_set();
			if (!targetScaleFactorSet)
				return CMZN_ERROR_MEMORY;
			targetScaleFactorSet->setName(sourceScaleFactorSet->getName());
		}
		component->set_scale_factor_set(targetScaleFactorSet);
		cmzn_mesh_scale_factor_set::deaccess(targetScaleFactorSet);
	}
	return CMZN_OK;
}

int FE_element_field_component_get_basis(
	struct FE_element_field_component *element_field_component,
	struct FE_basis **basis_address)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets the <basis> used by <element_field_component>.
If fails, puts NULL in *<basis_address> if supplied.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_component_get_basis);
	return_code = 0;
	if (element_field_component && basis_address)
	{
		if (NULL != (*basis_address = element_field_component->basis))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_component_get_basis.  Missing basis");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_get_basis.  Invalid argument(s)");
	}
	if ((!return_code) && basis_address)
	{
		*basis_address = (struct FE_basis *)NULL;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_get_basis */

int FE_element_field_component_get_grid_map_number_in_xi(
	struct FE_element_field_component *element_field_component,
	int xi_number, int *number_in_xi_address)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Gets the <number_in_xi> = number of spaces between grid points = one less than
the number of grid points on <xi_number> for <element_field_component> of type
ELEMENT_GRID_MAP. <xi_number> starts at 0 and must be less than the dimension
of the basis in <element_field_component>.
If fails, puts zero in *<number_in_xi_address> if supplied.
==============================================================================*/
{
	int dimension, return_code;

	ENTER(FE_element_field_component_get_grid_map_number_in_xi);
	if (element_field_component &&
		(ELEMENT_GRID_MAP == element_field_component->type) &&
		element_field_component->map.element_grid_based.number_in_xi &&
		(0 <= xi_number) &&
		FE_basis_get_dimension(element_field_component->basis, &dimension) &&
		(xi_number < dimension) && number_in_xi_address)
	{
		*number_in_xi_address =
			element_field_component->map.element_grid_based.number_in_xi[xi_number];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_get_grid_map_number_in_xi.  "
			"Invalid argument(s)");
		if (number_in_xi_address)
		{
			*number_in_xi_address = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_get_grid_map_number_in_xi */

int FE_element_field_component_set_grid_map_number_in_xi(
	struct FE_element_field_component *element_field_component,
	int xi_number, int number_in_xi)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Sets the <number_in_xi> = number of spaces between grid points = one less than
the number of grid points on <xi_number> for <element_field_component> of type
ELEMENT_GRID_MAP. <xi_number> starts at 0 and must be less than the dimension
of the basis in <element_field_component>. <number_in_xi> must be positive.
The number_in_xi must currently be unset for this <xi_number>.
==============================================================================*/
{
	enum FE_basis_type basis_type;
	int dimension, return_code;

	ENTER(FE_element_field_component_set_grid_map_number_in_xi);
	if (element_field_component &&
		(ELEMENT_GRID_MAP == element_field_component->type) &&
		element_field_component->map.element_grid_based.number_in_xi &&
		(0 <= xi_number) &&
		FE_basis_get_dimension(element_field_component->basis, &dimension) &&
		(xi_number < dimension) && (0 <= number_in_xi) && (0 ==
			element_field_component->map.element_grid_based.number_in_xi[xi_number]) &&
		FE_basis_get_xi_basis_type(element_field_component->basis, xi_number,
			&basis_type) &&
		(((0 == number_in_xi) && (FE_BASIS_CONSTANT == basis_type)) ||
		 ((0 < number_in_xi) && (LINEAR_LAGRANGE == basis_type))))
	{
		element_field_component->map.element_grid_based.number_in_xi[xi_number] =
			number_in_xi;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_set_grid_map_number_in_xi.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_set_grid_map_number_in_xi */

int FE_element_field_component_set_grid_map_value_index(
	struct FE_element_field_component *element_field_component, int value_index)
/*******************************************************************************
LAST MODIFIED : 16 October 2002

DESCRIPTION :
Sets the <value_index> = starting point in the element's value_storage for the
grid-based values for <element_field_component> of type ELEMENT_GRID_MAP.
<value_index> must be non-negative.
The value_index must currently be 0.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_component_set_grid_map_value_index);
	if (element_field_component &&
		(ELEMENT_GRID_MAP == element_field_component->type) &&
		(0 == element_field_component->map.element_grid_based.value_index))
	{
		element_field_component->map.element_grid_based.value_index = value_index;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_set_grid_map_value_index.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_set_grid_map_value_index */

int FE_element_field_component_get_modify(
	struct FE_element_field_component *element_field_component,
	FE_element_field_component_modify *modify_address)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Gets the <modify> function used by <element_field_component> -- can be NULL.
If fails, puts NULL in *<modify_address> if supplied.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_component_get_modify);
	if (element_field_component && modify_address)
	{
		*modify_address = element_field_component->modify;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_get_modify.  Invalid argument(s)");
		if (modify_address)
		{
			*modify_address = (FE_element_field_component_modify)NULL;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_get_modify */

int FE_element_field_component_set_modify(
	struct FE_element_field_component *element_field_component,
	FE_element_field_component_modify modify)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Sets the <modify> function used by <element_field_component> -- can be NULL.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_component_set_modify);
	if (element_field_component)
	{
		element_field_component->modify = modify;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_set_modify.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_set_modify */

int FE_element_field_component_get_number_of_nodes(
	struct FE_element_field_component *element_field_component,
	int *number_of_nodes_address)
{
	int return_code = 0;
	if (element_field_component && number_of_nodes_address)
	{
		switch (element_field_component->type)
		{
			case STANDARD_NODE_TO_ELEMENT_MAP:
			{
				*number_of_nodes_address =
					element_field_component->map.standard_node_based.number_of_nodes;
				return_code = 1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_component_get_number_of_nodes.  "
					"Invalid element field component type");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_get_number_of_nodes.  Invalid argument(s)");
	}
	if ((!return_code) && number_of_nodes_address)
	{
		*number_of_nodes_address = 0;
	}

	return (return_code);
}

int FE_element_field_component_get_local_node_in_use(
	FE_element_field_component *component, int numberOfLocalNodes, int *localNodeInUse)
{
	if (component && ((0 == numberOfLocalNodes) ||
		((0 < numberOfLocalNodes) && localNodeInUse)))
	{
		switch (component->type)
		{
			case STANDARD_NODE_TO_ELEMENT_MAP:
			{
				int number_of_nodes_in_component = component->map.standard_node_based.number_of_nodes;
				for (int j = 0; j < number_of_nodes_in_component; j++)
				{
					Standard_node_to_element_map *standard_node_map = component->map.standard_node_based.node_to_element_maps[j];
					int node_index = standard_node_map->node_index;
					if ((0 <= node_index) && (node_index < numberOfLocalNodes))
					{
						localNodeInUse[node_index] = 1;
					}
				}
			} break;
			case GENERAL_ELEMENT_MAP:
			{
				int numberOfMaps = component->map.general_map_based.number_of_maps;
				ElementDOFMap **maps = component->map.general_map_based.maps;
				for (int i = 0; i < numberOfMaps; ++i)
				{
					maps[i]->setLocalNodeInUse(numberOfLocalNodes, localNodeInUse);
				}
			} break;
			case ELEMENT_GRID_MAP:
			{
				// nothing to do: these maps do not use nodes
			} break;
		}
		return 1;
	}
	return 0;
}

cmzn_mesh_scale_factor_set *FE_element_field_component_get_scale_factor_set(
	FE_element_field_component *component)
{
	if (component)
		return component->get_scale_factor_set();
	return 0;
}

int FE_element_field_component_set_scale_factor_set(
	FE_element_field_component *component, cmzn_mesh_scale_factor_set *scale_factor_set)
{
	if (component)
		return component->set_scale_factor_set(scale_factor_set);
	return CMZN_ERROR_ARGUMENT;
}

int FE_element_field_component_get_standard_node_map(
	struct FE_element_field_component *element_field_component, int node_number,
	struct Standard_node_to_element_map **standard_node_map_address)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Gets the <standard_node_map> relating global node values to those at local
<node_number> for <element_field_component> of type
STANDARD_NODE_TO_ELEMENT_MAP. <node_number> starts at 0 and must be less than
the number of nodes in the component.
If fails, puts NULL in *<standard_node_map_address> if supplied.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_component_get_standard_node_map);
	return_code = 0;
	if (element_field_component &&
		(STANDARD_NODE_TO_ELEMENT_MAP == element_field_component->type) &&
		element_field_component->map.standard_node_based.node_to_element_maps &&
		(0 <= node_number) && (node_number <
			element_field_component->map.standard_node_based.number_of_nodes) &&
		standard_node_map_address)
	{
		if (NULL != (*standard_node_map_address = element_field_component->map.
			standard_node_based.node_to_element_maps[node_number]))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_component_get_standard_node_map.  "
				"Missing standard_node_to_element_map");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_get_standard_node_map.  Invalid argument(s)");
	}
	if ((!return_code) && standard_node_map_address)
	{
		*standard_node_map_address = (struct Standard_node_to_element_map *)NULL;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_get_standard_node_map */

int FE_element_field_component_set_standard_node_map(
	struct FE_element_field_component *element_field_component,
	int node_number, struct Standard_node_to_element_map *standard_node_map)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
Sets the <standard_node_map> relating global node values to those at local
<node_number> for <element_field_component> of type
STANDARD_NODE_TO_ELEMENT_MAP. <node_number> starts at 0 and must be less than
the number of nodes in the component.
The standard_node_map must currently be unset for this <xi_number>.
On successful return <standard_node_map> will be owned by the component.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_component_set_standard_node_map);
	if (element_field_component &&
		(STANDARD_NODE_TO_ELEMENT_MAP == element_field_component->type) &&
		element_field_component->map.standard_node_based.node_to_element_maps &&
		(0 <= node_number) && (node_number <
			element_field_component->map.standard_node_based.number_of_nodes) &&
		(!element_field_component->map.standard_node_based.node_to_element_maps[
			node_number]) && standard_node_map)
	{
		element_field_component->map.standard_node_based.
			node_to_element_maps[node_number] = standard_node_map;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_set_standard_node_map.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_set_standard_node_map */

int FE_element_field_component_get_type(
	struct FE_element_field_component *element_field_component,
	enum Global_to_element_map_type *type_address)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the type of mapping used by <element_field_component>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_component_get_type);
	if (element_field_component && type_address)
	{
		*type_address = element_field_component->type;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_component_get_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_component_get_type */

/**
 * Returns true if <component_1> and <component_2> are equivalent for
 * <info_1> and <info_2>, respectively.
 */
static bool FE_element_field_components_match(
	struct FE_element_field_component *component_1,
	struct FE_element_node_scale_field_info *info_1,
	struct FE_element_field_component *component_2,
	struct FE_element_node_scale_field_info *info_2)
{
	if (component_1 && info_1 && component_2 && info_2)
	{
		if ((component_1->type != component_2->type) ||
			(component_1->basis != component_2->basis) ||
			(component_1->modify != component_2->modify))
		{
			//display_message(ERROR_MESSAGE,
			//	"FE_element_field_components_match.  Inconsistent type or basis");
			return false;
		}

		cmzn_mesh_scale_factor_set *scale_factor_set1 = component_1->get_scale_factor_set();
		cmzn_mesh_scale_factor_set *scale_factor_set2 = component_2->get_scale_factor_set();
		// allow scale factor set to match by name to merge from other region
		if (((scale_factor_set1) && (!scale_factor_set2)) ||
			((!scale_factor_set1) && (scale_factor_set2)) ||
			(scale_factor_set1 && (scale_factor_set1 != scale_factor_set2) &&
				(0 != strcmp(scale_factor_set1->getName(), scale_factor_set2->getName()))))
		{
			//display_message(ERROR_MESSAGE,
			//	"FE_element_field_components_match.  Different scale factor sets");
			return false;
		}

		switch (component_1->type)
		{
			case STANDARD_NODE_TO_ELEMENT_MAP:
			{
				// since scale factor indices are absolute in the element
				// need to find position of scale factor set in source
				// and merged data. Future: make indices relative to set!
				int scale_factor_offset = 0;
				int number_of_scale_factors1 = 0;
				int number_of_scale_factors2 = 0;
				if (scale_factor_set1)
				{
					int scale_factor_set_offset1 = info_1->getScaleFactorSetOffset(
						component_1->get_scale_factor_set(), number_of_scale_factors1);
					int scale_factor_set_offset2 = info_2->getScaleFactorSetOffset(
						component_2->get_scale_factor_set(), number_of_scale_factors2);
					scale_factor_offset = scale_factor_set_offset1 - scale_factor_set_offset2;
				}
				if (number_of_scale_factors1 != number_of_scale_factors2)
				{
					//display_message(ERROR_MESSAGE, "FE_element_field_components_match.  "
					//	"Different numbers of scale factors in sets");
					return false;
				}
				int j = component_1->map.standard_node_based.number_of_nodes;
				Standard_node_to_element_map **standard_node_map_1 = component_1->map.standard_node_based.node_to_element_maps;
				Standard_node_to_element_map **standard_node_map_2 = component_2->map.standard_node_based.node_to_element_maps;
				if ((component_2->map.standard_node_based.number_of_nodes == j) &&
					(standard_node_map_1) && (standard_node_map_2))
				{
					/* check each standard node to element map */
					while (j > 0)
					{
						if (Standard_node_to_element_maps_match(*standard_node_map_1, info_1,
							*standard_node_map_2, info_2, scale_factor_offset))
						{
							++standard_node_map_1;
							++standard_node_map_2;
							--j;
						}
						else
						{
							//display_message(ERROR_MESSAGE, "FE_element_field_components_match.  "
							//	"Inconsistent standard node to element maps");
							return false;
						}
					}
				}
				else
				{
					//display_message(ERROR_MESSAGE, "FE_element_field_components_match.  "
					//	"Different or invalid standard node to element maps");
					return false;
				}
			} break;
			case GENERAL_ELEMENT_MAP:
			{
				int numberOfMaps = component_1->map.general_map_based.number_of_maps;
				if (numberOfMaps != component_2->map.general_map_based.number_of_maps)
				{
					//display_message(ERROR_MESSAGE,
					//	"FE_element_field_components_match.  Different numbers of element DOF maps");
					return false;
				}
				ElementDOFMapMatchCache cache(component_1->get_scale_factor_set(), info_1, info_2);
				if (cache.numberOfScaleFactors1 != cache.numberOfScaleFactors2)
				{
					//display_message(ERROR_MESSAGE,
					//	"FE_element_field_components_match.  Different numbers of scale factors for basis");
					return false;
				}
				// could compare scale factors here, otherwise make them overwritable
				ElementDOFMap **maps1 = component_1->map.general_map_based.maps;
				ElementDOFMap **maps2 = component_2->map.general_map_based.maps;
				for (int j = 0; j < numberOfMaps; ++j)
				{
					if (!maps1[j]->matchesWithInfo(maps2[j], cache))
					{
						//display_message(ERROR_MESSAGE,
						//	"FE_element_field_components_match.  Element DOF map %d is different", j + 1);
						return false;
					}
				}
			} break;
			case ELEMENT_GRID_MAP:
			{
				int *number_in_xi_1 = component_1->map.element_grid_based.number_in_xi;
				int *number_in_xi_2 = component_2->map.element_grid_based.number_in_xi;
				int j = 0;
				FE_basis_get_dimension(component_1->basis, &j);
				int number_of_values = 1;
				while (j && (*number_in_xi_1 == *number_in_xi_2))
				{
					number_of_values *= (*number_in_xi_1) + 1;
					j--;
					number_in_xi_1++;
					number_in_xi_2++;
				}
				if (j)
				{
					//display_message(ERROR_MESSAGE,
					//	"FE_element_field_components_match.  Inconsistent grids");
					return false;
				}
			} break;
		}
		return true; // matches!
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_components_match.  Invalid argument(s)");
	}
	return false;
}

struct FE_element_field *CREATE(FE_element_field)(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Allocates memory and assigns fields for an element field.  The storage is
allocated for the pointers to the components and set to NULL.
==============================================================================*/
{
	int i;
	struct FE_element_field *element_field;
	struct FE_element_field_component **component;

	ENTER(CREATE(FE_element_field));
	if (field)
	{
		if ((ALLOCATE(element_field,struct FE_element_field,1))&&
			(ALLOCATE(component,struct FE_element_field_component *,
			field->number_of_components)))
		{
			element_field->access_count=0;
			element_field->field=ACCESS(FE_field)(field);
			element_field->components=component;
			for (i=field->number_of_components;i>0;i--)
			{
				*component=(struct FE_element_field_component *)NULL;
				component++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"CREATE(FE_element_field).  Could not allocate memory for element field");
			DEALLOCATE(element_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_element_field).  Invalid argument(s)");
		element_field=(struct FE_element_field *)NULL;
	}
	LEAVE;

	return (element_field);
} /* CREATE(FE_element_field) */

int DESTROY(FE_element_field)(
	struct FE_element_field **element_field_address)
/*******************************************************************************
LAST MODIFIED : 24 September 1995

DESCRIPTION :
Frees the memory for element field and sets <*element_field_address> to NULL.
==============================================================================*/
{
	int i,return_code;
	struct FE_element_field *element_field;
	struct FE_element_field_component **component;

	ENTER(DESTROY(FE_element_field));
	if ((element_field_address)&&(element_field= *element_field_address))
	{
		if (0==element_field->access_count)
		{
			/* the element field will be destroyed as part as part of destroying
				element field information.  So it will already have been removed from
				the appropriate list */
			/* destroy the global to element maps */
			component=element_field->components;
			for (i=element_field->field->number_of_components;i>0;i--)
			{
				DESTROY(FE_element_field_component)(component);
				component++;
			}
			DEALLOCATE(element_field->components);
			DEACCESS(FE_field)(&(element_field->field));
			DEALLOCATE(*element_field_address);
		}
		else
		{
			*element_field_address=(struct FE_element_field *)NULL;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_element_field) */

DECLARE_OBJECT_FUNCTIONS(FE_element_field)

/**
 * @return 1 if <element_field> is not in the <element_field_list>, otherwise 0.
 */
static int FE_element_field_not_in_list(struct FE_element_field *element_field,
	void *element_field_list)
{
	int return_code = 0;
	struct LIST(FE_element_field) *list =
		reinterpret_cast<struct LIST(FE_element_field) *>(element_field_list);
	if (element_field && (element_field->field) && list)
	{
		FE_element_field *element_field_2;
		FE_element_field_component **component_1,**component_2;
		if ((element_field_2=FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,
			field)(element_field->field,list))&&
			(component_1=element_field->components)&&
			(component_2=element_field_2->components))
		{
			int i=element_field->field->number_of_components;
			while (!return_code&&(i>0))
			{
				if ((*component_1)&&(*component_2)&&
					((*component_1)->type==(*component_2)->type)&&
					((*component_1)->basis==(*component_2)->basis)&&
					((*component_1)->modify==(*component_2)->modify))
				{
					int number_of_xi_coordinates = 0;
					FE_basis_get_dimension((*component_1)->basis, &number_of_xi_coordinates);
					switch ((*component_1)->type)
					{
						case STANDARD_NODE_TO_ELEMENT_MAP:
						{
							int j = (*component_1)->map.standard_node_based.number_of_nodes;
							Standard_node_to_element_map **standard_node_map_1 =
								(*component_1)->map.standard_node_based.node_to_element_maps;
							Standard_node_to_element_map **standard_node_map_2 =
								(*component_2)->map.standard_node_based.node_to_element_maps;
							if (((*component_2)->map.standard_node_based.number_of_nodes == j) &&
								(standard_node_map_1) && (standard_node_map_2))
							{
								while (j > 0)
								{
									if (Standard_node_to_element_maps_match(*standard_node_map_1, /*info_1*/0,
										*standard_node_map_2, /*info_2*/0, /*scale_factor_offset_2_to_1*/0))
									{
										++standard_node_map_1;
										++standard_node_map_2;
										--j;
									}
									else
									{
										return_code = 1;
										break;
									}
								}
							}
							else
							{
								return_code=1;
							}
						} break;
						case GENERAL_ELEMENT_MAP:
						{
							int numberOfMaps = (*component_1)->map.general_map_based.number_of_maps;
							if (numberOfMaps != (*component_2)->map.general_map_based.number_of_maps)
							{
								return 1;
							}
							ElementDOFMap **maps1 = (*component_1)->map.general_map_based.maps;
							ElementDOFMap **maps2 = (*component_2)->map.general_map_based.maps;
							for (int j = 0; j < numberOfMaps; ++j)
							{
								if (!maps1[j]->matches(maps2[j]))
								{
									return 1;
								}
							}
						} break;
						case ELEMENT_GRID_MAP:
						{
							int *number_in_xi_1,*number_in_xi_2;
							if (((*component_1)->map.element_grid_based.value_index==
								(*component_2)->map.element_grid_based.value_index)&&
								(number_in_xi_1=
								(*component_1)->map.element_grid_based.number_in_xi)&&
								(number_in_xi_2=
								(*component_2)->map.element_grid_based.number_in_xi))
							{
								int j=number_of_xi_coordinates;
								while (!return_code&&(j>0))
								{
									if (*number_in_xi_1== *number_in_xi_2)
									{
										number_in_xi_1++;
										number_in_xi_2++;
										j--;
									}
									else
									{
										return_code=1;
									}
								}
							}
							else
							{
								return_code=1;
							}
						} break;
					}
					component_1++;
					component_2++;
					i--;
				}
				else
				{
					return_code=1;
				}
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_not_in_list.  Invalid argument(s)");
		return_code=1;
	}
	return (return_code);
}

static int FE_element_field_add_to_list_no_field_duplication(
	struct FE_element_field *element_field, void *element_field_list_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Adds <element_field> to <element_field_list>, but fails and reports an error if
the field in <element_field> is already used in the list.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_element_field) *element_field_list;

	ENTER(FE_element_field_add_to_list_no_field_duplication);
	if (element_field && (element_field->field) && (element_field_list =
		(struct LIST(FE_element_field) *)element_field_list_void))
	{
		if (FIND_BY_IDENTIFIER_IN_LIST(FE_element_field, field)(
			element_field->field, element_field_list))
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_add_to_list_no_field_duplication.  "
				"Field %s is used more than once in element field list",
				element_field->field->name);
			return_code = 0;
		}
		else
		{
			if (ADD_OBJECT_TO_LIST(FE_element_field)(element_field,
				element_field_list))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_add_to_list_no_field_duplication.  "
					"Could not add field %s to list",
					element_field->field->name);
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_add_to_list_no_field_duplication.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_add_to_list_no_field_duplication */

/**
 * Returns true if <element_field_1> and <element_field_2> produce equivalent
 * results with <info_1> and <info_2>, respectively.
 */
static bool FE_element_fields_match(struct FE_element_field *element_field_1,
	struct FE_element_node_scale_field_info *info_1,
	struct FE_element_field *element_field_2,
	struct FE_element_node_scale_field_info *info_2)
{
	if (element_field_1 && info_1 && element_field_2 && info_2)
	{
		FE_field *field = element_field_1->field;
		if (field && (element_field_2->field == field))
		{
			int number_of_components = get_FE_field_number_of_components(field);
			FE_element_field_component **component_1 = element_field_1->components;
			FE_element_field_component **component_2 = element_field_2->components;
			if (component_1 && component_2)
			{
				// only GENERAL_FE_FIELD has components to check
				if (GENERAL_FE_FIELD == field->fe_field_type)
				{
					for (int i = number_of_components; (0 < i); i--)
					{
						if (FE_element_field_components_match(*component_1, info_1,
							*component_2, info_2))
						{
							component_1++;
							component_2++;
						}
						else
							return false;
					}
				}
				return true;
			}
		}
	}
	return false;
}

#if !defined (WINDOWS_DEV_FLAG)
/***************************************************************************//**
 * Outputs details of how field is defined at element.
 */
static int list_FE_element_field(struct FE_element *element,
	struct FE_field *field, void *dummy_user_data)
{
	char *component_name;
	const char *type_string;
	int i,j,k,*nodal_value_index,*number_in_xi,
		number_of_components,number_of_nodal_values,
		number_of_xi_coordinates,return_code,*scale_factor_index;
	struct FE_element_field *element_field;
	struct FE_element_field_component **element_field_component;
	struct Standard_node_to_element_map **node_to_element_map;

	ENTER(list_FE_element_field);
	USE_PARAMETER(dummy_user_data);
	if (element && field)
	{
		element_field = (struct FE_element_field *)NULL;
		if (element->fields)
		{
			element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
				field, element->fields->element_field_list);
		}
		if (element_field && element_field->components)
		{
			return_code=1;
			display_message(INFORMATION_MESSAGE,"  %s",field->name);
			if (NULL != (type_string=ENUMERATOR_STRING(CM_field_type)(field->cm_field_type)))
			{
				display_message(INFORMATION_MESSAGE,", %s",type_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"list_FE_element_field.  Invalid CM field type");
				return_code=0;
			}
			if (NULL != (type_string=ENUMERATOR_STRING(Coordinate_system_type)(
				field->coordinate_system.type)))
			{
				display_message(INFORMATION_MESSAGE,", %s",type_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"list_FE_element_field.  Invalid field coordinate system");
				return_code=0;
			}
			number_of_components=field->number_of_components;
			display_message(INFORMATION_MESSAGE,", #Components=%d\n",
				number_of_components);
			element_field_component=element_field->components;
			i=0;
			while (return_code&&(i<number_of_components))
			{
				display_message(INFORMATION_MESSAGE,"    ");
				if (NULL != (component_name=get_FE_field_component_name(field,i)))
				{
					display_message(INFORMATION_MESSAGE,component_name);
					DEALLOCATE(component_name);
				}
				if (GENERAL_FE_FIELD==field->fe_field_type)
				{
					if (*element_field_component)
					{
						display_message(INFORMATION_MESSAGE,".  ");
						char *basis_string = FE_basis_get_description_string((*element_field_component)->basis);
						display_message(INFORMATION_MESSAGE, basis_string);
						DEALLOCATE(basis_string);
						if ((*element_field_component)->modify)
						{
							display_message(INFORMATION_MESSAGE,", modify");
						}
						else
						{
							display_message(INFORMATION_MESSAGE,", no modify");
						}
						switch ((*element_field_component)->type)
						{
							case STANDARD_NODE_TO_ELEMENT_MAP:
							{
								display_message(INFORMATION_MESSAGE,", standard node based\n");
								if (NULL != (node_to_element_map=(*element_field_component)->map.
									standard_node_based.node_to_element_maps))
								{
									j=(*element_field_component)->map.standard_node_based.
										number_of_nodes;
									while (j>0)
									{
										if (*node_to_element_map)
										{
											number_of_nodal_values=(*node_to_element_map)->
												number_of_nodal_values;
											display_message(INFORMATION_MESSAGE,"      %d.  #Values=%d\n",
												(*node_to_element_map)->node_index + 1,
												number_of_nodal_values);
											if (NULL != (nodal_value_index=(*node_to_element_map)->nodal_value_indices))
											{
												display_message(INFORMATION_MESSAGE,"        Value indices:");
												for (k = number_of_nodal_values; 0 < k; --k)
												{
													display_message(INFORMATION_MESSAGE," %d", *nodal_value_index + 1);
													nodal_value_index++;
												}
												display_message(INFORMATION_MESSAGE,"\n");
											}
											FE_nodal_value_type *nodal_value_type = (*node_to_element_map)->nodal_value_types;
											int *nodal_version = (*node_to_element_map)->nodal_versions;
											if ((nodal_value_type) && (nodal_version))
											{
												display_message(INFORMATION_MESSAGE,"        Value types (Versions > 1):");
												for (k = number_of_nodal_values; 0 < k; --k)
												{
													display_message(INFORMATION_MESSAGE, " %s",
														ENUMERATOR_STRING(FE_nodal_value_type)(*nodal_value_type));
													if (*nodal_version > 0)
														display_message(INFORMATION_MESSAGE, "(%d)", *nodal_version + 1);
													++nodal_value_type;
													++nodal_version;
												}
												display_message(INFORMATION_MESSAGE,"\n");
											}
											if (NULL != (scale_factor_index=(*node_to_element_map)->scale_factor_indices))
											{
												display_message(INFORMATION_MESSAGE,"        Scale factor indices:");
												for (k = number_of_nodal_values; 0 < k; --k)
												{
													display_message(INFORMATION_MESSAGE, " %d", *scale_factor_index + 1);
													scale_factor_index++;
												}
												display_message(INFORMATION_MESSAGE,"\n");
											}
										}
										j--;
										node_to_element_map++;
									}
								}
							} break;
							case GENERAL_ELEMENT_MAP:
							{
								display_message(INFORMATION_MESSAGE,", general map based\n");
								// GRC need to complete.
							} break;
							case ELEMENT_GRID_MAP:
							{
								display_message(INFORMATION_MESSAGE,", grid based\n");
								number_in_xi=(*element_field_component)->map.element_grid_based.
									number_in_xi;
								number_of_xi_coordinates = 0;
								FE_basis_get_dimension((*element_field_component)->basis, &number_of_xi_coordinates);
								display_message(INFORMATION_MESSAGE,"      ");
								for (j=0;j<number_of_xi_coordinates;j++)
								{
									if (j>0)
									{
										display_message(INFORMATION_MESSAGE,", ");
									}
									display_message(INFORMATION_MESSAGE,"#xi%d=%d",j+1,number_in_xi[j]);
								}
								display_message(INFORMATION_MESSAGE,"\n");
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"list_FE_element_field.  Missing element field component");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"\n");
				}
				element_field_component++;
				i++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"list_FE_element_field.  Field %s is not defined at element",
				field->name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_FE_element_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_FE_element_field */
#endif /* !defined (WINDOWS_DEV_FLAG) */

static int FE_element_field_private_get_component_FE_basis(
	struct FE_element_field *element_field, int component_number,
	struct FE_basis **fe_basis)
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION :
If <element_field> is standard node based, returns the <fe_basis> used for
<component_number>.
==============================================================================*/
{
	int return_code;
	struct FE_element_field_component *component;

	ENTER(FE_element_field_private_get_component_FE_basis);
	return_code = 0;
	if (element_field && element_field->field && fe_basis &&
		(0 <= component_number) &&
		(component_number < element_field->field->number_of_components))
	{
		*fe_basis = (struct FE_basis *)NULL;
		/* only GENERAL_FE_FIELD has components and can be grid-based */
		if (GENERAL_FE_FIELD == element_field->field->fe_field_type)
		{
			/* get first field component */
			if (element_field->components &&
				(component = element_field->components[component_number]))
			{
				if (component->basis)
				{
					*fe_basis = component->basis;
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_element_field_private_get_component_FE_basis.  "
						"Field does not have an FE_basis.");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_private_get_component_FE_basis.  "
					"Missing element field component");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_private_get_component_FE_basis.  "
				"Field is not general, not grid-based");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_private_get_component_FE_basis.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_private_get_component_FE_basis */

/***************************************************************************//**
 * If element_field is for a coordinate field, set it at supplied address if
 * currently none or name is alphabetically less than current field.
 * @param coordinate_field_address_void  struct FE_field **.
 */
static int FE_element_field_get_first_coordinate_field(struct FE_element_field
	*element_field, void *coordinate_field_address_void)
{
	int return_code;
	struct FE_field **coordinate_field_address, *field;

	ENTER(FE_element_field_get_first_coordinate_field);
	coordinate_field_address = (struct FE_field **)coordinate_field_address_void;
	if (element_field && coordinate_field_address)
	{
		return_code = 1;
		field = element_field->field;
		if (FE_field_is_coordinate_field(field, (void *)NULL))
		{
			if ((NULL == *coordinate_field_address) ||
				(strcmp(field->name, (*coordinate_field_address)->name) < 0))
			{
				*coordinate_field_address = field;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_get_first_coordinate_field.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int FE_element_field_is_anatomical_fibre_field(
	struct FE_element_field *element_field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
Returns a non-zero if the <element_field> is for a anatomical fibre field.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_is_anatomical_fibre_field);
	return_code=0;
	if (element_field)
	{
		return_code=FE_field_is_anatomical_fibre_field(element_field->field,
			dummy_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_is_anatomical_fibre_field. Invalid argument");
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_is_anatomical_fibre_field */

static int for_FE_field_at_element_iterator(
	struct FE_element_field *element_field,void *iterator_and_data_void)
/*******************************************************************************
LAST MODIFIED : 5 October 1999

DESCRIPTION :
FE_element_field iterator for for_each_FE_field_at_element.
==============================================================================*/
{
	int return_code;
	struct FE_element_field_iterator_and_data *iterator_and_data;

	ENTER(for_FE_field_at_element_iterator);
	if (element_field&&(iterator_and_data=
		(struct FE_element_field_iterator_and_data *)iterator_and_data_void)&&
		iterator_and_data->iterator)
	{
		return_code=(iterator_and_data->iterator)(iterator_and_data->element,
			element_field->field,iterator_and_data->user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_FE_field_at_element_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* for_FE_field_at_element_iterator */

/**
 * The data needed to merge an element_field into the list.  This structure is
 * local to this module.
 */
struct FE_element_field_lists_merge_data
{
	struct LIST(FE_element_field) *list;
	/* node_scale_field_info for merged elements in above <list> */
	struct FE_element_node_scale_field_info *merge_info;
	/* node_scale_field_info for all element fields passed directly to
		 merge_FE_element_field_into_list function */
	struct FE_element_node_scale_field_info *source_info;
	/* accumulating size of values_storage in the element. Incremented to fit
		 any grid-based element fields added by merge_FE_element_field_into_list */
	int values_storage_size;
}; /* struct FE_element_field_lists_merge_data */

/**
 * Merges the <new_element_field> into the <list>. The <new_element_field>
 * references nodes, scale factors and values relative to the <source_info>.
 * If an element field already exists in <list> for that field, this function
 * checks <new_element_field> refers to the same nodes and equivalent scale factor
 * sets and values_storage in <merge_info>.
 * If no such element field exists currently, a new one is constructed that refers
 * to nodes, scale factors and values in the <merge_info> in a compatible way to
 * <new_element_field>, and this is added to the <list>.
 * For new grid-based fields this function increments the values_storage_size in
 * the <data> to fit the new grid values and references them in the element field
 * constructed for it.
 * No values_storage arrays are allocated or copied by this function.
 */
static int merge_FE_element_field_into_list(
	struct FE_element_field *new_element_field, void *data_void)
{
	int i, j, *new_number_in_xi, new_values_storage_size,
		node_index, *number_in_xi, number_of_values,
		return_code;
	struct FE_element_field_component **component, **new_component;
	struct FE_element_field_lists_merge_data *data;
	struct FE_element_node_scale_field_info *merge_info, *source_info;
	struct FE_field *field;
	struct FE_node *new_node, **node;
	struct Standard_node_to_element_map **new_standard_node_map,
		**standard_node_map;

	if (new_element_field && (field = new_element_field->field) &&
		(data = (struct FE_element_field_lists_merge_data *)data_void) &&
		data->list)
	{
		merge_info = data->merge_info;
		source_info = data->source_info;

		return_code = 1;
		/* check if the new element field is in the existing list */
		FE_element_field *element_field =
			FIND_BY_IDENTIFIER_IN_LIST(FE_element_field, field)(field, data->list);
		if (element_field)
		{
			/* only GENERAL_FE_FIELD has components to check for merge */
			if (GENERAL_FE_FIELD == field->fe_field_type)
			{
				/* must have merge_info and source_info for general */
				if (merge_info && source_info)
				{
					/* check the new element field for consistency */
					if ((component = element_field->components) &&
						(new_component = new_element_field->components))
					{
						/* check each component */
						i = field->number_of_components;
						while (return_code && (i > 0))
						{
							if (FE_element_field_components_match(*component, merge_info,
								*new_component, source_info))
							{
								component++;
								new_component++;
								i--;
							}
							else
							{
								// replace if different
								REMOVE_OBJECT_FROM_LIST(FE_element_field)(element_field, data->list);
								element_field = 0;
								break;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"merge_FE_element_field_into_list.  Invalid element field");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"merge_FE_element_field_into_list.  "
						"Missing merge or source node field info");
					return_code = 0;
				}
			}
		}
		if (!element_field)
		{
			element_field = CREATE(FE_element_field)(field);
			if (element_field)
			{
				/* only GENERAL_FE_FIELD has components to copy for merge */
				if (GENERAL_FE_FIELD == field->fe_field_type)
				{
					/* must have merge_info and source_info for general */
					if (merge_info && source_info)
					{
						new_component = new_element_field->components;
						i = field->number_of_components;
						component = element_field->components;
						while (return_code && (i > 0))
						{
							if ((*new_component) && ((*new_component)->basis))
							{
								switch ((*new_component)->type)
								{
									case STANDARD_NODE_TO_ELEMENT_MAP:
									{
										// since scale factor indices are absolute in the element
										// need to find position of scale factor set in source
										// and merged data. Future: make indices relative to set!
										int scale_factor_offset = 0;
										int source_number_of_scale_factors = 0;
										int merge_number_of_scale_factors = 0;
										if ((*new_component)->get_scale_factor_set())
										{
											int source_scale_factor_set_offset = source_info->getScaleFactorSetOffset(
												(*new_component)->get_scale_factor_set(), source_number_of_scale_factors);
											int merge_scale_factor_set_offset = merge_info->getScaleFactorSetOffset(
												(*new_component)->get_scale_factor_set(), merge_number_of_scale_factors);
											scale_factor_offset = merge_scale_factor_set_offset - source_scale_factor_set_offset;
										}
										if (source_number_of_scale_factors != merge_number_of_scale_factors)
										{
											display_message(ERROR_MESSAGE,
												"merge_FE_element_field_into_list.  Different numbers of scale factors in sets");
											return_code = 0;
											break;
										}
										if ((new_standard_node_map = (*new_component)->map.
											standard_node_based.node_to_element_maps) &&
											((j = (*new_component)->map.standard_node_based.
												number_of_nodes) > 0) &&
											(*component = CREATE(FE_element_field_component)(
												STANDARD_NODE_TO_ELEMENT_MAP, j,
												(*new_component)->basis, (*new_component)->modify)))
										{
											standard_node_map = (*component)->map.standard_node_based.node_to_element_maps;
											while (return_code && (j > 0))
											{
												if (*new_standard_node_map)
												{
													/* check that the new node_index is actually for a
														 real node */
													node_index = (*new_standard_node_map)->node_index;
													if ((0 <= node_index) &&
														(node_index < source_info->number_of_nodes) &&
														source_info->nodes)
													{
														/* determine the node index */
														node = merge_info->nodes;
														new_node = source_info->nodes[node_index];
														/*???RC since using this function in
															define_FE_field_at_element, have to handle case of
															NULL nodes just by using existing node_index */
														if (new_node)
														{
															node_index = 0;
															while ((node_index < merge_info->number_of_nodes)
																&& (*node != new_node))
															{
																node_index++;
																node++;
															}
														}
														if ((*node == new_node) && (*standard_node_map =
															copy_create_Standard_node_to_element_map(*new_standard_node_map)))
														{
															Standard_node_to_element_map_set_node_index(*standard_node_map, node_index);
															if (0 != scale_factor_offset)
																Standard_node_to_element_map_offset_scale_factor_indices(*standard_node_map, scale_factor_offset);
															++standard_node_map;
															++new_standard_node_map;
															--j;
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"merge_FE_element_field_into_list.  "
																"Invalid node or scale factor information");
															return_code = 0;
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"merge_FE_element_field_into_list.  "
															"Node index out of range");
														return_code = 0;
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"merge_FE_element_field_into_list.  "
														"Invalid standard node to element map");
													return_code = 0;
												}
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"merge_FE_element_field_into_list.  "
												"Could not create element field component");
											return_code = 0;
										}
									} break;
									case GENERAL_ELEMENT_MAP:
									{
										int numberOfMaps = (*new_component)->map.general_map_based.number_of_maps;
										*component = CREATE(FE_element_field_component)(
											GENERAL_ELEMENT_MAP, numberOfMaps, (*new_component)->basis,
											(*new_component)->modify);
										ElementDOFMap **sourceMaps = (*new_component)->map.general_map_based.maps;
										ElementDOFMap **maps = (*component)->map.general_map_based.maps;
										for (int i = 0; i < numberOfMaps; ++i)
										{
											maps[i] = sourceMaps[i]->cloneWithNewNodeIndices(merge_info, source_info);
										}
									} break;
									case ELEMENT_GRID_MAP:
									{
										int size;

										if (NULL != (*component = CREATE(FE_element_field_component)(
											ELEMENT_GRID_MAP, 1, (*new_component)->basis,
											(*new_component)->modify)))
										{
											number_in_xi =
												((*component)->map).element_grid_based.number_in_xi;
											new_number_in_xi =
												((*new_component)->map).element_grid_based.number_in_xi;
											number_of_values = 1;
											int number_of_xi_coordinates = 0;
											FE_basis_get_dimension((*component)->basis, &number_of_xi_coordinates);
											for (j = number_of_xi_coordinates; j > 0; j--)
											{
												*number_in_xi = *new_number_in_xi;
												number_of_values *= (*number_in_xi) + 1;
												number_in_xi++;
												new_number_in_xi++;
											}
											size = get_Value_storage_size(
												field->value_type, (struct FE_time_sequence *)NULL);
											new_values_storage_size = size*number_of_values;
											ADJUST_VALUE_STORAGE_SIZE(new_values_storage_size);
											/* point the component to new space after the current
												 data->values_storage_size */
											((*component)->map).element_grid_based.value_index =
												data->values_storage_size;
											/* increase the data->values_storage_size to fit */
											data->values_storage_size += new_values_storage_size;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"merge_FE_element_field_into_list.  "
												"Could not create element field component");
											return_code = 0;
										}
									} break;
								}
								(*component)->set_scale_factor_set((*new_component)->get_scale_factor_set());
								component++;
								new_component++;
								i--;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"merge_FE_element_field_into_list.  "
									"Invalid element field component");
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"merge_FE_element_field_into_list.  "
							"Missing merge or source node field info");
						return_code = 0;
					}
				}
				if ((!return_code) ||
					(!ADD_OBJECT_TO_LIST(FE_element_field)(element_field, data->list)))
				{
					display_message(ERROR_MESSAGE, "merge_FE_element_field_into_list.  "
						"Could not add element field to list");
					return_code = 0;
					DESTROY(FE_element_field)(&element_field);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"merge_FE_element_field_into_list.  Could not create element field");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"merge_FE_element_field_into_list.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int calculate_grid_field_offsets(int element_dimension,
	int top_level_element_dimension, const int *top_level_number_in_xi,
	FE_value *element_to_top_level,int *number_in_xi,int *base_grid_offset,
	int *grid_offset_in_xi)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Calculates the factors for converting a grid position on a element of
<element_dimension> to a top_level_element of <top_level_element_dimension>
with <top_level_number_in_xi>, given affine transformation
<element_to_top_level> which has as many rows as <top_level_element_dimension>
and 1 more column than <element_dimension>, converting xi from element to
top_level as follows:
top_level_xi = b + A xi, with b the first column.
The <number_in_xi> of the element is returned, as is the <base_grid_offset> and
the <grid_offset_in_xi> which make up the grid point number conversion:
eg. top_level_grid_point_number = base_grid_offset +
grid_offset_in_xi[i]*grid_number_in_xi[i] (i summed over element_dimension).
Sets values appropriately if element_dimension = top_level_element_dimension.
==============================================================================*/
{
	FE_value *temp_element_to_top_level;
	int i, next_offset, return_code,
		top_level_grid_offset_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], xi_number;

	ENTER(calculate_grid_field_offsets);
	if ((0<element_dimension)&&(element_dimension<=top_level_element_dimension)&&
		(top_level_element_dimension<=MAXIMUM_ELEMENT_XI_DIMENSIONS)&&
		top_level_number_in_xi&&((element_dimension==top_level_element_dimension)||
			element_to_top_level)&&number_in_xi&&base_grid_offset&&grid_offset_in_xi)
	{
		return_code=1;
		/* clear offsets */
		*base_grid_offset = 0;
		for (i=0;i<element_dimension;i++)
		{
			grid_offset_in_xi[i]=0;
		}
		/* calculate offset in grid_point_number for adjacent points in each xi
			 direction on the top_level_element */
		next_offset = 1;
		for (i = 0; i < top_level_element_dimension; i++)
		{
			if (top_level_number_in_xi[i] > 0)
			{
				top_level_grid_offset_in_xi[i] = next_offset;
				next_offset *= top_level_number_in_xi[i] + 1;
			}
			else
			{
				/* zero offset means linear interpolation gives same result as constant */
				top_level_grid_offset_in_xi[i] = 0;
			}
		}
		if (element_dimension == top_level_element_dimension)
		{
			for (i=0;i<top_level_element_dimension;i++)
			{
				grid_offset_in_xi[i]=top_level_grid_offset_in_xi[i];
				number_in_xi[i]=top_level_number_in_xi[i];
			}
		}
		else
		{
			temp_element_to_top_level=element_to_top_level;
			for (i=0;i<top_level_element_dimension;i++)
			{
				/* a number in the first column indicates either xi decreasing
					 or the direction this is a face/line on */
				if (*temp_element_to_top_level)
				{
					*base_grid_offset +=
						top_level_number_in_xi[i]*top_level_grid_offset_in_xi[i];
				}
				/* find out how (if at all) element xi changes with this
					 field_element xi */
				for (xi_number=0;xi_number<element_dimension;xi_number++)
				{
					if (temp_element_to_top_level[xi_number+1])
					{
						number_in_xi[xi_number] = top_level_number_in_xi[i];
						if (0<temp_element_to_top_level[xi_number+1])
						{
							grid_offset_in_xi[xi_number] = top_level_grid_offset_in_xi[i];
						}
						else
						{
							grid_offset_in_xi[xi_number] = -top_level_grid_offset_in_xi[i];
						}
					}
				}
				temp_element_to_top_level += (element_dimension+1);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_grid_field_offsets.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_grid_field_offsets */

int FE_element_field_values_are_for_element_and_time(
	struct FE_element_field_values *element_field_values,
	struct FE_element *element,FE_value time,struct FE_element *field_element)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Returns true if the <element_field_values> originated from <element>, either
directly or inherited from <field_element>. If <field_element> is NULL no match
is required with the field_element in the <element_field_values>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_values_are_for_element_and_time);
	if (element_field_values&&element)
	{
		return_code=(element_field_values->element==element)&&
			((!field_element)||(element_field_values->field_element==field_element))
			&&((!element_field_values->time_dependent)||
		   (element_field_values->time==time));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_values_are_for_element_and_time.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_values_are_for_element_and_time */

int FE_element_field_values_have_derivatives_calculated(
	struct FE_element_field_values *element_field_values)
/*******************************************************************************
LAST MODIFIED : 10 March 2003

DESCRIPTION :
Returns true if the <element_field_values> are valid for calculating derivatives.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_values_have_derivatives_calculated);
	if (element_field_values)
	{
		if (element_field_values->derivatives_calculated)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_values_have_derivatives_calculated.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_values_have_derivatives_calculated */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(FE_element_field, field, \
	struct FE_field *, compare_FE_field)

DECLARE_INDEXED_LIST_FUNCTIONS(FE_element_field)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(FE_element_field, field, \
	struct FE_field *, compare_FE_field)

/** Data for passing to FE_element_field_copy_for_FE_mesh. */
struct FE_element_field_copy_for_FE_mesh_data
{
	FE_mesh *fe_mesh;
	struct LIST(FE_element_field) *element_field_list;
};

/**
 * Creates a copy of <element_field> using the same named fields, scale factor
 * sets etc. from <fe_region> and adds it to <element_field_list>.
 *Checks fields are equivalent.
 * <data_void> points at a struct FE_element_field_copy_for_FE_mesh_data.
 */
static int FE_element_field_copy_for_FE_mesh(
	struct FE_element_field *element_field, void *data_void)
{
	int i, return_code;
	struct FE_field *equivalent_field;
	struct FE_element_field *copy_element_field;
	struct FE_element_field_component **component, **copy_component;

	ENTER(FE_element_field_copy_for_FE_mesh);
	FE_element_field_copy_for_FE_mesh_data *data =
		static_cast<FE_element_field_copy_for_FE_mesh_data *>(data_void);
	if (element_field && element_field->field && data)
	{
		return_code = 1;
		if (NULL != (equivalent_field = FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(
			element_field->field->name, FE_region_get_FE_field_list(data->fe_mesh->get_FE_region()))))
		{
			if (FE_fields_match_fundamental(element_field->field, equivalent_field))
			{
				if (NULL != (copy_element_field = CREATE(FE_element_field)(equivalent_field)))
				{
					component = element_field->components;
					copy_component = copy_element_field->components;
					for (i = get_FE_field_number_of_components(equivalent_field);
						(0 < i) && return_code; i--)
					{
						if (*component)
						{
							*copy_component = copy_create_FE_element_field_component(*component);
							if (CMZN_OK != FE_element_field_component_switch_FE_mesh(*copy_component, data->fe_mesh))
							{
								return_code = 0;
							}
						}
						component++;
						copy_component++;
					}
					if (return_code)
					{
						if (!ADD_OBJECT_TO_LIST(FE_element_field)(copy_element_field,
							data->element_field_list))
						{
							return_code = 0;
						}
					}
					if (!return_code)
					{
						display_message(ERROR_MESSAGE,
							"FE_element_field_copy_for_FE_mesh.  "
							"Could not copy element field component");
						DESTROY(FE_element_field)(&copy_element_field);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_element_field_copy_for_FE_mesh.  "
						"Could not create element field");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_copy_for_FE_mesh.  "
					"Fields not equivalent");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_copy_for_FE_mesh.  No equivalent field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_copy_for_FE_mesh.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct LIST(FE_element_field) *FE_element_field_list_clone_for_FE_region(
	struct LIST(FE_element_field) *element_field_list, FE_mesh *fe_mesh)
{
	struct LIST(FE_element_field) *return_element_field_list = 0;
	if (element_field_list && fe_mesh)
	{
		struct FE_element_field_copy_for_FE_mesh_data data;
		data.fe_mesh = fe_mesh;
		data.element_field_list = CREATE(LIST(FE_element_field))();
		if (FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
			FE_element_field_copy_for_FE_mesh, (void *)&data,
			element_field_list))
		{
			return_element_field_list = data.element_field_list;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_list_clone_for_FE_region.  Failed");
			DESTROY(LIST(FE_element_field))(&data.element_field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_list_clone_for_FE_region.  Invalid argument(s)");
	}
	return (return_element_field_list);
}

struct FE_element_field_info *CREATE(FE_element_field_info)(
	FE_mesh *fe_mesh,
	struct LIST(FE_element_field) *fe_element_field_list)
{
	struct FE_element_field_info *fe_element_field_info;

	ENTER(CREATE(FE_element_field_info));
	fe_element_field_info = (struct FE_element_field_info *)NULL;
	if (fe_mesh)
	{
		if (ALLOCATE(fe_element_field_info, struct FE_element_field_info, 1))
		{
			fe_element_field_info->element_field_list =
				CREATE(LIST(FE_element_field)());
			/* maintain pointer to the the FE_region this information belongs to.
				 It is not ACCESSed since FE_region is the owning object and it
				 would prevent the FE_region from being destroyed. */
			fe_element_field_info->fe_mesh = fe_mesh;
			fe_element_field_info->access_count = 0;

			if (!(fe_element_field_info->element_field_list &&
				((!fe_element_field_list) ||
					FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
						FE_element_field_add_to_list_no_field_duplication,
						(void *)fe_element_field_info->element_field_list,
						fe_element_field_list))))
			{
				display_message(ERROR_MESSAGE,
					"CREATE(FE_element_field_info).  Unable to build element field list");
				DESTROY(FE_element_field_info)(&fe_element_field_info);
				fe_element_field_info = (struct FE_element_field_info *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_element_field_info).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_element_field_info).  Invalid argument(s)");
	}
	LEAVE;

	return (fe_element_field_info);
} /* CREATE(FE_element_field_info) */

int DESTROY(FE_element_field_info)(
	struct FE_element_field_info **fe_element_field_info_address)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Destroys the FE_element_field_info at *<element_field_info_address>. Frees the
memory for the information and sets <*element_field_info_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_element_field_info *fe_element_field_info;

	ENTER(DESTROY(FE_element_field_info));
	if ((fe_element_field_info_address) &&
		(fe_element_field_info = *fe_element_field_info_address))
	{
		if (0 == fe_element_field_info->access_count)
		{
			DESTROY(LIST(FE_element_field))(
				&(fe_element_field_info->element_field_list));
			DEALLOCATE(*fe_element_field_info_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_element_field_info).  Non-zero access count");
			return_code = 0;
		}
		*fe_element_field_info_address = (struct FE_element_field_info *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_element_field_info).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_element_field_info) */

DECLARE_ACCESS_OBJECT_FUNCTION(FE_element_field_info)

PROTOTYPE_DEACCESS_OBJECT_FUNCTION(FE_element_field_info)
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
Special version of DEACCESS which if the FE_element_field_info access_count
reaches 1 and it has an fe_region member, calls
FE_mesh::remove_FE_element_field_info.
Since the FE_region accesses the info once, this indicates no other object is
using it so it should be flushed from the FE_region. When the owning FE_region
deaccesses the info, it is destroyed in this function.
==============================================================================*/
{
	int return_code;
	struct FE_element_field_info *object;

	ENTER(DEACCESS(FE_element_field_info));
	if (object_address && (object = *object_address))
	{
		(object->access_count)--;
		return_code = 1;
		if (object->access_count <= 1)
		{
			if (1 == object->access_count)
			{
				if (object->fe_mesh)
					return_code = object->fe_mesh->remove_FE_element_field_info(object);
			}
			else
			{
				return_code = DESTROY(FE_element_field_info)(object_address);
			}
		}
		*object_address = (struct FE_element_field_info *)NULL;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(FE_element_field_info) */

PROTOTYPE_REACCESS_OBJECT_FUNCTION(FE_element_field_info)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Special version of REACCESS which if the FE_element_field_info access_count
reaches 1 and it has an fe_region member, calls
FE_mesh::remove_FE_element_field_info.
Since the FE_region accesses the info once, this indicates no other object is
using it so it should be flushed from the FE_region. When the owning FE_region
deaccesses the info, it is destroyed in this function.
==============================================================================*/
{
	int return_code;
	struct FE_element_field_info *current_object;

	ENTER(REACCESS(FE_element_field_info));
	if (object_address)
	{
		return_code = 1;
		if (new_object)
		{
			/* access the new object */
			(new_object->access_count)++;
		}
		if (NULL != (current_object = *object_address))
		{
			/* deaccess the current object */
			(current_object->access_count)--;
			if (current_object->access_count <= 1)
			{
				if (1 == current_object->access_count)
				{
					if (current_object->fe_mesh)
						return_code = current_object->fe_mesh->remove_FE_element_field_info(current_object);
				}
				else
				{
					return_code = DESTROY(FE_element_field_info)(object_address);
				}
			}
		}
		/* point to the new object */
		*object_address = new_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REACCESS(FE_element_field_info).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* REACCESS(FE_element_field_info) */

DECLARE_LIST_FUNCTIONS(FE_element_field_info)

int FE_element_field_info_clear_FE_mesh(
	struct FE_element_field_info *element_field_info, void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	if (element_field_info)
	{
		element_field_info->fe_mesh = 0;
		return 1;
	}
	return 0;
}

int FE_element_field_info_has_FE_field(
	struct FE_element_field_info *element_field_info, void *fe_field_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Returns true if <element_field_info> has an element field for <fe_field>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_info_has_FE_field);
	if (FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
		(struct FE_field *)fe_field_void, element_field_info->element_field_list))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_info_has_FE_field */

int FE_element_field_info_has_empty_FE_element_field_list(
	struct FE_element_field_info *element_field_info, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Returns true if <element_field_info> has no element fields.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_info_has_empty_FE_element_field_list);
	USE_PARAMETER(dummy_void);
	if (element_field_info)
	{
		if (0 == NUMBER_IN_LIST(FE_element_field)(
			element_field_info->element_field_list))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_info_has_empty_FE_element_field_list.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_info_has_empty_FE_element_field_list */

int FE_element_field_info_has_matching_FE_element_field_list(
	struct FE_element_field_info *element_field_info,
	void *element_field_list_void)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Returns true if <element_field_info> has a FE_element_field_list containing all
the same FE_element_fields as <element_field_list>.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_element_field) *element_field_list;

	ENTER(FE_element_field_info_has_matching_FE_element_field_list);
	if (element_field_info && (element_field_list =
		(struct LIST(FE_element_field) *)element_field_list_void))
	{
		if ((NUMBER_IN_LIST(FE_element_field)(element_field_list) ==
			NUMBER_IN_LIST(FE_element_field)(element_field_info->element_field_list)))
		{
			if (FIRST_OBJECT_IN_LIST_THAT(FE_element_field)(
				FE_element_field_not_in_list,
				(void *)(element_field_info->element_field_list),
				element_field_list))
			{
				return_code = 0;
			}
			else
			{
				return_code = 1;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_info_has_matching_FE_element_field_list.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_info_has_matching_FE_element_field_list */

struct LIST(FE_element_field) *FE_element_field_info_get_element_field_list(
	struct FE_element_field_info *fe_element_field_info)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Returns the element field list contained in the <element_field_info>.
==============================================================================*/
{
	struct LIST(FE_element_field) *element_field_list;

	ENTER(FE_element_field_info_get_element_field_list);
	if (fe_element_field_info)
	{
		element_field_list = fe_element_field_info->element_field_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_info_get_element_field_list.  Invalid argument(s)");
		element_field_list = (struct LIST(FE_element_field) *)NULL;
	}
	LEAVE;

	return (element_field_list);
} /* FE_element_field_info_get_element_field_list */

static int FE_element_field_log_FE_field_change(
	struct FE_element_field *element_field, void *fe_field_change_log_void)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
Logs the field in <element_field> as RELATED_OBJECT_CHANGED
in <fe_field_change_log>.
==============================================================================*/
{
	int return_code;
	struct CHANGE_LOG(FE_field) *fe_field_change_log;

	ENTER(FE_element_field_log_FE_field_change);
	if (element_field && (fe_field_change_log =
		(struct CHANGE_LOG(FE_field) *)fe_field_change_log_void))
	{
		return_code = CHANGE_LOG_OBJECT_CHANGE(FE_field)(fe_field_change_log,
			element_field->field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_log_FE_field_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_log_FE_field_change */

int FE_element_field_info_log_FE_field_changes(
	struct FE_element_field_info *fe_element_field_info,
	struct CHANGE_LOG(FE_field) *fe_field_change_log)
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Marks each FE_field in <fe_element_field_info> as RELATED_OBJECT_CHANGED
in <fe_field_change_log>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_info_log_FE_field_changes);
	if (fe_element_field_info && fe_field_change_log)
	{
		return_code = FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
			FE_element_field_log_FE_field_change, (void *)fe_field_change_log,
			fe_element_field_info->element_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_info_log_FE_field_changes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_info_log_FE_field_changes */

struct FE_element_field_copy_values_storage_data
{
	int dimension;
	Value_storage *new_values_storage;
	struct LIST(FE_element_field) *old_element_field_list;
	Value_storage *old_values_storage;
	struct LIST(FE_element_field) *add_element_field_list;
	Value_storage *add_values_storage;
}; /* FE_element_field_copy_values_storage_data */

static int FE_element_field_copy_values_storage(
	struct FE_element_field *new_element_field,	void *copy_data_void)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
If <new_element_field> uses values storage then:

... when <add_element_field_list> and <add_values_storage> provided:
Finds the equivalent element field in the <old_element_field_list> or
<add_element_field_list>, and copies values giving precedence to the latter.
If the element fields have times, the time arrays are allocated once, then the
old values are copied followed by the add values to correctly mere the times.

... when <add_element_field_list> and <add_values_storage> not provided:
Copies the values for <new_element_field> into <new_values_storage> from the
<old_values_storage> with the equivalent element field in
<old_element_field_list>.

Notes:
Assumes <new_values_storage> is already allocated to the appropriate size.
Assumes the only differences between equivalent element fields are in time
version; no checks on this are made here.
<copy_data_void> points at a struct FE_element_field_copy_values_storage_data.

???RC Ignore references to times in the above since not yet implemented; once
they are, should follow pattern of FE_node_field_copy_values_storage.
==============================================================================*/
{
	enum Value_type value_type;
	int cn, i, *number_in_xi, number_of_values, return_code;
	struct FE_field *field;
	struct FE_element_field *add_element_field, *old_element_field;
	struct FE_element_field_component *add_component, *component, *old_component;
	struct FE_element_field_copy_values_storage_data *copy_data;
	Value_storage *destination, *source;

	ENTER(FE_element_field_copy_values_storage);
	if (new_element_field && (field = new_element_field->field) &&
		(new_element_field->components) &&
		(copy_data =
			(struct FE_element_field_copy_values_storage_data *)copy_data_void))
	{
		return_code = 1;
		/* only GENERAL_FE_FIELD with ELEMENT_GRID_MAP has element values */
		if (GENERAL_FE_FIELD == field->fe_field_type)
		{
			old_element_field = NULL;
			add_element_field = NULL;
			for (cn = 0; (cn < field->number_of_components) && return_code; cn++)
			{
				component = new_element_field->components[cn];
				if (component && (ELEMENT_GRID_MAP == component->type))
				{
					if ((NULL == old_element_field) && (NULL == add_element_field))
					{
						old_element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
							field, copy_data->old_element_field_list);
						if (copy_data->add_element_field_list)
						{
							add_element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
								field, copy_data->add_element_field_list);
						}
						else
						{
							add_element_field = (struct FE_element_field *)NULL;
						}
					}
					if (old_element_field || add_element_field)
					{
						if (copy_data->new_values_storage)
						{
							/* destination in new_values_storage according to new_element_field */
							destination = copy_data->new_values_storage +
								component->map.element_grid_based.value_index;
							value_type = field->value_type;
							number_in_xi = component->map.element_grid_based.number_in_xi;
							number_of_values = 1;
							for (i = 0; i < copy_data->dimension; i++)
							{
								number_of_values *= (number_in_xi[i] + 1);
							}
							if (add_element_field)
							{
								if (copy_data->add_values_storage && add_element_field->components &&
									(add_component = add_element_field->components[cn]))
								{
									/* source in add_values_storage according to add_element_field */
									source = copy_data->add_values_storage +
										add_component->map.element_grid_based.value_index;
									return_code = copy_value_storage_array(destination, value_type,
										(struct FE_time_sequence *)NULL, (struct FE_time_sequence *)NULL,
										number_of_values, source, /*optimised_merge*/0);
								}
								else
								{
									return_code = 0;
								}
							}
							else
							{
								if (copy_data->old_values_storage && old_element_field->components &&
									(old_component = old_element_field->components[cn]))
								{
									/* source in old_values_storage according to old_element_field */
									source = copy_data->old_values_storage +
										old_component->map.element_grid_based.value_index;
									return_code = copy_value_storage_array(destination, value_type,
										(struct FE_time_sequence *)NULL, (struct FE_time_sequence *)NULL,
										number_of_values, source, /*optimised_merge*/0);
								}
								else
								{
									return_code = 0;
								}
							}
						}
						else
						{
							return_code = 0;
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"FE_element_field_copy_values_storage.  Unable to copy values");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "FE_element_field_copy_values_storage.  "
							"Could not find equivalent existing element field");
						return_code = 0;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_copy_values_storage.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_copy_values_storage */

/**
 * For each field in <new_element_field_list> requiring values storage, finds the
 * equivalent element field in either <element>, <add_element> or both. If only one
 * of <element> or <add_element> contains an equivalent element field, those values
 * are copied. If there is an equivalent element field in both, behaviour depends
 * on whether the element fields have times:
 * if the element fields have no times, values are taken from <add_element>.
 * if the element fields have times, the times arrays are allocated, then the
 * values at times in <element> are copied, followed by those in <add_element>.
 * Hence, the values in <add_element> take preference over those in <element>.
 * Notes:
 * Must be an equivalent element field at either <element> or <add_element>;
 * <add_element> is optional and used only by merge_FE_element. If NULL then an
 * element field must be found in <element>;
 * Values_storage must already be allocated to the appropriate size but is not
 * assumed to contain any information prior to being filled here;
 * Any objects or arrays referenced in the values_storage are accessed or
 * allocated in the new <values_storage> so <element> and <add_element> are
 * unchanged.
 * It is up to the calling function to have checked that the element fields in
 * <element>, <add_element> and <new_element_field_list> are compatible.
 * ???RC Ignore references to times in the above since not yet implemented; once
 * they are, should follow pattern of merge_FE_node_values_storage.
 */
static int copy_FE_element_values_storage(struct FE_element *element,
	Value_storage *values_storage,
	struct LIST(FE_element_field) *new_element_field_list,
	struct FE_element *add_element)
{
	int dimension, return_code;
	if (element && (dimension = get_FE_element_dimension(element)) &&
		element->fields && new_element_field_list &&
		((!add_element) || (add_element->fields &&
			(get_FE_element_dimension(add_element) == dimension))))
	{
		struct FE_element_field_copy_values_storage_data copy_data;
		copy_data.dimension = dimension;
		copy_data.new_values_storage = values_storage;
		copy_data.old_element_field_list = element->fields->element_field_list;
		if (element->information)
		{
			copy_data.old_values_storage = element->information->values_storage;
		}
		else
		{
			copy_data.old_values_storage = (Value_storage *)NULL;
		}
		if (add_element)
		{
			copy_data.add_element_field_list =
				add_element->fields->element_field_list;
			if (add_element->information)
			{
				copy_data.add_values_storage = add_element->information->values_storage;
			}
			else
			{
				copy_data.add_values_storage = (Value_storage *)NULL;
			}
		}
		else
		{
			copy_data.add_element_field_list = (struct LIST(FE_element_field) *)NULL;
			copy_data.add_values_storage = (Value_storage *)NULL;
		}
		return_code = FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
			FE_element_field_copy_values_storage, (void *)(&copy_data),
			new_element_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"copy_FE_element_values_storage.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int free_element_grid_map_values_storage(
	struct FE_element_field *element_field, void *values_storage_void)
/*******************************************************************************
LAST MODIFIED : 18 October 1999

DESCRIPTION :
If the <element_field> is grid based, finds where the values for its components
are stored in <values_storage> and frees any accesses and dynamic allocations
in them. Only certain value types, eg. arrays, strings, element_xi require this.
==============================================================================*/
{
	enum Value_type value_type;
	int i,j,*number_in_xi,number_of_values,return_code,value_index;
	struct FE_element_field_component **component;
	Value_storage *values_storage;

	ENTER(free_element_grid_map_values_storage);
	if (element_field&&element_field->field&&
		(values_storage=(Value_storage *)values_storage_void))
	{
		return_code=1;
		/* only GENERAL_FE_FIELD has components and can be grid-based */
		if (GENERAL_FE_FIELD==element_field->field->fe_field_type)
		{
			value_type = element_field->field->value_type;
			component=element_field->components;
			for (i=element_field->field->number_of_components;(0<i)&&return_code;i--)
			{
				if (ELEMENT_GRID_MAP==(*component)->type)
				{
					number_in_xi=((*component)->map).element_grid_based.number_in_xi;
					number_of_values=1;
					int number_of_xi_coordinates = 0;
					FE_basis_get_dimension((*component)->basis, &number_of_xi_coordinates);
					for (j = number_of_xi_coordinates; j > 0; j--)
					{
						number_of_values *= (*number_in_xi)+1;
						number_in_xi++;
					}
					value_index=((*component)->map).element_grid_based.value_index;
					return_code=free_value_storage_array(
						values_storage+value_index,value_type,
						(struct FE_time_sequence *)NULL,number_of_values);
				}
				component++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"free_element_grid_map_values_storage.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* free_element_grid_map_values_storage */

FE_element_node_scale_field_info::FE_element_node_scale_field_info() :
	values_storage_size(0),
	values_storage(0),
	number_of_nodes(0),
	nodes(0),
	number_of_scale_factor_sets(0),
	scale_factor_set_identifiers(0),
	numbers_in_scale_factor_sets(0),
	number_of_scale_factors(0),
	scale_factors(0)
{
}

FE_element_node_scale_field_info::~FE_element_node_scale_field_info()
{
	/* free values_storage for grid-based fields, if any */
	if (this->values_storage)
	{
		DEALLOCATE(this->values_storage);
	}
	int i = this->number_of_nodes;
	cmzn_node **node = this->nodes;
	while (i > 0)
	{
		if (*node)
		{
			DEACCESS(FE_node)(node);
		}
		node++;
		i--;
	}
	DEALLOCATE(this->nodes);
	for (i = 0; i < this->number_of_scale_factor_sets; ++i)
	{
		cmzn_mesh_scale_factor_set::deaccess(this->scale_factor_set_identifiers[i]);
	}
	DEALLOCATE(this->scale_factor_set_identifiers);
	DEALLOCATE(this->numbers_in_scale_factor_sets);
	DEALLOCATE(this->scale_factors);
}

void FE_element_node_scale_field_info::destroyDynamic(
	FE_element_node_scale_field_info* &info, FE_element_field_info *field_info)
{
	if (info->values_storage)
	{
		FOR_EACH_OBJECT_IN_LIST(FE_element_field)(free_element_grid_map_values_storage,
			(void *)info->values_storage, field_info->element_field_list);
	}
	destroy(info);
}

FE_element_node_scale_field_info *FE_element_node_scale_field_info::cloneWithoutValuesStorage()
{
	FE_element_node_scale_field_info *cloneInfo = this->create();
	if (0 < this->number_of_nodes)
	{
		ALLOCATE(cloneInfo->nodes, cmzn_node *, this->number_of_nodes);
		if (0 == cloneInfo->nodes)
		{
			FE_element_node_scale_field_info::destroy(cloneInfo);
			return 0;
		}
		cloneInfo->number_of_nodes = this->number_of_nodes;
		for (int i = 0; i < number_of_nodes; i++)
		{
			cloneInfo->nodes[i] = this->nodes[i] ? this->nodes[i]->access() : 0;
		}
	}
	if (CMZN_OK != cloneInfo->setScaleFactorSets(this->number_of_scale_factor_sets,
		this->scale_factor_set_identifiers, this->numbers_in_scale_factor_sets, this->scale_factors))
	{
		FE_element_node_scale_field_info::destroy(cloneInfo);
		return 0;
	}
	return cloneInfo;
}

/**
 * Function used exclusively in merge_FE_element.
 * Creates a new FE_element_node_scale_field_info that contains all the nodes
 * and scale factors from targetInfo and in the same sequence, plus any new
 * ones added from sourceInfo. Checks that the same number of scale factors are
 * used for any scale factor sets in common; returns 0 on any mismatch.
 * values_storage_size and values_storage are zeroed, under the expectation
 * that they will be constructed by merge_FE_element.
 * On successful return the changedScaleFactorSets contains the list of sets
 * whose values have been added or overwritten. It is up to the caller to
 * propagate change messages for all fields using changed scale factors sets.
 * Note: nodes and scale factors can become 'orphaned' by this merge if
 * replacing field definitions i.e. they are stored but unused.
 */
FE_element_node_scale_field_info *FE_element_node_scale_field_info::createMergeWithoutValuesStorage(
	FE_element_node_scale_field_info& targetInfo,
	FE_element_node_scale_field_info& sourceInfo,
	std::vector<cmzn_mesh_scale_factor_set*> &changedExistingScaleFactorSets)
{
	changedExistingScaleFactorSets.clear();
	// get the total number of nodes, starting with those in targetInfo
	int targetNumberOfNodes = targetInfo.number_of_nodes;
	int sourceNumberOfNodes = sourceInfo.number_of_nodes;
	int mergeNumberOfNodes = targetNumberOfNodes;
	FE_node *sourceNode;
	FE_node **targetNodeAddress;
	for (int i = 0; i < sourceNumberOfNodes; i++)
	{
		sourceNode = sourceInfo.nodes[i];
		targetNodeAddress = targetInfo.nodes;
		int j = targetNumberOfNodes;
		while (j && (sourceNode != *targetNodeAddress))
		{
			++targetNodeAddress;
			--j;
		}
		if (!j)
		{
			mergeNumberOfNodes++;
		}
	}

	// get the total number of scale factor sets and scale factors, starting
	// with those in targetInfo. Check sizes of scale factor sets
	int targetNumberOfScaleFactorSets = targetInfo.number_of_scale_factor_sets;
	int sourceNumberOfScaleFactorSets = sourceInfo.number_of_scale_factor_sets;
	int mergeNumberOfScaleFactorSets = targetNumberOfScaleFactorSets;
	int targetNumberOfScaleFactors = targetInfo.number_of_scale_factors;
	int mergeNumberOfScaleFactors = targetNumberOfScaleFactors;
	cmzn_mesh_scale_factor_set *sourceScaleFactorSet;
	cmzn_mesh_scale_factor_set **targetScaleFactorSetAddress;
	for (int i = 0; i < sourceNumberOfScaleFactorSets; i++)
	{
		sourceScaleFactorSet = sourceInfo.scale_factor_set_identifiers[i];
		targetScaleFactorSetAddress = targetInfo.scale_factor_set_identifiers;
		int j = targetNumberOfScaleFactorSets;
		while (j && (sourceScaleFactorSet != *targetScaleFactorSetAddress))
		{
			++targetScaleFactorSetAddress;
			--j;
		}
		if (j)
		{
			if (targetInfo.numbers_in_scale_factor_sets[targetNumberOfScaleFactorSets - j]
				!= sourceInfo.numbers_in_scale_factor_sets[i])
			{
				display_message(ERROR_MESSAGE,
					"FE_element_node_scale_field_info::createMergeWithoutValuesStorage.  "
					"Different numbers of scale factors for scale factor set %s", sourceScaleFactorSet->getName());
				return 0;
			}
		}
		else
		{
			++mergeNumberOfScaleFactorSets;
			mergeNumberOfScaleFactors += sourceInfo.numbers_in_scale_factor_sets[i];
		}
	}

	FE_element_node_scale_field_info *mergeInfo = FE_element_node_scale_field_info::create();
	if (!mergeInfo)
		return 0;

	if (!mergeInfo->setNumberOfNodes(mergeNumberOfNodes))
	{
		FE_element_node_scale_field_info::destroy(mergeInfo);
		return 0;
	}
	for (int j = 0; j < targetNumberOfNodes; j++)
	{
		mergeInfo->nodes[j] = targetInfo.nodes[j]->access();
	}
	if (targetNumberOfNodes < mergeNumberOfNodes)
	{
		/* extract the new nodes from the source */
		mergeNumberOfNodes = targetNumberOfNodes;
		for (int i = 0; i < sourceNumberOfNodes; i++)
		{
			sourceNode = sourceInfo.nodes[i];
			targetNodeAddress = targetInfo.nodes;
			int j = targetNumberOfNodes;
			while (j && (sourceNode != *targetNodeAddress))
			{
				++targetNodeAddress;
				--j;
			}
			if (!j)
			{
				mergeInfo->setNode(mergeNumberOfNodes, sourceNode);
				++mergeNumberOfNodes;
			}
		}
	}

	if (0 < mergeNumberOfScaleFactorSets)
	{
		ALLOCATE(mergeInfo->scale_factor_set_identifiers, cmzn_mesh_scale_factor_set *, mergeNumberOfScaleFactorSets);
		ALLOCATE(mergeInfo->numbers_in_scale_factor_sets, int, mergeNumberOfScaleFactorSets);
		ALLOCATE(mergeInfo->scale_factors, FE_value, mergeNumberOfScaleFactors);
		if (!(mergeInfo->scale_factor_set_identifiers &&
			mergeInfo->numbers_in_scale_factor_sets &&
			mergeInfo->scale_factors))
		{
			DEALLOCATE(mergeInfo->scale_factor_set_identifiers);
			FE_element_node_scale_field_info::destroy(mergeInfo);
			return 0;
		}
		// copy the scale factor sets from targetInfo
		for (int j = 0; j < targetNumberOfScaleFactorSets; ++j)
		{
			mergeInfo->scale_factor_set_identifiers[j] = targetInfo.scale_factor_set_identifiers[j]->access();
			mergeInfo->numbers_in_scale_factor_sets[j] = targetInfo.numbers_in_scale_factor_sets[j];
		}
		// copy all scale factors from targetInfo, even though
		// some may be overwritten by matching sets from sourceInfo
		if (0 < targetNumberOfScaleFactors)
		{
			memcpy(mergeInfo->scale_factors, targetInfo.scale_factors,
				targetNumberOfScaleFactors*sizeof(FE_value));
		}
		// incorporate new scale factor sets from sourceInfo. Compare old and new
		// scale factors for existing sets and if changing, use values from the
		// sourceInfo and remember the scale factor set identifier
		mergeNumberOfScaleFactorSets = targetNumberOfScaleFactorSets;
		mergeNumberOfScaleFactors = targetNumberOfScaleFactors;
		FE_value *source_scale_factor_position = sourceInfo.scale_factors;
		for (int i = 0; i < sourceNumberOfScaleFactorSets; i++)
		{
			sourceScaleFactorSet = sourceInfo.scale_factor_set_identifiers[i];
			targetScaleFactorSetAddress = mergeInfo->scale_factor_set_identifiers;
			int *tmp_numbers_in_scale_factors_sets = mergeInfo->numbers_in_scale_factor_sets;
			int source_number_in_scale_factor_set = sourceInfo.numbers_in_scale_factor_sets[i];
			FE_value *scale_factor_position = mergeInfo->scale_factors;
			int j = targetNumberOfScaleFactorSets;
			while (j && (sourceScaleFactorSet != *targetScaleFactorSetAddress))
			{
				scale_factor_position += *tmp_numbers_in_scale_factors_sets;
				++tmp_numbers_in_scale_factors_sets;
				++targetScaleFactorSetAddress;
				--j;
			}
			bool copy_scale_factors = (0 < source_number_in_scale_factor_set);
			if (j)
			{
				// scale factors only transferred if changing: compare array
				if (memcmp(scale_factor_position, source_scale_factor_position,
					source_number_in_scale_factor_set*sizeof(FE_value)))
				{
					changedExistingScaleFactorSets.push_back(sourceScaleFactorSet);
				}
				else
				{
					copy_scale_factors = false;
				}
			}
			else
			{
				// put new scale factors after end of currently used scale factors
				scale_factor_position = mergeInfo->scale_factors + mergeNumberOfScaleFactors;
				mergeInfo->scale_factor_set_identifiers[mergeNumberOfScaleFactorSets] = sourceScaleFactorSet->access();
				mergeInfo->numbers_in_scale_factor_sets[mergeNumberOfScaleFactorSets] = source_number_in_scale_factor_set;
				++mergeNumberOfScaleFactorSets;
				mergeNumberOfScaleFactors += source_number_in_scale_factor_set;
			}
			if (copy_scale_factors)
			{
				memcpy(scale_factor_position, source_scale_factor_position,
					source_number_in_scale_factor_set*sizeof(FE_value));
			}
			source_scale_factor_position += source_number_in_scale_factor_set;
		}
		mergeInfo->number_of_scale_factor_sets = mergeNumberOfScaleFactorSets;
		mergeInfo->number_of_scale_factors = mergeNumberOfScaleFactors;
	}
	return (mergeInfo);
}

struct Copy_element_grid_map_data
{
	Value_storage *destination_values_storage,*source_values_storage;
}; /* struct Copy_element_grid_map_data */

static int copy_element_grid_map_values_storage(
	struct FE_element_field *element_field,void *copy_element_grid_map_data_void)
/*******************************************************************************
LAST MODIFIED : 18 October 1999

DESCRIPTION :
If the <element_field> is grid based, finds where the values for its components
are stored in <source_values_storage> and copies them to the same location in
<destination_values_storage>. Assumes <destination_values_storage> has already
been allocated but is uninitialised.
==============================================================================*/
{
	enum Value_type value_type;
	int i,j,*number_in_xi,number_of_values,return_code,value_index;
	struct Copy_element_grid_map_data *copy_data;
	struct FE_element_field_component **component;

	ENTER(copy_element_grid_map_values_storage);
	if (element_field&&element_field->field&&(copy_data=
		(struct Copy_element_grid_map_data *)copy_element_grid_map_data_void))
	{
		return_code=1;
		/* only GENERAL_FE_FIELD has components and can be grid-based */
		if (GENERAL_FE_FIELD==element_field->field->fe_field_type)
		{
			value_type = element_field->field->value_type;
			component=element_field->components;
			for (i=element_field->field->number_of_components;(0<i)&&return_code;i--)
			{
				if (ELEMENT_GRID_MAP==(*component)->type)
				{
					number_in_xi=((*component)->map).element_grid_based.number_in_xi;
					number_of_values=1;
					int number_of_xi_coordinates = 0;
					FE_basis_get_dimension((*component)->basis, &number_of_xi_coordinates);
					for (j = number_of_xi_coordinates; j > 0; j--)
					{
						number_of_values *= (*number_in_xi)+1;
						number_in_xi++;
					}
					value_index=((*component)->map).element_grid_based.value_index;
					if (copy_data->destination_values_storage&&
						copy_data->source_values_storage)
					{
						return_code=copy_value_storage_array(
							copy_data->destination_values_storage+value_index,
							value_type,(struct FE_time_sequence *)NULL,
							(struct FE_time_sequence *)NULL,number_of_values,
							copy_data->source_values_storage+value_index, /*optimised_merge*/0);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"copy_element_grid_map_values_storage.  "
							"Missing source or destination values_storage");
						return_code=0;
					}
				}
				component++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"copy_element_grid_map_values_storage.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* copy_element_grid_map_values_storage */

FE_element_node_scale_field_info *FE_element_node_scale_field_info::clone(
	FE_element_field_info *field_info)
{
	if (0 < this->values_storage_size)
	{
		// not sure why this check is necessary
		struct Check_element_grid_map_values_storage_data check_grid_data;
		check_grid_data.check_sum = 0;
		check_grid_data.values_storage_size = this->values_storage_size;
		if (!FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
			check_element_grid_map_values_storage, (void *)&check_grid_data,
			field_info->element_field_list) ||
			(check_grid_data.check_sum != this->values_storage_size))
		{
			display_message(ERROR_MESSAGE,
				"FE_element_node_scale_field_info::clone.  Inconsistent element values");
			return 0;
		}
	}
	FE_element_node_scale_field_info *cloneInfo = this->cloneWithoutValuesStorage();
	if (!cloneInfo)
		return 0;
	if (0 < this->values_storage_size)
	{
		ALLOCATE(cloneInfo->values_storage, Value_storage, this->values_storage_size);
		if (!cloneInfo->values_storage)
		{
			FE_element_node_scale_field_info::destroy(cloneInfo);
			return 0;
		}
		cloneInfo->values_storage_size = this->values_storage_size;
		// copy values storage including dynamic arrays
		Copy_element_grid_map_data copy_element_grid_map_data;
		copy_element_grid_map_data.destination_values_storage = cloneInfo->values_storage;
		copy_element_grid_map_data.source_values_storage = this->values_storage;
		if (!FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
			copy_element_grid_map_values_storage,
			&copy_element_grid_map_data,field_info->element_field_list))
		{
			display_message(ERROR_MESSAGE,
				"FE_element_node_scale_field_info::clone.  Failed to copy element values");
			FE_element_node_scale_field_info::destroy(cloneInfo);
			return 0;
		}
	}
	return cloneInfo;
}

int FE_element_node_scale_field_info::setNumberOfNodes(int numberOfNodesIn)
{
	if (0 > number_of_nodes)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	if (numberOfNodesIn < this->number_of_nodes)
	{
		display_message(ERROR_MESSAGE,
			"FE_element_node_scale_field::setNumberOfNodes.  "
			"Cannot reduce the number of nodes");
		return CMZN_ERROR_ARGUMENT;
	}
	if (numberOfNodesIn != this->number_of_nodes)
	{
		struct FE_node **temp_nodes;
		if (REALLOCATE(temp_nodes, this->nodes, struct FE_node *, numberOfNodesIn))
		{
			this->nodes = temp_nodes;
			for (int i = this->number_of_nodes; i < numberOfNodesIn; i++)
			{
				this->nodes[i] = 0;
			}
			this->number_of_nodes = numberOfNodesIn;
		}
		else
		{
			return CMZN_ERROR_MEMORY;
		}
	}
	return CMZN_OK;
}

int FE_element_node_scale_field_info::setNode(int nodeNumber, cmzn_node *node)
{
	if ((0 <= nodeNumber) && (nodeNumber < this->number_of_nodes))
	{
		REACCESS(FE_node)(&(this->nodes[nodeNumber]), node);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int FE_element_node_scale_field_info::setScaleFactorSets(int numberOfScaleFactorSetsIn,
	cmzn_mesh_scale_factor_set **scaleFactorSetIdentifiersIn,
	int *numbersInScaleFactorSetsIn, FE_value *scaleFactorsIn)
{
	if ((0 == numberOfScaleFactorSetsIn) || ((0 < numberOfScaleFactorSetsIn) &&
		scaleFactorSetIdentifiersIn && numbersInScaleFactorSetsIn))
	{
		/* check scale factor set identifiers and numbers and count number of scale factors */
		int scaleFactorsCount = 0;
		for (int i = 0; i < numberOfScaleFactorSetsIn; ++i)
		{
			if (scaleFactorSetIdentifiersIn[i] && (0 < numbersInScaleFactorSetsIn[i]))
			{
				scaleFactorsCount += numbersInScaleFactorSetsIn[i];
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_node_scale_field_info::setScaleFactorSets.  "
					"Invalid scale factor set identifier or number");
				return CMZN_ERROR_ARGUMENT;
			}
		}
		if (0 < this->getNumberOfScaleFactorSets())
		{
			display_message(ERROR_MESSAGE,
				"FE_element_node_scale_field_info::setScaleFactorSets.  "
				"Number of scale factor sets is already set");
			return CMZN_ERROR_ARGUMENT;
		}
		if (0 < numberOfScaleFactorSetsIn)
		{
			cmzn_mesh_scale_factor_set **tempIdentifiers = 0;
			int *tempNumbers = 0;
			FE_value *tempValues = 0;
			ALLOCATE(tempIdentifiers, cmzn_mesh_scale_factor_set *, numberOfScaleFactorSetsIn);
			ALLOCATE(tempNumbers, int, numberOfScaleFactorSetsIn);
			ALLOCATE(tempValues, FE_value, scaleFactorsCount);
			if (!(tempIdentifiers && tempNumbers && tempValues))
			{
				return CMZN_ERROR_MEMORY;
			}
			this->number_of_scale_factor_sets = numberOfScaleFactorSetsIn;
			this->scale_factor_set_identifiers = tempIdentifiers;
			this->numbers_in_scale_factor_sets = tempNumbers;
			for (int i = 0; i < numberOfScaleFactorSetsIn; i++)
			{
				this->scale_factor_set_identifiers[i] = scaleFactorSetIdentifiersIn[i]->access();
				this->numbers_in_scale_factor_sets[i] = numbersInScaleFactorSetsIn[i];
			}
			this->number_of_scale_factors = scaleFactorsCount;
			this->scale_factors = tempValues;
			if (scaleFactorsIn)
			{
				memcpy(this->scale_factors, scaleFactorsIn, scaleFactorsCount*sizeof(FE_value));
			}
			else
			{
				for (int i = 0; i < scaleFactorsCount; i++)
				{
					this->scale_factors[i] = FE_VALUE_INITIALIZER;
				}
			}
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

struct FE_element_shape *CREATE(FE_element_shape)(int dimension,
	const int *type, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 8 July 2003

DESCRIPTION :
Searchs the <element_shape_list> for a shape with the specified <dimension> and
<type>.  If one is not found, a shape is created (with <type> duplicated) and
added to the list.  The shape is returned.
<type> is analogous to the basis type array, except that the entries are 0 or 1.
If <type> is omitted an "unspecified" shape of the given <dimension> is
returned. An element with such a shape may not have fields defined on it until
it is given a proper shape.
==============================================================================*/
{
	FE_value *face_to_element, *face_normals, weight;
	const int *type_entry;
	int dimension_2,*face,i,j,k,k_set,l,linked_coordinate,linked_k,linked_offset,
		*linked_offsets,no_error,number_of_faces,number_of_polygon_verticies,offset,
		*shape_type,simplex_coordinate,simplex_dimension,temp_int, xi_coordinate;
	struct FE_element_shape *shape;

	ENTER(CREATE(FE_element_shape));
	shape = (struct FE_element_shape *)NULL;
/*???debug */
/*printf("enter CREATE(FE_element_shape)\n");*/
	if (dimension > 0)
	{
		/* check if the shape already exists */
		if (!(shape = find_FE_element_shape_in_list(dimension, type,
			FE_region_get_FE_element_shape_list(fe_region))))
		{
			if (ALLOCATE(shape, struct FE_element_shape, 1))
			{
				/* make an unspecified shape = no type array or faces */
				shape->dimension = dimension;
				shape->type = (int *)NULL;
				shape->number_of_faces = 0;
				shape->faces = (int *)NULL;
				shape->face_normals = (FE_value *)NULL;
				shape->face_to_element = (FE_value *)NULL;
				shape->access_count = 0;
			}
			shape_type = (int *)NULL;
			linked_offsets = (int *)NULL;
			if (type)
			{
				ALLOCATE(shape_type,int,(dimension*(dimension+1))/2);
				/* offsets is working storage used within this function */
				ALLOCATE(linked_offsets,int,dimension);
			}
			if (shape && shape_type && linked_offsets)
			{
				shape->type = shape_type;
				if (1==dimension)
				{
					if ((LINE_SHAPE == *type) ||
						(UNSPECIFIED_SHAPE == *type))
					{
						*(shape_type) = *type;
						shape->number_of_faces = 0;
						shape->faces = (int *)NULL;
						shape->face_to_element = (FE_value *)NULL;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(FE_element_shape).  Invalid shape type");
						shape=(struct FE_element_shape *)NULL;
					}
				}
				else
				{
					/* copy the type */
					type_entry=type;
					for (i=(dimension*(dimension+1))/2;i>0;i--)
					{
						*shape_type= *type_entry;
						shape_type++;
						type_entry++;
					}
					/* check that the type is valid and calculate the number of faces */
					no_error=1;
					number_of_faces=0;
					xi_coordinate=0;
					shape_type=shape->type;
					while (no_error&&(xi_coordinate<dimension))
					{
						linked_offsets[xi_coordinate]=0;
						xi_coordinate++;
						switch (*shape_type)
						{
							case UNSPECIFIED_SHAPE:
							{
								/* unspecified */
								number_of_faces = 0;
								/* check that not connected to anything else */
								i = dimension - xi_coordinate;
								shape_type++;
								while (no_error && (i>0))
								{
									if (0 != *shape_type)
									{
										no_error = 0;
									}
									shape_type++;
									i--;
								}
							} break;
							case LINE_SHAPE:
							{
								/* line */
								number_of_faces += 2;
								/* check that not connected to anything else */
								i=dimension-xi_coordinate;
								shape_type++;
								while (no_error&&(i>0))
								{
									if (0!= *shape_type)
									{
										no_error=0;
									}
									shape_type++;
									i--;
								}
							} break;
							case POLYGON_SHAPE:
							{
								/* polygon */
								/* check if the other polygon coordinate is before */
								type_entry=shape_type;
								i=xi_coordinate-1;
								j=dimension-xi_coordinate;
								number_of_polygon_verticies=0;
								while (no_error&&(i>0))
								{
									j++;
									type_entry -= j;
									if (*type_entry)
									{
										if (number_of_polygon_verticies)
										{
											no_error=0;
										}
										else
										{
											number_of_polygon_verticies= *type_entry;
											if (number_of_polygon_verticies>=3)
											{
												linked_offsets[xi_coordinate-1]=i-xi_coordinate;
											}
											else
											{
												no_error=0;
											}
										}
									}
									i--;
								}
								if (no_error)
								{
									if (number_of_polygon_verticies)
									{
										/* other polygon coordinate is before */
										/* check that not connected to anything else */
										i=dimension-xi_coordinate;
										shape_type++;
										while (no_error&&(i>0))
										{
											if (0!= *shape_type)
											{
												no_error=0;
											}
											shape_type++;
											i--;
										}
									}
									else
									{
										/* check if the other polygon coordinate is after */
										shape_type++;
										i=dimension-xi_coordinate;
										number_of_polygon_verticies=0;
										while (no_error&&(i>0))
										{
											if (*shape_type)
											{
												if (number_of_polygon_verticies)
												{
													no_error=0;
												}
												else
												{
													number_of_polygon_verticies= *shape_type;
													if (3<=number_of_polygon_verticies)
													{
														linked_offset=dimension-xi_coordinate+1-i;
														if (POLYGON_SHAPE==shape_type[(linked_offset*
															(2*(dimension-xi_coordinate+1)-linked_offset+1))/
															2-linked_offset])
														{
															linked_offsets[xi_coordinate-1]=linked_offset;
															number_of_faces += number_of_polygon_verticies;
														}
														else
														{
															no_error=0;
														}
													}
													else
													{
														no_error=0;
													}
												}
											}
											shape_type++;
											i--;
										}
										if (number_of_polygon_verticies<3)
										{
											no_error=0;
										}
									}
								}
							} break;
							case SIMPLEX_SHAPE:
							{
								/* simplex */
								/* check preceding dimensions */
								type_entry=shape_type;
								i=xi_coordinate-1;
								j=dimension-xi_coordinate;
								while (no_error&&(i>0))
								{
									j++;
									type_entry -= j;
									if (*type_entry)
									{
										if (SIMPLEX_SHAPE== *(type_entry-(xi_coordinate-i)))
										{
											linked_offsets[xi_coordinate-1]=i-xi_coordinate;
										}
										else
										{
											no_error=0;
										}
									}
									i--;
								}
								if (0==linked_offsets[xi_coordinate-1])
								{
									/* this is first simplex coordinate */
									number_of_faces++;
								}
								else
								{
									/* check intermediary links */
									/* calculate first simplex coordinate */
									i=xi_coordinate+linked_offsets[xi_coordinate-1];
									type_entry=(shape->type)+((i-1)*(2*dimension-i+2))/2;
									j=dimension-i;
									k=xi_coordinate-i;
									i++;
									while (no_error&&(i<xi_coordinate))
									{
										j--;
										k += j;
										type_entry++;
										if (((0== *type_entry)&&(0!=type_entry[k]))||
											((0!= *type_entry)&&(0==type_entry[k])))
										{
											no_error=0;
										}
										i++;
									}
								}
								number_of_faces++;
								/* check succeeding dimensions */
								shape_type++;
								i=dimension-xi_coordinate;
								while (no_error&&(i>0))
								{
									if (*shape_type)
									{
										linked_offset=dimension-xi_coordinate+1-i;
										if (SIMPLEX_SHAPE==shape_type[(linked_offset*
											(2*(dimension-xi_coordinate+1)-linked_offset+1))/
											2-linked_offset])
										{
											if (linked_offsets[xi_coordinate-1]<=0)
											{
												linked_offsets[xi_coordinate-1]=linked_offset;
											}
										}
										else
										{
											no_error=0;
										}
									}
									shape_type++;
									i--;
								}
								if (0==linked_offsets[xi_coordinate-1])
								{
									no_error=0;
								}
							} break;
							default:
							{
								no_error=0;
							} break;
						}
					}
					if (no_error)
					{
						dimension_2=dimension*dimension;
						if (ALLOCATE(face,int,number_of_faces)&&
							ALLOCATE(face_normals,FE_value,number_of_faces*dimension)&&
							ALLOCATE(face_to_element,FE_value,number_of_faces*dimension_2))
						{
							shape->number_of_faces=number_of_faces;
							shape->faces=face;
							shape->face_normals=face_normals;
							shape->face_to_element=face_to_element;
							for (i=number_of_faces*dimension_2;i>0;i--)
							{
								*face_to_element=0;
								face_to_element++;
							}
							face_to_element=shape->face_to_element;
							for (i=number_of_faces;i>0;i--)
							{
								*face=0;
								face++;
							}
							face=shape->faces;
							for (i=number_of_faces*dimension;i>0;i--)
							{
								*face_normals=0.0;
								face_normals++;
							}
							face_normals=shape->face_normals;
							shape_type=shape->type;
							offset=1;
							/* loop over the coordinates calculating the face matrices
								(number dependent on <*shape_type>) for each */
							for (xi_coordinate=0;xi_coordinate<dimension;xi_coordinate++)
							{
								switch (*shape_type)
								{
									case LINE_SHAPE:
									{
										/* line */
										/* two faces for this coordinate */
										offset *= 2;
										*face=offset;
										face++;
										*face=offset+1;
										face++;
										*(face_normals + xi_coordinate) = -1.0;
										face_normals += dimension;
										*(face_normals + xi_coordinate) = 1.0;
										face_normals += dimension;
										face_to_element[dimension_2+xi_coordinate*dimension]=1;
#define CALCULATE_K_SET() \
k_set=k; \
if (POLYGON_SHAPE== *type_entry) \
{ \
	linked_offset=linked_offsets[j]; \
	/* for when *shape_type is POLYGON_SHAPE */ \
	if (xi_coordinate!=j+linked_offset) \
	{ \
		if (0<linked_offset) \
		{ \
			/* first polygon coordinate */ \
			linked_k=k; \
			if (linked_k<=xi_coordinate) \
			{ \
				linked_k--; \
			} \
			linked_k += linked_offset; \
			if (linked_k>=dimension) \
			{ \
				linked_k -= dimension; \
			} \
			if (linked_k<xi_coordinate) \
			{ \
				linked_k++; \
			} \
			if (linked_k<k) \
			{ \
				k_set=linked_k; \
			} \
		} \
		else \
		{ \
			/* second polygon coordinate */ \
			linked_k=k; \
			if (linked_k<=xi_coordinate) \
			{ \
				linked_k--; \
			} \
			linked_k -= linked_offset; \
			if (linked_k>=dimension) \
			{ \
				linked_k -= dimension; \
			} \
			if (linked_k<xi_coordinate) \
			{ \
				linked_k++; \
			} \
			if (k<linked_k) \
			{ \
				k_set=linked_k; \
			} \
		} \
	} \
}
										k=xi_coordinate+1;
										if (k>=dimension)
										{
											k=1;
										}
										type_entry=shape->type;
										for (j=0;j<dimension;j++)
										{
											if (j!=xi_coordinate)
											{
												CALCULATE_K_SET();
												face_to_element[k_set]=1;
												face_to_element[dimension_2+k_set]=1;
												k++;
												if (k>=dimension)
												{
													k=1;
												}
											}
											face_to_element += dimension;
											type_entry += dimension-j;
										}
										face_to_element += dimension_2;
									} break;
									case POLYGON_SHAPE:
									{
										/* polygon */
										/* <number_of_polygon_verticies>+1 faces in total for the
											two polygon dimension */
										if (0<(linked_offset=linked_offsets[xi_coordinate]))
										{
											/* first polygon dimension */
											number_of_polygon_verticies=shape_type[linked_offset];
											offset *= number_of_polygon_verticies;
											linked_offset += xi_coordinate;
											for (i=0;i<number_of_polygon_verticies;i++)
											{
												*face=offset;
												face++;
												offset++;
												*(face_normals + linked_offset) = 1.0;
												face_normals += dimension;
												k=xi_coordinate+1;
												if (k>=dimension)
												{
													k=1;
												}
												type_entry=shape->type;
												j=0;
												while (j<xi_coordinate)
												{
													CALCULATE_K_SET();
													face_to_element[k_set]=1;
													k++;
													if (k>=dimension)
													{
														k=1;
													}
													type_entry += dimension-j;
													j++;
													face_to_element += dimension;
												}
												*face_to_element=
													(FE_value)i/(FE_value)number_of_polygon_verticies;
												face_to_element[k]=
													1./(FE_value)number_of_polygon_verticies;
												k++;
												if (k>=dimension)
												{
													k=1;
												}
												type_entry += dimension-j;
												j++;
												face_to_element += dimension;
												while (j<linked_offset)
												{
													CALCULATE_K_SET();
													face_to_element[k_set]=1;
													k++;
													if (k>=dimension)
													{
														k=1;
													}
													type_entry += dimension-j;
													j++;
													face_to_element += dimension;
												}
												*face_to_element=1;
												face_to_element += dimension;
												type_entry += dimension-j;
												j++;
												while (j<dimension)
												{
													CALCULATE_K_SET();
													face_to_element[k_set]=1;
													k++;
													if (k>=dimension)
													{
														k=1;
													}
													type_entry += dimension-j;
													j++;
													face_to_element += dimension;
												}
											}
										}
									} break;
									case SIMPLEX_SHAPE:
									{
										offset *= 2;
										*face=offset;
										face++;
										*(face_normals + xi_coordinate) = -1.0;
										face_normals += dimension;
										simplex_dimension=0;
										/* calculate the simplex dimension */
										simplex_coordinate=xi_coordinate;
										do
										{
											simplex_coordinate += linked_offsets[simplex_coordinate];
											simplex_dimension++;
										} while (simplex_coordinate!=xi_coordinate);
										simplex_coordinate=xi_coordinate;
										linked_offset=simplex_dimension;
										if (0<linked_offsets[xi_coordinate])
										{
											do
											{
												simplex_coordinate +=
													linked_offsets[simplex_coordinate];
												linked_offset--;
											} while (linked_offsets[simplex_coordinate]>0);
											face[simplex_coordinate-xi_coordinate] += offset;
										}
										else
										{
											/* last simplex dimension */
											*face += offset+1;
											face++;
											weight = 1.0/sqrt((double)simplex_dimension);
											simplex_coordinate=xi_coordinate;
											for (j=simplex_dimension;j>0;j--)
											{
												face_normals[simplex_coordinate]=weight;
												simplex_coordinate += linked_offsets[simplex_coordinate];
											}
											face_normals += dimension;
											simplex_coordinate=xi_coordinate;
										}
										linked_offset--;
										simplex_coordinate += linked_offsets[simplex_coordinate];
										linked_coordinate=simplex_coordinate;
										for (j=simplex_dimension-linked_offset;j>0;j--)
										{
											linked_coordinate += linked_offsets[linked_coordinate];
										}
										k=xi_coordinate+1;
										if (k>=dimension)
										{
											k=1;
										}
										if (simplex_dimension<dimension)
										{
											/* make sure that k is not in the simplex containing
												xi_coordinate */
											i=1;
											do
											{
												if (k>xi_coordinate)
												{
													i=k;
												}
												else
												{
													i=k-1;
												}
												if (0==linked_offsets[i])
												{
													i=0;
												}
												else
												{
													l=i;
													do
													{
														l += linked_offsets[l];
													} while ((l!=i)&&(l!=xi_coordinate));
													if (l==xi_coordinate)
													{
														k++;
														if (k>=dimension)
														{
															k=1;
														}
														i=1;
													}
													else
													{
														i=0;
													}
												}
											} while (1==i);
										}
										/* start with
											- simplex_coordinate being the first simplex coordinate
											- (simplex_dimension-linked_offset)-1 being the
												coordinate in the simplex of the xi_coordinate
											- linked_coordinate being the coordinate after
												xi_coordinate in the simplex
											*/
										type_entry=shape->type;
										for (j=0;j<dimension;j++)
										{
											if (j==simplex_coordinate)
											{
												simplex_coordinate +=
													linked_offsets[simplex_coordinate];
												linked_offset--;
												if (j!=xi_coordinate)
												{
													if (0==linked_offset)
													{
														face_to_element[0]=1;
														temp_int=xi_coordinate+
															linked_offsets[xi_coordinate];
														while (xi_coordinate<temp_int)
														{
															face_to_element[temp_int]= -1;
															temp_int += linked_offsets[temp_int];
														}
														while (temp_int<xi_coordinate)
														{
															face_to_element[temp_int+1]= -1;
															temp_int += linked_offsets[temp_int];
														}
													}
													else
													{
														if (linked_coordinate<xi_coordinate)
														{
															face_to_element[linked_coordinate+1]=1;
														}
														else
														{
															if (xi_coordinate<linked_coordinate)
															{
																face_to_element[linked_coordinate]=1;
															}
														}
														linked_coordinate +=
															linked_offsets[linked_coordinate];
													}
												}
												else
												{
													linked_coordinate +=
														linked_offsets[linked_coordinate];
												}
											}
											else
											{
												if (j!=xi_coordinate)
												{
													CALCULATE_K_SET();
													face_to_element[k_set]=1;
													k++;
													if (k>=dimension)
													{
														k=1;
													}
													if (simplex_dimension<dimension)
													{
														/* make sure that k is not in the simplex containing
															xi_coordinate */
														i=1;
														do
														{
															if (k>xi_coordinate)
															{
																i=k;
															}
															else
															{
																i=k-1;
															}
															if (0==linked_offsets[i])
															{
																i=0;
															}
															else
															{
																l=i;
																do
																{
																	l += linked_offsets[l];
																} while ((l!=i)&&(l!=xi_coordinate));
																if (l==xi_coordinate)
																{
																	k++;
																	if (k>=dimension)
																	{
																		k=1;
																	}
																	i=1;
																}
																else
																{
																	i=0;
																}
															}
														} while (1==i);
													}
												}
											}
											face_to_element += dimension;
											type_entry += dimension-j;
										}
										if (linked_offsets[xi_coordinate]<0)
										{
											/* last simplex dimension */
											linked_offset=simplex_dimension;
											simplex_coordinate=xi_coordinate+
												linked_offsets[xi_coordinate];
											linked_coordinate=simplex_coordinate;
											k=xi_coordinate+1;
											if (k>=dimension)
											{
												k=1;
											}
											if (simplex_dimension<dimension)
											{
												/* make sure that k is not in the simplex containing
													xi_coordinate */
												i=1;
												do
												{
													if (k>xi_coordinate)
													{
														i=k;
													}
													else
													{
														i=k-1;
													}
													if (0==linked_offsets[i])
													{
														i=0;
													}
													else
													{
														l=i;
														do
														{
															l += linked_offsets[l];
														} while ((l!=i)&&(l!=xi_coordinate));
														if (l==xi_coordinate)
														{
															k++;
															if (k>=dimension)
															{
																k=1;
															}
															i=1;
														}
														else
														{
															i=0;
														}
													}
												} while (1==i);
											}
											type_entry=shape->type;
											for (j=0;j<dimension;j++)
											{
												if (j==simplex_coordinate)
												{
													simplex_coordinate +=
														linked_offsets[simplex_coordinate];
													linked_offset--;
													if (0==linked_offset)
													{
														face_to_element[0]=1;
														temp_int=xi_coordinate+
															linked_offsets[xi_coordinate];
														while (xi_coordinate<temp_int)
														{
															face_to_element[temp_int]= -1;
															temp_int += linked_offsets[temp_int];
														}
														while (temp_int<xi_coordinate)
														{
															face_to_element[temp_int+1]= -1;
															temp_int += linked_offsets[temp_int];
														}
													}
													else
													{
														if (linked_coordinate<xi_coordinate)
														{
															face_to_element[linked_coordinate+1]=1;
														}
														else
														{
															if (xi_coordinate<linked_coordinate)
															{
																face_to_element[linked_coordinate]=1;
															}
														}
														linked_coordinate +=
															linked_offsets[linked_coordinate];
													}
												}
												else
												{
													if (j!=xi_coordinate)
													{
														CALCULATE_K_SET();
														face_to_element[k_set]=1;
														k++;
														if (k>=dimension)
														{
															k=1;
														}
														if (simplex_dimension<dimension)
														{
															/* make sure that k is not in the simplex
																containing xi_coordinate */
															i=1;
															do
															{
																if (k>xi_coordinate)
																{
																	i=k;
																}
																else
																{
																	i=k-1;
																}
																if (0==linked_offsets[i])
																{
																	i=0;
																}
																else
																{
																	l=i;
																	do
																	{
																		l += linked_offsets[l];
																	} while ((l!=i)&&(l!=xi_coordinate));
																	if (l==xi_coordinate)
																	{
																		k++;
																		if (k>=dimension)
																		{
																			k=1;
																		}
																		i=1;
																	}
																	else
																	{
																		i=0;
																	}
																}
															} while (1==i);
														}
													}
												}
												face_to_element += dimension;
												type_entry += dimension-j;
											}
										}
									} break;
								}
								shape_type += dimension-xi_coordinate;
							}
#if defined (DEBUG_CODE)
							/*???debug */
							face_to_element=shape->face_to_element;
							face=shape->faces;
							for (i=1;i<=number_of_faces;i++)
							{
								printf("face %d %d:\n",i,*face);
								face++;
								for (j=dimension;j>0;j--)
								{
									for (k=dimension;k>0;k--)
									{
										printf(" %g",*face_to_element);
										face_to_element++;
									}
									printf("\n");
								}
							}
#endif /* defined (DEBUG_CODE) */
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"CREATE(FE_element_shape).  Could not allocate memory for faces");
							DEALLOCATE(shape);
							DEALLOCATE(shape_type);
							DEALLOCATE(face);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(FE_element_shape).  Invalid shape type");
						DEALLOCATE(shape->type);
						DEALLOCATE(shape);
					}
				}
			}
			else
			{
				/* unspecified shape creator ends up here legitimately */
				if (type || (!shape))
				{
					display_message(ERROR_MESSAGE,
						"CREATE(FE_element_shape).  Could not allocate memory for shape");
					DEALLOCATE(shape);
					DEALLOCATE(shape_type);
				}
			}
			if (shape)
			{
#if defined (DEBUG_CODE)
				printf("Created shape: %p\n", shape);
				printf("  shape->dimension: %d\n", shape->dimension);
				printf("  shape->type:");
				for (i = 0 ; i < (shape->dimension*(shape->dimension+1))/2 ; i++)
				{
					printf(" %d", shape->type[i]);
				}
				printf("\n");
				printf("  shape->number_of_faces: %d\n", shape->number_of_faces);
				printf("  shape->faces:");
				for (i = 0 ; i < shape->number_of_faces ; i++)
				{
					printf(" %d (", shape->faces[i]);
					for (j = 0 ; j < shape->dimension ; j++)
					{
						printf("%f,", shape->face_normals[j + i * shape->dimension]);
					}
					printf(")\n");
				}
#endif /* defined (DEBUG_CODE) */
				/* add the shape to the list of all shapes */
				if (!ADD_OBJECT_TO_LIST(FE_element_shape)(shape,
					FE_region_get_FE_element_shape_list(fe_region)))
				{
					display_message(ERROR_MESSAGE, "CREATE(FE_element_shape).  "
						"Could not add shape to the list of all shapes");
					DEALLOCATE(shape->type);
					DEALLOCATE(shape->faces);
					DEALLOCATE(shape->face_to_element);
					DEALLOCATE(shape);
				}
			}
			if (linked_offsets)
			{
				DEALLOCATE(linked_offsets);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_element_shape).  Invalid argument");
	}
/*???debug */
/*printf("leave CREATE(FE_element_shape)\n");*/
	LEAVE;

	return (shape);
} /* CREATE(FE_element_shape) */

int DESTROY(FE_element_shape)(struct FE_element_shape **element_shape_address)
/*******************************************************************************
LAST MODIFIED : 24 September 1995

DESCRIPTION :
Remove the shape from the list of all shapes.  Free the memory for the shape and
sets <*element_shape_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_element_shape *shape;

	ENTER(DESTROY(FE_element_shape));
	/* check the arguments */
	if ((element_shape_address)&&(shape= *element_shape_address))
	{
		if (0==shape->access_count)
		{
			DEALLOCATE(shape->type);
			DEALLOCATE(shape->faces);
			DEALLOCATE(shape->face_normals);
			DEALLOCATE(shape->face_to_element);
			DEALLOCATE(*element_shape_address);
			return_code=1;
		}
		else
		{
			return_code=1;
			*element_shape_address=(struct FE_element_shape *)NULL;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_element_shape) */

DECLARE_OBJECT_FUNCTIONS(FE_element_shape)
DECLARE_LIST_FUNCTIONS(FE_element_shape)

#if (MAXIMUM_ELEMENT_XI_DIMENSIONS > 3)
#error Resize following define.
#endif
#define INT_SHAPE_TYPE_ARRAY_LENGTH 6

struct cmzn_element_shape_type_map
{
	enum cmzn_element_shape_type shape_type;
	const int dimension;
	const int int_shape_type_array[INT_SHAPE_TYPE_ARRAY_LENGTH];
};

const struct cmzn_element_shape_type_map standard_shape_type_maps[] =
{
	{ CMZN_ELEMENT_SHAPE_TYPE_LINE,        1, { LINE_SHAPE, 0, 0, 0, 0, 0 } },
	{ CMZN_ELEMENT_SHAPE_TYPE_SQUARE,      2, { LINE_SHAPE, 0, LINE_SHAPE, 0, 0, 0 } },
	{ CMZN_ELEMENT_SHAPE_TYPE_TRIANGLE,    2, { SIMPLEX_SHAPE, 1, SIMPLEX_SHAPE, 0, 0, 0 } },
	{ CMZN_ELEMENT_SHAPE_TYPE_CUBE,        3, { LINE_SHAPE, 0, 0, LINE_SHAPE, 0, LINE_SHAPE } },
	{ CMZN_ELEMENT_SHAPE_TYPE_TETRAHEDRON, 3, { SIMPLEX_SHAPE, 1, 1, SIMPLEX_SHAPE, 1, SIMPLEX_SHAPE } },
	{ CMZN_ELEMENT_SHAPE_TYPE_WEDGE12,     3, { SIMPLEX_SHAPE, 1, 0, SIMPLEX_SHAPE, 0, LINE_SHAPE } },
	{ CMZN_ELEMENT_SHAPE_TYPE_WEDGE13,     3, { SIMPLEX_SHAPE, 0, 1, LINE_SHAPE, 0, SIMPLEX_SHAPE } },
	{ CMZN_ELEMENT_SHAPE_TYPE_WEDGE23,     3, { LINE_SHAPE, 0, 0, SIMPLEX_SHAPE, 1, SIMPLEX_SHAPE } }
};

const int standard_shape_type_maps_length = sizeof(standard_shape_type_maps) / sizeof(struct cmzn_element_shape_type_map);

struct FE_element_shape *FE_element_shape_create_simple_type(
	struct FE_region *fe_region, enum cmzn_element_shape_type shape_type)
{
	struct FE_element_shape *fe_element_shape;
	int i;

	fe_element_shape = NULL;
	if (fe_region)
	{
		for (i = 0; i < standard_shape_type_maps_length; i++)
		{
			if (standard_shape_type_maps[i].shape_type == shape_type)
			{
				fe_element_shape = ACCESS(FE_element_shape)(
					CREATE(FE_element_shape)(standard_shape_type_maps[i].dimension,
						standard_shape_type_maps[i].int_shape_type_array, fe_region));
				break;
			}
		}
	}
	if (!fe_element_shape)
	{
		display_message(ERROR_MESSAGE,
			"FE_element_shape_create_simple_type.  Invalid arguments");
	}
	return fe_element_shape;
}

int FE_element_shape_get_number_of_faces(FE_element_shape *element_shape)
{
	if (element_shape)
		return element_shape->number_of_faces;
	return 0;
}

enum cmzn_element_shape_type FE_element_shape_get_simple_type(
	struct FE_element_shape *element_shape)
{
	int dimension, i, j, length;
	enum cmzn_element_shape_type shape_type;

	shape_type = CMZN_ELEMENT_SHAPE_TYPE_INVALID;
	if (element_shape && element_shape->type)
	{
		dimension = element_shape->dimension;
		length = dimension*(dimension + 1)/2;
		for (i = 0; i < standard_shape_type_maps_length; i++)
		{
			if (standard_shape_type_maps[i].dimension == dimension)
			{
				for (j = 0; j < length; j++)
				{
					if (element_shape->type[j] != standard_shape_type_maps[i].int_shape_type_array[j])
						break;
				}
				if (j == length)
				{
					shape_type = standard_shape_type_maps[i].shape_type;
					break;
				}
			}
		}
	}
	return shape_type;
}

// Currently limited to handling one polygon or one simplex. Will have to
// be rewritten for 4-D and above elements.
char *FE_element_shape_get_EX_description(struct FE_element_shape *element_shape)
{
	if (!element_shape)
		return 0;
	char *description = 0;
	int next_xi_number, number_of_polygon_vertices;
	int error = 0;
	enum FE_element_shape_type shape_type;
	int linked_dimensions = 0;
	for (int xi_number = 0; xi_number < element_shape->dimension; xi_number++)
	{
		if (xi_number > 0)
		{
			append_string(&description, "*", &error);
		}
		if (get_FE_element_shape_xi_shape_type(element_shape, xi_number, &shape_type))
		{
			switch (shape_type)
			{
				case LINE_SHAPE:
				{
					append_string(&description, "line", &error);
				} break;
				case POLYGON_SHAPE:
				{
					/* logic currently limited to one polygon in shape - ok up to 3D */
					append_string(&description, "polygon", &error);
					if (0 == linked_dimensions)
					{
						if ((!get_FE_element_shape_next_linked_xi_number(element_shape,
							xi_number, &next_xi_number, &number_of_polygon_vertices)) ||
							(next_xi_number <= 0))
						{
							display_message(ERROR_MESSAGE, "write_FE_element_shape.  "
								"No second linked dimensions in polygon");
							DEALLOCATE(description);
							return 0;
						}
						else if (number_of_polygon_vertices < 3)
						{
							display_message(ERROR_MESSAGE, "write_FE_element_shape.  "
								"Invalid number of vertices in polygon: %d",
								number_of_polygon_vertices);
							DEALLOCATE(description);
							return 0;
						}
						else
						{
							char tmp_string[50];
							sprintf(tmp_string, "(%d;%d)", number_of_polygon_vertices, next_xi_number + 1);
							append_string(&description, tmp_string, &error);
						}
					}
					linked_dimensions++;
					if (2 < linked_dimensions)
					{
						display_message(ERROR_MESSAGE, "write_FE_element_shape.  "
							"Too many linked dimensions in polygon");
						DEALLOCATE(description);
						return 0;
					}
				} break;
				case SIMPLEX_SHAPE:
				{
					/* logic currently limited to one simplex in shape - OK up to 3D */
					append_string(&description, "simplex", &error);
					if (0 == linked_dimensions)
					{
						char tmp_string[50];
						linked_dimensions++;
						/* for first linked simplex dimension write (N1[;N2]) where N1 is
							 first linked dimension, N2 is the second - for tetrahedra */
						append_string(&description, "(", &error);
						next_xi_number = xi_number;
						while (next_xi_number < element_shape->dimension)
						{
							if (get_FE_element_shape_next_linked_xi_number(element_shape,
								next_xi_number, &next_xi_number, &number_of_polygon_vertices))
							{
								if (0 < next_xi_number)
								{
									linked_dimensions++;
									if (2 < linked_dimensions)
									{
										append_string(&description, ";", &error);
									}
									sprintf(tmp_string, "%d", next_xi_number + 1);
									append_string(&description, tmp_string, &error);
								}
								else
								{
									next_xi_number = element_shape->dimension;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE, "write_FE_element_shape.  "
									"Could not get next linked xi number for simplex");
								DEALLOCATE(description);
								return 0;
							}
						}
						append_string(&description, ")", &error);
						if (1 == linked_dimensions)
						{
							display_message(ERROR_MESSAGE,"write_FE_element_shape.  "
								"Too few linked dimensions in simplex");
							DEALLOCATE(description);
							return 0;
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"write_FE_element_shape.  Unknown shape type");
					DEALLOCATE(description);
					return 0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_FE_element_shape.  Could not get shape type");
			DEALLOCATE(description);
			return 0;
		}
	}
	return description;
}

struct FE_element_shape *FE_element_shape_create_unspecified(
	struct FE_region *fe_region, int dimension)
{
	struct FE_element_shape *fe_element_shape = NULL;
	if (fe_region && (1 <= dimension) && (dimension <= 3))
	{
		fe_element_shape = ACCESS(FE_element_shape)(
			CREATE(FE_element_shape)(dimension, (int *)NULL, fe_region));
	}
	if (!fe_element_shape)
	{
		display_message(ERROR_MESSAGE,
			"FE_element_shape_create_simple_type.  Invalid arguments");
	}
	return fe_element_shape;
}

bool FE_element_shape_is_unspecified(struct FE_element_shape *element_shape)
{
	return ((0 != element_shape) && (0 == element_shape->type));
}

int FE_element_shape_is_line(struct FE_element_shape *element_shape)
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Returns true if the <element_shape> has only LINE_SHAPE in each dimension.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_shape_is_line);
	return_code = 0;
	if (element_shape)
	{
		if (element_shape->type)
		{
			return_code = (element_shape->type[0] == LINE_SHAPE) && (
				 (1 == element_shape->dimension) ||
				((2 == element_shape->dimension) &&
					(element_shape->type[2] == LINE_SHAPE)) ||
				((3 == element_shape->dimension) &&
					(element_shape->type[3] == LINE_SHAPE) &&
					(element_shape->type[5] == LINE_SHAPE)));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_shape_is_line.  Missing shape");
	}
	LEAVE;

	return (return_code);
} /* FE_element_shape_is_line */

struct FE_element_shape *get_FE_element_shape_of_face(
	struct FE_element_shape *shape,int face_number, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 7 July 2003

DESCRIPTION :
From the parent <shape> returns the FE_element_shape for its face <face_number>.
The <shape> must be of dimension 2 or 3. Faces of 2-D elements are always lines.
==============================================================================*/
{
	int face_code,face_type[3],line_xi_bit,number_of_polygon_vertices,
		polygon_face;
	struct FE_element_shape *face_shape;

	ENTER(get_FE_element_shape_of_face);
	face_shape = (struct FE_element_shape *)NULL;
	if (shape&&(0<=face_number)&&(face_number<shape->number_of_faces)&&
		shape->type&&shape->faces)
	{
		switch (shape->dimension)
		{
			case 2:
			{
				/* faces of 2-D shapes are always lines */
				face_type[0]=LINE_SHAPE;
				face_shape=CREATE(FE_element_shape)(/*dimension*/1,face_type,
					fe_region);
			} break;
			case 3:
			{
				/* if all shape types on main diagonal are the same, then it is either
					 a square block or a tetrahedron. Sub-triangle of shape->type is then
					 valid for any of its faces */
				if (((shape->type)[0]==(shape->type)[3])&&
					((shape->type)[0]==(shape->type)[5]))
				{
					face_shape=CREATE(FE_element_shape)(/*dimension*/2,shape->type+3,
						fe_region);
				}
				else if ((POLYGON_SHAPE==(shape->type)[0])||
					(POLYGON_SHAPE==(shape->type)[3])||
					(POLYGON_SHAPE==(shape->type)[5]))
				{
					/* 2 out of 3 xi directions must be linked in a polygon: hence need
						to determine if face is a polygon or a square */
					face_code=shape->faces[face_number];
					/* work out number_of_polygon_vertices */
					polygon_face=0;
					if (POLYGON_SHAPE==(shape->type)[0])
					{
						if (POLYGON_SHAPE==(shape->type)[3])
						{
							number_of_polygon_vertices=(shape->type)[1];
							/* polygon-polygon-line */
							if (face_code>=number_of_polygon_vertices*2)
							{
								polygon_face=1;
							}
						}
						else
						{
							number_of_polygon_vertices=(shape->type)[2];
							/* polygon-line-polygon */
							if (face_code>=number_of_polygon_vertices*2)
							{
								polygon_face=1;
							}
						}
					}
					else
					{
						number_of_polygon_vertices=(shape->type)[4];
						/* line-polygon-polygon */
						if (face_code<number_of_polygon_vertices*2)
						{
							polygon_face=1;
						}
					}
					if (polygon_face)
					{
						/* face has a polygon shape */
						face_type[0]=POLYGON_SHAPE;
						face_type[1]=number_of_polygon_vertices;
						face_type[2]=POLYGON_SHAPE;
					}
					else
					{
						/* face has a square shape */
						face_type[0]=LINE_SHAPE;
						face_type[1]=0;
						face_type[2]=LINE_SHAPE;
					}
					face_shape=CREATE(FE_element_shape)(/*dimension*/2,face_type,
						fe_region);
				}
				else if ((SIMPLEX_SHAPE==(shape->type)[0])||
					(SIMPLEX_SHAPE==(shape->type)[3])||
					(SIMPLEX_SHAPE==(shape->type)[5]))
				{
					/* 2 out of 3 xi directions must be linked in a triangle: hence need
						 to determine if face is a triangle or a square */
					face_code=shape->faces[face_number];
					/* work out which xi direction is not SIMPLEX_SHAPE (=LINE_SHAPE) */
					if (SIMPLEX_SHAPE==(shape->type)[0])
					{
						if (SIMPLEX_SHAPE==(shape->type)[3])
						{
							line_xi_bit=8;
						}
						else
						{
							line_xi_bit=4;
						}
					}
					else
					{
						line_xi_bit=2;
					}
					if (face_code & line_xi_bit)
					{
						/* face has a triangle shape */
						face_type[0]=SIMPLEX_SHAPE;
						face_type[1]=1;
						face_type[2]=SIMPLEX_SHAPE;
					}
					else
					{
						/* face has a square shape */
						face_type[0]=LINE_SHAPE;
						face_type[1]=0;
						face_type[2]=LINE_SHAPE;
					}
					face_shape=CREATE(FE_element_shape)(/*dimension*/2,face_type,
						fe_region);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_FE_element_shape_of_face.  Unknown element shape");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"get_FE_element_shape_of_face.  Invalid dimension");
			} break;
		}
		if (!face_shape)
		{
			display_message(ERROR_MESSAGE,"get_FE_element_shape_of_face.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_shape_of_face.  Invalid argument(s)");
	}
	LEAVE;

	return (face_shape);
} /* get_FE_element_shape_of_face */

int get_FE_element_shape_dimension(struct FE_element_shape *element_shape)
{
	if (element_shape)
		return element_shape->dimension;
	return 0;
}

const FE_value *get_FE_element_shape_face_to_element(
	struct FE_element_shape *element_shape, int face_number)
{
	if (element_shape && (0 <= face_number) &&
		(face_number < element_shape->number_of_faces))
	{
		return element_shape->face_to_element +
			(face_number*element_shape->dimension*element_shape->dimension);
	}
	display_message(ERROR_MESSAGE, "get_FE_element_shape_face_to_element.  Invalid argument(s)");
	return 0;
}

int FE_element_shape_find_face_number_for_xi(struct FE_element_shape *shape,
	FE_value *xi, int *face_number)
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This function checks to see if the given <xi> location (of dimension
<shape>->dimension) specifys a location on a face.  If it does then the function
returns 1 and <face_number> is set.  Otherwise the function returns 0.
SAB Doesn't work for polygons at the moment.
==============================================================================*/
{
	int bit, i, j, return_code;
	FE_value sum;

	ENTER(FE_element_shape_find_face_number_for_xi);

	if (shape&&face_number)
	{
		return_code = 0;
		for (i = 0 ; (!return_code) && (i < shape->number_of_faces) ; i++)
		{
			sum = 0.0;
			bit = 2;
			for (j = 0 ; j < shape->dimension ; j++)
			{
				if (shape->faces[i] & bit)
				{
					sum += xi[j];
				}
				bit *= 2;
			}
			if (shape->faces[i] & 1)
			{
				if (sum >= 1.0)
				{
					*face_number = i;
					return_code = 1;
				}
			}
			else
			{
				if (sum <= 0.0)
				{
					*face_number = i;
					return_code = 1;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"find_face_number_of_face_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_shape_find_face_number_for_xi */

int get_FE_element_shape_xi_linkage_number(
	struct FE_element_shape *element_shape, int xi_number1, int xi_number2,
	int *xi_linkage_number_address)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Returns a number indicating how the dimension <xi_number1> and <xi_number2> are
linked in <element_shape>.
If they are linked in a simplex, a non-zero return indicates they are linked.
If they are linked in a polygon, the returned number is the number of sides in
the polygon.
A value of zero indicates the dimensions are not linked.
Note the first xi_number is 0.
==============================================================================*/
{
	int i, offset, return_code, tmp;

	ENTER(get_FE_element_shape_xi_linkage_number);
	if (element_shape && element_shape->type && xi_linkage_number_address &&
		(0 <= xi_number1) && (xi_number1 < element_shape->dimension) &&
		(0 <= xi_number2) && (xi_number2 < element_shape->dimension) &&
		(xi_number1 != xi_number2))
	{
		/* make sure xi_number1 < xi_number2 */
		if (xi_number2 < xi_number1)
		{
			tmp = xi_number1;
			xi_number1 = xi_number2;
			xi_number2 = tmp;
		}
		offset = 0;
		for (i = 0; i < xi_number1; i++)
		{
			offset += element_shape->dimension - i;
		}
		*xi_linkage_number_address =
			element_shape->type[offset + xi_number2 - xi_number1];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_shape_xi_linkage_number.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_shape_xi_linkage_number */

int get_FE_element_shape_xi_shape_type(struct FE_element_shape *element_shape,
	int xi_number, enum FE_element_shape_type *shape_type_address)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Returns the shape type of <element_shape> on <xi_number> -- on main diagonal of
type array. The first xi_number is 0.
==============================================================================*/
{
	int i, offset, return_code;

	ENTER(get_FE_element_shape_xi_shape_type);
	if (element_shape && element_shape->type && (0 <= xi_number) &&
		(xi_number < element_shape->dimension) && shape_type_address)
	{
		offset = 0;
		for (i = 0; i < xi_number; i++)
		{
			offset += element_shape->dimension - i;
		}
		*shape_type_address =
			(enum FE_element_shape_type)(element_shape->type[offset]);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_shape_xi_shape_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_shape_xi_shape_type */

int get_FE_element_shape_next_linked_xi_number(
	struct FE_element_shape *element_shape, int xi_number,
	int *next_xi_number_address, int *xi_link_number_address)
/*******************************************************************************
LAST MODIFIED : 25 February 2003

DESCRIPTION :
Returns in <next_xi_number_address> the next xi number higher than <xi_number>
which is linked in shape with it, plus in <xi_link_number_address> the number
denoting how it is linked; currently used only for polygon shapes to denote the
number of polygon sides.
If there is no remaining linked dimension, 0 is returned in both addresses.
<xi_number> is from 0 to one less than the shape dimension.
Also checks that the linked xi numbers have the same shape type.
==============================================================================*/
{
	enum FE_element_shape_type shape_type;
	int i, limit, offset, return_code;

	ENTER(get_FE_element_shape_next_linked_xi_number);
	if (element_shape && element_shape->type &&
		(0 <= xi_number) && (xi_number < element_shape->dimension) &&
		next_xi_number_address && xi_link_number_address)
	{
		return_code = 1;
		offset = 0;
		for (i = 0; i < xi_number; i++)
		{
			offset += element_shape->dimension - i;
		}
		shape_type = (enum FE_element_shape_type)(element_shape->type[offset]);
		limit = element_shape->dimension - xi_number;
		offset++;
		for (i = 1; (i < limit) && (0 == element_shape->type[offset]); i++)
		{
			offset++;
		}
		if (i < limit)
		{
			*next_xi_number_address = i + xi_number;
			*xi_link_number_address = element_shape->type[offset];
			/* finally check the shape type matches */
			offset = 0;
			for (i = 0; i < *next_xi_number_address; i++)
			{
				offset += element_shape->dimension - i;
			}
			if ((enum FE_element_shape_type)element_shape->type[offset] != shape_type)
			{
				display_message(ERROR_MESSAGE,
					"get_FE_element_shape_next_linked_xi_number.  "
					"Shape has linked xi directions with different shape type");
				return_code = 0;
			}
		}
		else
		{
			*next_xi_number_address = 0;
			*xi_link_number_address = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_shape_next_linked_xi_number.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_shape_next_linked_xi_number */

int FE_element_shape_limit_xi_to_element(struct FE_element_shape *shape,
	FE_value *xi, FE_value tolerance)
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Checks that the <xi> location is valid for elements with <shape>.
The <tolerance> allows the location to go slightly outside.  If the values for
<xi> location are further than <tolerance> outside the element then the values
are modified to put it on the nearest face.
==============================================================================*/
{
	int i, return_code, simplex_dimensions,
		simplex_direction[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	FE_value delta;

	ENTER(FE_element_shape_limit_xi_to_element);
	if (shape && xi)
	{
		return_code = 1;
		/* determine whether the element is simplex to limit xi space */
		simplex_dimensions = 0;
		switch (shape->dimension)
		{
			case 2:
			{
				if (SIMPLEX_SHAPE == shape->type[0])
				{
					simplex_dimensions = 2;
					simplex_direction[0] = 0;
					simplex_direction[1] = 1;
				}
			} break;
			case 3:
			{
				if (SIMPLEX_SHAPE == shape->type[0])
				{
					if (LINE_SHAPE == shape->type[3])
					{
						simplex_dimensions = 2;
						simplex_direction[0] = 0;
						simplex_direction[1] = 2;
					}
					else if (LINE_SHAPE == shape->type[5])
					{
						simplex_dimensions = 2;
						simplex_direction[0] = 0;
						simplex_direction[1] = 1;
					}
					else
					{
						/* tetrahedron */
						simplex_dimensions = 3;
						simplex_direction[0] = 0;
						simplex_direction[1] = 1;
						simplex_direction[2] = 2;
					}
				}
				else if (SIMPLEX_SHAPE == shape->type[3])
				{
					simplex_dimensions = 2;
					simplex_direction[0] = 1;
					simplex_direction[1] = 2;
				}
			} break;
		}
		/* keep xi within simplex bounds plus tolerance */
		if (simplex_dimensions)
		{
			/* calculate distance out of element in xi space */
			delta = -1.0 - tolerance;
			for (i = 0; i < simplex_dimensions; i++)
			{
				delta += xi[simplex_direction[i]];
			}
			if (delta > 0.0)
			{
				/* subtract delta equally from all directions */
				delta /= simplex_dimensions;
				for (i = 0; i < simplex_dimensions; i++)
				{
					xi[simplex_direction[i]] -= delta;
				}
			}
		}
		/* keep xi within 0.0 to 1.0 bounds plus tolerance */
		for (i = 0; i < shape->dimension; i++)
		{
			if (xi[i] < -tolerance)
			{
				xi[i] = -tolerance;
			}
			else if (xi[i] > 1.0 + tolerance)
			{
				xi[i] = 1.0 + tolerance;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_shape_limit_xi_to_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_shape_limit_xi_to_element */

/*******************************************************************************
 * @return  1 if element has a parent matching or descended from ancestor, 0 if not.
 */
static int FE_element_ancestor_matches_recursive(
	struct FE_element *element, struct FE_element *ancestor)
{
	int i;
	for (i = 0; i < element->number_of_parents; i++)
	{
		if ((element->parents[i] == ancestor) || ((element->parents[i]->parents) &&
			FE_element_ancestor_matches_recursive(element->parents[i], ancestor)))
			return 1;
	}
	return 0;
}

/*******************************************************************************
 * @return  first parent element matching or descending from ancestor, or
 * NULL if none or no match.
 */
static struct FE_element *FE_element_get_first_parent_with_ancestor(
	struct FE_element *element, struct FE_element *ancestor)
{
	int i;
	for (i = 0; i < element->number_of_parents; i++)
	{
		if ((element->parents[i] == ancestor) ||
				FE_element_ancestor_matches_recursive(element->parents[i], ancestor))
			return (element->parents[i]);
	}
	return NULL;
}

/***************************************************************************//**
 * WARNING: only returns face_number of first instance of that child as face
 * of parent.
 * @return  Face number of child in parent (from 0 to shape->number_of_faces),
 * or -1 if not in face list.
 */
static int FE_element_get_child_face_number(struct FE_element *parent,
	struct FE_element *child)
{
	if (parent && parent->fields)
	{
		FE_mesh *fe_mesh = parent->fields->fe_mesh;
		FE_element_shape *element_shape = fe_mesh->getElementShape(parent->index);
		if ((element_shape) && (child) && (parent->faces))
		{
			for (int i = 0; i < element_shape->number_of_faces; i++)
			{
				if (parent->faces[i] == child)
					return (i);
			}
		}
	}
	return (-1);
}

/**
 * Get first parent of element that is on <face> of a top-level
 * element, or which has any parent in this state, with optional conditional
 * check on which top-level element qualify.
 *
 * @param element  Element to get parent from
 * @param face  from 0 to top_level_element->shape->number_of_faces-1
 * @param conditional  Optional conditional function. If supplied, limits search to
 * top-level elements for which this function passes.
 * @param  conditional_data  User data to pass to optional conditional function.
 * @return  Parent element matching criteria, or NULL if none found.
 */
struct FE_element *FE_element_get_parent_on_face(
	struct FE_element *element, cmzn_element_face_type face,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional, void *conditional_data)
{
	int face_number = static_cast<int>(face) - CMZN_ELEMENT_FACE_TYPE_XI1_0;
	if (element && (0 <= face_number))
	{
		struct FE_element *face_element;
		for (int i = 0; i < element->number_of_parents; i++)
		{
			if (0 == element->parents[i]->number_of_parents)
			{
				if (get_FE_element_face(element->parents[i], face_number, &face_element) &&
					(face_element == element) && ((NULL == conditional) ||
						(conditional)(element->parents[i], conditional_data)))
				{
					return element->parents[i];
				}
			}
			else
			{
				for (int j = 0; j < element->parents[i]->number_of_parents; j++)
				{
					if (get_FE_element_face(element->parents[i]->parents[j], face_number, &face_element) &&
						(face_element == element->parents[i]) && ((NULL == conditional) ||
							(conditional)(element->parents[i]->parents[j], conditional_data)))
					{
						return element->parents[i];
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_get_parent_on_face.  Invalid argument(s)");
	}
	return (NULL);
}

int clear_FE_element_faces(struct FE_element *element, int number_of_faces)
{
	if ((element) && ((!element->faces) || (0 < number_of_faces)))
	{
		if (element->faces)
		{
			for (int i = 0; i < number_of_faces; ++i)
				set_FE_element_face(element, i, (struct FE_element *)NULL);
			DEALLOCATE(element->faces);
		}
		return CMZN_OK;
	}
	display_message(ERROR_MESSAGE, "clear_FE_element_faces.  Invalid argument(s)");
	return CMZN_ERROR_ARGUMENT;
}

int set_FE_element_number_of_faces(struct FE_element *element, int number_of_faces)
{
	if ((element) && (0 == element->faces))
	{
		if (0 < number_of_faces)
		{
			if (!ALLOCATE(element->faces, FE_element *, number_of_faces))
			{
				display_message(ERROR_MESSAGE, "set_FE_element_number_of_faces.  Not enough memory for faces");
				return CMZN_ERROR_MEMORY;
			}
			for (int i = 0; i < number_of_faces; ++i)
				element->faces[i] = 0;
		}
		return CMZN_OK;
	}
	display_message(ERROR_MESSAGE, "set_FE_element_number_of_faces.  Invalid argument(s)");
	return CMZN_ERROR_ARGUMENT;
}

/**
 * Creates a blank element with access count of 1.
 */
struct FE_element *CREATE(FE_element)()
{
	struct FE_element *element;
	if (ALLOCATE(element, struct FE_element, 1))
	{
		// not a global element until mesh gives a non-negative index:
		element->index = DS_LABEL_INDEX_INVALID;
		element->faces = (struct FE_element **)NULL;
		element->fields = (struct FE_element_field_info *)NULL;
		element->information = (struct FE_element_node_scale_field_info *)NULL;
		element->parents = (struct FE_element **)NULL;
		element->number_of_parents = 0;
		element->access_count = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "CREATE(FE_element).  Could not allocate memory for element");
		return 0;
	}
	return (element);
}

int DESTROY(FE_element)(struct FE_element **element_address)
/*******************************************************************************
LAST MODIFIED : 24 March 2003

DESCRIPTION :
Frees the memory for the element, sets <*element_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;

	ENTER(DESTROY(FE_element));
	if ((element_address)&&(element= *element_address))
	{
		if (0 == element->access_count)
		{
			FE_element_invalidate(element);
			/* parent elements are not ACCESSed */
			if (element->parents)
			{
				DEALLOCATE(element->parents);
			}
			/* free the memory associated with the element */
			DEALLOCATE(*element_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_element).  Element has non-zero access count %d",
				element->access_count);
			*element_address=(struct FE_element *)NULL;
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_element) */

struct FE_element *create_template_FE_element(FE_element_field_info *element_field_info)
{
	if (!element_field_info)
	{
		display_message(ERROR_MESSAGE, "create_template_FE_element.  Invalid argument");
		return 0;
	}
	struct FE_element *template_element = CREATE(FE_element)();
	if (!template_element)
		return 0;
	template_element->fields = ACCESS(FE_element_field_info)(element_field_info);
	return template_element;
}

struct FE_element *create_FE_element_from_template(DsLabelIndex index, struct FE_element *template_element)
{
	// Assumes DS_LABEL_INDEX_INVALID == -1
	if ((index < DS_LABEL_INDEX_INVALID) || (0 == template_element))
	{
		display_message(ERROR_MESSAGE, "create_FE_element_from_template.  Invalid argument(s)");
		return 0;
	}
	struct FE_element *element = CREATE(FE_element)();
	if (element)
	{
		bool success = true;
		element->index = index;
		if (!(element->fields = ACCESS(FE_element_field_info)(template_element->fields)))
		{
			display_message(ERROR_MESSAGE, "create_FE_element_from_template.  Could not set field info from template element");
			success = false;
		}
		if (template_element->information)
		{
			element->information = template_element->information->clone(template_element->fields);
			if (!element->information)
			{
				display_message(ERROR_MESSAGE, "create_FE_element_from_template.  Could not copy node scale field info from template element");
				success = false;
			}
		}
		if (!success)
			DEACCESS(FE_element)(&element);
	}
	return element;
}

void FE_element_invalidate(struct FE_element *element)
{
	if (element && element->fields)
	{
		FE_element_shape *element_shape = element->fields->fe_mesh->getElementShape(element->index);
		if (element_shape)
			clear_FE_element_faces(element, element_shape->number_of_faces);
		if (element->information)
			FE_element_node_scale_field_info::destroyDynamic(element->information, element->fields);
		DEACCESS(FE_element_field_info)(&(element->fields));
		element->index = DS_LABEL_INDEX_INVALID;
	}
}

DECLARE_OBJECT_FUNCTIONS(FE_element)

cmzn_elementiterator_id cmzn_elementiterator_access(cmzn_elementiterator_id element_iterator)
{
	if (element_iterator)
		return cmzn::Access(element_iterator);
	return 0;
}

int cmzn_elementiterator_destroy(cmzn_elementiterator_id *element_iterator_address)
{
	if (element_iterator_address)
	{
		cmzn::Deaccess(*element_iterator_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_element_id cmzn_elementiterator_next(cmzn_elementiterator_id element_iterator)
{
	if (element_iterator)
	{
		cmzn_element *element = element_iterator->nextElement();
		if (element)
			element->access();
		return element;
	}
	return 0;
}

cmzn_element_id cmzn_elementiterator_next_non_access(cmzn_elementiterator_id element_iterator)
{
	if (element_iterator)
		return element_iterator->nextElement();
	return 0;
}

/**
 * If <field> is NULL, element values are calculated for the coordinate field.
 * The optional <top_level_element> forces inheritance from it as needed.
 * If the dimension of <element> is less than that of the <field_element> from
 * which the field is inherited, then a <coordinate_transformation> is returned.
 * This consist of a matrix of dimension(field_element) rows X dimension(element)+1
 * columns. This represents an affine transformation, b + A xi for calculating the
 * field_element xi coordinates from those of <element>, where b is the first
 * column of the <coordinate_transformation> matrix.
 * @param inherit_face_number  If non-negative, inherit onto this face number
 * of element, as if the face element were supplied to this function.
 */
int inherit_FE_element_field(struct FE_element *element,
	int inherit_face_number, struct FE_field *field,
	struct FE_element_field **element_field_address,
	struct FE_element **field_element_address,
	FE_value **coordinate_transformation_address,
	struct FE_element *top_level_element)
{
	FE_value *coordinate_transformation,*coordinate_transformation_value,
		*face_to_element,*face_to_element_value,*new_coordinate_transformation,
		*new_coordinate_transformation_value;
	int dimension,dimension_minus_1,field_element_dimension,i,j,k,
		parent_number, return_code,transformation_size;
	struct FE_element *field_element;
	struct FE_element_field *element_field;
	struct FE_element_field_info *field_info;
#if defined (DOUBLE_FOR_DOT_PRODUCT)
	double sum;
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
	FE_value sum;
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */

	ENTER(inherit_FE_element_field);
	/* check the arguments */
	FE_mesh *fe_mesh;
	FE_element_shape *element_shape;
	if (element && (element->fields) && (0 != (fe_mesh = element->fields->fe_mesh)) &&
		(0 != (element_shape = fe_mesh->getElementShape(element->index))) &&
		element_field_address&&field_element_address&&
		coordinate_transformation_address &&
		(inherit_face_number < element_shape->number_of_faces))
	{
		/* initialize values to be returned on success */
		element_field=(struct FE_element_field *)NULL;
		field_element=(struct FE_element *)NULL;
		coordinate_transformation=(FE_value *)NULL;
#if defined (DEBUG_CODE)
		/*???debug */
		printf("element %d \n",element->get_identifier());
#endif /* defined (DEBUG_CODE) */
		/* check if the field is defined for the element */
		if ((field_info = element->fields) && element->information)
		{
			if (!field)
			{
				// non-inheriting part of get_FE_element_default_coordinate_field
				FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
					FE_element_field_get_first_coordinate_field,
					(void *)&field, field_info->element_field_list);
			}
			if (field)
			{
				element_field=FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,
					field)(field,field_info->element_field_list);
			}
			return_code=1;
		}
		else
		{
			element_field=(struct FE_element_field *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			FE_element *parent = (inherit_face_number >= 0) ? element : 0;
			int face_number = inherit_face_number;
			if (element_field)
			{
				coordinate_transformation=(FE_value *)NULL;
				field_element=element;
			}
			else if (parent)
			{
				// inherit from parent's parent
				if (!(((NULL == top_level_element) || (parent == top_level_element) ||
						FE_element_ancestor_matches_recursive(parent, top_level_element)) &&
					inherit_FE_element_field(parent, /*inherit_face_number*/-1, field,
						&element_field, &field_element, &coordinate_transformation, top_level_element)))
				{
					return_code = 0;
					parent = 0;
				}
			}
			else
			{
				return_code = 0;
				/* check if the field is defined for any of the element's parents */
				for (parent_number = 0; parent_number < element->number_of_parents; parent_number++)
				{
					parent = element->parents[parent_number];
					if ((NULL == top_level_element) || (parent == top_level_element) ||
						FE_element_ancestor_matches_recursive(parent, top_level_element))
					{
						return_code = inherit_FE_element_field(parent, /*inherit_face_number*/-1, field,
							&element_field, &field_element, &coordinate_transformation, top_level_element);
						if (return_code)
						{
							break;
						}
					}
				}
				if (return_code)
					face_number = FE_element_get_child_face_number(parent, element);
				else
					parent = 0;
			}
			if (parent)
			{
				FE_element_shape *parent_shape = get_FE_element_shape(parent);
				if (!parent_shape)
				{
					display_message(ERROR_MESSAGE, "inherit_FE_element_field.  Missing parent mesh or parent shape");
					return_code = 0;
				}
				else
				{
					dimension = parent_shape->dimension;
					field_element_dimension = field_element->getDimension();
					if (coordinate_transformation)
					{
						if (ALLOCATE(new_coordinate_transformation,FE_value,
							field_element_dimension*dimension))
						{
							/* incorporate the face to element map in the coordinate
								transformation */
							face_to_element=(parent_shape->face_to_element)+
								(face_number*dimension*dimension);
	#if defined (DEBUG_CODE)
							/*???debug */
							printf("face to element:\n");
							face_to_element_value=face_to_element;
							for (i=dimension;i>0;i--)
							{
								for (j=dimension;j>0;j--)
								{
									printf(" %g",*face_to_element_value);
									face_to_element_value++;
								}
								printf("\n");
							}
	#endif /* defined (DEBUG_CODE) */
	#if defined (DEBUG_CODE)
							/*???debug */
							printf("new coordinate transformation:\n");
	#endif /* defined (DEBUG_CODE) */
							coordinate_transformation_value=coordinate_transformation;
							new_coordinate_transformation_value=
								new_coordinate_transformation;
							dimension_minus_1=dimension-1;
							for (i=field_element_dimension;i>0;i--)
							{
								/* calculate b entry for this row */
	#if defined (DOUBLE_FOR_DOT_PRODUCT)
								sum=(double)(*coordinate_transformation_value);
	#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
								sum= *coordinate_transformation_value;
	#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
								coordinate_transformation_value++;
								face_to_element_value=face_to_element;
								for (k=dimension;k>0;k--)
								{
	#if defined (DOUBLE_FOR_DOT_PRODUCT)
									sum += (double)(*coordinate_transformation_value)*
										(double)(*face_to_element_value);
	#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
									sum += (*coordinate_transformation_value)*
										(*face_to_element_value);
	#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
									coordinate_transformation_value++;
									face_to_element_value += dimension;
								}
								*new_coordinate_transformation_value=(FE_value)sum;
	#if defined (DEBUG_CODE)
								/*???debug */
								printf(" %g",sum);
	#endif /* defined (DEBUG_CODE) */
								new_coordinate_transformation_value++;
								/* calculate A entries for this row */
								for (j=dimension_minus_1;j>0;j--)
								{
									face_to_element++;
									face_to_element_value=face_to_element;
									coordinate_transformation_value -= dimension;
									sum=0;
									for (k=dimension;k>0;k--)
									{
	#if defined (DOUBLE_FOR_DOT_PRODUCT)
										sum += (double)(*coordinate_transformation_value)*
											(double)(*face_to_element_value);
	#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
										sum += (*coordinate_transformation_value)*
											(*face_to_element_value);
	#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
										coordinate_transformation_value++;
										face_to_element_value += dimension;
									}
									*new_coordinate_transformation_value=(FE_value)sum;
	#if defined (DEBUG_CODE)
									/*???debug */
									printf(" %g",sum);
	#endif /* defined (DEBUG_CODE) */
									new_coordinate_transformation_value++;
								}
	#if defined (DEBUG_CODE)
								/*???debug */
								printf("\n");
	#endif /* defined (DEBUG_CODE) */
								face_to_element -= dimension_minus_1;
							}
							DEALLOCATE(coordinate_transformation);
							coordinate_transformation=new_coordinate_transformation;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"inherit_FE_element_field.  Insufficient memory");
							DEALLOCATE(coordinate_transformation);
							return_code=0;
						}
					}
					else
					{
	#if defined (DEBUG_CODE)
						/*???debug */
						printf("new coordinate transformation %d %d :\n",dimension,
							field_element_dimension);
	#endif /* defined (DEBUG_CODE) */
						/* use the face to element map as the transformation */
						transformation_size=field_element_dimension*dimension;
						if (ALLOCATE(coordinate_transformation,FE_value,
							transformation_size))
						{
							coordinate_transformation_value=coordinate_transformation;
							face_to_element_value=(parent_shape->face_to_element)+
								(face_number*transformation_size);
							while (transformation_size>0)
							{
								*coordinate_transformation_value= *face_to_element_value;
	#if defined (DEBUG_CODE)
								/*???debug */
								printf(" %g",*face_to_element_value);
	#endif /* defined (DEBUG_CODE) */
								coordinate_transformation_value++;
								face_to_element_value++;
								transformation_size--;
	#if defined (DEBUG_CODE)
								/*???debug */
								if (0==transformation_size%dimension)
								{
									printf("\n");
								}
	#endif /* defined (DEBUG_CODE) */
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"inherit_FE_element_field.  Insufficient memory");
							return_code=0;
						}
					}
				}
			}
		}
		if (return_code)
		{
			/* guarantee this function returns element_field and field_element */
			if (element_field&&field_element)
			{
				*element_field_address=element_field;
				*field_element_address=field_element;
				*coordinate_transformation_address=coordinate_transformation;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"inherit_FE_element_field.  No element_field or field_element");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"inherit_FE_element_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* inherit_FE_element_field */

int adjacent_FE_element(struct FE_element *element,
	int face_number, int *number_of_adjacent_elements,
	struct FE_element ***adjacent_elements)
{
	int i, j, return_code = CMZN_OK;
	struct FE_element *face;
	FE_element_shape *element_shape = get_FE_element_shape(element);
	if ((element_shape) && (0 <= face_number) && (face_number < element_shape->number_of_faces))
	{
		j = 0;
		if ((element->faces) && (0 != (face = (element->faces)[face_number])) &&
			(face->parents))
		{
			if (ALLOCATE(*adjacent_elements, struct FE_element *, face->number_of_parents))
			{
				for (i = 0; i < face->number_of_parents; ++i)
				{
					if ((face->parents[i]) && (face->parents[i] != element))
					{
						(*adjacent_elements)[j] = face->parents[i];
						++j;
					}
				}
				return_code = CMZN_OK;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"adjacent_FE_element.  Unable to allocate array");
				return_code = CMZN_ERROR_MEMORY;
			}
		}
		else
		{
			*adjacent_elements = 0;
			return_code = CMZN_ERROR_NOT_FOUND;
		}
		*number_of_adjacent_elements = j;
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjacent_FE_element.  Invalid argument(s)");
		return_code = CMZN_ERROR_ARGUMENT;
	}

	return (return_code);
} /* adjacent_FE_element */

int FE_element_log_FE_field_changes(struct FE_element *element,
	struct CHANGE_LOG(FE_field) *fe_field_change_log, bool recurseParents)
{
	int return_code = 1;
	if (element && fe_field_change_log)
	{
		// elements that have been orphaned from parent mesh have no fields member
		if (element->fields)
		{
			/* log fields in this element, if any, and if different set from last */
			if (element->fields != element->fields->fe_mesh->get_last_fe_element_field_info())
			{
				if (0 < NUMBER_IN_LIST(FE_element_field)(
					element->fields->element_field_list))
				{
					if (!FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
						FE_element_field_log_FE_field_change, (void *)fe_field_change_log,
						element->fields->element_field_list))
					{
						return_code = 0;
					}
					element->fields->fe_mesh->set_last_fe_element_field_info(element->fields);
				}
			}
			if (recurseParents)
			{
				for (int i = 0; i < element->number_of_parents; i++)
				{
					if (!FE_element_log_FE_field_changes(element->parents[i], fe_field_change_log, recurseParents))
					{
						return_code = 0;
						break;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_log_FE_field_changes.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int FE_element_meets_topological_criteria(struct FE_element *element,
	int dimension, int exterior, cmzn_element_face_type face,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional, void *conditional_data)
{
	int i, return_code;

	return_code = 0;
	if (element)
	{
		if (dimension == get_FE_element_dimension(element))
		{
			return_code = 1;
			if (0 < element->number_of_parents)
			{
				/* test for exterior element */
				if (exterior)
				{
					if (1 != element->number_of_parents)
					{
						return_code = 0;
						if (1 == dimension)
						{
							for (i = 0; i < element->number_of_parents; i++)
							{
								/* following is not <= 1 so 'exterior' works appropriately on 2-D meshes */
								if (element->parents[i]->number_of_parents == 1)
								{
									return_code = 1;
									break;
								}
							}
						}
					}
				}
				/* test for on correct face */
				if (return_code && (CMZN_ELEMENT_FACE_TYPE_XI1_0 <= face))
				{
					if (NULL == FE_element_get_parent_on_face(
						element, face, conditional, conditional_data))
					{
						return_code = 0;
					}
				}
			}
			else if (exterior || (CMZN_ELEMENT_FACE_TYPE_INVALID != face))
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_meets_topological_criteria.  Invalid argument(s)");
	}
	return (return_code);
}

struct FE_element_field_values *CREATE(FE_element_field_values)(void)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Creates a blank struct FE_element_field_values which can be passed to
calculate_FE_element_field_values. The structure can be restored to its
blank state by calling clear_FE_element_field_values; this should be done
before calling calculate_FE_element_field_values again, and if leaving the
structure unused for some time so it is not accessing objects.
==============================================================================*/
{
	struct FE_element_field_values *element_field_values;

	ENTER(CREATE(FE_element_field_values));
	if (ALLOCATE(element_field_values, struct FE_element_field_values, 1))
	{
		/* clear the contents of the structure */
		element_field_values->field = (struct FE_field *)NULL;
		element_field_values->element = (struct FE_element *)NULL;
		element_field_values->field_element = (struct FE_element *)NULL;
		element_field_values->time_dependent = 0;
		element_field_values->time = 0.0;
		element_field_values->component_number_in_xi = (int **)NULL;
		element_field_values->derivatives_calculated = 0;
		element_field_values->no_modify = (char)0;
		element_field_values->destroy_standard_basis_arguments = 0;
		element_field_values->number_of_components = 0;
		element_field_values->component_number_of_values = (int *)NULL;
		element_field_values->component_grid_values_storage =
			(Value_storage **)NULL;
		element_field_values->component_base_grid_offset = (int *)NULL;
		element_field_values->component_grid_offset_in_xi = (int **)NULL;
		element_field_values->element_value_offsets = (int *)NULL;
		element_field_values->component_values = (FE_value **)NULL;
		element_field_values->component_standard_basis_functions =
			(Standard_basis_function **)NULL;
		element_field_values->component_standard_basis_function_arguments =
			(int **)NULL;
		element_field_values->basis_function_values = (FE_value *)NULL;
		element_field_values->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_element_field_values).  Could not allocate structure");
	}
	LEAVE;

	return (element_field_values);
} /* CREATE(FE_element_field_values) */

int calculate_FE_element_field_values(struct FE_element *element,
	struct FE_field *field, FE_value time, char calculate_derivatives,
	struct FE_element_field_values *element_field_values,
	struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 10 April 2008

DESCRIPTION :
If <field> is NULL, element values are calculated for the coordinate field.  The
function fills in the fields of the <element_field_values> structure, but does
not allocate memory for the structure.
The optional <top_level_element> forces inheritance from it as needed.
???DB.  I think that the field=NULL special case should be removed.
==============================================================================*/
{
	FE_element_field_component_modify modify;
	FE_value *blending_matrix,*coordinate_transformation,
		*derivative_value,*inherited_value,*inherited_values,scalar,
		*second_derivative_value,*transformation,*value,**values_address;
	int component_number,cn,**component_number_in_xi,element_dimension,
		*component_base_grid_offset, *element_value_offsets,
		field_element_dimension,i,
		j,k,grid_maximum_number_of_values, maximum_number_of_values,number_of_components,
		number_of_grid_based_components,
		number_of_inherited_values,number_of_polygon_verticies,number_of_values,
		*number_of_values_address,offset,order,*orders,polygon_offset,power,
		*top_level_component_number_in_xi,
		return_code,row_size,**standard_basis_arguments_address;
	Standard_basis_function **standard_basis_address;
	struct FE_basis *previous_basis;
	struct FE_element *field_element;
	struct FE_element_field *element_field;
	struct FE_element_field_component **component;
	Value_storage **component_grid_values_storage;
#if defined (DOUBLE_FOR_DOT_PRODUCT)
	double sum;
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
	FE_value sum;
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */

	ENTER(calculate_FE_element_field_values);
#if defined (DEBUG_CODE)
	/*???debug */
	printf("enter calculate_FE_element_field_values\n");
#endif /* defined (DEBUG_CODE) */
	/* check the arguments */
	if ((element) && (element->fields) && (element_field_values))
	{
		/* retrieve the element field from which this element inherits the field
			and calculate the affine transformation from the element xi coordinates
			to the xi coordinates for the element field */
		element_field=(struct FE_element_field *)NULL;
		field_element=(struct FE_element *)NULL;
		coordinate_transformation=(FE_value *)NULL;
		if (inherit_FE_element_field(element, /*inherit_face_number*/-1, field, &element_field,
			&field_element, &coordinate_transformation, top_level_element))
		{
			return_code=1;
#if defined (DEBUG_CODE)
			/*???debug */
			printf("element : %d \n",element->get_identifier());
			printf("field element : %d \n",field_element->get_identifier());
#endif /* defined (DEBUG_CODE) */
			element_dimension = element->getDimension();
			field_element_dimension = field_element->getDimension();
			number_of_components=element_field->field->number_of_components;
#if defined (DEBUG_CODE)
			/*???debug */
			printf("coordinate_transformation: %p\n",coordinate_transformation);
			value=coordinate_transformation;
			if (value != 0)
			{
				for (i=field_element_dimension;i>0;i--)
				{
					for (j=element_dimension+1;j>0;j--)
					{
						printf(" %g",*value);
						value++;
					}
					printf("\n");
				}
			}
			printf("%d #components=%d\n",field_element_dimension,
				number_of_components);
#endif /* defined (DEBUG_CODE) */
			switch (field->fe_field_type)
			{
				case CONSTANT_FE_FIELD:
				{
					/* constant fields do not use the values except to remember the
						 element and field they are for */
					element_field_values->field=ACCESS(FE_field)(field);
					element_field_values->element=ACCESS(FE_element)(element);
					/* store field_element since we are now able to suggest through the
						 top_level_element clue which one we get. Must compare element
						 and field_element to ensure field values are still valid for
						 a given line or face. */
					element_field_values->field_element=
						ACCESS(FE_element)(field_element);
					element_field_values->component_number_in_xi=(int **)NULL;
					/* derivatives will be calculated in calculate_FE_element_field */
					/*???DB.  Assuming linear */
					element_field_values->derivatives_calculated=1;
					element_field_values->destroy_standard_basis_arguments=0;
					element_field_values->number_of_components=number_of_components;
					element_field_values->component_number_of_values=(int *)NULL;
					element_field_values->component_grid_values_storage=
						(Value_storage **)NULL;
					element_field_values->component_base_grid_offset=(int *)NULL;
					element_field_values->component_grid_offset_in_xi=(int **)NULL;
					element_field_values->element_value_offsets=(int *)NULL;
					/* clear arrays not used for grid-based fields */
					element_field_values->component_values=(FE_value **)NULL;
					element_field_values->component_standard_basis_functions=
						(Standard_basis_function **)NULL;
					element_field_values->component_standard_basis_function_arguments=
						(int **)NULL;
					element_field_values->basis_function_values=(FE_value *)NULL;
					element_field_values->time_dependent = 0;
					element_field_values->time = time;
				} break;
				case INDEXED_FE_FIELD:
				{
					if (calculate_FE_element_field_values(element,field->indexer_field,
						time,calculate_derivatives,element_field_values,top_level_element))
					{
						/* restore pointer to original field - has the indexer_field in
							 it anyway */
						REACCESS(FE_field)(&(element_field_values->field),field);
					}
					else
					{
						display_message(ERROR_MESSAGE,"calculate_FE_element_field_values.  "
							"Cannot calculate element field values for indexer field");
						return_code=0;
					}
				} break;
				case GENERAL_FE_FIELD:
				{
					ALLOCATE(number_of_values_address,int,number_of_components);
					ALLOCATE(values_address,FE_value *,number_of_components);
					ALLOCATE(standard_basis_address,Standard_basis_function *,
						number_of_components);
					ALLOCATE(standard_basis_arguments_address,int *,
						number_of_components);
					blending_matrix=(FE_value *)NULL;
					ALLOCATE(component_number_in_xi, int *, number_of_components);
					if (number_of_values_address&&values_address&&
						standard_basis_address&&standard_basis_arguments_address&&
						component_number_in_xi)
					{
						for (i=0;i<number_of_components;i++)
						{
							number_of_values_address[i] = 0;
							values_address[i] = (FE_value *)NULL;
							standard_basis_address[i] = (Standard_basis_function *)NULL;
							standard_basis_arguments_address[i]=(int *)NULL;
							/* following is non-NULL only for grid-based components */
							component_number_in_xi[i] = NULL;
						}
						element_field_values->component_number_in_xi = component_number_in_xi;
						element_field_values->field = ACCESS(FE_field)(element_field->field);
						element_field_values->element = ACCESS(FE_element)(element);
						/* store field_element since we are now able to suggest through
							 the top_level_element clue which one we get. Must compare
							 element and field_element to ensure field values are still
							 valid for a given line or face. */
						element_field_values->field_element = ACCESS(FE_element)(field_element);
						element_field_values->time_dependent =
							FE_field_has_multiple_times(element_field_values->field);
						element_field_values->time = time;
						element_field_values->derivatives_calculated=calculate_derivatives;
						if (coordinate_transformation)
						{
							element_field_values->destroy_standard_basis_arguments=1;
						}
						else
						{
							element_field_values->destroy_standard_basis_arguments=0;
						}
						element_field_values->number_of_components=number_of_components;
						element_field_values->component_number_of_values=
							number_of_values_address;
						/* clear arrays only used for grid-based fields */
						element_field_values->component_grid_values_storage=
							(Value_storage **)NULL;
						element_field_values->component_grid_offset_in_xi=(int **)NULL;
						element_field_values->element_value_offsets=(int *)NULL;

						element_field_values->component_values=values_address;
						element_field_values->component_standard_basis_functions=
							standard_basis_address;
						element_field_values->component_standard_basis_function_arguments=
							standard_basis_arguments_address;
						/* maximum_number_of_values starts off big enough for linear grid with derivatives */
						grid_maximum_number_of_values=element_dimension+1;
						for (i=element_dimension;i>0;i--)
						{
							grid_maximum_number_of_values *= 2;
						}
						maximum_number_of_values = grid_maximum_number_of_values;
						/* for each component */
						component=element_field->components;
						previous_basis=(struct FE_basis *)NULL;
						component_number=0;
						return_code=1;
						number_of_grid_based_components = 0;
						int **component_grid_offset_in_xi = 0;
						while (return_code&&(component_number<number_of_components))
						{
							if (ELEMENT_GRID_MAP == (*component)->type)
							{
								number_of_grid_based_components++;
								if (number_of_grid_based_components == number_of_components)
								{
									element_field_values->derivatives_calculated=1;
								}
								/* one-off allocation of arrays only needed for grid-based components */
								if (NULL == element_field_values->component_grid_values_storage)
								{
									ALLOCATE(component_grid_values_storage, Value_storage *, number_of_components);
									ALLOCATE(component_base_grid_offset, int, number_of_components);
									ALLOCATE(component_grid_offset_in_xi, int*, number_of_components);
									ALLOCATE(element_value_offsets, int, grid_maximum_number_of_values);
									if (component_grid_values_storage && component_base_grid_offset &&
										component_grid_offset_in_xi && element_value_offsets)
									{
										for (cn = 0; (cn < number_of_components); cn++)
										{
											component_grid_values_storage[cn]=NULL;
											component_base_grid_offset[cn]=0;
											component_grid_offset_in_xi[cn]=NULL;
										}
										element_field_values->component_grid_values_storage = component_grid_values_storage;
										element_field_values->component_base_grid_offset = component_base_grid_offset;
										element_field_values->component_grid_offset_in_xi = component_grid_offset_in_xi;
										element_field_values->element_value_offsets = element_value_offsets;
									}
									else
									{
										return_code = 0;
									}
								}
								if (return_code)
								{
									element_field_values->component_grid_values_storage[component_number] =
										(field_element->information->values_storage)+
										(((*component)->map).element_grid_based.value_index);
									ALLOCATE(component_number_in_xi[component_number], int, element_dimension);
									ALLOCATE(component_grid_offset_in_xi[component_number], int, element_dimension);
									if ((NULL != component_number_in_xi[component_number]) &&
										(NULL != component_grid_offset_in_xi[component_number]))
									{
										element_field_values->component_base_grid_offset[component_number]=0;
										top_level_component_number_in_xi=
											((*component)->map).element_grid_based.number_in_xi;
										if (!calculate_grid_field_offsets(element_dimension,
											field_element_dimension, top_level_component_number_in_xi,
											coordinate_transformation, component_number_in_xi[component_number],
											&(element_field_values->component_base_grid_offset[component_number]),
											component_grid_offset_in_xi[component_number]))
										{
											display_message(ERROR_MESSAGE,
												"calculate_FE_element_field_values.  "
												"Could not calculate grid field offsets");
											return_code=0;
										}
									}
									else
									{
										return_code = 0;
									}
								}
							}
							else /* not grid-based */
							{
								modify=(FE_element_field_component_modify)NULL;
								if ((element_field_values->no_modify)&&element_field&&
									(element_field->components)[component_number]&&(modify=
									(element_field->components)[component_number]->modify))
								{
									(element_field->components)[component_number]->modify=
										(FE_element_field_component_modify)NULL;
								}
								/* calculate element values for the element field component */
								if (global_to_element_map_values(field_element,element_field,
									time,component_number,number_of_values_address,
									values_address))
								{
									if (modify)
									{
										(element_field->components)[component_number]->modify=
											modify;
										modify=(FE_element_field_component_modify)NULL;
									}
#if defined (DEBUG_CODE)
									/*???debug */
									printf("component_number %d\n",component_number);
#endif /* defined (DEBUG_CODE) */
#if defined (DEBUG_CODE)
									/*???debug */
									{
										FE_value *value;
										int i;

										i= *number_of_values_address;
										printf("component=%d, #values=%d\n",component_number,i);
										value= *values_address;
										while (i>0)
										{
											printf("%.10g ",*value);
											i--;
											value++;
										}
										printf("\n");
									}
#endif /* defined (DEBUG_CODE) */
									if (previous_basis==(*component)->basis)
									{
										*standard_basis_address= *(standard_basis_address-1);
										*standard_basis_arguments_address=
											*(standard_basis_arguments_address-1);
									}
									else
									{
										previous_basis=(*component)->basis;
										if (blending_matrix)
										{
											/* SAB We don't want to keep the old one */
											DEALLOCATE(blending_matrix);
											blending_matrix=NULL;
										}
										*standard_basis_address = FE_basis_get_standard_basis_function(previous_basis);
										if (coordinate_transformation)
										{
											return_code=calculate_standard_basis_transformation(
												previous_basis,coordinate_transformation,
												element_dimension,standard_basis_arguments_address,
												&number_of_inherited_values,standard_basis_address,
												&blending_matrix);
										}
										else
										{
											/* standard basis transformation is just a big identity matrix, so don't compute */
											/* also use the real basis arguments */
											*standard_basis_arguments_address =
												const_cast<int *>(FE_basis_get_standard_basis_function_arguments(previous_basis));
										}
									}
									if (return_code)
									{
										if (!coordinate_transformation)
										{
											/* values already correct regardless of basis, but must make space for derivatives if needed */
											if (calculate_derivatives)
											{
												if (REALLOCATE(inherited_values,*values_address,FE_value,
													(element_dimension+1)*(*number_of_values_address)))
												{
													*values_address=inherited_values;
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"calculate_FE_element_field_values.  Could not reallocate values");
													return_code=0;
												}
											}
										}
										else if ((monomial_basis_functions== *standard_basis_address)||
											(polygon_basis_functions== *standard_basis_address))
										{
											/* project the field_element values onto the lower-dimension element
												 using the affine transformation */
											/* allocate memory for the element values */
											if (calculate_derivatives)
											{
												ALLOCATE(inherited_values,FE_value,
													(element_dimension+1)*number_of_inherited_values);
											}
											else
											{
												ALLOCATE(inherited_values,FE_value,
													number_of_inherited_values);
											}
											if (inherited_values)
											{
												row_size= *number_of_values_address;
												inherited_value=inherited_values;
												for (j=0;j<number_of_inherited_values;j++)
												{
													sum=0;
													value= *values_address;
													transformation=blending_matrix+j;
													for (i=row_size;i>0;i--)
													{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
														sum += (double)(*transformation)*(double)(*value);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
														sum += (*transformation)*(*value);
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
														value++;
														transformation += number_of_inherited_values;
													}
													*inherited_value=(FE_value)sum;
													inherited_value++;
												}
												DEALLOCATE(*values_address);
												*values_address=inherited_values;
												*number_of_values_address=number_of_inherited_values;
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"calculate_FE_element_field_values.  "
													"Insufficient memory for inherited_values");
												DEALLOCATE(*values_address);
												return_code=0;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"calculate_FE_element_field_values.  Invalid basis");
											return_code=0;
										}
										if (return_code)
										{
#if defined (DEBUG_CODE)
											/*???debug */
											printf("number of values=%d\n",
												*number_of_values_address);
											printf("inherited values :");
											value= *values_address;
											for (i= *number_of_values_address;i>0;i--)
											{
												printf(" %g",*value);
												value++;
											}
											printf("\n");
											printf("inherited arguments :");
											orders= *standard_basis_arguments_address;
											for (i=element_dimension;i>=0;i--)
											{
												printf(" %d",*orders);
												orders++;
											}
											printf("\n");
#endif /* defined (DEBUG_CODE) */
											if (calculate_derivatives)
											{
												/* calculate the derivatives with respect to the xi
													 coordinates */
												if (monomial_basis_functions==
													*standard_basis_address)
												{
													number_of_values= *number_of_values_address;
													value= *values_address;
													derivative_value=value+number_of_values;
													orders= *standard_basis_arguments_address;
													offset=1;
													for (i=element_dimension;i>0;i--)
													{
														orders++;
														order= *orders;
														for (j=0;j<number_of_values;j++)
														{
															/* calculate derivative value */
															power=(j/offset)%(order+1);
															if (order==power)
															{
																*derivative_value=0;
															}
															else
															{
																*derivative_value=
																	(FE_value)(power+1)*value[j+offset];
															}
															/* move to the next derivative value */
															derivative_value++;
														}
														offset *= (order+1);
													}
												}
												else if (polygon_basis_functions==
													*standard_basis_address)
												{
													number_of_values= *number_of_values_address;
													value= *values_address;
													derivative_value=value+number_of_values;
													orders= *standard_basis_arguments_address;
													offset=1;
													for (i=element_dimension;i>0;i--)
													{
														orders++;
														order= *orders;
														if (order<0)
														{
															/* polygon */
															order= -order;
															if (order%2)
															{
																/* calculate derivatives with respect to
																	both polygon coordinates */
																order /= 2;
																polygon_offset=order%element_dimension;
																order /= element_dimension;
																number_of_polygon_verticies=
																	(-orders[polygon_offset])/2;
																/* first polygon coordinate is
																	circumferential */
																second_derivative_value=derivative_value+
																	(polygon_offset*number_of_values);
																order=4*number_of_polygon_verticies;
																scalar=
																	(FE_value)number_of_polygon_verticies;
																for (j=0;j<number_of_values;j++)
																{
																	/* calculate derivative values */
																	k=(j/offset)%order;
																	switch (k/number_of_polygon_verticies)
																	{
																		case 0:
																		{
																			*derivative_value=scalar*value[j+
																				number_of_polygon_verticies*offset];
																			*second_derivative_value=value[j+
																				2*number_of_polygon_verticies*
																				offset];
																		} break;
																		case 1:
																		{
																			*derivative_value=0;
																			*second_derivative_value=value[j+
																				2*number_of_polygon_verticies*
																				offset];
																		} break;
																		case 2:
																		{
																			*derivative_value=scalar*value[j+
																				number_of_polygon_verticies*offset];
																			*second_derivative_value=0;
																		} break;
																		case 3:
																		{
																			*derivative_value=0;
																			*second_derivative_value=0;
																		} break;
																	}
																	/* move to the next derivative value */
																	derivative_value++;
																	second_derivative_value++;
																}
																offset *= order;
															}
															else
															{
																/* second polgon xi.  Derivatives already
																	calculated */
																derivative_value += number_of_values;
															}
														}
														else
														{
															/* not polygon */
															for (j=0;j<number_of_values;j++)
															{
																/* calculate derivative value */
																power=(j/offset)%(order+1);
																if (order==power)
																{
																	*derivative_value=0;
																}
																else
																{
																	*derivative_value=
																		(FE_value)(power+1)*value[j+offset];
																}
																/* move to the next derivative value */
																derivative_value++;
															}
															offset *= (order+1);
														}
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"calculate_FE_element_field_values.  "
														"Invalid basis");
													DEALLOCATE(*values_address);
													return_code=0;
												}
											}
#if defined (DEBUG_CODE)
											/*???debug */
											number_of_values= *number_of_values_address;
											for (i=0;i<3;i++)
											{
												printf("%d :",i);
												for (j=0;j<number_of_values;j++)
												{
													printf(" %g",value[i*number_of_values+j]);
												}
												printf("\n");
											}
#endif /* defined (DEBUG_CODE) */
										}
									}
									if (*number_of_values_address>maximum_number_of_values)
									{
										maximum_number_of_values= *number_of_values_address;
									}
									number_of_values_address++;
									values_address++;
									standard_basis_address++;
									standard_basis_arguments_address++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"calculate_FE_element_field_values.  "
										"Could not calculate values");
									return_code=0;
									if (modify)
									{
										(element_field->components)[component_number]->modify=
											modify;
										modify=(FE_element_field_component_modify)NULL;
									}
								}
							}
							component_number++;
							component++;
						}
						if (return_code)
						{
							if (!((maximum_number_of_values>0)&&
								(ALLOCATE(element_field_values->basis_function_values,
								FE_value,maximum_number_of_values))))
							{
								display_message(ERROR_MESSAGE,
									"calculate_FE_element_field_values.  "
									"Could not allocate basis_function_values");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_values.  Insufficient memory");
						DEALLOCATE(number_of_values_address);
						DEALLOCATE(values_address);
						DEALLOCATE(standard_basis_address);
						DEALLOCATE(standard_basis_arguments_address);
						return_code=0;
					}
					if (blending_matrix)
					{
						DEALLOCATE(blending_matrix);
					}
					if (coordinate_transformation)
					{
						DEALLOCATE(coordinate_transformation);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"calculate_FE_element_field_values.  Unknown field type");
					return_code=0;
				} break;
			} /* switch (field->fe_field_type) */
		}
		else
		{
			if (field)
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field_values.  %s not defined for %d-D element %d",
					field->name, get_FE_element_dimension(element), element->get_identifier());
			}
			else
			{
				display_message(ERROR_MESSAGE,"calculate_FE_element_field_values.  "
					"No coordinate fields defined for %d-D element %d",
					get_FE_element_dimension(element), element->get_identifier());
			}
			return_code=0;
#if defined (DEBUG_CODE)
			/*???debug*/
			printf("BAD coordinate_transformation=%p\n",coordinate_transformation);
#endif /* defined (DEBUG_CODE) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_FE_element_field_values.  Invalid argument(s)");
		return_code=0;
	}
#if defined (DEBUG_CODE)
	/*???debug */
	printf("leave calculate_FE_element_field_values %d\n",return_code);
#endif /* defined (DEBUG_CODE) */
	LEAVE;

	return (return_code);
} /* calculate_FE_element_field_values */

int FE_element_field_values_differentiate(
	struct FE_element_field_values *element_field_values, int xi_index)
/*******************************************************************************
LAST MODIFIED : 9 May 2007

DESCRIPTION :
Modifies the calculated values for an FE_field so that it will calculate
derivatives wrt xi_index for the original field.  The <element_field_values>
must have already been calculated.  Currently only implemented for monomials.
==============================================================================*/
{
	FE_value *derivative_value, *value;
	int element_dimension, i, j, k, number_of_values, offset, order,
		*orders, power, return_code;

	ENTER(FE_element_field_values_differentiate);
	if (element_field_values && element_field_values->derivatives_calculated)
	{
		return_code = 1;
		element_dimension = element_field_values->element->getDimension();
		for (k = 0 ; k < element_field_values->number_of_components ; k++)
		{
			if (monomial_basis_functions==
				element_field_values->component_standard_basis_functions[k])
			{
				number_of_values = element_field_values->component_number_of_values[k];
				value = element_field_values->component_values[k];

				/* Copy the specified derivative back into the values */
				derivative_value = value + number_of_values * (xi_index + 1);
				for (j=0;j<number_of_values;j++)
				{
					*value = *derivative_value;
					value++;
					derivative_value++;
				}

				/* Now differentiate the values monomial as we did to calculate them above */

				value = element_field_values->component_values[k];
				derivative_value = value + number_of_values;

				orders= element_field_values->component_standard_basis_function_arguments[k];
				offset = 1;

				for (i=element_dimension;i>0;i--)
				{
					orders++;
					order= *orders;
					for (j=0;j<number_of_values;j++)
					{
						/* calculate derivative value */
						power=(j/offset)%(order+1);
						if (order==power)
						{
							*derivative_value=0;
						}
						else
						{
							*derivative_value=
								(FE_value)(power+1)*value[j+offset];
						}
						/* move to the next derivative value */
						derivative_value++;
					}
					offset *= (order+1);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_values_differentiate.  Unsupported basis type");
				return_code=0;
				break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_values_differentiate.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_values_differentiate */

int clear_FE_element_field_values(
	struct FE_element_field_values *element_field_values)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Frees the memory for the fields of the <element_field_values> structure.
Restores <element_field_values> to the blank state it was created with. This
function must be called before calling calculate_FE_element_field_values again.
==============================================================================*/
{
	FE_value **component_values;
	int i, return_code;
	int **component_standard_basis_function_arguments;

	ENTER(clear_FE_element_field_values);
	return_code=0;
	if (element_field_values)
	{
		return_code=1;
		if (element_field_values->component_number_in_xi)
		{
			if (element_field_values->field)
			{
				for (i = 0; i < element_field_values->number_of_components; i++)
				{
					if (element_field_values->component_number_in_xi[i])
					{
						DEALLOCATE(element_field_values->component_number_in_xi[i]);
					}
				}
			}
			DEALLOCATE(element_field_values->component_number_in_xi);
		}
		DEACCESS(FE_field)(&(element_field_values->field));
		DEACCESS(FE_element)(&(element_field_values->element));
		DEACCESS(FE_element)(&(element_field_values->field_element));
		if (element_field_values->component_number_of_values)
		{
			DEALLOCATE(element_field_values->component_number_of_values);
		}
		if (element_field_values->component_grid_values_storage)
		{
			DEALLOCATE(element_field_values->component_grid_values_storage);
		}
		if (element_field_values->component_base_grid_offset)
		{
			DEALLOCATE(element_field_values->component_base_grid_offset);
		}
		if (element_field_values->component_grid_offset_in_xi)
		{
			for (i = 0; i < element_field_values->number_of_components; i++)
			{
				DEALLOCATE(element_field_values->component_grid_offset_in_xi[i]);
			}
			DEALLOCATE(element_field_values->component_grid_offset_in_xi);
		}
		if (element_field_values->element_value_offsets)
		{
			DEALLOCATE(element_field_values->element_value_offsets);
		}
		if (element_field_values->component_values)
		{
			component_values=element_field_values->component_values;
			for (i=element_field_values->number_of_components;i>0;i--)
			{
				DEALLOCATE(*component_values);
				component_values++;
			}
			DEALLOCATE(element_field_values->component_values);
		}
		if (element_field_values->component_standard_basis_function_arguments)
		{
			if (element_field_values->destroy_standard_basis_arguments)
			{
				component_standard_basis_function_arguments=
					element_field_values->component_standard_basis_function_arguments;
				for (i=element_field_values->number_of_components;i>0;i--)
				{
					if (*component_standard_basis_function_arguments&&((1==i)||
						(*component_standard_basis_function_arguments!=
							component_standard_basis_function_arguments[1])))
					{
						DEALLOCATE(*component_standard_basis_function_arguments);
					}
					component_standard_basis_function_arguments++;
				}
			}
			DEALLOCATE(element_field_values->
				component_standard_basis_function_arguments);
		}
		if (element_field_values->component_standard_basis_functions)
		{
			DEALLOCATE(element_field_values->component_standard_basis_functions);
		}
		if (element_field_values->basis_function_values)
		{
			DEALLOCATE(element_field_values->basis_function_values);
		}
		element_field_values->no_modify=(char)0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"clear_FE_element_field_values.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* clear_FE_element_field_values */

int DESTROY(FE_element_field_values)(
	struct FE_element_field_values **element_field_values_address)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Destroys the element_field_values at *<element_field_info_address>. Frees the
memory for the information and sets <*element_field_info_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_element_field_values *element_field_values;

	ENTER(DESTROY(FE_element_field_values));
	if ((element_field_values_address) &&
		(element_field_values = *element_field_values_address))
	{
		clear_FE_element_field_values(element_field_values);
		DEALLOCATE(*element_field_values_address);
		*element_field_values_address = (struct FE_element_field_values *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_element_field_values).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_element_field_values) */

DECLARE_OBJECT_FUNCTIONS(FE_element_field_values)

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(FE_element_field_values, element, \
	struct FE_element *, compare_pointer)

DECLARE_INDEXED_LIST_FUNCTIONS(FE_element_field_values)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(FE_element_field_values, element, \
	struct FE_element *, compare_pointer)

int FE_element_field_values_set_no_modify(
	struct FE_element_field_values *element_field_values)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Sets the FE_element_field_values no_modify flag.  When an element field values
structure is created, the no_modify flag is unset.
clear_FE_element_field_values also unsets the no_modify flag.

When calculate_FE_element_field_values is called, if the no_modify flag is set
then the field component modify function, if present, is not called.

???DB.  This was added to fix calculating nodal value derivatives for computed
	variables.  It was added as a set function because it is specialized and
	will hopefully be replaced (either by a specialized function for calculating
	nodal value derivatives instead of calculate_FE_element_field_values or a
	better way of doing the modify).
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_values_set_no_modify);
	return_code=0;
	if (element_field_values)
	{
		element_field_values->no_modify=(char)1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_field_values_set_no_modify.  "
			"Missing <element_field_values>");
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_values_set_no_modify */

int FE_element_field_values_get_component_values(
	struct FE_element_field_values *element_field_values,int component_number,
	int *number_of_component_values_address,FE_value **component_values_address)
/*******************************************************************************
LAST MODIFIED : 1 June 2004

DESCRIPTION :
Allocates and returns to <component_values_address> the component values for
<component_number> in <element_field_values>. The number of values is returned
in <number_of_component_values>.
It is up to the calling function to deallocate any returned component values.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_values_get_component_values);
	return_code=0;
	if (element_field_values&&(element_field_values->element)&&
		(0<=component_number)&&
		(component_number<element_field_values->number_of_components)&&
		number_of_component_values_address&&component_values_address)
	{
		if ((element_field_values->component_number_of_values)&&
			(0<(*number_of_component_values_address=
			element_field_values->component_number_of_values[component_number]))&&
			(element_field_values->component_values)&&
			element_field_values->component_values[component_number]&&
			ALLOCATE(*component_values_address,FE_value,
				*number_of_component_values_address))
		{
			memcpy(*component_values_address,
				element_field_values->component_values[component_number],
				(*number_of_component_values_address)*sizeof(FE_value));
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_values_get_component_values.  "
				"Component has no values");
		}
	}
	else
	{
		if (element_field_values)
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_values_get_component_values.  "
				"Invalid argument(s).  %p %p %d %d %p %p",element_field_values,
				element_field_values->element,component_number,
				element_field_values->number_of_components,
				number_of_component_values_address,component_values_address);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_values_get_component_values.  "
				"Invalid argument(s).  %p %d %p %p",element_field_values,
				component_number,number_of_component_values_address,
				component_values_address);
		}
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_values_get_component_values */

int FE_element_field_values_get_monomial_component_info(
	struct FE_element_field_values *element_field_values,int component_number,
	int *monomial_info)
/*******************************************************************************
LAST MODIFIED : 21 May 2004

DESCRIPTION :
If <component_number> in the <element_field_values> is monomial, integer values
describing the monomial basis are returned. The first number is the dimension,
the following numbers are the order of the monomial in each direction, where
3=cubic, for example.
<monomial_info> should point to a block of memory big enough to take
1 + MAXIMUM_ELEMENT_XI_DIMENSIONS integers.
==============================================================================*/
{
	int i, return_code, *source_monomial_info;

	ENTER(FE_element_field_values_get_monomial_component_info);
	return_code=0;
	if (element_field_values&&element_field_values->element&&
		(0<=component_number)&&
		(component_number<element_field_values->number_of_components)&&
		monomial_info)
	{
		if ((element_field_values->component_standard_basis_function_arguments)&&
			(source_monomial_info=element_field_values->
			component_standard_basis_function_arguments[component_number])&&
			standard_basis_function_is_monomial(element_field_values->
			component_standard_basis_functions[component_number],
			(void *)source_monomial_info))
		{
			*monomial_info= *source_monomial_info;
			for (i=1;i<= *monomial_info;i++)
			{
				monomial_info[i]=source_monomial_info[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_values_get_monomial_component_info.  "
				"Component is not monomial");
		}
	}
	else
	{
		if (element_field_values)
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_values_get_monomial_component_info.  "
				"Invalid argument(s).  %p %p %d %d %p",element_field_values,
				element_field_values->element,component_number,
				element_field_values->number_of_components,monomial_info);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_values_get_monomial_component_info.  "
				"Invalid argument(s).  %p %d %p",element_field_values,component_number,
				monomial_info);
		}
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_values_get_monomial_component_info */

int calculate_FE_element_field_nodes(struct FE_element *element,
	int face_number, struct FE_field *field,
	int *number_of_element_field_nodes_address,
	struct FE_node ***element_field_nodes_array_address,
	struct FE_element *top_level_element)
{
	FE_value *blending_matrix, *combined_blending_matrix,
		*coordinate_transformation, *transformation;
	int add,component_number,element_dimension,i,*inherited_basis_arguments,j,k,
		number_of_components,number_of_element_values = 0,number_of_element_field_nodes,
		number_of_inherited_values, number_of_blended_values,
		previous_number_of_element_values,return_code;
	struct FE_basis *basis,*previous_basis;
	struct FE_element *field_element;
	struct FE_element_field *element_field;
	struct FE_element_field_component *component,**component_address;
	struct FE_node **element_field_nodes_array,**element_value,**element_values = NULL,
		**previous_element_values,**temp_element_field_nodes_array;
	Standard_basis_function *standard_basis_function;

	ENTER(calculate_FE_element_field_nodes);
	return_code=0;
	/* check the arguments */
	if ((element) && (element->fields) && (number_of_element_field_nodes_address) &&
		(element_field_nodes_array_address))
	{
		/* retrieve the element field from which this element inherits the field
			and calculate the affine transformation from the element xi coordinates
			to the xi coordinates for the element field */
		coordinate_transformation=(FE_value *)NULL;
		if (inherit_FE_element_field(element, face_number,field,&element_field,&field_element,
			&coordinate_transformation,top_level_element)&&element_field)
		{
			return_code=1;
			number_of_element_field_nodes=0;
			element_field_nodes_array=(struct FE_node **)NULL;
			element_dimension = element->getDimension();
			if (face_number >= 0)
				--element_dimension;
			number_of_components=element_field->field->number_of_components;
			/* for each component */
			component_address=element_field->components;
			previous_basis=(struct FE_basis *)NULL;
			previous_number_of_element_values= -1;
			previous_element_values=(struct FE_node **)NULL;
			component_number=0;
			while (return_code&&(component_number<number_of_components))
			{
				component= *component_address;
				if ((STANDARD_NODE_TO_ELEMENT_MAP==component->type)||
					(GENERAL_ELEMENT_MAP==component->type))
				{
					/* calculate the nodes used by the component in the field_element */
					if (global_to_element_map_nodes(component,field_element,
						element_field->field,&number_of_element_values,&element_values))
					{
						basis = component->basis;
						if (FE_basis_get_number_of_functions(basis) == number_of_element_values)
						{
							if ((i=number_of_element_values)==
								previous_number_of_element_values)
							{
								i--;
								while ((i>=0)&&(element_values[i]==previous_element_values[i]))
								{
									i--;
								}
							}
							if ((i>=0)||(basis!=previous_basis))
							{
								DEALLOCATE(previous_element_values);
								previous_element_values=element_values;
								previous_number_of_element_values=number_of_element_values;
								previous_basis=basis;
								if (calculate_standard_basis_transformation(basis,
									coordinate_transformation,element_dimension,
									&inherited_basis_arguments,&number_of_inherited_values,
									&standard_basis_function,&blending_matrix))
								{
									number_of_blended_values = FE_basis_get_number_of_blended_functions(basis);
									if (number_of_blended_values > 0)
									{
										combined_blending_matrix = FE_basis_calculate_combined_blending_matrix(basis,
											number_of_blended_values, number_of_inherited_values, blending_matrix);
										DEALLOCATE(blending_matrix);
										blending_matrix = combined_blending_matrix;
										if (!blending_matrix)
										{
											display_message(ERROR_MESSAGE,
												"calculate_FE_element_field_nodes.  Could not allocate combined_blending_matrix");
											return_code = 0;
										}
									}
									if (return_code)
									{
										transformation=blending_matrix;
										element_value=element_values;
										i=number_of_element_values;
										while (return_code&&(i>0))
										{
											add=0;
											j=number_of_inherited_values;
											while (!add&&(j>0))
											{
												if (1.e-8<fabs(*transformation))
												{
													add=1;
												}
												transformation++;
												j--;
											}
											transformation += j;
											if (add)
											{
												k=0;
												while ((k<number_of_element_field_nodes)&&
													(*element_value!=element_field_nodes_array[k]))
												{
													k++;
												}
												if (k>=number_of_element_field_nodes)
												{
													if (REALLOCATE(temp_element_field_nodes_array,
														element_field_nodes_array,struct FE_node *,
														number_of_element_field_nodes+1))
													{
														element_field_nodes_array=
															temp_element_field_nodes_array;
														element_field_nodes_array[
															number_of_element_field_nodes]=
															ACCESS(FE_node)(*element_value);
														number_of_element_field_nodes++;
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"calculate_FE_element_field_nodes.  "
															"Could not REALLOCATE element_field_nodes_array");
														return_code=0;
													}
												}
											}
											element_value++;
											i--;
										}
									}
									DEALLOCATE(blending_matrix);
									DEALLOCATE(inherited_basis_arguments);
								}
								else
								{
									return_code=0;
								}
							}
							else
							{
								DEALLOCATE(element_values);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"calculate_FE_element_field_nodes.  Invalid basis");
							DEALLOCATE(element_values);
							return_code=0;
						}
					}
				}
				component_number++;
				component_address++;
			}
			DEALLOCATE(previous_element_values);
			if (return_code)
			{
				*number_of_element_field_nodes_address=number_of_element_field_nodes;
				*element_field_nodes_array_address=element_field_nodes_array;
			}
			else
			{
				for (i=0;i<number_of_element_field_nodes;i++)
				{
					DEACCESS(FE_node)(element_field_nodes_array+i);
				}
				DEALLOCATE(element_field_nodes_array);
			}
		}
		else
		{
			if (field)
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field_nodes.  %s not defined for %d-D element %d",
					field->name, get_FE_element_dimension(element), element->get_identifier());
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field_nodes.  No coordinate fields defined for %d-D element %d",
					get_FE_element_dimension(element), element->get_identifier());
			}
			return_code=0;
		}
		DEALLOCATE(coordinate_transformation);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_FE_element_field_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_FE_element_field_nodes */

int calculate_FE_element_field(int component_number,
	struct FE_element_field_values *element_field_values,
	const FE_value *xi_coordinates, FE_value *values, FE_value *jacobian)
/*******************************************************************************
LAST MODIFIED : 5 August 2001

DESCRIPTION :
Calculates the <values> of the field specified by the <element_field_values> at
the <xi_coordinates>.  The storage for the <values> should have been allocated
outside the function.  The <jacobian> will be calculated if it is not NULL (and
the derivatives values have been calculated).  Only the <component_number>+1
component will be calculated if 0<=component_number<number of components.  For a
single component, the value will be put in the first position of <values> and
the derivatives will start at the first position of <jacobian>.
==============================================================================*/
{
	int cn,comp_no,*component_number_of_values,components_to_calculate,
		i,j,k,l,m, *last_number_in_xi,*number_in_xi,
		number_of_xi_coordinates,recompute_basis,
		return_code,size,this_comp_no,xi_offset;
	FE_value *basis_value,*calculated_value,
		**component_values,*derivative,*element_value,temp,xi_coordinate;
	Standard_basis_function *current_standard_basis_function,
		**component_standard_basis_function;
	struct FE_field *field;
	int **component_standard_basis_function_arguments,
		*current_standard_basis_function_arguments;
	Value_storage **component_grid_values_storage,*element_values_storage;
#if defined (DOUBLE_FOR_DOT_PRODUCT)
	double sum;
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
	FE_value sum;
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */

	ENTER(calculate_FE_element_field);
	return_code=0;
	FE_value *basis_function_values = 0;
	if (element_field_values&&xi_coordinates&&values&&
		(!jacobian||(jacobian&&(element_field_values->derivatives_calculated)))&&
		(field=element_field_values->field)&&
		((GENERAL_FE_FIELD != field->fe_field_type)||
			(basis_function_values=element_field_values->basis_function_values)))
	{
		const int dimension = element_field_values->element->getDimension();
		if ((0<=component_number)&&(component_number<field->number_of_components))
		{
			comp_no=component_number;
			components_to_calculate=1;
		}
		else
		{
			comp_no=0;
			components_to_calculate=field->number_of_components;
		}
		switch (field->fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				return_code=1;
				for (i=0;(i<components_to_calculate)&&return_code;i++)
				{
					if (!get_FE_field_FE_value_value(field,comp_no,&values[i]))
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field.  "
							"Could not get values for constant field %s",field->name);
						return_code=0;
					}
					comp_no++;
				}
				if (jacobian)
				{
					/* derivatives are zero for constant fields */
					derivative=jacobian;
					for (i = (field->number_of_components)*dimension; 0 < i ; --i)
					{
						*derivative = 0.0;
						derivative++;
					}
				}
			} break;
			case INDEXED_FE_FIELD:
			{
				int index,value_no;

				REACCESS(FE_field)(&(element_field_values->field),field->indexer_field);
				if (calculate_FE_element_field_int_values(/*component_number*/0,
					element_field_values,xi_coordinates,&index))
				{
					/* index numbers start at 1 */
					if ((1<=index)&&(index<=field->number_of_indexed_values))
					{
						return_code=1;
						value_no = index-1 + comp_no*field->number_of_indexed_values;
						for (i=0;(i<components_to_calculate)&&return_code;i++)
						{
							if (!get_FE_field_FE_value_value(field,value_no,&values[i]))
							{
								display_message(ERROR_MESSAGE,
									"calculate_FE_element_field.  "
									"Could not get values for constant field %s",field->name);
								return_code=0;
							}
							value_no += field->number_of_indexed_values;
						}
						if (jacobian)
						{
							/* derivatives are zero for indexed fields */
							derivative=jacobian;
							for (i = (field->number_of_components)*dimension; 0 < i ; --i)
							{
								*derivative = 0.0;
								derivative++;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"calculate_FE_element_field.  "
							"Index field %s gave out-of-range index %d in field %s",
							field->indexer_field->name,index,field->name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"calculate_FE_element_field.  "
						"Could not calculate index field %s for field %s at %d-D element %",
						field->indexer_field->name,field->name,
						get_FE_element_dimension(element_field_values->element),
						element_field_values->element->get_identifier());
				}
				REACCESS(FE_field)(&(element_field_values->field),field);
			} break;
			case GENERAL_FE_FIELD:
			{
				/* calculate the value for the element field */
				return_code=1;
				/* calculate a value for each component */
				current_standard_basis_function=(Standard_basis_function *)NULL;
				current_standard_basis_function_arguments=(int *)NULL;
				component_number_of_values=
					element_field_values->component_number_of_values;
				component_values=element_field_values->component_values;
				component_standard_basis_function=
					element_field_values->component_standard_basis_functions;
				component_standard_basis_function_arguments=
					element_field_values->component_standard_basis_function_arguments;
				calculated_value=values;
				if (element_field_values->derivatives_calculated)
				{
					derivative=jacobian;
				}
				else
				{
					derivative=(FE_value *)NULL;
				}
				component_number_of_values += comp_no;
				component_values += comp_no;
				component_standard_basis_function += comp_no;
				component_standard_basis_function_arguments += comp_no;
				number_of_xi_coordinates = dimension;
				int *element_value_offsets = 0;
				int *element_value_offset = 0;
				int number_of_values = 0;
				int offset = 0;
				for (cn=0;(cn<components_to_calculate)&&return_code;cn++)
				{
					this_comp_no = comp_no + cn;
					number_in_xi = element_field_values->component_number_in_xi[this_comp_no];
					if (number_in_xi)
					{
						/* grid based */
						current_standard_basis_function = NULL;
						current_standard_basis_function_arguments = NULL;
						/* optimisation: reuse basis from last component if same number_in_xi */
						recompute_basis = 1;
						if ((cn > 0) && (last_number_in_xi = element_field_values->component_number_in_xi[this_comp_no - 1]))
						{
							recompute_basis = 0;
							for (i = 0; i < number_of_xi_coordinates; i++)
							{
								if (number_in_xi[i] != last_number_in_xi[i])
								{
									recompute_basis = 1;
									break;
								}
							}
						}
						if (recompute_basis)
						{
							number_of_values=1;
							for (i=number_of_xi_coordinates;i>0;i--)
							{
								number_of_values *= 2;
							}
							return_code=1;
							i=0;
							offset=element_field_values->component_base_grid_offset[this_comp_no];
							*basis_function_values=1;
							element_value_offsets = element_field_values->element_value_offsets;
							*element_value_offsets=0;
							m=1;
							while (return_code&&(i<number_of_xi_coordinates))
							{
								xi_coordinate=xi_coordinates[i];
								if (0.0 > xi_coordinate)
								{
									xi_coordinate = 0.0;
								}
								if (xi_coordinate > 1.0)
								{
									xi_coordinate = 1.0;
								}
								/* get xi_offset = lower grid number for cell in xi_coordinate
									 i, and xi_coordinate = fractional xi value in grid cell */
								if (1.0 == xi_coordinate)
								{
									if (number_in_xi[i] > 0)
									{
										xi_offset=number_in_xi[i]-1;
									}
									else
									{
										xi_offset=0;
									}
								}
								else
								{
									xi_coordinate *= (FE_value)(number_in_xi[i]);
									xi_offset=(int)floor((double)xi_coordinate);
									xi_coordinate -= (FE_value)xi_offset;
								}
								offset += xi_offset*element_field_values->component_grid_offset_in_xi[this_comp_no][i];
								/* add grid_offset in xi_coordinate i for neighbouring grid
									 points around the linear cell */
								element_value_offset=element_value_offsets;
								for (l=m;l>0;l--)
								{
									element_value_offset[m]=(*element_value_offset)+
										element_field_values->component_grid_offset_in_xi[this_comp_no][i];
									element_value_offset++;
								}
								temp = 1.0 - xi_coordinate;
								basis_value=basis_function_values;
								if (derivative)
								{
									// derivatives are w.r.t. element xi, not the grid element xi
									FE_value grid_xi_scaling = static_cast<FE_value>(number_in_xi[i]);
									for (j=1;j<=i;j++)
									{
										basis_value=basis_function_values+(j*number_of_values+m);
										for (l=m;l>0;l--)
										{
											basis_value--;
											basis_value[m]=(*basis_value)*xi_coordinate;
											*basis_value *= temp;
										}
									}
									j=(i+1)*number_of_values;
									basis_value=basis_function_values+m;
									for (l=m;l>0;l--)
									{
										basis_value--;
										basis_value[j]= -(*basis_value)*grid_xi_scaling;
										basis_value[j+m]= (*basis_value)*grid_xi_scaling;
									}
								}
								basis_value=basis_function_values+m;
								for (l=m;l>0;l--)
								{
									basis_value--;
									basis_value[m]=(*basis_value)*xi_coordinate;
									*basis_value *= temp;
								}
								m *= 2;
								i++;
							}
						}
						if (return_code)
						{
							size=get_Value_storage_size(field->value_type,
								(struct FE_time_sequence *)NULL);
							component_grid_values_storage=
								element_field_values->component_grid_values_storage + this_comp_no;

							element_values_storage=
								(*component_grid_values_storage)+size*offset;
							basis_value=basis_function_values;
							sum=0;
							element_value_offset=element_value_offsets;
							for (j=number_of_values;j>0;j--)
							{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
								sum += (double)(*basis_value)*(double)(*((FE_value *)(
									element_values_storage+size*(*element_value_offset))));
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
								sum += (*basis_value)*(*((FE_value *)(
									element_values_storage+size*(*element_value_offset))));
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
								element_value_offset++;
								basis_value++;
							}
							*calculated_value=(FE_value)sum;
							if (derivative)
							{
								for (k=number_of_xi_coordinates;k>0;k--)
								{
									sum=0;
									element_value_offset=element_value_offsets;
									for (j=number_of_values;j>0;j--)
									{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
										sum += (double)(*basis_value)*(double)(*((FE_value *)(
											element_values_storage+size*(*element_value_offset))));
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
										sum += (*basis_value)*(*((FE_value *)(
											element_values_storage+size*(*element_value_offset))));
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
										element_value_offset++;
										basis_value++;
									}
									*derivative=(FE_value)sum;
									derivative++;
								}
							}
						}
					}
					else
					{
						/* standard interpolation */
						/* save calculation when all components use the same basis */
						/*???DB.  Only good when consecutive components have the same basis.
							Can do better ? */
						if ((*component_standard_basis_function!=
							current_standard_basis_function)||
							(*component_standard_basis_function_arguments!=
								current_standard_basis_function_arguments))
						{
							current_standard_basis_function=
								*component_standard_basis_function;
							current_standard_basis_function_arguments=
								*component_standard_basis_function_arguments;
							number_of_values= *component_number_of_values;
							/* calculate the values for the standard basis functions */
							if (!(current_standard_basis_function)(
								current_standard_basis_function_arguments,xi_coordinates,
								basis_function_values))
							{
								display_message(ERROR_MESSAGE,"calculate_FE_element_field.  "
									"Error calculating standard basis");
								return_code=0;
							}
						}
						/* calculate the element field value as a dot product of the element
							 values and the basis function values */
						basis_value=basis_function_values;
						element_value= *component_values;
						sum=0;
						for (j=number_of_values;j>0;j--)
						{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
							sum += (double)(*element_value)*(double)(*basis_value);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
							sum += (*element_value)*(*basis_value);
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
							basis_value++;
							element_value++;
						}
						*calculated_value=(FE_value)sum;
						if (derivative)
						{
							for (k=number_of_xi_coordinates;k>0;k--)
							{
								sum=0;
								basis_value=basis_function_values;
								for (j=number_of_values;j>0;j--)
								{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
									sum += (double)(*element_value)*(double)(*basis_value);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
									sum += (*element_value)*(*basis_value);
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
									basis_value++;
									element_value++;
								}
								*derivative=(FE_value)sum;
								derivative++;
							}
						}
					}
					component_number_of_values++;
					component_values++;
					component_standard_basis_function++;
					component_standard_basis_function_arguments++;
					calculated_value++;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field.  Unknown field type");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_FE_element_field.  Invalid argument(s)\n"
			"element_field_values %p, xi_coordinates %p, values %p, jacobian %p",
			element_field_values,xi_coordinates,values,jacobian);
	}
	LEAVE;

	return (return_code);
} /* calculate_FE_element_field */

int calculate_FE_element_field_as_string(int component_number,
	struct FE_element_field_values *element_field_values,
	const FE_value *xi_coordinates, char **string)
/*******************************************************************************
LAST MODIFIED : 19 October 1999

DESCRIPTION :
Calculates the values of element field specified by the <element_field_values>
at the <xi_coordinates> and returns them as the allocated <string>. Only the
<component_number>+1 component will be calculated if
0<=component_number<number of components. If more than 1 component is calculated
then values are comma separated. Derivatives are not included in the string,
even if calculated for the <element_field_values>.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/
{
	char temp_string[40];
	int components_to_calculate,error,i,return_code;
	struct FE_field *field;

	ENTER(calculate_FE_element_field_as_string);
	return_code=0;
	(*string)=(char *)NULL;
	if (element_field_values&&xi_coordinates&&string&&
		(field=element_field_values->field))
	{
		if ((0<=component_number)&&(component_number<field->number_of_components))
		{
			components_to_calculate=1;
		}
		else
		{
			components_to_calculate=field->number_of_components;
		}
		switch (field->value_type)
		{
			case FE_VALUE_VALUE:
			{
				FE_value *values;

				if (ALLOCATE(values,FE_value,components_to_calculate))
				{
					if (calculate_FE_element_field(component_number,
						element_field_values,xi_coordinates,values,
						/*jacobian*/(FE_value *)NULL))
					{
						error=0;
						for (i=0;i<components_to_calculate;i++)
						{
							if (0<i)
							{
								sprintf(temp_string,",%g",values[i]);
							}
							else
							{
								sprintf(temp_string,"%g",values[i]);
							}
							append_string(string,temp_string,&error);
						}
						return_code= !error;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_as_string.  "
							"Could not calculate FE_value values");
					}
					DEALLOCATE(values);
				}
			} break;
			case INT_VALUE:
			{
				int *values;

				if (ALLOCATE(values,int,components_to_calculate))
				{
					if (calculate_FE_element_field_int_values(component_number,
						element_field_values,xi_coordinates,values))
					{
						error=0;
						for (i=0;i<components_to_calculate;i++)
						{
							if (0<i)
							{
								sprintf(temp_string,",%d",values[i]);
							}
							else
							{
								sprintf(temp_string,"%d",values[i]);
							}
							append_string(string,temp_string,&error);
						}
						return_code= !error;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_as_string.  "
							"Could not calculate int values");
					}
					DEALLOCATE(values);
				}
			} break;
			case STRING_VALUE:
			{
				char **values;

				if (ALLOCATE(values,char *,components_to_calculate))
				{
					if (calculate_FE_element_field_string_values(component_number,
						element_field_values,xi_coordinates,values))
					{
						error=0;
						for (i=0;i<components_to_calculate;i++)
						{
							if (0<i)
							{
								append_string(string,",",&error);
							}
							append_string(string,values[i],&error);
						}
						for (i=0;i<components_to_calculate;i++)
						{
							DEALLOCATE(values[i]);
						}
						return_code= !error;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_as_string.  "
							"Could not calculate string values");
					}
					DEALLOCATE(values);
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field_as_string.  Unknown value type %s",
					Value_type_string(field->value_type));
			} break;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"calculate_FE_element_field_as_string.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_FE_element_field_as_string.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* calculate_FE_element_field_as_string */

int calculate_FE_element_field_int_values(int component_number,
	struct FE_element_field_values *element_field_values,
	const FE_value *xi_coordinates, int *values)
/*******************************************************************************
LAST MODIFIED : 19 October 1999

DESCRIPTION :
Calculates the <values> of the integer field specified by the
<element_field_values> at the <xi_coordinates>. The storage for the <values>
should have been allocated outside the function. Only the <component_number>+1
component will be calculated if 0<=component_number<number of components. For a
single component, the value will be put in the first position of <values>.
==============================================================================*/
{
	int cn,comp_no,components_to_calculate,*element_int_values,i,*number_in_xi,
		offset,return_code,this_comp_no,xi_offset;
	FE_value xi_coordinate;
	struct FE_field *field;

	ENTER(calculate_FE_element_field_int_values);
	return_code=0;
	if (element_field_values&&xi_coordinates&&values&&
		(field=element_field_values->field)&&(INT_VALUE==field->value_type))
	{
		if ((0<=component_number)&&(component_number<field->number_of_components))
		{
			comp_no=component_number;
			components_to_calculate=1;
		}
		else
		{
			comp_no=0;
			components_to_calculate=field->number_of_components;
		}
		switch (field->fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				return_code=1;
				for (i=0;(i<components_to_calculate)&&return_code;i++)
				{
					if (!get_FE_field_int_value(field,comp_no,&values[i]))
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_int_values.  "
							"Could not get values for constant field %s",field->name);
						return_code=0;
					}
					comp_no++;
				}
			} break;
			case INDEXED_FE_FIELD:
			{
				int index,value_no;

				REACCESS(FE_field)(&(element_field_values->field),field->indexer_field);
				if (calculate_FE_element_field_int_values(/*component_number*/0,
					element_field_values,xi_coordinates,&index))
				{
					/* index numbers start at 1 */
					if ((1<=index)&&(index<=field->number_of_indexed_values))
					{
						return_code=1;
						value_no = index-1 + comp_no*field->number_of_indexed_values;
						for (i=0;(i<components_to_calculate)&&return_code;i++)
						{
							if (!get_FE_field_int_value(field,value_no,&values[i]))
							{
								display_message(ERROR_MESSAGE,
									"calculate_FE_element_field_int_values.  "
									"Could not get values for constant field %s",field->name);
								return_code=0;
							}
							value_no += field->number_of_indexed_values;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_int_values.  "
							"Index field %s gave out-of-range index %d in field %s",
							field->indexer_field->name,index,field->name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_FE_element_field_int_values.  "
						"Could not calculate index field %s for field %s at %d-D element %",
						field->indexer_field->name,field->name,
						get_FE_element_dimension(element_field_values->element),
						element_field_values->element->get_identifier());
				}
				REACCESS(FE_field)(&(element_field_values->field),field);
			} break;
			case GENERAL_FE_FIELD:
			{
				return_code = 1;
				const int number_of_xi_coordinates = element_field_values->element->getDimension();
				for (cn=0;(cn<components_to_calculate)&&return_code;cn++)
				{
					this_comp_no = comp_no + cn;
					number_in_xi = element_field_values->component_number_in_xi[this_comp_no];
					if (number_in_xi)
					{
						/* for integer, get value at nearest grid point */
						return_code=1;
						offset=element_field_values->component_base_grid_offset[this_comp_no];
						for (i=0;(i<number_of_xi_coordinates)&&return_code;i++)
						{
							xi_coordinate=xi_coordinates[i];
							if (xi_coordinate < 0.0)
							{
								xi_coordinate = 0.0;
							}
							else if (xi_coordinate > 1.0)
							{
								xi_coordinate = 1.0;
							}
							/* get nearest xi_offset */
							xi_offset=(int)floor((double)number_in_xi[i]*(double)xi_coordinate+0.5);
							offset += xi_offset*element_field_values->component_grid_offset_in_xi[this_comp_no][i];
						}
						element_int_values = (int *)element_field_values->component_grid_values_storage[this_comp_no];
						values[cn] = element_int_values[offset];
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_int_values.  "
							"Non-grid-based component for integer valued field");
						return_code = 0;
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field_int_values.  Unknown field type");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_FE_element_field_int_values.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* calculate_FE_element_field_int_values */

int calculate_FE_element_field_string_values(int component_number,
	struct FE_element_field_values *element_field_values,
	const FE_value *xi_coordinates, char **values)
/*******************************************************************************
LAST MODIFIED : 19 October 1999

DESCRIPTION :
Returns allocated copies of the string values of the field specified by the
<element_field_values> at the <xi_coordinates>. <values> must be allocated with
enough space for the number_of_components strings, but the strings themselves
are allocated here. Only the <component_number>+1 component will be calculated
if 0<=component_number<number of components. For a single component, the value
will be put in the first position of <values>.
It is up to the calling function to deallocate the returned string values.
==============================================================================*/
{
	int comp_no,components_to_calculate,i,j,return_code;
	struct FE_field *field;

	ENTER(calculate_FE_element_field_string_values);
	return_code=0;
	if (element_field_values&&xi_coordinates&&values&&
		(field=element_field_values->field)&&(STRING_VALUE==field->value_type))
	{
		if ((0<=component_number)&&(component_number<field->number_of_components))
		{
			comp_no=component_number;
			components_to_calculate=1;
		}
		else
		{
			comp_no=0;
			components_to_calculate=field->number_of_components;
		}
		switch (field->fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				return_code=1;
				for (i=0;(i<components_to_calculate)&&return_code;i++)
				{
					if (!get_FE_field_string_value(field,comp_no,&values[i]))
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_string_values.  "
							"Could not get values for constant field %s",field->name);
						return_code=0;
						for (j=0;j<i;j++)
						{
							DEALLOCATE(values[j]);
						}
					}
					comp_no++;
				}
			} break;
			case INDEXED_FE_FIELD:
			{
				int index,value_no;

				REACCESS(FE_field)(&(element_field_values->field),field->indexer_field);
				if (calculate_FE_element_field_int_values(/*component_number*/0,
					element_field_values,xi_coordinates,&index))
				{
					/* index numbers start at 1 */
					if ((1<=index)&&(index<=field->number_of_indexed_values))
					{
						return_code=1;
						value_no = index-1 + comp_no*field->number_of_indexed_values;
						for (i=0;(i<components_to_calculate)&&return_code;i++)
						{
							if (!get_FE_field_string_value(field,value_no,&values[i]))
							{
								display_message(ERROR_MESSAGE,
									"calculate_FE_element_field_string_values.  "
									"Could not get values for constant field %s",field->name);
								return_code=0;
								for (j=0;j<i;j++)
								{
									DEALLOCATE(values[j]);
								}
							}
							value_no += field->number_of_indexed_values;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_element_field_string_values.  "
							"Index field %s gave out-of-range index %d in field %s",
							field->indexer_field->name,index,field->name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_FE_element_field_string_values.  "
						"Could not calculate index field %s for field %s at %d-D element %",
						field->indexer_field->name,field->name,
						get_FE_element_dimension(element_field_values->element),
						element_field_values->element->get_identifier());
				}
				REACCESS(FE_field)(&(element_field_values->field),field);
			} break;
			case GENERAL_FE_FIELD:
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field_string_values.  "
					"General fields not supported");
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field_string_values.  Unknown field type");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_FE_element_field_string_values.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* calculate_FE_element_field_string_values */

bool equivalent_FE_field_in_elements(struct FE_field *field,
	struct FE_element *element_1, struct FE_element *element_2)
{
	if (field && element_1 && element_1->fields && element_2 && element_2->fields)
	{
		if (element_1->fields == element_2->fields)
			return true;
		else
		{
			FE_element_field *element_field_1 = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field, field)(
				field, element_1->fields->element_field_list);
			FE_element_field *element_field_2 = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field, field)(
				field, element_2->fields->element_field_list);
			if (((!element_field_1) && (!element_field_2)) ||
				FE_element_fields_match(element_field_1, element_1->information,
					element_field_2, element_2->information))
				return true;
		}
	}
	return false;
}

int equivalent_FE_fields_in_elements(struct FE_element *element_1,
	struct FE_element *element_2)
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns true if all fields are defined in the same way at the two elements.
==============================================================================*/
{
	int return_code;

	ENTER(equivalent_FE_fields_in_elements);
	if (element_1 && element_2)
	{
		return_code = (element_1->fields == element_2->fields);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"equivalent_FE_fields_in_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* equivalent_FE_fields_in_elements */

int get_FE_element_dimension(struct FE_element *element)
{
	if (element && element->fields)
		return element->fields->fe_mesh->getDimension();
	display_message(ERROR_MESSAGE, "get_FE_element_dimension.  Invalid element");
	return 0;
}

DsLabelIdentifier get_FE_element_identifier(struct FE_element *element)
{
	if (element)
		return element->get_identifier();
	return DS_LABEL_IDENTIFIER_INVALID;
}

DsLabelIndex get_FE_element_index(struct FE_element *element)
{
	if (element)
		return element->index;
	return DS_LABEL_INDEX_INVALID;
}

void set_FE_element_index(struct FE_element *element, DsLabelIndex index)
{
	if (element)
		element->index = index;
}

bool FE_element_or_parent_changed(struct FE_element *element,
	DsLabelsChangeLog *elementChangeLogs[MAXIMUM_ELEMENT_XI_DIMENSIONS],
	struct CHANGE_LOG(FE_node) *fe_node_change_log)
{
	int dimension = get_FE_element_dimension(element);
	DsLabelsChangeLog *elementChangeLog;
	if (element && (0 < dimension) && elementChangeLogs &&
		((elementChangeLog = elementChangeLogs[dimension - 1])) && fe_node_change_log)
	{
		if (elementChangeLog->isIndexChange(element->index) &&
			(elementChangeLog->getChangeSummary() & (
				DS_LABEL_CHANGE_TYPE_IDENTIFIER | DS_LABEL_CHANGE_TYPE_DEFINITION |
				DS_LABEL_CHANGE_TYPE_RELATED | DS_LABEL_CHANGE_TYPE_REMOVE )))
		{
			return true;
		}
		else
		{
			struct FE_node *node, **nodes;
			/* check nodes, if any; try to make as efficient as possible */
			if (element->information && (nodes = element->information->nodes))
			{
				int node_change;
				int number_of_nodes = element->information->number_of_nodes;
				for (int i = 0; i < number_of_nodes; ++i)
				{
					if ((node = nodes[i]) &&
						CHANGE_LOG_QUERY(FE_node)(fe_node_change_log, node, &node_change) &&
						(node_change & (
							CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_node) |
							CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_node) |
							CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_node))))
					{
						return true;
					}
				}
			}
			/* try parents */
			for (int i = 0; i < element->number_of_parents; i++)
			{
				if (FE_element_or_parent_changed(element->parents[i],
					elementChangeLogs, fe_node_change_log))
				{
					return true;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_or_parent_changed.  Invalid argument(s)");
	}
	return false;
}

int get_FE_element_number_of_fields(struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 4 November 2002

DESCRIPTION :
Returns the number of fields defined at <element>.
Does not include fields inherited from parent elements.
==============================================================================*/
{
	int number_of_fields;

	ENTER(get_FE_element_number_of_fields);
	if (element && element->fields)
	{
		number_of_fields =
			NUMBER_IN_LIST(FE_element_field)(element->fields->element_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_number_of_fields.  Missing element");
		number_of_fields = 0;
	}
	LEAVE;

	return (number_of_fields);
} /* get_FE_element_number_of_fields */

int get_FE_element_number_of_parents(struct FE_element *element)
{
	if (element)
		return element->number_of_parents;
	display_message(ERROR_MESSAGE, "get_FE_element_number_of_parents.  Invalid argument(s)");
	return 0;
}

bool FE_element_has_no_parents(cmzn_element *element)
{
	if (element)
		return (element->number_of_parents == 0);
	return false;
}

struct FE_element *get_FE_element_parent(struct FE_element *element, int index)
{
	if ((element) && (0 <= index) && (index < element->number_of_parents))
		return element->parents[index];
	display_message(ERROR_MESSAGE, "get_FE_element_parent.  Invalid argument(s)");
	return 0;
}

int FE_element_get_first_parent(struct FE_element *element,
	struct FE_element **parent_element_address, int *face_number_address)
{
	int return_code;
	if ((element) && (parent_element_address) && (face_number_address))
	{
		if ((element->parents) && (0 < element->number_of_parents))
		{
			*parent_element_address = element->parents[0];
			*face_number_address =
				FE_element_get_child_face_number(*parent_element_address, element);
		}
		else
		{
			*parent_element_address = (struct FE_element *)NULL;
			*face_number_address = 0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_get_first_parent.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

FE_element_shape *get_FE_element_shape(struct FE_element *element)
{
	if (element)
	{
		if (element->fields)
			return element->fields->fe_mesh->getElementShape(element->index);
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_FE_element_shape.  Invalid argument(s)");
	}
	return 0;
}

int get_FE_element_face(struct FE_element *element,int face_number,
	struct FE_element **face_element)
/*******************************************************************************
LAST MODIFIED : 12 October 1999

DESCRIPTION :
Returns the <face_element> for face <face_number> of <element>, where NULL means
there is no face. Element must have a shape and face.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_element_face);
	// Temporary:
	FE_element_shape *element_shape = get_FE_element_shape(element);
	if ((element_shape) && (0<=face_number) &&
		(face_number<element_shape->number_of_faces)&&face_element)
	{
		if (element->faces)
			*face_element = element->faces[face_number];
		else
			*face_element = 0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_FE_element_face.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_face */

int set_FE_element_face(struct FE_element *element, int face_number,
	struct FE_element *face_element)
{
	int return_code;
	// Temporary:
	FE_element_shape *element_shape = get_FE_element_shape(element);
	if ((element_shape) && (0 <= face_number) &&
		(face_number < element_shape->number_of_faces))
	{
		if (!element->faces)
		{
			if (!set_FE_element_number_of_faces(element, element_shape->number_of_faces))
				return 0;
		}
		return_code = 1;
		/* check this face not already set, else following logic will fail! */
		FE_element *old_face_element = element->faces[face_number];
		if (face_element != old_face_element)
		{
			if (face_element)
			{
				/* set up new face->parent relationship */
				FE_element **new_parents;
				if (REALLOCATE(new_parents, face_element->parents,
					struct FE_element *, face_element->number_of_parents + 1))
				{
					face_element->parents = new_parents;
					face_element->parents[face_element->number_of_parents] = element;
					(face_element->number_of_parents)++;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_FE_element_face.  Could not set element as parent of face");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (old_face_element)
				{
					/* remove first instance of element from face->parents array */
					for (int i = 0; i < old_face_element->number_of_parents; i++)
					{
						if (old_face_element->parents[i] == element)
						{
							old_face_element->number_of_parents--;
							for (int j = i; j < old_face_element->number_of_parents; j++)
							{
								old_face_element->parents[j] = old_face_element->parents[j + 1];
							}
							break;
						}
					}
					if (0 == old_face_element->number_of_parents)
					{
						DEALLOCATE(old_face_element->parents);
					}
				}
				/* set the new face */
				REACCESS(FE_element)(&(element->faces[face_number]), face_element);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_FE_element_face.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int get_FE_element_face_number(struct FE_element *element, struct FE_element *face)
{
	FE_element_shape *element_shape = get_FE_element_shape(element);
	if ((element_shape) && (element->faces) && face)
	{
		const int nFaces = element_shape->number_of_faces;
		for (int i = 0; i < nFaces; ++i)
		{
			if (element->faces[i] == face)
				return i;
		}
	}
	else
		display_message(ERROR_MESSAGE, "get_FE_element_face_number.  Invalid argument(s)");
	return -1;
}

int set_FE_element_number_of_nodes(struct FE_element *element,
	int number_of_nodes)
/*******************************************************************************
LAST MODIFIED : 11 February 2003

DESCRIPTION :
Establishes storage for <number_of_nodes> in <element>.
May only be set once; should only be called for unmanaged elements.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_element_number_of_nodes);
	if (element && (0 <= number_of_nodes))
	{
		if (element->information ||
			(element->information = FE_element_node_scale_field_info::create()))
		{
			return_code = (CMZN_OK == element->information->setNumberOfNodes(number_of_nodes));
		}
		else
		{
			display_message(ERROR_MESSAGE, "set_FE_element_number_of_nodes.  "
				"Could not create node_scale_field_info");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_element_number_of_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_element_number_of_nodes */

int get_FE_element_number_of_nodes(struct FE_element *element,
	int *number_of_nodes_address)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Returns the number of nodes directly referenced by <element>; does not include
nodes used by fields inherited from parent elements.
If fails, puts zero at <number_of_nodes_address>.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_element_number_of_nodes);
	if (element && number_of_nodes_address)
	{
		if (element->information)
		{
			*number_of_nodes_address = element->information->number_of_nodes;
		}
		else
		{
			*number_of_nodes_address = 0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_number_of_nodes.  Invalid argument(s)");
		if (number_of_nodes_address)
		{
			*number_of_nodes_address = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_number_of_nodes */

int FE_element_has_FE_node(struct FE_element *element, void *node_void)
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Returns true if <element> references the <node>.
==============================================================================*/
{
	int i, number_of_nodes, return_code;
	struct FE_node *node, **nodes;

	ENTER(FE_element_has_FE_node);
	return_code = 0;
	if (element && (node = (struct FE_node *)node_void))
	{
		if (element->information && (nodes = element->information->nodes) &&
			(0 < (number_of_nodes = element->information->number_of_nodes)))
		{
			for (i = 0; i < number_of_nodes; i++)
			{
				if (nodes[i] == node)
				{
					return_code = 1;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_has_FE_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_has_FE_node */

int get_FE_element_node(struct FE_element *element,int node_number,
	struct FE_node **node)
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Gets node <node_number>, from 0 to number_of_nodes-1 of <element> in <node>.
<element> must already have a shape and node_scale_field_information.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_element_node);
	if (element && element->information &&
		element->information->nodes&&(0<=node_number)&&
		(node_number<element->information->number_of_nodes)&&node)
	{
		*node=element->information->nodes[node_number];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_FE_element_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_node */

int set_FE_element_node(struct FE_element *element,int node_number,
  struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 11 February 2003

DESCRIPTION :
Sets node <node_number>, from 0 to number_of_nodes-1 of <element> to <node>.
<element> must already have a shape and node_scale_field_information.
Should only be called for unmanaged elements.
==============================================================================*/
{
  int return_code;

  ENTER(set_FE_element_node);
  if (element && element->information && node)
  {
	  return_code = (CMZN_OK == element->information->setNode(node_number, node));
  }
  else
  {
	  display_message(ERROR_MESSAGE,"set_FE_element_node.  Invalid argument(s)");
	  return_code=0;
  }
  LEAVE;

  return (return_code);
} /* set_FE_element_node */

int get_FE_element_scale_factors_address(struct FE_element *element,
	cmzn_mesh_scale_factor_set *scale_factor_set, FE_value **scale_factors_address)
{
	int number_of_scale_factors = 0;
	if (element && element->information && scale_factor_set && scale_factors_address)
	{
		*scale_factors_address = element->information->getScaleFactorsForSet(scale_factor_set, number_of_scale_factors);
	}
	else if (scale_factors_address)
	{
		*scale_factors_address = 0;
	}
	return number_of_scale_factors;
}

int set_FE_element_number_of_scale_factor_sets(struct FE_element *element,
	int number_of_scale_factor_sets, cmzn_mesh_scale_factor_set **scale_factor_set_identifiers,
	int *numbers_in_scale_factor_sets)
{
	if (element)
	{
		if ((0 < number_of_scale_factor_sets) && !element->information)
		{
			element->information = FE_element_node_scale_field_info::create();
			if (!element->information)
			{
				return CMZN_ERROR_MEMORY;
			}
		}
		if (element->information)
		{
			return element->information->setScaleFactorSets(number_of_scale_factor_sets,
				scale_factor_set_identifiers, numbers_in_scale_factor_sets, static_cast<FE_value*>(0));
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int get_FE_element_number_of_scale_factor_sets(struct FE_element *element,
	int *number_of_scale_factor_sets_address)
{
	int return_code;

	ENTER(get_FE_element_number_of_scale_factor_sets);
	if (element && number_of_scale_factor_sets_address)
	{
		if (element->information)
		{
			*number_of_scale_factor_sets_address =
				element->information->getNumberOfScaleFactorSets();
		}
		else
		{
			*number_of_scale_factor_sets_address = 0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_number_of_scale_factor_sets.  Invalid element");
		if (number_of_scale_factor_sets_address)
		{
			*number_of_scale_factor_sets_address = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int get_FE_element_number_in_scale_factor_set_at_index(struct FE_element *element,
	int scale_factor_set_number)
{
	if (element && element->information)
	{
		return element->information->getNumberInScaleFactorSetAtIndex(scale_factor_set_number);
	}
	return 0;
}

cmzn_mesh_scale_factor_set *get_FE_element_scale_factor_set_identifier_at_index(
	struct FE_element *element, int scale_factor_set_number)
{
	if (element && element->information)
	{
		return element->information->getScaleFactorSetIdentifierAtIndex(scale_factor_set_number);
	}
	return 0;
}

int set_FE_element_scale_factor_set_identifier_at_index(
	struct FE_element *element, int scale_factor_set_number,
	cmzn_mesh_scale_factor_set *scale_factor_set_identifier)
{
	if (element && element->information)
	{
		return element->information->setScaleFactorSetIdentifierAtIndex(
			scale_factor_set_number, scale_factor_set_identifier);
	}
	return CMZN_ERROR_ARGUMENT;
}

int get_FE_element_number_of_scale_factors(struct FE_element *element,
	int *number_of_scale_factors_address)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the number of scale factors stored with <element>.
If fails, puts zero at <number_of_scale_factors_address>.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_element_number_of_scale_factors);
	if (element && number_of_scale_factors_address)
	{
		if (element->information)
		{
			*number_of_scale_factors_address =
				element->information->number_of_scale_factors;
		}
		else
		{
			*number_of_scale_factors_address = 0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_number_of_scale_factors.  Invalid element");
		if (number_of_scale_factors_address)
		{
			*number_of_scale_factors_address = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_number_of_scale_factors */

int get_FE_element_scale_factor(struct FE_element *element,
	int scale_factor_number, FE_value *scale_factor_address)
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Gets scale_factor <scale_factor_number>, from 0 to number_of_scale_factors-1 of
<element> to <scale_factor>.
<element> must already have a shape and node_scale_field_information.
If fails, sets *<scale_factor_address> to 0.0;
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_element_scale_factor);
	if (element && element->information &&
		element->information->scale_factors && (0 <= scale_factor_number) &&
		(scale_factor_number < element->information->number_of_scale_factors) &&
		scale_factor_address)
	{
		*scale_factor_address =
			element->information->scale_factors[scale_factor_number];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_scale_factor.  Invalid argument(s)");
		if (scale_factor_address)
		{
			*scale_factor_address = 0.0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_scale_factor */

int set_FE_element_scale_factor(struct FE_element *element,
	int scale_factor_number, FE_value scale_factor)
{
	int return_code;
	if (element && element->information&&
		element->information->scale_factors&&(0<=scale_factor_number)&&
		(scale_factor_number<element->information->number_of_scale_factors))
	{
		return_code=1;
		element->information->scale_factors[scale_factor_number]=scale_factor;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_element_scale_factor.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

int define_FE_field_at_element(struct FE_element *element,
	struct FE_field *field, struct FE_element_field_component **components)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Defines <field> at <element> using the given <components>. <element> must
already have a shape and node_scale_field_information.
Checks the range of nodes, scale factors etc. referred to by the components are
within the range of the node_scale_field_information, and that the basis
functions are compatible with the element shape.
Value types other than FE_VALUE_VALUE are only supported for grid-based element
fields and constant and indexed FE_fields.
The <components> are duplicated by this functions, so the calling function must
destroy them.
Should only be called for unmanaged elements.
???RC Should have more checks for STANDARD_NODE_TO_ELEMENT_MAP etc.
==============================================================================*/
{
	enum FE_basis_type grid_basis_type;
	int component_number_of_values, dimension, i, j, number_of_components,
		number_of_grid_based_components, old_values_storage_size,
		return_code, *this_number_in_xi;
	struct FE_element_field *element_field;
	struct FE_element_field_component **component;
	struct FE_element_field_info *existing_element_field_info,
		*new_element_field_info;
	struct FE_element_field_lists_merge_data merge_data;
	FE_mesh *fe_mesh;
	struct FE_region *fe_region;
	struct LIST(FE_element_field) *element_field_list;
	Value_storage *values_storage;

	ENTER(define_FE_field_at_element);
	return_code = 0;
	if (element && (dimension = get_FE_element_dimension(element)) &&
		field && (fe_region = FE_field_get_FE_region(field)) &&
		(existing_element_field_info = element->fields) &&
		(fe_mesh = existing_element_field_info->fe_mesh) &&
		(fe_mesh->get_FE_region() == fe_region) &&
		element->information &&
		(0 < (number_of_components = get_FE_field_number_of_components(field))) &&
		components)
	{
		return_code = 1;
		/* check if the field is already defined at the element */
		if (FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(field,
			existing_element_field_info->element_field_list))
		{
			display_message(ERROR_MESSAGE,
				"define_FE_field_at_element.  Field %s already defined at %d-D element %d",
				field->name, get_FE_element_dimension(element),
				element->get_identifier());
			return_code = 0;
		}
		int number_of_values = 0;
		switch (field->fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				/* nothing to check */
			} break;
			case INDEXED_FE_FIELD:
			{
				/* no longer perform this check since it prevents us from reading in
					 indexed fields when the indexer is not in the same header */
			} break;
			case GENERAL_FE_FIELD:
			{
				/* check the components are all there, and if grid-based ensure they
					 have consistent number_in_xi and all have value_index set to 0 */
				number_of_grid_based_components = 0;
				component = components;
				for (i = 0; (i < number_of_components) && return_code; i++)
				{
					if (*component)
					{
						if (ELEMENT_GRID_MAP == (*component)->type)
						{
							number_of_grid_based_components++;
							this_number_in_xi =
								(*component)->map.element_grid_based.number_in_xi;
							component_number_of_values = 1;
							for (j = 0; j < dimension; j++)
							{
								if (0 <= this_number_in_xi[j])
								{
									component_number_of_values *= (this_number_in_xi[j] + 1);
									grid_basis_type = FE_BASIS_TYPE_INVALID;
									FE_basis_get_xi_basis_type((*component)->basis,
										/*xi_number*/j, &grid_basis_type);
									if ((0 == this_number_in_xi[j]) && (grid_basis_type != FE_BASIS_CONSTANT))
									{
										display_message(ERROR_MESSAGE, "define_FE_field_at_element.  "
											"Grid-map component must have constant basis for 0 cells in xi %d", j+1);
										return_code = 0;
									}
									else if ((0 < this_number_in_xi[j]) && (grid_basis_type != LINEAR_LAGRANGE))
									{
										display_message(ERROR_MESSAGE, "define_FE_field_at_element.  "
											"Grid-map component must have linear basis for > 0 cells in xi %d", j+1);
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"define_FE_field_at_element.  "
										"Grid-map components number of cells in xi %d < 0", j+1);
									return_code = 0;
								}
							}
							number_of_values += component_number_of_values;
							/* check value_index is 0 for all components, ie. pointing at
								 start of values_storage allocated below */
							if ((*component)->map.element_grid_based.value_index != 0)
							{
								display_message(ERROR_MESSAGE,"define_FE_field_at_element.  "
									"Grid-map components must have 0 value_index");
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"define_FE_field_at_element.  Missing component");
						return_code = 0;
					}
					component++;
				}
				/* only FE_VALUE_VALUE and SHORT_VALUE supported by general */
				if ((number_of_grid_based_components < number_of_components) &&
					(FE_VALUE_VALUE != field->value_type) &&
					(SHORT_VALUE != field->value_type))
				{
					display_message(ERROR_MESSAGE,"define_FE_field_at_element.  "
						"%s type only supported for grid-based field components",
						Value_type_string(field->value_type));
					return_code = 0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"define_FE_field_at_element.  Unknown field type");
				return_code = 0;
			} break;
		}
		if (return_code)
		{
			if (NULL != (element_field = CREATE(FE_element_field)(field)))
			{
				ACCESS(FE_element_field)(element_field);
				/* make a copy of the element_field_list to put new element_field in */
				element_field_list = CREATE_LIST(FE_element_field)();
				if (COPY_LIST(FE_element_field)(element_field_list,
					existing_element_field_info->element_field_list))
				{
					if (GENERAL_FE_FIELD == field->fe_field_type)
					{
						/* put components in element_field for merge */
						for (i = 0; i < number_of_components; i++)
						{
							element_field->components[i] = components[i];
						}
						/* use merge_FE_element_field_into_list to add field to element */
						merge_data.list = element_field_list;
						merge_data.merge_info = element->information;
						merge_data.source_info = element->information;
						if (existing_element_field_info)
						{
							merge_data.values_storage_size =
								element->information->values_storage_size;
						}
						else
						{
							merge_data.values_storage_size = 0;
						}
						old_values_storage_size = merge_data.values_storage_size;
						if (merge_FE_element_field_into_list(element_field,
							(void *)&merge_data))
						{
							/* allocate and initialise any added values_storage */
							if (merge_data.values_storage_size > old_values_storage_size)
							{
								if (REALLOCATE(values_storage,
									element->information->values_storage,
									Value_storage, merge_data.values_storage_size))
								{
									element->information->values_storage = values_storage;
									if (initialise_value_storage_array(
										values_storage + old_values_storage_size,
										field->value_type, (struct FE_time_sequence *)NULL,
										number_of_values))
									{
										element->information->values_storage_size =
											merge_data.values_storage_size;
									}
									else
									{
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"define_FE_field_at_element.  "
										"Could not reallocate values_storage");
									return_code = 0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_FE_field_at_element.  Could not merge element field");
							return_code = 0;
						}
						/* clear the components from element_field since they are owned by
							 the calling function and do not want them destroyed here */
						for (i = 0; i < number_of_components; i++)
						{
							element_field->components[i] =
								(struct FE_element_field_component *)NULL;
						}
					}
					else
					{
						if (!ADD_OBJECT_TO_LIST(FE_element_field)(element_field,
							element_field_list))
						{
							display_message(ERROR_MESSAGE,
								"define_FE_field_at_element.  Could not add element field");
							return_code = 0;
						}
					}
					if (return_code)
					{
						if (NULL != (new_element_field_info =
							fe_mesh->get_FE_element_field_info(element_field_list)))
						{
							REACCESS(FE_element_field_info)(&(element->fields),
								new_element_field_info);
						}
						else
						{
							display_message(ERROR_MESSAGE, "define_FE_field_at_element.  "
								"Could not get element field information");
							return_code = 0;
						}
					}
					DESTROY_LIST(FE_element_field)(&element_field_list);
				}
				else
				{
					display_message(ERROR_MESSAGE, "define_FE_field_at_element.  "
						"Could not duplicate element_field_list");
					return_code = 0;
				}
				DEACCESS(FE_element_field)(&element_field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_FE_field_at_element.  Could not create element_field");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_FE_field_at_element.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* define_FE_field_at_element */

static int FE_element_field_has_element_grid_map(
	struct FE_element_field *element_field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1999

DESCRIPTION :
Returns true if <element_field> components are of type ELEMENT_GRID_MAP.
Only checks the first component since we assume all subsequent components have
the same basis and numbers of grid cells in xi.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_has_element_grid_map);
	USE_PARAMETER(dummy_void);
	return_code=0;
	if (element_field&&element_field->field)
	{
		if (GENERAL_FE_FIELD==element_field->field->fe_field_type)
		{
			if (element_field->components)
			{
				if (ELEMENT_GRID_MAP == (*(element_field->components))->type)
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_has_element_grid_map.  Missing components");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_has_element_grid_map.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_has_element_grid_map */

int FE_element_has_grid_based_fields(struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns true if any of the fields defined for element is grid-based.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_has_grid_based_fields);
	return_code = 0;
	if (element && element->fields)
	{
		/* would have to have values_storage to have a grid-based field */
		if (element->information && element->information->values_storage)
		{
			if (FIRST_OBJECT_IN_LIST_THAT(FE_element_field)(
				FE_element_field_has_element_grid_map, (void *)NULL,
				element->fields->element_field_list))
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_has_grid_based_fields.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_has_grid_based_fields */

int FE_element_field_is_standard_node_based(struct FE_element *element,
	struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 12 March 2003

DESCRIPTION :
Returns true if <fe_field> is defined on <element> using a standard node to
element map for any element. Does not consider inherited fields.
==============================================================================*/
{
	int i, number_of_components, return_code;
	struct FE_element_field *element_field;
	struct FE_element_field_component **components;

	ENTER(FE_element_field_is_standard_node_based);
	return_code = 0;
	if (element && element->fields && fe_field)
	{
		/* would have to have values_storage to have a grid-based field */
		if ((element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
			fe_field, element->fields->element_field_list)) &&
			(number_of_components = get_FE_field_number_of_components(fe_field)) &&
			(components = element_field->components))
		{
			for (i = 0; (!return_code) && (i < number_of_components); i++)
			{
				if (components[i] &&
					(STANDARD_NODE_TO_ELEMENT_MAP == (components[i])->type))
				{
					return_code = 1;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_is_standard_node_based.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_is_standard_node_based */

static int FE_element_field_has_FE_field_values(
	struct FE_element_field *element_field,void *dummy)
/*******************************************************************************
LAST MODIFIED: 19 October 1999

DESCRIPTION:
Returns true if <element_field> has a field with values_storage.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_has_FE_field_values);
	USE_PARAMETER(dummy);
	if (element_field&&element_field->field)
	{
		return_code=(0<element_field->field->number_of_values);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_has_FE_field_values.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_has_FE_field_values */

int FE_element_has_FE_field_values(struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns true if any single field defined at <element> has values stored with
the field. Returns 0 without error if no field information at element.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_has_FE_field_values);
	return_code=0;
	if (element && element->fields)
	{
		return_code = ((struct FE_element_field *)NULL !=
			FIRST_OBJECT_IN_LIST_THAT(FE_element_field)(
				FE_element_field_has_FE_field_values, (void *)NULL,
				element->fields->element_field_list));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_has_FE_field_values.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_has_FE_field_values */

int FE_element_has_values_storage(struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 24 October 2002

DESCRIPTION :
Returns true if <element> has values_storage, eg. for grid-based fields.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_has_values_storage);
	if (element)
	{
		if (element->information && element->information->values_storage)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_has_values_storage.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_has_values_storage */

int for_FE_field_at_element(struct FE_field *field,
	FE_element_field_iterator_function *iterator, void *user_data,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
If an <iterator> is supplied and the <field> is defined at the <element> then
the result of the <iterator> is returned.  Otherwise, if an <iterator> is not
supplied and the <field> is defined at the <element> then a non-zero is
returned. Otherwise, zero is returned.
???DB.  Multiple behaviour dangerous ?
==============================================================================*/
{
	int return_code;
	struct FE_element_field *element_field;
	struct FE_element_field_iterator_and_data iterator_and_data;

	ENTER(for_FE_field_at_element);
	return_code = 0;
	if (field && element && element->fields)
	{
		if (NULL != (element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
			field, element->fields->element_field_list)))
		{
			if (iterator)
			{
				iterator_and_data.iterator = iterator;
				iterator_and_data.user_data = user_data;
				iterator_and_data.element = element;
				return_code = for_FE_field_at_element_iterator(element_field,
					&iterator_and_data);
			}
			else
			{
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"for_FE_field_at_element.  Field not defined at element");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_FE_field_at_element.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* for_FE_field_at_element */

int for_each_FE_field_at_element(FE_element_field_iterator_function *iterator,
	void *user_data,struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Calls the <iterator> for each field defined at the <element> until the
<iterator> returns 0 or it runs out of fields.  Returns the result of the last
<iterator> called.
==============================================================================*/
{
	int return_code;
	struct FE_element_field_iterator_and_data iterator_and_data;

	ENTER(for_each_FE_field_at_element);
	if (iterator && element && element->fields)
	{
		iterator_and_data.iterator = iterator;
		iterator_and_data.user_data = user_data;
		iterator_and_data.element = element;
		return_code = FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
			for_FE_field_at_element_iterator, &iterator_and_data,
			element->fields->element_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_FE_field_at_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* for_each_FE_field_at_element */

int for_each_FE_field_at_element_alphabetical_indexer_priority(
	FE_element_field_iterator_function *iterator,void *user_data,
	struct FE_element *element)
{
	int i, number_of_fields, return_code;
	struct FE_field *field;
	struct FE_field_order_info *field_order_info;
	struct FE_element_field *element_field;
	struct FE_region *fe_region;

	ENTER(for_each_FE_field_at_element_alphabetical_indexer_priority);
	return_code = 0;
	if (iterator && element && element->fields)
	{
		// get list of all fields in default alphabetical order
		field_order_info = CREATE(FE_field_order_info)();
		fe_region = element->fields->fe_mesh->get_FE_region();
		return_code = FE_region_for_each_FE_field(fe_region,
			FE_field_add_to_FE_field_order_info, (void *)field_order_info);
		FE_field_order_info_prioritise_indexer_fields(field_order_info);
		number_of_fields = get_FE_field_order_info_number_of_fields(field_order_info);
		for (i = 0; i < number_of_fields; i++)
		{
			field = get_FE_field_order_info_field(field_order_info, i);
			element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(field,
				element->fields->element_field_list);
			if (element_field)
			{
				return_code = (iterator)(element, field, user_data);
			}
		}
		DESTROY(FE_field_order_info)(&field_order_info);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_FE_field_at_element_alphabetical_indexer_priority.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

struct FE_field *get_FE_element_default_coordinate_field(
	struct FE_element *element)
{
	struct FE_field *default_coordinate_field;

	ENTER(get_FE_element_default_coordinate_field);
	default_coordinate_field = (struct FE_field *)NULL;
	if (element && element->fields)
	{
		FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
			FE_element_field_get_first_coordinate_field,
			(void *)&default_coordinate_field, element->fields->element_field_list);
		if ((!default_coordinate_field) &&
			element->parents && (0 < element->number_of_parents))
		{
			default_coordinate_field =
				get_FE_element_default_coordinate_field(element->parents[0]);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_default_coordinate_field.  Invalid element");
	}
	LEAVE;

	return (default_coordinate_field);
}

int FE_element_find_default_coordinate_field_iterator(
	struct FE_element *element, void *fe_field_ptr_void)
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
An FE_element iterator that returns 1 when an appropriate default_coordinate
fe_field is found.  The fe_field found is returned as fe_field_void.
==============================================================================*/
{
	int return_code;
	struct FE_field *field;

	ENTER(FE_element_find_default_coordinate_field_iterator);
	if (element)
	{
		if (NULL != (field = get_FE_element_default_coordinate_field(element)))
		{
			*((struct FE_field **)fe_field_ptr_void) = field;
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_find_default_coordinate_field_iterator.  Missing element");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_find_default_coordinate_field_iterator */

int FE_element_number_is_in_Multi_range(struct FE_element *element,
	void *multi_range_void)
{
	int return_code;
	struct Multi_range *multi_range = (struct Multi_range *)multi_range_void;
	if (element && multi_range)
	{
		return_code = Multi_range_is_value_in_range(multi_range,
			element->get_identifier());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_number_is_in_Multi_range.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int FE_element_add_number_to_Multi_range(
	struct FE_element *element, void *multi_range_void)
{
	int return_code;
	struct Multi_range *multi_range = (struct Multi_range *)multi_range_void;
	if (element && multi_range)
	{
		const DsLabelIdentifier identifier = element->get_identifier();
		return_code = Multi_range_add_range(multi_range, identifier, identifier);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_add_number_to_Multi_range.   Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

bool FE_elements_can_merge_faces(int faceCount,
	FE_mesh &target_face_fe_mesh, struct FE_element *targetElement,
	FE_mesh &source_face_fe_mesh, struct FE_element *sourceElement)
{
	if ((targetElement->faces) && (sourceElement->faces))
	{
		for (int i = 0; i < faceCount; ++i)
		{
			FE_element *sourceFace = sourceElement->faces[i];
			FE_element *targetFace = targetElement->faces[i];
			if (sourceFace && targetFace &&
				(source_face_fe_mesh.getElementIdentifier(sourceFace->index) != target_face_fe_mesh.getElementIdentifier(targetFace->index)))
			{
				return false;
			}
		}
	}
	return true;
}

static int FE_element_field_add_FE_field_to_list(
	struct FE_element_field *element_field, void *fe_field_list_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION :
FE_element_field iterator which adds its FE_field to the LIST pointed to by
<fe_field_list_void>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_field_add_FE_field_to_list);
	/*???RC try to make this as inexpensive as possible */
	if (element_field)
	{
		/* the field is not expected to be in the list already */
		return_code = ADD_OBJECT_TO_LIST(FE_field)(element_field->field,
			(struct LIST(FE_field) *)fe_field_list_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_add_FE_field_to_list.  Missing element_field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_add_FE_field_to_list */

/**
 * Data for passing to function
 * FE_element_field_FE_field_to_list_if_uses_scale_factor_set
 */
struct FE_element_field_FE_field_to_list_if_uses_scale_factor_set_data
{
	struct LIST(FE_field) *fe_field_list;
	cmzn_mesh_scale_factor_set *scale_factor_set;
};

/**
 * FE_element_field iterator which ensures its FE_field is in <fe_field_list> if
 * it uses the <scale_factor_set_identifier>. In practise, the identifier has to
 * point to an FE_basis. <scale_factor_set_data_void> points at a struct
 * FE_element_field_FE_field_to_list_if_uses_scale_factor_set_data.
 */
static int FE_element_field_FE_field_to_list_if_uses_scale_factor_set(
	struct FE_element_field *element_field, void *scale_factor_set_data_void)
{
	struct FE_element_field_FE_field_to_list_if_uses_scale_factor_set_data *scale_factor_set_data =
		reinterpret_cast<struct FE_element_field_FE_field_to_list_if_uses_scale_factor_set_data *>(scale_factor_set_data_void);
	if (element_field && element_field->field && scale_factor_set_data)
	{
		if (GENERAL_FE_FIELD == element_field->field->fe_field_type)
		{
			if (!IS_OBJECT_IN_LIST(FE_field)(element_field->field,
				scale_factor_set_data->fe_field_list))
			{
				const int number_of_components = element_field->field->number_of_components;
				for (int i = 0; i < number_of_components; ++i)
				{
					if (element_field->components[i]->get_scale_factor_set() ==
						scale_factor_set_data->scale_factor_set)
					{
						ADD_OBJECT_TO_LIST(FE_field)(element_field->field,
							scale_factor_set_data->fe_field_list);
						break;
					}
				}
			}
		}
		return 1;
	}
	return 0;
}

int merge_FE_element(struct FE_element *destination, struct FE_element *source,
	struct LIST(FE_field) *changed_fe_field_list)
{
	int return_code, values_storage_size;
	struct FE_element_field_FE_field_to_list_if_uses_scale_factor_set_data scale_factor_set_data;
	struct FE_element_field_info *destination_fields, *element_field_info,
		*source_fields;
	struct FE_element_field_lists_merge_data merge_data;
	FE_mesh *fe_mesh;
	struct LIST(FE_element_field) *element_field_list;
	Value_storage *values_storage;

	ENTER(merge_FE_element);
	if (destination && (destination_fields = destination->fields) &&
		(fe_mesh = destination_fields->fe_mesh) &&
		source && (source_fields = source->fields) &&
		(source_fields->fe_mesh == fe_mesh) && changed_fe_field_list)
	{
		return_code = 1;
		/* changed_fe_field_list should start with all the fields in <source> */
		REMOVE_ALL_OBJECTS_FROM_LIST(FE_field)(changed_fe_field_list);
		FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
			FE_element_field_add_FE_field_to_list, (void *)changed_fe_field_list,
			source_fields->element_field_list);

		/* make a merged element node scale field info without values_storage */
		FE_element_node_scale_field_info *node_scale_field_info = 0;
		FE_element_node_scale_field_info *destination_info = destination->information;
		FE_element_node_scale_field_info *source_info = source->information;
		if (return_code)
		{
			if (destination_info)
			{
				if (source_info)
				{
					std::vector<cmzn_mesh_scale_factor_set*> changedExistingScaleFactorSets;
					node_scale_field_info = FE_element_node_scale_field_info::createMergeWithoutValuesStorage(
						*destination_info, *source_info, changedExistingScaleFactorSets);
					if (node_scale_field_info)
					{
						size_t number_of_changed_existing_scale_factor_sets = changedExistingScaleFactorSets.size();
						if (number_of_changed_existing_scale_factor_sets)
						{
							// determine which fields are affected by changed scale factor sets
							scale_factor_set_data.fe_field_list = changed_fe_field_list;
							for (size_t i = 0; i < number_of_changed_existing_scale_factor_sets; i++)
							{
								scale_factor_set_data.scale_factor_set = changedExistingScaleFactorSets[i];
								if (!FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
									FE_element_field_FE_field_to_list_if_uses_scale_factor_set,
									(void *)&scale_factor_set_data,
									destination_fields->element_field_list))
								{
									display_message(ERROR_MESSAGE, "merge_FE_element.  Could not "
										"determine fields affected by changed scale factor set");
									return_code = 0;
								}
							}
						}
					}
				}
				else
				{
					node_scale_field_info = destination_info->cloneWithoutValuesStorage();
				}
			}
			else if (source_info)
			{
				node_scale_field_info = source_info->cloneWithoutValuesStorage();
			}
			if ((destination_info || source_info) && (!node_scale_field_info))
			{
				display_message(ERROR_MESSAGE,
					"merge_FE_element.  Could not create node scale field info");
				return_code = 0;
			}
		}
		if (return_code)
		{
			// create element field list containing fields in destination
			// these are replaced for new fields from source
			element_field_list = CREATE_LIST(FE_element_field)();
			if (COPY_LIST(FE_element_field)(element_field_list,
				destination_fields->element_field_list))
			{
				/* merge in the fields from source, putting any new fields using
					 values_storage after that for fields in destination */
				merge_data.list = element_field_list;
				merge_data.merge_info = node_scale_field_info;
				merge_data.source_info = source_info;
				if (destination_info)
				{
					merge_data.values_storage_size = destination_info->values_storage_size;
				}
				else
				{
					merge_data.values_storage_size = 0;
				}
				if (FOR_EACH_OBJECT_IN_LIST(FE_element_field)(
					merge_FE_element_field_into_list, (void *)(&merge_data),
					source_fields->element_field_list))
				{
					values_storage_size = merge_data.values_storage_size;
					values_storage = (Value_storage *)NULL;
					if ((0 == values_storage_size) ||
						(ALLOCATE(values_storage, Value_storage, values_storage_size) &&
						copy_FE_element_values_storage(destination, values_storage,
							element_field_list, source)))
					{
						/* create an element field info for the combined list */
						if (NULL != (element_field_info =
							fe_mesh->get_FE_element_field_info(element_field_list)))
						{
							/* put values storage in node_scale_field_info */
							if (node_scale_field_info)
							{
								node_scale_field_info->values_storage_size =
									values_storage_size;
								node_scale_field_info->values_storage = values_storage;
							}
							/* clean up old destination information */
							if (destination_info)
							{
								FE_element_node_scale_field_info::destroyDynamic(destination_info, destination_fields);
							}
							/* insert new fields and information */
							REACCESS(FE_element_field_info)(&(destination->fields),
								element_field_info);
							destination->information = node_scale_field_info;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"merge_FE_element.  Could not get element field info");
							/* do not bother to clean up dynamic contents of values_storage */
							DEALLOCATE(values_storage);
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"merge_FE_element.  Could not copy values_storage");
						/* cannot clean up dynamic contents of values_storage */
						DEALLOCATE(values_storage);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"merge_FE_element.  Error merging element field list");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"merge_FE_element.  Could not copy element field list");
				return_code = 0;
			}
			DESTROY(LIST(FE_element_field))(&element_field_list);
		}
		if (!return_code && node_scale_field_info)
		{
			FE_element_node_scale_field_info::destroy(node_scale_field_info);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "merge_FE_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* merge_FE_element */

int list_FE_element(struct FE_element *element)
{
	char line[93];
	FE_value *scale_factor;
	int i,return_code;
	struct FE_element **face;

	ENTER(list_FE_element);
	if (element)
	{
		return_code=1;
		/* write the identifier */
		display_message(INFORMATION_MESSAGE,"element : %d \n",element->get_identifier());
		display_message(INFORMATION_MESSAGE,"  access count=%d\n",element->access_count);
		FE_element_shape *element_shape = get_FE_element_shape(element);
		if (element_shape)
		{
			display_message(INFORMATION_MESSAGE,"  dimension=%d\n",element_shape->dimension);
			char *shape_description = FE_element_shape_get_EX_description(element_shape);
			if (shape_description)
			{
				display_message(INFORMATION_MESSAGE, "  shape=%s\n", shape_description);
				DEALLOCATE(shape_description);
			}
			/* write the faces */
			if ((face=element->faces)&&
				(0<(i=element_shape->number_of_faces)))
			{
				display_message(INFORMATION_MESSAGE,"  #faces=%d\n",i);
				while (i>0)
				{
					if (*face)
					{
						display_message(INFORMATION_MESSAGE," (%d)",(*face)->get_identifier());
					}
					else
					{
						display_message(INFORMATION_MESSAGE," (0 0 0)");
					}
					face++;
					i--;
				}
				display_message(INFORMATION_MESSAGE,"\n");
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"  No faces\n");
			}
			/* write the parents */
			if ((0 < element->number_of_parents) && (element->parents))
			{
				display_message(INFORMATION_MESSAGE,"  parents\n");
#if !defined (WINDOWS_DEV_FLAG)
				*line='\0';
				for (i = 0; i < element->number_of_parents; i++)
				{
					if (element->parents[i])
					{
						sprintf(line+strlen(line)," (%d)", element->parents[i]->get_identifier());
						if (70<=strlen(line))
						{
							display_message(INFORMATION_MESSAGE,line);
							display_message(INFORMATION_MESSAGE,"\n");
							*line='\0';
						}
					}
				}
				if (0<strlen(line))
				{
					display_message(INFORMATION_MESSAGE,line);
					display_message(INFORMATION_MESSAGE,"\n");
				}
#endif /* !defined (WINDOWS_DEV_FLAG) */
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"  No parents\n");
			}
			if (element->fields)
			{
				if (0 < NUMBER_IN_LIST(FE_element_field)(
					element->fields->element_field_list))
				{
					display_message(INFORMATION_MESSAGE,"  Field information\n");
					for_each_FE_field_at_element_alphabetical_indexer_priority(
						list_FE_element_field, (void *)NULL, element);
					if (element->information)
					{
						if ((scale_factor = element->information->scale_factors)&&
							(i=element->information->number_of_scale_factors))
						{
							display_message(INFORMATION_MESSAGE,"  #Scale factors=%d\n",i);
							strcpy(line,"    ");
							while (i>0)
							{
								sprintf(line+strlen(line),"%13g ",*scale_factor);
								if ((1==i) || (strlen(line)>=70))
								{
									strcat(line,"\n");
									display_message(INFORMATION_MESSAGE,line);
									strcpy(line,"    ");
								}
								i--;
								scale_factor++;
							}
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"list_FE_element.  Missing field info");
				return_code = 0;
			}
			/* write the nodes */
			if ((element->information) && (element->information->nodes) && (0 < element->information->number_of_nodes))
			{
				display_message(INFORMATION_MESSAGE,"  nodes\n   ");
				for (i = 0; i < element->information->number_of_nodes; i++)
				{
					if (element->information->nodes[i])
					{
						display_message(INFORMATION_MESSAGE," %d",element->information->nodes[i]->cm_node_identifier);
					}
					else
					{
						display_message(INFORMATION_MESSAGE," -");
					}
				}
				display_message(INFORMATION_MESSAGE, "\n");
			}
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  Missing shape\n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_FE_element.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_FE_element */

enum Modify_theta_in_xi1_mode
{
	MODIFY_THETA_CLOSEST_IN_XI1,
	MODIFY_THETA_DECREASING_IN_XI1,
	MODIFY_THETA_INCREASING_IN_XI1,
	MODIFY_THETA_NON_DECREASING_IN_XI1,
	MODIFY_THETA_NON_INCREASING_IN_XI1
};

static int modify_theta_in_xi1(struct FE_element_field_component *component,
	struct FE_element *element,struct FE_field *field,FE_value time,
	int number_of_values,FE_value *values, enum Modify_theta_in_xi1_mode mode)
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Modifies the already calculated <values>.
???DB.  Only for certain bases
==============================================================================*/
{
	char all_on_axis;
	enum Coordinate_system_type coordinate_system_type;
	FE_value *element_value,*element_value_2,offset_xi1_xi2,offset_xi2_xi3,
		value_xi1,value_xi2,value_xi3;
	const int *basis_type;
	int i,j,k,return_code,xi2_basis_type;
	struct FE_element_field *element_field;
	struct FE_node **node;
	struct Standard_node_to_element_map **node_to_element_map,
		**node_to_element_map_2;

	ENTER(modify_theta_in_xi1);
	int basis_dimension = 0;
	if (component&&(STANDARD_NODE_TO_ELEMENT_MAP==component->type)&&
		(node_to_element_map=(component->map).standard_node_based.
		node_to_element_maps)&&(component->basis)&&
		(basis_type = FE_basis_get_basis_type(component->basis)) &&
		(basis_dimension = *basis_type) &&
		((1 == basis_dimension) ||
		((2 == basis_dimension) && (NO_RELATION==basis_type[2]))||
		((3 == basis_dimension) && (NO_RELATION==basis_type[2])&&
		(NO_RELATION==basis_type[3])&&(NO_RELATION==basis_type[5])))&&
		element&&field&&(0<number_of_values)&&values)
	{
		coordinate_system_type=get_coordinate_system_type(
			get_FE_field_coordinate_system(field));
		if ((3==get_FE_field_number_of_components(field))&&
			((CYLINDRICAL_POLAR==coordinate_system_type)||
			(OBLATE_SPHEROIDAL==coordinate_system_type)||
			(PROLATE_SPHEROIDAL==coordinate_system_type)||
			(SPHERICAL_POLAR==coordinate_system_type)))
		{
			struct FE_element_field_component *axis_component = 0;
			element_field=FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
				field,element->fields->element_field_list);
			if (element_field)
			{
				switch (coordinate_system_type)
				{
					case CYLINDRICAL_POLAR:
					{
						if (component==(element_field->components)[1])
						{
							axis_component=(element_field->components)[0];
						}
						else
						{
							element_field=(struct FE_element_field *)NULL;
						}
					} break;
					case OBLATE_SPHEROIDAL:
					case PROLATE_SPHEROIDAL:
					{
						if (component==(element_field->components)[2])
						{
							axis_component=(element_field->components)[1];
						}
						else
						{
							element_field=(struct FE_element_field *)NULL;
						}
					} break;
					case SPHERICAL_POLAR:
					{
						if (component==(element_field->components)[1])
						{
							axis_component=(element_field->components)[2];
						}
						else
						{
							element_field=(struct FE_element_field *)NULL;
						}
					} break;
					default:
					{
						// do nothing; no valid element_field
					} break;
				}
			}
			if (element_field)
			{
				int number_of_nodes_in_xi1 = 0;
				int number_of_nodes_in_xi2 = 0;
				int number_of_nodes_in_xi3 = 0;
				/* determine the number of nodes in the xi1 direction */
				switch (basis_type[1])
				{
					case LINEAR_LAGRANGE: case CUBIC_HERMITE: case LAGRANGE_HERMITE:
						case HERMITE_LAGRANGE:
					{
						number_of_nodes_in_xi1=2;
					} break;
					case QUADRATIC_LAGRANGE:
					{
						number_of_nodes_in_xi1=3;
					} break;
					case CUBIC_LAGRANGE:
					{
						number_of_nodes_in_xi1=4;
					} break;
				}
				/* determine the number of nodes in the xi2 direction */
				if (1 == basis_dimension)
				{
					number_of_nodes_in_xi2=1;
				}
				else
				{
					if (2 == basis_dimension)
					{
						xi2_basis_type=basis_type[3];
					}
					else
					{
						xi2_basis_type=basis_type[4];
					}
					switch (xi2_basis_type)
					{
						case LINEAR_LAGRANGE: case CUBIC_HERMITE: case LAGRANGE_HERMITE:
							case HERMITE_LAGRANGE:
						{
							number_of_nodes_in_xi2=2;
						} break;
						case QUADRATIC_LAGRANGE:
						{
							number_of_nodes_in_xi2=3;
						} break;
						case CUBIC_LAGRANGE:
						{
							number_of_nodes_in_xi2=4;
						} break;
					}
				}
				/* determine the number of nodes in the xi3 direction */
				if (3 == basis_dimension)
				{
					switch (basis_type[6])
					{
						case LINEAR_LAGRANGE: case CUBIC_HERMITE: case LAGRANGE_HERMITE:
							case HERMITE_LAGRANGE:
						{
							number_of_nodes_in_xi3=2;
						} break;
						case QUADRATIC_LAGRANGE:
						{
							number_of_nodes_in_xi3=3;
						} break;
						case CUBIC_LAGRANGE:
						{
							number_of_nodes_in_xi3=4;
						} break;
					}
				}
				else
				{
					number_of_nodes_in_xi3=1;
				}
				/* check for nodes on the z axis */
				node=element->information->nodes;
				/* xi2=0 face */
				if (1<number_of_nodes_in_xi2)
				{
					all_on_axis=1;
				}
				else
				{
					all_on_axis=0;
				}
				node_to_element_map=(axis_component->map).standard_node_based.
					node_to_element_maps;
				k=number_of_nodes_in_xi3;
				while (all_on_axis&&(k>0))
				{
					i=number_of_nodes_in_xi1;
					while (all_on_axis&&(i>0))
					{
						all_on_axis=node_on_axis(node[(*node_to_element_map)->node_index],
							field,time,coordinate_system_type);
						node_to_element_map++;
						i--;
					}
					node_to_element_map +=
						(number_of_nodes_in_xi2-1)*number_of_nodes_in_xi1;
					k--;
				}
				if (all_on_axis)
				{
					element_value=values;
					node_to_element_map=(component->map).standard_node_based.
						node_to_element_maps;
					element_value_2=values;
					node_to_element_map_2=node_to_element_map;
					for (i=number_of_nodes_in_xi1;i>0;i--)
					{
						element_value_2 += (*node_to_element_map_2)->number_of_nodal_values;
						node_to_element_map_2++;
					}
					for (k=number_of_nodes_in_xi3;k>0;k--)
					{
						for (i=number_of_nodes_in_xi1;i>0;i--)
						{
							*element_value= *element_value_2;
							element_value += (*node_to_element_map)->number_of_nodal_values;
							node_to_element_map++;
							element_value_2 +=
								(*node_to_element_map_2)->number_of_nodal_values;
							node_to_element_map_2++;
						}
						if (k>1)
						{
							for (j=number_of_nodes_in_xi2;j>1;j--)
							{
								for (i=number_of_nodes_in_xi1;i>0;i--)
								{
									element_value +=
										(*node_to_element_map)->number_of_nodal_values;
									node_to_element_map++;
									element_value_2 += (*node_to_element_map_2)->
										number_of_nodal_values;
									node_to_element_map_2++;
								}
							}
						}
					}
				}
				else
				{
					/* xi2=1 face */
					if (1<number_of_nodes_in_xi2)
					{
						all_on_axis=1;
					}
					else
					{
						all_on_axis=0;
					}
					node_to_element_map=(axis_component->map).standard_node_based.
						node_to_element_maps;
					node_to_element_map +=
						(number_of_nodes_in_xi2-1)*number_of_nodes_in_xi1;
					k=number_of_nodes_in_xi3;
					while (all_on_axis&&(k>0))
					{
						i=number_of_nodes_in_xi1;
						while (all_on_axis&&(i>0))
						{
							all_on_axis=node_on_axis(node[(*node_to_element_map)->node_index],
								field,time,coordinate_system_type);
							node_to_element_map++;
							i--;
						}
						node_to_element_map +=
							(number_of_nodes_in_xi2-1)*number_of_nodes_in_xi1;
						k--;
					}
					if (all_on_axis)
					{
						element_value=values;
						node_to_element_map=(component->map).standard_node_based.
							node_to_element_maps;
						for (j=number_of_nodes_in_xi2;j>1;j--)
						{
							for (i=number_of_nodes_in_xi1;i>0;i--)
							{
								element_value += (*node_to_element_map)->number_of_nodal_values;
								node_to_element_map++;
							}
						}
						element_value_2=element_value;
						node_to_element_map_2=node_to_element_map;
						for (i=number_of_nodes_in_xi1;i>0;i--)
						{
							node_to_element_map_2--;
							element_value_2 -=
								(*node_to_element_map_2)->number_of_nodal_values;
						}
						for (k=number_of_nodes_in_xi3;k>0;k--)
						{
							for (i=number_of_nodes_in_xi1;i>0;i--)
							{
								*element_value= *element_value_2;
								element_value += (*node_to_element_map)->number_of_nodal_values;
								node_to_element_map++;
								element_value_2 += (*node_to_element_map_2)->
									number_of_nodal_values;
								node_to_element_map_2++;
							}
							if (k>1)
							{
								for (j=number_of_nodes_in_xi2;j>1;j--)
								{
									for (i=number_of_nodes_in_xi1;i>0;i--)
									{
										element_value +=
											(*node_to_element_map)->number_of_nodal_values;
										node_to_element_map++;
										element_value_2 += (*node_to_element_map_2)->
											number_of_nodal_values;
										node_to_element_map_2++;
									}
								}
							}
						}
					}
					else
					{
						/* xi3=0 face */
						if (1<number_of_nodes_in_xi3)
						{
							all_on_axis=1;
						}
						else
						{
							all_on_axis=0;
						}
						node_to_element_map=(axis_component->map).standard_node_based.
							node_to_element_maps;
						j=number_of_nodes_in_xi2;
						while (all_on_axis&&(j>0))
						{
							i=number_of_nodes_in_xi1;
							while (all_on_axis&&(i>0))
							{
								all_on_axis=node_on_axis(node[(*node_to_element_map)->
									node_index],field,time,coordinate_system_type);
								node_to_element_map++;
								i--;
							}
							j--;
						}
						if (all_on_axis)
						{
							element_value=values;
							node_to_element_map=(component->map).standard_node_based.
								node_to_element_maps;
							element_value_2=values;
							node_to_element_map_2=node_to_element_map;
							for (i=number_of_nodes_in_xi1;i>0;i--)
							{
								element_value_2 += (*node_to_element_map_2)->
									number_of_nodal_values;
								node_to_element_map_2++;
							}
							for (j=number_of_nodes_in_xi2;j>0;j--)
							{
								for (i=number_of_nodes_in_xi1;i>0;i--)
								{
									*element_value= *element_value_2;
									element_value +=
										(*node_to_element_map)->number_of_nodal_values;
									node_to_element_map++;
									element_value_2 += (*node_to_element_map_2)->
										number_of_nodal_values;
									node_to_element_map_2++;
								}
							}
						}
						else
						{
							/* xi3=1 face */
							if (1<number_of_nodes_in_xi3)
							{
								all_on_axis=1;
							}
							else
							{
								all_on_axis=0;
							}
							node_to_element_map=(component->map).standard_node_based.
								node_to_element_maps;
							node_to_element_map += (number_of_nodes_in_xi3-1)*
								number_of_nodes_in_xi2*number_of_nodes_in_xi1;
							j=number_of_nodes_in_xi2;
							while (all_on_axis&&(j>0))
							{
								i=number_of_nodes_in_xi1;
								while (all_on_axis&&(i>0))
								{
									all_on_axis=node_on_axis(node[(*node_to_element_map)->
										node_index],field,time,coordinate_system_type);
									node_to_element_map++;
									i--;
								}
								j--;
							}
							if (all_on_axis)
							{
								element_value=values;
								node_to_element_map=(component->map).standard_node_based.
									node_to_element_maps;
								for (k=number_of_nodes_in_xi3;k>1;k--)
								{
									for (j=number_of_nodes_in_xi2;j>0;j--)
									{
										for (i=number_of_nodes_in_xi1;i>0;i--)
										{
											element_value +=
												(*node_to_element_map)->number_of_nodal_values;
											node_to_element_map++;
										}
									}
								}
								element_value_2=element_value;
								node_to_element_map_2=node_to_element_map;
								for (j=number_of_nodes_in_xi2;j>0;j--)
								{
									for (i=number_of_nodes_in_xi1;i>0;i--)
									{
										node_to_element_map_2--;
										element_value_2 -= (*node_to_element_map_2)->
											number_of_nodal_values;
									}
								}
								for (j=number_of_nodes_in_xi2;j>0;j--)
								{
									for (i=number_of_nodes_in_xi1;i>0;i--)
									{
										*element_value= *element_value_2;
										element_value += (*node_to_element_map)->
											number_of_nodal_values;
										node_to_element_map++;
										element_value_2 += (*node_to_element_map_2)->
											number_of_nodal_values;
										node_to_element_map_2++;
									}
								}
							}
						}
					}
				}
				element_value=values;
				node_to_element_map=(component->map).standard_node_based.
					node_to_element_maps;
				offset_xi1_xi2=0;
				offset_xi2_xi3=0;
				/* apply condition on xi1 and make sure smooth in xi2 & xi3 */
				for (k=number_of_nodes_in_xi3;k>0;k--)
				{
					value_xi3= *element_value;
					for (j=number_of_nodes_in_xi2;j>0;j--)
					{
						value_xi2= *element_value;
						for (i=number_of_nodes_in_xi1;i>1;i--)
						{
							value_xi1= *element_value;
							element_value += (*node_to_element_map)->number_of_nodal_values;
							*element_value += offset_xi1_xi2+offset_xi2_xi3;
							/*???DB.  <= needed for single prolate, but seems to cause
								problems for heart */
							switch (mode)
							{
								case MODIFY_THETA_CLOSEST_IN_XI1:
								{
									if (value_xi1 < (*element_value - PI))
									{
										*element_value -= 2*PI;
									}
									else if (value_xi1 > (*element_value + PI))
									{
										*element_value += 2*PI;
									}
								} break;
								case MODIFY_THETA_DECREASING_IN_XI1:
								{
									if (value_xi1 <= *element_value)
									{
										*element_value -= 2*PI;
									}
								} break;
								case MODIFY_THETA_INCREASING_IN_XI1:
								{
									if (value_xi1 >= *element_value)
									{
										*element_value += 2*PI;
									}
								} break;
								case MODIFY_THETA_NON_DECREASING_IN_XI1:
								{
									if (value_xi1 > *element_value)
									{
										*element_value += 2*PI;
									}
								} break;
								case MODIFY_THETA_NON_INCREASING_IN_XI1:
								{
									if (value_xi1 < *element_value)
									{
										*element_value -= 2*PI;
									}
								} break;
							}
							node_to_element_map++;
						}
						element_value += (*node_to_element_map)->number_of_nodal_values;
						node_to_element_map++;
						if (j>1)
						{
							value_xi1= *element_value;
							if (value_xi1>value_xi2+PI)
							{
								offset_xi1_xi2= -2*PI;
								*element_value += offset_xi1_xi2;
							}
							else
							{
								if (value_xi1<value_xi2-PI)
								{
									offset_xi1_xi2=2*PI;
									*element_value += offset_xi1_xi2;
								}
								else
								{
									offset_xi1_xi2=0;
								}
							}
						}
					}
					if (k>1)
					{
						offset_xi1_xi2=0;
						value_xi2= *element_value;
						if (value_xi2>value_xi3+PI)
						{
							offset_xi2_xi3= -2*PI;
							*element_value += offset_xi2_xi3;
						}
						else
						{
							if (value_xi2<value_xi3-PI)
							{
								offset_xi2_xi3=2*PI;
								*element_value += offset_xi2_xi3;
							}
							else
							{
								offset_xi2_xi3=0;
							}
						}
					}
				}
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_theta_in_xi1.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_theta_in_xi1 */

int theta_closest_in_xi1(struct FE_element_field_component *component,
	struct FE_element *element,struct FE_field *field,FE_value time,
	int number_of_values,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Calls modify_theta_in_xi1 with mode MODIFY_THETA_CLOSEST_IN_XI1.
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/
{
	int return_code;

	ENTER(theta_closest_in_xi1);
	return_code = modify_theta_in_xi1(component, element, field, time,
		number_of_values, values, MODIFY_THETA_CLOSEST_IN_XI1);
	LEAVE;

	return (return_code);
} /* theta_closest_in_xi1 */

int theta_decreasing_in_xi1(struct FE_element_field_component *component,
	struct FE_element *element,struct FE_field *field,FE_value time,
	int number_of_values,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Calls modify_theta_in_xi1 with mode MODIFY_THETA_DECREASING_IN_XI1.
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/
{
	int return_code;

	ENTER(theta_decreasing_in_xi1);
	return_code = modify_theta_in_xi1(component, element, field, time,
		number_of_values, values, MODIFY_THETA_DECREASING_IN_XI1);
	LEAVE;

	return (return_code);
} /* theta_decreasing_in_xi1 */

int theta_increasing_in_xi1(struct FE_element_field_component *component,
	struct FE_element *element,struct FE_field *field,FE_value time,
	int number_of_values,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Calls modify_theta_in_xi1 with mode MODIFY_THETA_INCREASING_IN_XI1.
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/
{
	int return_code;

	ENTER(theta_increasing_in_xi1);
	return_code = modify_theta_in_xi1(component, element, field, time,
		number_of_values, values, MODIFY_THETA_INCREASING_IN_XI1);
	LEAVE;

	return (return_code);
} /* theta_increasing_in_xi1 */

int theta_non_decreasing_in_xi1(
	struct FE_element_field_component *component,struct FE_element *element,
	struct FE_field *field,FE_value time,int number_of_values,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Calls modify_theta_in_xi1 with mode MODIFY_THETA_NON_DECREASING_IN_XI1.
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/
{
	int return_code;

	ENTER(theta_non_decreasing_in_xi1);
	return_code = modify_theta_in_xi1(component, element, field, time,
		number_of_values, values, MODIFY_THETA_NON_DECREASING_IN_XI1);
	LEAVE;

	return (return_code);
} /* theta_non_decreasing_in_xi1 */

int theta_non_increasing_in_xi1(
	struct FE_element_field_component *component,struct FE_element *element,
	struct FE_field *field,FE_value time,int number_of_values,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 1 February 2002

DESCRIPTION :
Calls modify_theta_in_xi1 with mode MODIFY_THETA_NON_INCREASING_IN_XI1.
???RC.  Needs to be global to allow writing function in export_finite_element.
==============================================================================*/
{
	int return_code;

	ENTER(theta_non_increasing_in_xi1);
	return_code = modify_theta_in_xi1(component, element, field, time,
		number_of_values, values, MODIFY_THETA_NON_INCREASING_IN_XI1);
	LEAVE;

	return (return_code);
} /* theta_non_increasing_in_xi1 */

int calculate_FE_field(struct FE_field *field,int component_number,
	struct FE_node *node,struct FE_element *element,FE_value *xi_coordinates,
	FE_value time, FE_value *value)
/*******************************************************************************
LAST MODIFIED : 11 May 2003

DESCRIPTION :
Calculates the <value> of the <field> for the specified <node> or <element> and
<xi_coordinates>.  If 0<=component_number<=number_of_components, then only the
specified component is calculated, otherwise all components are calculated.  The
storage for the <value> should have been allocated outside the function.
???DB.  Picks up the first version for the node.
==============================================================================*/
{
	int return_code = 1;

	ENTER(calculate_FE_field);
	if (field&&((node&&!element&&!xi_coordinates)||
		(!node&&element&&xi_coordinates))&&value)
	{
		if (node)
		{
			/* calculate field for node */
			if (field->value_type == FE_VALUE_VALUE)
			{
				int components_to_calculate, first_comp, i;

				first_comp = 0;
				components_to_calculate = field->number_of_components;
				if ((0 <= component_number) && (component_number < components_to_calculate))
				{
					first_comp = component_number;
					components_to_calculate = 1;
				}
				for (i = 0; i < components_to_calculate; i++)
				{
					if (!get_FE_nodal_FE_value_value(node, field, first_comp + i,
						/*version*/0, FE_NODAL_VALUE, time, value + i))
					{
						display_message(ERROR_MESSAGE,
							"calculate_FE_field.  Field or component not defined for node");
						break;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
						"calculate_FE_field. field->value_type must be FE_VALUE_VALUE ");
					return_code=0;
			}
		}
		else
		{
			struct FE_element_field_values *element_field_values;

			/* calculate field for element */
			/* determine if the field is defined over the element */
			element_field_values=CREATE(FE_element_field_values)();
			if (calculate_FE_element_field_values(element,field,
				time,/*calculate_derivatives*/0,element_field_values,
				/*top_level_element*/(struct FE_element *)NULL))
			{
				/* calculate the value for the element field */
				return_code=calculate_FE_element_field(component_number,
					element_field_values,xi_coordinates,value,(FE_value *)NULL);
				clear_FE_element_field_values(element_field_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_field.  Field not defined for element");
				return_code=0;
			}
			DESTROY(FE_element_field_values)(&element_field_values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_FE_field.  Invalid argument(s)\n"
			"field %p, node %p, element %p, xi_coordinates %p, value %p",field,node,
			element,xi_coordinates,value);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* calculate_FE_field */

struct FE_field *find_first_time_field_at_FE_node(struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

Find the first time based field at a node
==============================================================================*/
{
	struct FE_field *time_field;
	struct FE_node_field *time_node_field;
	ENTER(find_first_time_field_at_FE_node);
	time_field = (struct FE_field *)NULL;
	time_node_field = (struct FE_node_field *)NULL;
	if (node)
	{
		if (NULL != (time_node_field=FIRST_OBJECT_IN_LIST_THAT(FE_node_field)(
			FE_node_field_has_time,(void *)(NULL),
			node->fields->node_field_list)))
		{
			time_field = time_node_field->field;
		}
		else
		{
			display_message(ERROR_MESSAGE,"find_first_time_field_at_FE_node."
				" Failed to find time_field in node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"find_first_time_field_at_FE_node."
				" Invalid arguments");
	}
	LEAVE;
	return (time_field);
} /* find_first_time_field_at_FE_node */

struct FE_element_field_info *FE_element_get_FE_element_field_info(
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns the FE_element_field_info from <element>. Must not be modified!
==============================================================================*/
{
	struct FE_element_field_info *fe_element_field_info;

	ENTER(FE_element_get_FE_element_field_info);
	if (element)
	{
		fe_element_field_info = element->fields;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_get_FE_element_field_info.  Invalid argument(s)");
		fe_element_field_info = (struct FE_element_field_info *)NULL;
	}
	LEAVE;

	return (fe_element_field_info);
} /* FE_element_get_FE_element_field_info */

int FE_element_set_FE_element_field_info(struct FE_element *element,
	struct FE_element_field_info *fe_element_field_info)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Changes the FE_element_field_info at <element> to <fe_element_field_info>.
Note it is very important that the old and the new FE_element_field_info
structures describe the same data layout in the element information!
Private function only to be called by FE_region when merging FE_regions!
==============================================================================*/
{
	int return_code;

	ENTER(FE_element_set_FE_element_field_info);
	if (element && fe_element_field_info)
	{
		return_code = REACCESS(FE_element_field_info)(&(element->fields),
			fe_element_field_info);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_set_FE_element_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_set_FE_element_field_info */

FE_mesh *FE_element_get_FE_mesh(struct FE_element *element)
{
	if (element && element->fields)
	{
		return element->fields->fe_mesh;
	}
	return 0;
}

struct FE_region *FE_element_get_FE_region(struct FE_element *element)
{
	if (element && element->fields)
	{
		return element->fields->fe_mesh->get_FE_region();
	}
	return 0;
}

int FE_element_has_top_level_element(struct FE_element *element,
	void *top_level_element_void)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Returns true if <top_level_element> is indeed a top_level parent of <element>.
==============================================================================*/
{
	int return_code;
	struct FE_element *top_level_element;

	ENTER(FE_element_has_top_level_element);
	if (element&&(top_level_element=(struct FE_element *)top_level_element_void)&&
		(0 == top_level_element->number_of_parents))
	{
		if ((element==top_level_element)||
			FE_element_ancestor_matches_recursive(element, top_level_element))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_has_top_level_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_has_top_level_element */

int FE_element_is_top_level_parent_of_element(
	struct FE_element *top_level_element,void *element_void)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Returns true if <top_level_element> is a top_level parent of <element>.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;

	ENTER(FE_element_is_top_level_parent_of_element);
	if (top_level_element&&(element=(struct FE_element *)element_void))
	{
		if ((0 == top_level_element->number_of_parents) &&
			((element==top_level_element)||
				FE_element_ancestor_matches_recursive(element, top_level_element)))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_is_top_level_parent_of_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_is_top_level_parent_of_element */

struct FE_element *FE_element_get_top_level_element_conversion(
	struct FE_element *element,struct FE_element *check_top_level_element,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional, void *conditional_data,
	cmzn_element_face_type specified_face, FE_value *element_to_top_level)
{
	int face_number, i,  size;
	FE_value *face_to_element;
	struct FE_element *parent, *top_level_element;

	if (element&&element_to_top_level)
	{
		if (0 < element->number_of_parents)
		{
			parent = NULL;
			if (check_top_level_element)
			{
				parent = FE_element_get_first_parent_with_ancestor(element, check_top_level_element);
			}
			if (!parent)
			{
				if (CMZN_ELEMENT_FACE_TYPE_XI1_0 <= specified_face)
				{
					parent = FE_element_get_parent_on_face(
						element, specified_face, conditional, conditional_data);
				}
				else if (conditional)
				{
					for (i = 0; i < element->number_of_parents; i++)
					{
						if ((conditional)(element->parents[i], conditional_data))
						{
							parent = element->parents[i];
							break;
						}
					}
				}
			}
			if (!parent)
			{
				parent = element->parents[0];
			}

			FE_element_shape *parent_shape = get_FE_element_shape(parent);
			if ((parent_shape) && (parent_shape->face_to_element) &&
				(0 <= (face_number = FE_element_get_child_face_number(parent, element))) &&
				(top_level_element = FE_element_get_top_level_element_conversion(
					parent, check_top_level_element, conditional, conditional_data,
					specified_face, element_to_top_level)))
			{
				face_to_element=parent_shape->face_to_element +
					(face_number * parent_shape->dimension*parent_shape->dimension);
				size=top_level_element->getDimension();
				if (parent == top_level_element)
				{
					/* element_to_top_level = face_to_element of appropriate face */
					for (i=size*size-1;0<=i;i--)
					{
						element_to_top_level[i]=face_to_element[i];
					}
				}
				else
				{
					/* multiply face_to_element of top_level_element (currently in
						 element_to_top_level) by face_to_element of parent */
					/* this is the 1:3 case */
#if defined (DEBUG_CODE)
					if (1==element->shape->dimension)
					{
						printf("\n");
						printf("f=[%6.3f %6.3f]\n",face_to_element[0],face_to_element[1]);
						printf("  [%6.3f %6.3f]\n",face_to_element[2],face_to_element[3]);
						printf("e=[%6.3f %6.3f %6.3f]\n",element_to_top_level[0],
							element_to_top_level[1],element_to_top_level[2]);
						printf("  [%6.3f %6.3f %6.3f]\n",element_to_top_level[3],
							element_to_top_level[4],element_to_top_level[5]);
						printf("  [%6.3f %6.3f %6.3f]\n",element_to_top_level[6],
							element_to_top_level[7],element_to_top_level[8]);
					}
#endif /* defined (DEBUG_CODE) */
					for (i=0;i<size;i++)
					{
						element_to_top_level[i*2  ] = element_to_top_level[i*size] +
							element_to_top_level[i*size+1]*face_to_element[0]+
							element_to_top_level[i*size+2]*face_to_element[2];
						element_to_top_level[i*2+1] =
							element_to_top_level[i*size+1]*face_to_element[1]+
							element_to_top_level[i*size+2]*face_to_element[3];
					}
				}
#if defined (DEBUG_CODE)
				/*???debug*/
				{
					FE_value *value;
					int a,b;

					value=element_to_top_level;
					for (b=0;b<top_level_element->shape->dimension;b++)
					{
						printf("[");
						for (a=0;a<=element->shape->dimension;a++)
						{
							printf(" %6.2f",*value);
							value++;
						}
						printf(" ]\n");
					}
					printf("\n");
				}
#endif /* defined (DEBUG_CODE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_get_top_level_element_conversion.  Invalid parent");
				top_level_element=(struct FE_element *)NULL;
			}
		}
		else
		{
			/* no parents */
			top_level_element=element;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_get_top_level_element_conversion.  Invalid argument(s)");
		top_level_element=(struct FE_element *)NULL;
	}

	return (top_level_element);
} /* FE_element_get_top_level_element_conversion */

int FE_element_get_top_level_element_and_xi(struct FE_element *element,
	const FE_value *xi, int element_dimension,
	struct FE_element **top_level_element, FE_value *top_level_xi,
	int *top_level_element_dimension)
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Finds the <top_level_element>, <top_level_xi> and <top_level_element_dimension>
for the given <element> and <xi>.  If <top_level_element> is already set it
is checked and the <top_level_xi> calculated.
==============================================================================*/
{
	FE_value element_to_top_level[9];
	int i,j,k,return_code;

	ENTER(FE_element_get_top_level_element_and_xi);
	if (element&&xi&&top_level_element&&top_level_xi&&top_level_element_dimension)
	{
		return_code = 1;
		if (0 == element->number_of_parents)
		{
			*top_level_element = element;
			for (i=0;i<element_dimension;i++)
			{
				top_level_xi[i]=xi[i];
			}
			/* do not set element_to_top_level */
			*top_level_element_dimension=element_dimension;
		}
		else
		{
			/* check or get top_level element and xi coordinates for it */
			if (NULL != (*top_level_element = FE_element_get_top_level_element_conversion(
				element,*top_level_element,
				(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL, (void *)NULL,
				CMZN_ELEMENT_FACE_TYPE_INVALID, element_to_top_level)))
			{
				/* convert xi to top_level_xi */
				*top_level_element_dimension = (*top_level_element)->getDimension();
				for (j=0;j<*top_level_element_dimension;j++)
				{
					top_level_xi[j] = element_to_top_level[j*(element_dimension+1)];
					for (k=0;k<element_dimension;k++)
					{
						top_level_xi[j] +=
							element_to_top_level[j*(element_dimension+1)+k+1]*xi[k];
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_get_top_level_element_and_xi.  "
					"No top-level element found to evaluate on");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_get_top_level_element_and_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_get_top_level_element_and_xi */

int get_FE_element_discretization_from_top_level(struct FE_element *element,
	int *number_in_xi,struct FE_element *top_level_element,
	int *top_level_number_in_xi,FE_value *element_to_top_level)
/*******************************************************************************
LAST MODIFIED : 3 May 2001

DESCRIPTION :
Returns in <number_in_xi> the equivalent discretization of <element> for its
position - element, face or line - in <top_level_element>. Uses
<element_to_top_level> array for line/face conversion as returned by
FE_element_get_top_level_element_conversion.
<number_in_xi> must have space at lease MAXIMUM_ELEMENT_XI_DIMENSIONS integers,
as remaining values up to this size are cleared to zero.
==============================================================================*/
{
	int dimension,i,j,maximum_number_in_xi,return_code,top_level_dimension;

	ENTER(get_FE_element_discretization_from_top_level);
	if (element&&number_in_xi&&top_level_element&&top_level_number_in_xi)
	{
		return_code=1;
		dimension=get_FE_element_dimension(element);
		if (top_level_element==element)
		{
			for (i=0;i<dimension;i++)
			{
				number_in_xi[i]=top_level_number_in_xi[i];
			}
		}
		else if (element_to_top_level)
		{
			top_level_dimension=get_FE_element_dimension(top_level_element);
			/* use largest number_in_xi of any linked xi directions */
			for (i=0;(i<dimension)&&return_code;i++)
			{
				maximum_number_in_xi=0;
				number_in_xi[i]=0;
				for (j=0;j<top_level_dimension;j++)
				{
					if (0.0!=fabs(element_to_top_level[j*(dimension+1)+i+1]))
					{
						if (top_level_number_in_xi[j]>maximum_number_in_xi)
						{
							maximum_number_in_xi=top_level_number_in_xi[j];
						}
					}
				}
				number_in_xi[i] = maximum_number_in_xi;
				if (0==maximum_number_in_xi)
				{
					display_message(ERROR_MESSAGE,
						"get_FE_element_discretization_from_top_level.  "
						"Could not get discretization");
					return_code=0;
				}
			}
			for (i=dimension;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
			{
				number_in_xi[i]=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_element_discretization_from_top_level.  "
				"Missing element_to_top_level matrix");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_discretization_from_top_level.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_discretization_from_top_level */

int get_FE_element_discretization(struct FE_element *element,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional, void *conditional_data,
	cmzn_element_face_type face, struct FE_field *native_discretization_field,
	int *top_level_number_in_xi,struct FE_element **top_level_element,
	int *number_in_xi)
{
	FE_value element_to_top_level[9];
	int return_code;

	ENTER(get_FE_element_discretization);
	if (element&&top_level_number_in_xi&&top_level_element&&number_in_xi)
	{
		if (NULL != (*top_level_element=FE_element_get_top_level_element_conversion(
			element,*top_level_element,conditional,conditional_data,face,
			element_to_top_level)))
		{
			/* get the discretization requested for top-level element, from native
				 discretization field if not NULL and is element based in element */
			if (native_discretization_field&&
				FE_element_field_is_grid_based(*top_level_element,
					native_discretization_field))
			{
				int native_top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
				int dim;
				for (dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; dim++)
				{
					native_top_level_number_in_xi[dim] = 1;
				}
				/* use first component only */
				get_FE_element_field_component_grid_map_number_in_xi(*top_level_element,
					native_discretization_field, /*component_number*/0, native_top_level_number_in_xi);
				for (dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; dim++)
				{
					top_level_number_in_xi[dim] *= native_top_level_number_in_xi[dim];
				}
			}
			if (get_FE_element_discretization_from_top_level(element,number_in_xi,
				*top_level_element,top_level_number_in_xi,element_to_top_level))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_element_discretization.  Error getting discretization");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"get_FE_element_discretization.  "
				"Error getting top_level_element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_discretization */

/***************************************************************************//**
 * Return whether the shape-face mapping for the supplied face number gives the
 * face an inward normal.
 *
 * @param shape  The shape - must be 3 dimensional.
 * @param face_number  Face index from 0 to (shape->number_of_faces - 1).
 * @return  1 if face has inward normal, 0 otherwise.
 */
int FE_element_shape_face_has_inward_normal(struct FE_element_shape *shape,
	int face_number)
{
	int return_code;

	ENTER(FE_element_shape_face_has_inward_normal);
	return_code = 0;
	if (shape && (3 == shape->dimension) &&
		(0 <= face_number) && (face_number <= shape->number_of_faces))
	{
		FE_value *face_to_element, *outward_face_normal, result;
		FE_value face_xi1[3], face_xi2[3], actual_face_normal[3];
		face_to_element = shape->face_to_element +
			face_number*shape->dimension*shape->dimension;
		face_xi1[0] = face_to_element[1];
		face_xi1[1] = face_to_element[4];
		face_xi1[2] = face_to_element[7];
		face_xi2[0] = face_to_element[2];
		face_xi2[1] = face_to_element[5];
		face_xi2[2] = face_to_element[8];
		cross_product_FE_value_vector3(face_xi1, face_xi2, actual_face_normal);
		outward_face_normal = shape->face_normals + face_number*shape->dimension;
		result = actual_face_normal[0]*outward_face_normal[0] +
			actual_face_normal[1]*outward_face_normal[1] +
			actual_face_normal[2]*outward_face_normal[2];
		if (result < 0.0)
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_shape_face_has_inward_normal.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int FE_element_is_exterior_face_with_inward_normal(struct FE_element *element)
{
	int return_code;
	ENTER(FE_element_is_exterior_face_with_inward_normal);
	return_code = 0;
	if (element)
	{
		if ((element->getDimension() == 2) &&
			(1 == element->number_of_parents) && (element->parents[0]))
		{
			return_code =
				FE_element_shape_face_has_inward_normal(get_FE_element_shape(element->parents[0]),
					FE_element_get_child_face_number(element->parents[0], element));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_is_exterior_face_with_inward_normal.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int cmzn_element_add_stored_nodes_to_list(cmzn_element *element,
	LIST(cmzn_node) *nodeList, LIST(cmzn_node) *onlyFromNodeList)
{
	if (!(element && nodeList && onlyFromNodeList))
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
	if ((element->information) && (element->information->nodes))
	{
		cmzn_node *node;
		const int localNodesCount = element->information->number_of_nodes;
		for (int i = 0; i < localNodesCount; i++)
		{
			node = element->information->nodes[i];
			if (node && IS_OBJECT_IN_LIST(cmzn_node)(node, onlyFromNodeList))
			{
				if (!(ADD_OBJECT_TO_LIST(FE_node)(node, nodeList) ||
					IS_OBJECT_IN_LIST(FE_node)(node, nodeList)))
				{
					return_code = CMZN_ERROR_GENERAL;
					break;
				}
			}
		}
	}
	return return_code;
}

int cmzn_element_add_nodes_to_list(cmzn_element *element, LIST(cmzn_node) *nodeList)
{
	if (!(element && nodeList))
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
	cmzn_node *node;
	if ((element->information) && (element->information->nodes))
	{
		const int localNodesCount = element->information->number_of_nodes;
		for (int i = 0; i < localNodesCount; i++)
		{
			node = element->information->nodes[i];
			if ((node) && (!(ADD_OBJECT_TO_LIST(FE_node)(node, nodeList) ||
				IS_OBJECT_IN_LIST(FE_node)(node, nodeList))))
			{
				return_code = CMZN_ERROR_GENERAL;
				break;
			}
		}
	}
	if (0 < element->number_of_parents)
	{
		int number_of_element_field_nodes;
		cmzn_node_id *element_field_nodes_array;
		if (calculate_FE_element_field_nodes(element, /*face_number*/-1, (struct FE_field *)NULL,
			&number_of_element_field_nodes, &element_field_nodes_array,
			/*top_level_element*/(struct FE_element *)NULL))
		{
			for (int i = 0; i < number_of_element_field_nodes; i++)
			{
				node = element_field_nodes_array[i];
				if ((node) && (!(ADD_OBJECT_TO_LIST(FE_node)(node, nodeList) ||
						IS_OBJECT_IN_LIST(FE_node)(node, nodeList))))
					return_code = CMZN_ERROR_GENERAL;
				cmzn_node_destroy(&node);
			}
			DEALLOCATE(element_field_nodes_array);
		}
		else
			return_code = CMZN_ERROR_GENERAL;
	}
	return return_code;
}

int cmzn_element_remove_nodes_from_list(cmzn_element *element, LIST(cmzn_node) *nodeList)
{
	if (!(element && nodeList))
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
	cmzn_node *node;
	if ((element->information) && (element->information->nodes))
	{
		const int localNodesCount = element->information->number_of_nodes;
		for (int i = 0; i < localNodesCount; i++)
			REMOVE_OBJECT_FROM_LIST(cmzn_node)(element->information->nodes[i], nodeList);
	}
	if (0 < element->number_of_parents)
	{
		int number_of_element_field_nodes;
		cmzn_node_id *element_field_nodes_array;
		if (calculate_FE_element_field_nodes(element, /*face_number*/-1, (struct FE_field *)NULL,
			&number_of_element_field_nodes, &element_field_nodes_array,
			/*top_level_element*/(struct FE_element *)NULL))
		{
			for (int i = 0; i < number_of_element_field_nodes; i++)
			{
				node = element_field_nodes_array[i];
				REMOVE_OBJECT_FROM_LIST(cmzn_node)(element_field_nodes_array[i], nodeList);
				cmzn_node_destroy(&node);
			}
			DEALLOCATE(element_field_nodes_array);
		}
		else
			return_code = CMZN_ERROR_GENERAL;
	}
	return return_code;
}

int FE_element_is_top_level(struct FE_element *element,void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	if (element)
		return (0 == element->number_of_parents);
	return 0;
}

int FE_element_or_parent_has_field(struct FE_element *element,
	struct FE_field *field,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional, void *conditional_data)
{
	int i, return_code;

	ENTER(FE_element_or_parent_has_field);
	return_code = 0;
	if (element && field && element->fields)
	{
		if ((!conditional) || (conditional)(element, conditional_data))
		{
			if (FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(field,
				element->fields->element_field_list))
			{
				return_code = 1;
			}
			else
			{
				for (i = 0; i < element->number_of_parents; i++)
				{
					if (FE_element_or_parent_has_field(element->parents[i], field,
						conditional, conditional_data))
					{
						return_code = 1;
						break;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_or_parent_has_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_or_parent_has_field */

int FE_node_get_position_cartesian(struct FE_node *node,
	struct FE_field *coordinate_field, FE_value *node_x, FE_value *node_y,
	FE_value *node_z, FE_value *coordinate_jacobian)
{
	struct Coordinate_system *coordinate_system;
	FE_value node_1,node_2,node_3;
	int number_of_coordinate_components, return_code;

	ENTER(FE_node_get_position_cartesian);
	return_code = 0;
	if (node && coordinate_field && node_x && node_y && node_z)
	{
		if (coordinate_field->value_type==FE_VALUE_VALUE)
		{
			return_code = 1;
			number_of_coordinate_components=
				coordinate_field->number_of_components;
			if (!get_FE_nodal_FE_value_value(node,coordinate_field,/*component_number*/0,
				/*version*/0,FE_NODAL_VALUE,/*time*/0,&node_1))
			{
				return_code = 0;
			}
			if (1<number_of_coordinate_components)
			{
				if (!get_FE_nodal_FE_value_value(node,coordinate_field,/*component_number*/1,
					/*version*/0,FE_NODAL_VALUE,/*time*/0,&node_2))
				{
					return_code = 0;
				}
				if (2<number_of_coordinate_components)
				{
					if (!get_FE_nodal_FE_value_value(node,coordinate_field,/*component_number*/2,
						/*version*/0,FE_NODAL_VALUE,/*time*/0,&node_3))
					{
						return_code = 0;
					}
				}
				else
				{
					node_3=0.;
				}
			}
			else
			{
				node_2=0.;
				node_3=0.;
			}
			if (return_code)
			{
				coordinate_system=get_FE_field_coordinate_system(coordinate_field);
				/* transform points to cartesian coordinates */
				switch (coordinate_system->type)
				{
					case CYLINDRICAL_POLAR:
					{
						cylindrical_polar_to_cartesian(node_1,node_2,
							node_3,node_x,node_y,node_z,coordinate_jacobian);
					} break;
					case SPHERICAL_POLAR:
					{
						spherical_polar_to_cartesian(node_1,node_2,
							node_3,node_x,node_y,node_z,coordinate_jacobian);
					} break;
					case PROLATE_SPHEROIDAL:
					{

						prolate_spheroidal_to_cartesian(node_1,node_2,
							node_3,coordinate_system->parameters.focus,
							node_x,node_y,node_z,coordinate_jacobian);
					} break;
					case OBLATE_SPHEROIDAL:
					{

						oblate_spheroidal_to_cartesian(node_1,node_2,
							node_3,coordinate_system->parameters.focus,
							node_x,node_y,node_z,coordinate_jacobian);
					} break;
					default:
					{
						*node_x=node_1;
						*node_y=node_2;
						*node_z=node_3;
						if (coordinate_jacobian)
						{
							coordinate_jacobian[0]=1;
							coordinate_jacobian[1]=0;
							coordinate_jacobian[2]=0;
							coordinate_jacobian[3]=0;
							coordinate_jacobian[4]=1;
							coordinate_jacobian[5]=0;
							coordinate_jacobian[6]=0;
							coordinate_jacobian[7]=0;
							coordinate_jacobian[8]=1;
						}
					} break;
				} /* switch */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_get_position_cartesian.  Field not defined at node");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_get_position_cartesian.  Only supports FE_VALUE type");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_get_position_cartesian.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int FE_node_set_position_cartesian(struct FE_node *node,
	struct FE_field *coordinate_field,
	FE_value node_x,FE_value node_y,FE_value node_z)
{
	struct Coordinate_system *coordinate_system;
	FE_value node_1,node_2,node_3;
	int number_of_coordinate_components,return_code,version;
	struct FE_node_field *coordinate_node_field;
	struct FE_node_field_component *coordinate_node_field_component;

	ENTER(FE_node_set_position_cartesian);
	return_code = 0;
	if (node && coordinate_field && node->fields)
	{
		if (coordinate_field->value_type==FE_VALUE_VALUE)
		{
			coordinate_node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(
				coordinate_field, node->fields->node_field_list);
			if (coordinate_node_field)
			{
				return_code=1;
				number_of_coordinate_components = coordinate_field->number_of_components;

				coordinate_system = get_FE_field_coordinate_system(
					coordinate_node_field->field);

				switch (coordinate_system->type)
				{
					case CYLINDRICAL_POLAR:
					{
						cartesian_to_cylindrical_polar(node_x,node_y,node_z,
							&node_1,&node_2,&node_3,(FE_value *)NULL);
					} break;
					case PROLATE_SPHEROIDAL:
					{
						cartesian_to_prolate_spheroidal(node_x,node_y,node_z,
							coordinate_system->parameters.focus,
							&node_1,&node_2,&node_3,(FE_value *)NULL);
					} break;
					case OBLATE_SPHEROIDAL:
					{

						/*???DB.  Haven't written geometry function yet */
						node_1=node_x;
						node_2=node_y;
						node_3=node_z;
					} break;
					case SPHERICAL_POLAR:
					{

						/*???DB.  Haven't written geometry function yet */
						node_1=node_x;
						node_2=node_y;
						node_3=node_z;
					} break;
					default:
					{

						node_1=node_x;
						node_2=node_y;
						node_3=node_z;
					} break;
				}
				coordinate_node_field_component=coordinate_node_field->components;
				for (version=0;
						 version<coordinate_node_field_component->number_of_versions;version++)
				{
					set_FE_nodal_FE_value_value(node,coordinate_node_field->field,
						/*component_number*/0,version,FE_NODAL_VALUE,
						/*time*/0, node_1);
				}
				if (1<number_of_coordinate_components)
				{
					coordinate_node_field_component++;
					for (version=0;
							 version<coordinate_node_field_component->number_of_versions;version++)
					{
						set_FE_nodal_FE_value_value(node,coordinate_node_field->field,
						/*component_number*/1,version,
							FE_NODAL_VALUE, /*time*/0, node_2);
					}
					if (2<number_of_coordinate_components)
					{
						coordinate_node_field_component++;
						for (version=0;version<
									 coordinate_node_field_component->number_of_versions;version++)
						{
							set_FE_nodal_FE_value_value(node,coordinate_node_field->field,
								/*component_number*/2,version,
								FE_NODAL_VALUE, /*time*/0, node_3);
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_set_position_cartesian.  Field is not defined at node");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_set_position_cartesian.  Only supports FE_VALUE type");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_set_position_cartesian.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int FE_field_is_1_component_integer(struct FE_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Conditional function returning true if <field> has exactly 1 component and a
value type of integer.
This type of field is used for storing eg. grid_point_number.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_is_1_component_integer);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(INT_VALUE==field->value_type)&&
			(1==field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_is_1_component_integer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_1_component_integer */

int FE_field_is_coordinate_field(struct FE_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2001

DESCRIPTION :
Conditional function returning true if the <field> is a coordinate field
(defined by having a CM_field_type of coordinate) has a Value_type of
FE_VALUE_VALUE and has from 1 to 3 components.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_is_coordinate_field);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=
			(CM_COORDINATE_FIELD==field->cm_field_type)&&
			(FE_VALUE_VALUE==field->value_type)&&
			(1<=field->number_of_components)&&
			(3>=field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_is_coordinate_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_coordinate_field */

int FE_field_is_anatomical_fibre_field(struct FE_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
Conditional function returning true if the <field> is a anatomical field
(defined by having a CM_field_type of anatomical), has a Value_type of
FE_VALUE_VALUE, has from 1 to 3 components, and has a FIBRE coordinate system.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_is_anatomical_fibre_field);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(CM_ANATOMICAL_FIELD==field->cm_field_type)&&
			(FE_VALUE_VALUE==field->value_type)&&
			(1<=field->number_of_components)&&
			(3>=field->number_of_components)&&
			(FIBRE==field->coordinate_system.type);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_is_anatomical_fibre_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_anatomical_fibre_field */

int FE_field_is_embedded(struct FE_field *field, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 5 June 2003

DESCRIPTION :
Returns true if the values returned by <field> are a location in an FE_region,
either an element_xi value, or eventually a node.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_is_embedded);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code = (ELEMENT_XI_VALUE == field->value_type);
	}
	else
	{
		display_message(ERROR_MESSAGE, "FE_field_is_embedded.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_embedded */

int FE_field_is_defined_at_node(struct FE_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Returns true if the <field> is defined for the <node>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_is_defined_at_node);
	return_code=0;
	if (field&&node&&(node->fields))
	{
		if (FIND_BY_IDENTIFIER_IN_LIST(FE_node_field,field)(field,
			node->fields->node_field_list))
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_is_defined_at_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_defined_at_node */

int FE_node_field_is_not_defined(struct FE_node *node,void *field_void)
/*******************************************************************************
LAST MODIFIED : 15 September 2000

DESCRIPTION :
FE_node iterator version of FE_field_is_defined_at_node.
==============================================================================*/
{
	int return_code;
	struct FE_field *field;

	ENTER(FE_node_field_is_not_defined);
	if (node&&(field=(struct FE_field *)field_void))
	{
		return_code = !FE_field_is_defined_at_node(field,node);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_is_not_defined.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_field_is_not_defined */

int FE_field_is_defined_in_element(struct FE_field *field,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Returns true if the <field> is defined for the <element>.
==============================================================================*/
{
	int i, return_code;

	ENTER(FE_field_is_defined_in_element);
	return_code = 0;
	if (element && element->fields && field)
	{
		if (FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(field,
			element->fields->element_field_list))
		{
			return_code = 1;
		}
		else
		{
			for (i = 0; i < element->number_of_parents; i++)
			{
				if (FE_field_is_defined_in_element(field, element->parents[i]))
				{
					return_code = 1;
					break;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_is_defined_in_element.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_defined_in_element */

int FE_field_is_defined_in_element_not_inherited(struct FE_field *field,
	struct FE_element *element)
{
	int return_code;

	return_code = 0;
	if (element && element->fields)
	{
		if (FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(field,
			element->fields->element_field_list))
		{
			return_code = 1;
		}
	}

	return (return_code);
}

int FE_element_field_is_grid_based(struct FE_element *element,
	struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Returns true if <field> is grid-based in <element>. Only checks the first
component since we assume all subsequent components have the same basis and
numbers of grid cells in xi.
Returns 0 with no error if <field> is not defined over element or not element-
based in it.
==============================================================================*/
{
	int return_code;
	struct FE_element_field *element_field;

	ENTER(FE_element_field_is_grid_based);
	return_code=0;
	if (element && field && element->fields)
	{
		/* must have element->information for grid-based values_storage */
		if (element->information)
		{
			if ((element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
				field, element->fields->element_field_list)))
			{
				return_code =
					FE_element_field_has_element_grid_map(element_field, (void *)NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_is_grid_based.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_is_grid_based */

int get_FE_element_field_component_grid_map_number_in_xi(struct FE_element *element,
	struct FE_field *field, int component_number, int *number_in_xi)
{
	int *component_number_in_xi, dimension, i, return_code;
	struct FE_element_field *element_field;
	struct FE_element_field_component *component;

	ENTER(get_FE_element_field_component_grid_map_number_in_xi);
	return_code = 0;
	if (element && element->fields && number_in_xi &&
		(dimension = element->fields->fe_mesh->getDimension()) &&
		(component_number >= 0) && (component_number < field->number_of_components))
	{
		if ((element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
			field, element->fields->element_field_list)))
		{
			/* only GENERAL_FE_FIELD has components and can be grid-based */
			if (GENERAL_FE_FIELD == element_field->field->fe_field_type)
			{
				/* get first field component */
				if (element_field->components &&
					(component = element_field->components[component_number]))
				{
					if (ELEMENT_GRID_MAP==component->type)
					{
						if (NULL != (component_number_in_xi=
							component->map.element_grid_based.number_in_xi))
						{
							return_code=1;
							for (i=0;i<dimension;i++)
							{
								number_in_xi[i]=component_number_in_xi[i];
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"get_FE_element_field_component_grid_map_number_in_xi.  "
								"Missing component number_in_xi");
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_FE_element_field_component_grid_map_number_in_xi.  "
						"Missing element field component");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_element_field_component_grid_map_number_in_xi.  "
				"Field not defined for element");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_field_component_grid_map_number_in_xi.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_field_component_grid_map_number_in_xi */

int get_FE_element_field_component_number_of_grid_values(struct FE_element *element,
	struct FE_field *field, int component_number)
{
	int *component_number_in_xi,dimension,i,number_of_grid_values;
	struct FE_element_field *element_field;
	struct FE_element_field_component *component;

	ENTER(get_FE_element_field_component_number_of_grid_values);
	number_of_grid_values=0;
	if (element && element->fields && (dimension = element->fields->fe_mesh->getDimension()) &&
		(component_number >= 0) && (component_number < field->number_of_components))
	{
		if ((element_field=FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
			field, element->fields->element_field_list)) && (element_field->components))
		{
			/* only GENERAL_FE_FIELD has components and can be grid-based */
			if (GENERAL_FE_FIELD==element_field->field->fe_field_type)
			{
				if ((component = element_field->components[component_number]))
				{
					if (ELEMENT_GRID_MAP==component->type)
					{
						if (NULL != (component_number_in_xi=
							component->map.element_grid_based.number_in_xi))
						{
							number_of_grid_values=1;
							for (i=0;i<dimension;i++)
							{
								number_of_grid_values *= (component_number_in_xi[i] + 1);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"get_FE_element_field_component_number_of_grid_values.  "
								"Missing component number_in_xi");
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_FE_element_field_component_number_of_grid_values.  "
						"Missing element field component");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_element_field_component_number_of_grid_values.  "
				"Field not defined for element");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_field_component_number_of_grid_values.  Invalid argument(s)");
	}
	LEAVE;

	return (number_of_grid_values);
} /* get_FE_element_field_component_number_of_grid_values */

int get_FE_element_field_component(struct FE_element *element,
	struct FE_field *field, int component_number,
	struct FE_element_field_component **component_address)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Returns the element field component structure for <component_number> of <field>
at <element> if defined there; otherwise reports an error.
If fails, puts NULL in *<component_address> if supplied.
Note: returned component must not be modified or destroyed!
==============================================================================*/
{
	int return_code;
	struct FE_element_field *element_field;

	ENTER(get_FE_element_field_component);
	return_code = 0;
	if (element && element->fields && field && (0 <= component_number) &&
		(component_number < get_FE_field_number_of_components(field)) &&
		component_address)
	{
		if (NULL != (element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
			field, element->fields->element_field_list)))
		{
			if (element_field->components)
			{
				if (NULL != (*component_address = element_field->components[component_number]))
				{
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE, "get_FE_element_field_component.  "
						"Missing element field component");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "get_FE_element_field_component.  "
					"Missing element field components array");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "get_FE_element_field_component.  "
				"Field %s not defined for element", get_FE_field_name(field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_field_component.  Invalid argument(s)");
	}
	if ((!return_code) && component_address)
	{
		*component_address = (struct FE_element_field_component *)NULL;
	}
	LEAVE;

	return (return_code);
} /* get_FE_element_field_component */

#define INSTANTIATE_GET_FE_ELEMENT_FIELD_COMPONENT_FUNCTION( macro_value_type, value_enum ) \
int get_FE_element_field_component_grid_ ## macro_value_type ## _values( \
	struct FE_element *element,struct FE_field *field,int component_number, \
	macro_value_type **values) \
/******************************************************************************* \
LAST MODIFIED : 22 April 2005 \
 \
DESCRIPTION : \
If <field> is grid-based in <element>, returns an allocated array of the grid \
values stored for <component_number>. To get number of values returned, call \
get_FE_element_field_component_number_of_grid_values; Grids change in xi0 fastest. \
It is up to the calling function to DEALLOCATE the returned values. \
==============================================================================*/ \
{ \
	macro_value_type *value; \
	int *component_number_in_xi,dimension,i,number_of_grid_values,return_code, \
		size; \
	struct FE_element_field *element_field; \
	struct FE_element_field_component *component; \
	Value_storage *values_storage; \
 \
	ENTER(get_FE_element_field_component_grid_ ## macro_value_type ## _values); \
	return_code=0; \
	if (element && element->fields && element->information &&  \
		(dimension = element->fields->fe_mesh->getDimension()) && field && \
		(0<=component_number)&&(component_number<field->number_of_components)&& \
		(value_enum==field->value_type)&&values) \
	{ \
		if (NULL != (element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)( \
			field,element->fields->element_field_list))) \
		{ \
			/* get the component */ \
			if (element_field->components&& \
				(component=element_field->components[component_number])) \
			{ \
				if ((ELEMENT_GRID_MAP==component->type)&& \
					(NULL != (values_storage=element->information->values_storage))) \
				{ \
					if (NULL != (component_number_in_xi = \
						component->map.element_grid_based.number_in_xi)) \
					{ \
						values_storage += component->map.element_grid_based.value_index; \
						size = get_Value_storage_size(value_enum, \
							(struct FE_time_sequence *)NULL); \
						number_of_grid_values=1; \
						for (i=0;i<dimension;i++) \
						{ \
							number_of_grid_values *= (component_number_in_xi[i] + 1); \
						} \
						if (ALLOCATE(*values,macro_value_type,number_of_grid_values)) \
						{ \
							return_code=1; \
							value= *values; \
							for (i=number_of_grid_values;0<i;i--) \
							{ \
								*value = *((macro_value_type *)values_storage); \
								value++; \
								values_storage += size; \
							} \
						} \
						else \
						{ \
							display_message(ERROR_MESSAGE, \
								"get_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
								"Not enough memory"); \
						} \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"get_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
							"Missing component number_in_xi"); \
					} \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"get_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
						"Field is not grid-based in element"); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"get_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
					"Missing element field component"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"get_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
				"Field not defined for element"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"get_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
			"Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* get_FE_element_field_component_grid_ ## macro_value_type ## _values */

#define INSTANTIATE_SET_FE_ELEMENT_FIELD_COMPONENT_FUNCTION( macro_value_type, value_enum ) \
int set_FE_element_field_component_grid_ ## macro_value_type ## _values( \
	struct FE_element *element,struct FE_field *field,int component_number, \
	macro_value_type *values) \
/******************************************************************************* \
LAST MODIFIED : 21 April 2005 \
\
DESCRIPTION : \
If <field> is grid-based in <element>, copies <values> into the values storage \
for <component_number>. To get number of values to pass, call \
get_FE_element_field_component_number_of_grid_values; Grids change in xi0 fastest. \
==============================================================================*/ \
{ \
	macro_value_type *value; \
	int *component_number_in_xi,dimension,i,number_of_grid_values,return_code, \
		size; \
	struct FE_element_field *element_field; \
	struct FE_element_field_component *component; \
	Value_storage *values_storage; \
 \
	ENTER(set_FE_element_field_component_grid_ ## macro_value_type ## _values); \
	return_code=0; \
	if (element&&element->fields && element->information && \
		(dimension = element->fields->fe_mesh->getDimension()) && field && \
		(0<=component_number)&&(component_number<field->number_of_components)&& \
		(value_enum==field->value_type)&&values) \
	{ \
		if (NULL != (element_field=FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)( \
			field,element->fields->element_field_list))) \
		{ \
			/* get the component */ \
			if (element_field->components&& \
				(NULL != (component = element_field->components[component_number]))) \
			{ \
				if ((ELEMENT_GRID_MAP==component->type)&& \
					(NULL != (values_storage=element->information->values_storage))) \
				{ \
					if (NULL != (component_number_in_xi = \
						component->map.element_grid_based.number_in_xi)) \
					{ \
						return_code=1; \
						values_storage += component->map.element_grid_based.value_index; \
						size = get_Value_storage_size(value_enum, \
							(struct FE_time_sequence *)NULL); \
						number_of_grid_values=1; \
						for (i=0;i<dimension;i++) \
						{ \
							number_of_grid_values *= (component_number_in_xi[i] + 1); \
						} \
						value=values; \
						for (i=number_of_grid_values;0<i;i--) \
						{ \
							/*???RC following should be a macro */ \
							*((macro_value_type *)values_storage) = *value; \
							value++; \
							values_storage += size; \
						} \
						if (return_code) \
						{ \
							element->fields->fe_mesh->elementFieldChange(element, field); \
						} \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"set_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
							"Missing component number_in_xi"); \
					} \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"set_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
						"Field is not grid-based in element"); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"set_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
					"Missing element field component"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"set_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
				"Field not defined for element"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"set_FE_element_field_component_grid_ ## macro_value_type ## _values.  " \
			"Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* set_FE_element_field_component_grid_ ## macro_value_type ## _values */

#define INSTANTIATE_FE_ELEMENT_FIELD_COMPONENT_FUNCTIONS( macro_value_type , value_enum ) \
INSTANTIATE_GET_FE_ELEMENT_FIELD_COMPONENT_FUNCTION(macro_value_type,value_enum) \
INSTANTIATE_SET_FE_ELEMENT_FIELD_COMPONENT_FUNCTION(macro_value_type,value_enum)

INSTANTIATE_FE_ELEMENT_FIELD_COMPONENT_FUNCTIONS( FE_value , FE_VALUE_VALUE )
INSTANTIATE_FE_ELEMENT_FIELD_COMPONENT_FUNCTIONS( int , INT_VALUE )

int FE_element_field_get_component_FE_basis(struct FE_element *element,
	struct FE_field *field, int component_number, struct FE_basis **fe_basis)
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION :
If <field> is standard node based in <element>, returns the <fe_basis> used for
<component_number>.
==============================================================================*/
{
	int return_code;
	struct FE_element_field *element_field;

	ENTER(FE_element_field_get_component_FE_basis);
	return_code = 0;
	if (element && field && fe_basis && element->fields)
	{
		*fe_basis = (struct FE_basis *)NULL;
		if ((element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
			field, element->fields->element_field_list)))
		{
			return_code = FE_element_field_private_get_component_FE_basis(
				element_field, component_number, fe_basis);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_get_component_FE_basis.  "
				"Field not defined for element");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_get_component_FE_basis.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_get_component_FE_basis */

int FE_node_smooth_FE_field(struct FE_node *node, struct FE_field *fe_field,
	FE_value time, struct FE_field *element_count_fe_field)
{
	int return_code;

	if (node && fe_field && element_count_fe_field)
	{
		const FE_nodal_value_type types[3] =
			{FE_NODAL_D_DS1, FE_NODAL_D_DS2, FE_NODAL_D_DS3};
		const int number_of_components = get_FE_field_number_of_components(fe_field);
		FE_value value;
		int count;
		return_code = 1;
		for (int component = 0; (component < number_of_components) && return_code; ++component)
		{
			// Note: number of versions will eventually depend on value type
			const int number_of_versions =
				get_FE_node_field_component_number_of_versions(node, fe_field, component);
			for (int j = 0; (j < 3) && return_code; j++)
			{
				const FE_nodal_value_type type = types[j];
				for (int version = 0; (version < number_of_versions) && return_code; ++version)
				{
					if (FE_nodal_value_version_exists(node, fe_field, component, version, type))
					{
						if (get_FE_nodal_FE_value_value(node, fe_field, component, version, type, time, &value) &&
							get_FE_nodal_int_value(node, element_count_fe_field, component, version, type, time, &count))
						{
							if (0 < count)
							{
								if (!set_FE_nodal_FE_value_value(node, fe_field, component,
									version, type, time, value / (FE_value)count))
								{
									return_code = 0;
								}
							}
						}
						else
						{
							return_code = 0;
						}
					}
				}
			}
		}
		if (return_code)
		{
			/* undefine the element_count_fe_field at <node> */
			return_code = undefine_FE_field_at_node(node, element_count_fe_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_smooth_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/**
 * Used by FE_element_smooth_FE_field.
 * Adds <delta> to the identified quantity in <fe_field> and increments the
 * integer counter for the corresponding quantity in <count_fe_field>.
 */
static int FE_node_field_component_accumulate_value(struct FE_node *node,
	struct FE_field *fe_field, struct FE_field *count_fe_field,
	int component_number, int version, enum FE_nodal_value_type type,
	FE_value time, FE_value delta)
{
	FE_value value;
	int int_value, return_code;

	if (node && fe_field && count_fe_field && (0 <= component_number) &&
		(component_number <= get_FE_field_number_of_components(fe_field)) &&
		(0 <= version))
	{
		return_code = 1;
		if (FE_nodal_value_version_exists(node, fe_field, component_number, version, type))
		{
			if (!(get_FE_nodal_FE_value_value(node, fe_field, component_number, version,
				type, time, &value) &&
				set_FE_nodal_FE_value_value(node, fe_field, component_number, version, type,
					time, value + delta) &&
				get_FE_nodal_int_value(node, count_fe_field, component_number, version, type,
					time, &int_value) &&
				set_FE_nodal_int_value(node, count_fe_field, component_number, version, type,
					time, int_value + 1)))
			{
				display_message(ERROR_MESSAGE,
					"FE_node_field_component_accumulate_value.  Failed");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_field_component_accumulate_value.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

class FE_element_accumulate_node_values
{
	FE_element *element;
	Standard_node_to_element_map **standard_node_maps;
	FE_node **copy_nodes;
	FE_field *fe_field, *element_count_fe_field;
	int component_number;
	FE_value time;
	FE_value *component_values;

public:
	FE_element_accumulate_node_values(FE_element *elementIn,
			Standard_node_to_element_map **standard_node_mapsIn,
			FE_node **copy_nodesIn, FE_field *fe_fieldIn, FE_field *element_count_fe_fieldIn,
			int component_numberIn, FE_value timeIn, FE_value *component_valuesIn) :
		element(elementIn),
		standard_node_maps(standard_node_mapsIn),
		copy_nodes(copy_nodesIn),
		fe_field(fe_fieldIn),
		element_count_fe_field(element_count_fe_fieldIn),
		component_number(component_numberIn),
		time(timeIn),
		component_values(component_valuesIn)
	{
	}

	/**
	 * @param xiIndex  Element chart xi index starting at 0.
	 * @param localNode1  Local node index into copy_nodes array for edge node 1.
	 * @param localNode2  Local node index into copy_nodes array for edge node 2.
	 */
	void accumulate_edge(int xiIndex, int localNode1, int localNode2)
	{
		const int valueIndex =
			(xiIndex == 0) ? 1 :
			(xiIndex == 1) ? 2 :
			(xiIndex == 2) ? 4 :
			0;
		if (valueIndex == 0)
			return;
		FE_value delta = this->component_values[localNode2] - this->component_values[localNode1];
		for (int n = 0; n < 2; ++n)
		{
			const int localNode = (n == 0) ? localNode1 : localNode2;
			Standard_node_to_element_map *standard_node_map = this->standard_node_maps[localNode];
			FE_nodal_value_type derivativeType = Standard_node_to_element_map_get_nodal_value_type(standard_node_map, valueIndex);
			if ((derivativeType == FE_NODAL_D_DS1) || (derivativeType == FE_NODAL_D_DS2) || (derivativeType == FE_NODAL_D_DS3))
			{
				int version = Standard_node_to_element_map_get_nodal_version(standard_node_map, valueIndex);
				FE_node_field_component_accumulate_value(this->copy_nodes[localNode],
					this->fe_field, this->element_count_fe_field, this->component_number,
					version - 1, derivativeType, this->time, delta);
			}
		}
	}
};

int FE_element_smooth_FE_field(struct FE_element *element,
	struct FE_field *fe_field, FE_value time,
	struct FE_field *element_count_fe_field,
	struct LIST(FE_node) *copy_node_list)
{
	FE_value component_value[8], value,
		xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, i, index, j, k, n, number_of_components,
		number_of_nodes, number_of_values, return_code;
	struct FE_element_field *element_field;
	struct FE_element_field_component *element_field_component;
	struct FE_element_field_values *fe_element_field_values;
	struct FE_node *copy_node[8], *node;
	struct FE_node_field_creator *fe_node_field_creator;
	struct LIST(FE_field) *fe_field_list;

	ENTER(FE_element_smooth_FE_field);
	FE_element_shape *element_shape = get_FE_element_shape(element);
	if (element_shape && fe_field && element_count_fe_field && copy_node_list &&
		(FE_VALUE_VALUE == get_FE_field_value_type(fe_field)) &&
		(INT_VALUE == get_FE_field_value_type(element_count_fe_field)) &&
		(number_of_components = get_FE_field_number_of_components(fe_field)) &&
		(get_FE_field_number_of_components(element_count_fe_field) ==
			number_of_components))
	{
		return_code = 1;
		/* work out if element has this fe_field defined and if it is a "square"
			 shape */
		if (element->information && element->fields &&
			(element_field = FIND_BY_IDENTIFIER_IN_LIST(FE_element_field,field)(
				fe_field, element->fields->element_field_list)) &&
			element_field->components &&
			FE_element_shape_is_line(element_shape))
		{
			fe_field_list = (struct LIST(FE_field) *)NULL;
			element_dimension = get_FE_element_dimension(element);
			for (i = 0; (i < number_of_components) && return_code; i++)
			{
				element_field_component = (element_field->components)[i];
				/* work out if the node map is appropriate for smoothing */
				if ((STANDARD_NODE_TO_ELEMENT_MAP == element_field_component->type) &&
					element_field_component->map.standard_node_based.node_to_element_maps
					&& (0 < (number_of_nodes = element_field_component->
						map.standard_node_based.number_of_nodes)) && (
							((1 == element_dimension) && (2 == number_of_nodes)) ||
							((2 == element_dimension) && (4 == number_of_nodes)) ||
							((3 == element_dimension) && (8 == number_of_nodes))))
				{
					for (n = 0; (n < number_of_nodes) && return_code; n++)
					{
						/* get the node_to_element_map and hence the node */
						Standard_node_to_element_map *standard_node_map =
							element_field_component->map.standard_node_based.node_to_element_maps[n];
						if ((standard_node_map) &&
							get_FE_element_node(element, standard_node_map->node_index,
								&node) && node)
						{
							copy_node[n] =
								FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(
									get_FE_node_identifier(node), copy_node_list);
							if (!(copy_node[n]))
							{
								fe_node_field_creator = (struct FE_node_field_creator *)NULL;
								if ((fe_field_list ||
									((fe_field_list = CREATE(LIST(FE_field))()) &&
										ADD_OBJECT_TO_LIST(FE_field)(fe_field, fe_field_list))) &&
									(copy_node[n] =
										FE_node_copy_with_FE_field_list(node, fe_field_list)) &&
									(fe_node_field_creator =
										create_FE_node_field_creator_from_node_field(node,
											fe_field)) &&
									define_FE_field_at_node(copy_node[n], element_count_fe_field,
										(struct FE_time_sequence *)NULL, fe_node_field_creator) &&
									ADD_OBJECT_TO_LIST(FE_node)(copy_node[n], copy_node_list))
								{
									FE_value *values;
									/* assume that element_count_fe_field values are cleared to
										 zero by define_FE_field_at_node */
									/* first clear all values for fe_field in copy_node */
									if (get_FE_nodal_field_FE_value_values(fe_field, node,
										&number_of_values, time, &values))
									{
										for (k = 0; k < number_of_values; k++)
										{
											values[k] = 0.0;
										}
										if (!set_FE_nodal_field_FE_value_values(fe_field,
											copy_node[n], values, &number_of_values, time))
										{
											display_message(ERROR_MESSAGE,
												"FE_element_smooth_FE_field.  Could clear node values");
											return_code = 0;
										}
										DEALLOCATE(values);
										/* restore the nodal values, all versions */
										for (j = 0; j < number_of_components; j++)
										{
											const int number_of_versions =
												get_FE_node_field_component_number_of_versions(node, fe_field, /*component*/j);
											for (int v = 0; v < number_of_versions; ++v)
											{
												if (!(get_FE_nodal_FE_value_value(node,
													fe_field, /*component_number*/j, v,
													FE_NODAL_VALUE, time, &value) &&
													set_FE_nodal_FE_value_value(copy_node[n],
														fe_field, /*component_number*/j, v,
														FE_NODAL_VALUE, time, value)))
												{
													display_message(ERROR_MESSAGE,
														"FE_element_smooth_FE_field.  "
														"Could not copy node value");
													return_code = 0;
												}
											}
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"FE_element_smooth_FE_field.  Could get all node values");
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"FE_element_smooth_FE_field.  Could not copy node");
									DESTROY(FE_node)(&(copy_node[n]));
									return_code = 0;
								}
								if (fe_node_field_creator)
								{
									DESTROY(FE_node_field_creator)(&fe_node_field_creator);
								}
							}
							/* set unit scale factors */
							if (return_code && standard_node_map->scale_factor_indices)
							{
								for (k = 0; (k < standard_node_map->number_of_nodal_values) &&
									return_code; k++)
								{
									if (0 <=
										(index = standard_node_map->scale_factor_indices[k]))
									{
										if (!set_FE_element_scale_factor(element, index, 1.0))
										{
											display_message(ERROR_MESSAGE,
												"FE_element_smooth_FE_field.  "
												"Could set unit scale factor");
											return_code = 0;
										}
									}
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"FE_element_smooth_FE_field.  Element is missing a node");
							return_code = 0;
						}
					}
					if (return_code)
					{
						/* get nodal values */
						/* need to calculate field values so that optional modify function can
								make its changes, and also to handle version mapping */
						fe_element_field_values = CREATE(FE_element_field_values)();
						if (calculate_FE_element_field_values(element, fe_field, time,
							/*calculate_derivatives*/0, fe_element_field_values,
							/*top_level_element*/(struct FE_element *)NULL))
						{
							for (n = 0; (n < number_of_nodes) && return_code; n++)
							{
								xi[0] = (FE_value)(n & 1);
								xi[1] = (FE_value)((n & 2)/2);
								xi[2] = (FE_value)((n & 4)/4);
								if (!calculate_FE_element_field(/*component_number*/i,
									fe_element_field_values, xi, &(component_value[n]),
									/*jacobian*/(FE_value *)NULL))
								{
									display_message(ERROR_MESSAGE,
										"FE_element_smooth_FE_field.  Could not calculate element field");
									return_code = 0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"FE_element_smooth_FE_field.  Could not get element field values");
							return_code = 0;
						}
						DESTROY(FE_element_field_values)(&fe_element_field_values);
					}
					if (return_code)
					{
						FE_element_accumulate_node_values element_accumulate_node_values(element,
							element_field_component->map.standard_node_based.node_to_element_maps,
							copy_node, fe_field, element_count_fe_field, /*component_number*/i, time,
							component_value);
						element_accumulate_node_values.accumulate_edge(/*xi*/0, 0, 1);
						if (1 < element_dimension)
						{
							element_accumulate_node_values.accumulate_edge(/*xi*/0, 2, 3);
							element_accumulate_node_values.accumulate_edge(/*xi*/1, 0, 2);
							element_accumulate_node_values.accumulate_edge(/*xi*/1, 1, 3);
							if (2 < element_dimension)
							{
								element_accumulate_node_values.accumulate_edge(/*xi*/0, 4, 5);
								element_accumulate_node_values.accumulate_edge(/*xi*/0, 6, 7);
								element_accumulate_node_values.accumulate_edge(/*xi*/1, 4, 6);
								element_accumulate_node_values.accumulate_edge(/*xi*/1, 5, 7);
								element_accumulate_node_values.accumulate_edge(/*xi*/2, 0, 4);
								element_accumulate_node_values.accumulate_edge(/*xi*/2, 1, 5);
								element_accumulate_node_values.accumulate_edge(/*xi*/2, 2, 6);
								element_accumulate_node_values.accumulate_edge(/*xi*/2, 3, 7);
							}
						}
					}
				}
			}
			/* clean up */
			if (fe_field_list)
			{
				DESTROY(LIST(FE_field))(&fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_smooth_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_smooth_FE_field */

struct FE_field_order_info *CREATE(FE_field_order_info)(void)
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Creates an empty FE_field_order_info structure.
==============================================================================*/
{
	struct FE_field_order_info *field_order_info;

	ENTER(CREATE(FE_field_order_info));
	if (ALLOCATE(field_order_info,struct FE_field_order_info,1))
	{
		field_order_info->allocated_number_of_fields = 0;
		field_order_info->number_of_fields = 0;
		field_order_info->fields = (struct FE_field **)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_field_order_info).  Not enough memory");
	}
	LEAVE;

	return (field_order_info);
} /* CREATE(FE_field_order_info) */

int DESTROY(FE_field_order_info)(
	struct FE_field_order_info **field_order_info_address)
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION
Frees them memory used by field_order_info.
==============================================================================*/
{
	int return_code, i;
	struct FE_field_order_info *field_order_info;

	ENTER(DESTROY(FE_field_order_info));
	if ((field_order_info_address) &&
		(field_order_info = *field_order_info_address))
	{
		if (field_order_info->fields)
		{
			for (i = 0; i < field_order_info->number_of_fields; i++)
			{
				DEACCESS(FE_field)(&(field_order_info->fields[i]));
			}
			DEALLOCATE(field_order_info->fields);
		}
		DEALLOCATE(*field_order_info_address);
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_field_order_info) */

int add_FE_field_order_info_field(
	struct FE_field_order_info *field_order_info, struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Adds <field> to the end of the list of fields in <field_order_info>.
==============================================================================*/
{
#define FE_FIELD_ORDER_INFO_ALLOCATE_SIZE 10
	int return_code;
	struct FE_field **temp_fields;

	ENTER(add_FE_field_order_info_field);
	if (field_order_info && field)
	{
		return_code = 1;
		if (field_order_info->number_of_fields ==
			field_order_info->allocated_number_of_fields)
		{
			field_order_info->allocated_number_of_fields +=
				FE_FIELD_ORDER_INFO_ALLOCATE_SIZE;
			if (REALLOCATE(temp_fields, field_order_info->fields, struct FE_field *,
				field_order_info->allocated_number_of_fields))
			{
				field_order_info->fields = temp_fields;
			}
			else
			{
				field_order_info->allocated_number_of_fields -=
					FE_FIELD_ORDER_INFO_ALLOCATE_SIZE;
				display_message(ERROR_MESSAGE,
					"add_FE_field_order_info_field.  Not enough memory");
				return_code = 0;
			}
		}
		if (return_code)
		{
			field_order_info->fields[field_order_info->number_of_fields] =
				ACCESS(FE_field)(field);
			field_order_info->number_of_fields++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_FE_field_order_info_field.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* add_FE_field_order_info_field */

int FE_field_add_to_FE_field_order_info(struct FE_field *field,
	void *field_order_info_void)
{
	return (add_FE_field_order_info_field(
		(struct FE_field_order_info *)field_order_info_void, field));
}

int FE_field_order_info_prioritise_indexer_fields(
	struct FE_field_order_info *field_order_info)
{
	int i, j, k, number_of_indexed_values,return_code;
	struct FE_field *indexer_field;

	return_code = 0;
	if (field_order_info)
	{
		for (i = 0; i < field_order_info->number_of_fields; i++)
		{
			indexer_field = NULL;
			if ((INDEXED_FE_FIELD ==
				get_FE_field_FE_field_type(field_order_info->fields[i])) &&
				get_FE_field_type_indexed(field_order_info->fields[i], &indexer_field,
					/*ignored*/&number_of_indexed_values))
			{
				for (j = i + 1; j < field_order_info->number_of_fields; j++)
				{
					if (field_order_info->fields[j] == indexer_field)
					{
						for (k = j; k > i; k--)
						{
							field_order_info->fields[k] = field_order_info->fields[k - 1];
						}
						field_order_info->fields[i] = indexer_field;
						break;
					}
				}
			}
		}
		return_code = 1;
	}
	return return_code;
}

int clear_FE_field_order_info(struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Clears the fields from <field_order_info>.
==============================================================================*/
{
	int i, return_code;

	ENTER(clear_FE_field_order_info_field);
	if (field_order_info)
	{
		for (i = 0; i < field_order_info->number_of_fields; i++)
		{
			DEACCESS(FE_field)(&(field_order_info->fields[i]));
		}
		field_order_info->number_of_fields = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"clear_FE_field_order_info_field.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* clear_FE_field_order_field */

int get_FE_field_order_info_number_of_fields(
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <field_order_info> number_of_fields
==============================================================================*/
{
	int number_of_fields;

	ENTER(get_FE_field_order_info_number_of_fields);
	if (field_order_info)
	{
		number_of_fields=field_order_info->number_of_fields;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_order_info_number_of_fields.  Invalid argument");
		number_of_fields=0;
	}
	LEAVE;

	return (number_of_fields);
} /* get_FE_field_order_info_number_of_fields */

struct FE_field *get_FE_field_order_info_field(
	struct FE_field_order_info *field_order_info,int field_number)
/*******************************************************************************
LAST MODIFIED : 4 September 2001

DESCRIPTION :
Gets the <field_order_info> field at the specified field_number.
==============================================================================*/
{
	struct FE_field *field;

	ENTER(get_FE_field_order_info_field);
	if (field_order_info &&
		(field_number <= field_order_info->number_of_fields))
	{
		field=field_order_info->fields[field_number];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_order_info_field.  Invalid argument(s)");
		field = (struct FE_field *)NULL;
	}
	LEAVE;

	return (field);
} /* get_FE_field_order_field */

int define_node_field_and_field_order_info(struct FE_node *node,
	struct FE_field *field, struct FE_node_field_creator *node_field_creator,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Helper function for create_config_template_node() and
create_mapping_template_node() that, given the node, field and
field_order_info, defines the field at the node, and places it at the end of
the field_order_info list.
==============================================================================*/
{
	int return_code;

	ENTER(define_node_field_and_field_order_info);
	return_code = 0;
	if (node && field && field_order_info && node_field_creator)
	{
		return_code = 1;
		if (define_FE_field_at_node(node, field, (struct FE_time_sequence *)NULL,
			node_field_creator))
		{
			if (!add_FE_field_order_info_field(field_order_info, field))
			{
				display_message(ERROR_MESSAGE,
					"define_node_field_and_field_order_info.  "
					"Could not add field to list");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_node_field_and_field_order_info.  "
				"Could not define field at node");
			/*???RC This should not have been here: */
			/* DESTROY(FE_field)(&field); */
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_node_field_and_field_order_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_node_field_and_field_order_info */

struct FE_node_order_info *CREATE(FE_node_order_info)(
	int number_of_nodes)
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Allocate space for an array of pointers to nodes of length number_of_nodes,
set these to NULL, copy the number_of_nodes.
==============================================================================*/
{
	int i;
	struct FE_node_order_info *node_order_info;

	ENTER(CREATE(FE_node_order_info));
	if (ALLOCATE(node_order_info,struct FE_node_order_info,1))
	{
		if (number_of_nodes>0)
		{
			if (ALLOCATE(node_order_info->nodes,struct FE_node *,
				number_of_nodes))
			{
				node_order_info->number_of_nodes = number_of_nodes;
				for (i=0;i<number_of_nodes;i++)
				{
					node_order_info->nodes[i]=(struct FE_node *)NULL;
				}
				node_order_info->access_count=0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"CREATE(FE_node_order_info).  Could not allocate memory for node_field_info->nodes");
				DEALLOCATE(node_order_info->nodes);
				DEALLOCATE(node_order_info);
			}
		}
		else
		{
			node_order_info->number_of_nodes=0;
			node_order_info->nodes=(struct FE_node **)NULL;
			node_order_info->access_count=0;
		}
		node_order_info->current_node_number=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"CREATE(FE_node_order_info).  Could not allocate memory for node field info");
		DEALLOCATE(node_order_info);
	}
	LEAVE;

	return (node_order_info);
} /* CREATE(FE_node_order_info) */

int DESTROY(FE_node_order_info)(
	struct FE_node_order_info **node_order_info_address)
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Frees them memory used by node_order_info.
==============================================================================*/
{
	int i,return_code;
	struct FE_node_order_info *node_order_info;
	struct FE_node *node;

	ENTER(DESTROY(FE_node_order_info));
	return_code=0;
	if ((node_order_info_address)&&
		(node_order_info= *node_order_info_address))
	{
		/* free the components */
		for (i=0;i<node_order_info->number_of_nodes;i++)
		{
			node=node_order_info->nodes[i];
			DEACCESS(FE_node)(&node);
		}
		DEALLOCATE(node_order_info->nodes);
		DEALLOCATE(*node_order_info_address);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_node_order_info) */

DECLARE_OBJECT_FUNCTIONS(FE_node_order_info)

PROTOTYPE_COPY_OBJECT_FUNCTION(FE_node_order_info)
/*******************************************************************************
LAST MODIFIED : 11 August 1999

DESCRIPTION :
Makes an exacy copy of the FE_node_order_info
==============================================================================*/
{
	int return_code,i;

	ENTER(COPY(FE_node_order_info));
	return_code=0;
	/* check the arguments */
	if (source&&destination)
	{
		/* free any existing the destination things */
		if (destination->number_of_nodes)
		{
			for (i=0;i<destination->number_of_nodes;i++)
			{
				DEACCESS(FE_node)(&(destination->nodes[i]));
			}
			DEALLOCATE(destination->nodes);
			destination->number_of_nodes=0;
		}
		if (ALLOCATE(destination->nodes,struct FE_node*,source->number_of_nodes))
		{
			/* copy the new */
			destination->number_of_nodes=source->number_of_nodes;
			for (i=0;i<destination->number_of_nodes;i++)
			{
				destination->nodes[i]=ACCESS(FE_node)(source->nodes[i]);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"COPY(FE_node_order_info).  Out of memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"COPY(FE_node_order_info).  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* COPY(FE_node_order_info) */

int get_FE_node_order_info_number_of_nodes(
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <node_order_info> number_of_nodes
==============================================================================*/
{
	int number_of_nodes;

	ENTER(get_FE_node_order_info_number_of_nodes);
	if (node_order_info)
	{
		number_of_nodes=node_order_info->number_of_nodes;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_order_info_number_of_nodes.  Invalid argument");
		number_of_nodes=0;
	}
	LEAVE;

	return (number_of_nodes);
} /* get_FE_node_order_info_number_of_nodes */

struct FE_node *get_FE_node_order_info_node(
	struct FE_node_order_info *node_order_info,int node_number)
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the <node_order_info> node at the specified node_number
==============================================================================*/
{
	struct FE_node *node;

	ENTER(get_FE_node_order_info_node);
	if ((node_order_info)&&
		(node_number<=node_order_info->number_of_nodes))
	{
		node=node_order_info->nodes[node_number];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_order_info_node.  Invalid argument");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /* get_FE_node_order_info_node */

int set_FE_node_order_info_node(
	struct FE_node_order_info *node_order_info,int node_number,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
Sets the <node_order_info> node at the specified node_number.
Also sets the current_node_number to <the node_number>
==============================================================================*/
{
	int return_code

	ENTER(set_FE_node_order_info_node);
	return_code=0;
	if ((node_order_info)&&
		(node_number<=node_order_info->number_of_nodes)&&(node))
	{
		REACCESS(FE_node)(&(node_order_info->nodes[node_number]),node);
		node_order_info->current_node_number=node_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_node_order_info_node.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_group_order_node */

int get_FE_node_order_info_current_node_number(
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
gets the <node_order_info> <current_node_number>
==============================================================================*/
{
	int current_node_number;

	ENTER(get_FE_node_order_info_current_node_number);
	if (node_order_info&&(node_order_info->number_of_nodes>0))
	{
		current_node_number=node_order_info->current_node_number;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_order_info_current_node_number.  Invalid argument");
		current_node_number=-1;
	}
	LEAVE;

	return (current_node_number);
} /* get_FE_node_order_info_current_node_number */

int set_FE_node_order_info_current_node_number(
	struct FE_node_order_info *node_order_info,int current_node_number)
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Sets the <node_order_info> <current_node_number>
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_node_order_info_current_node_number);
	return_code=0;
	if ((node_order_info)&&(current_node_number>-1)&&
		(current_node_number<=node_order_info->number_of_nodes))
	{
		node_order_info->current_node_number=current_node_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_node_order_info_current_node_number.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_order_info_current_node_number */

struct FE_node *get_FE_node_order_info_current_node(
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> node at the current_node_number
==============================================================================*/
{
	struct FE_node *node;

	ENTER(get_FE_node_order_info_current_node);
	if (node_order_info)
	{
		if (node_order_info->number_of_nodes)
		{
			node=node_order_info->nodes[node_order_info->current_node_number];
		}
		else
		{
			node=(struct FE_node *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_order_info_current_node.  Invalid argument");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /*get_FE_node_order_info_current_node  */

struct FE_node *get_FE_node_order_info_next_node(
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> next node by incrementing the current_node_number,
and returning the new current node. If at the end of the array, return null.
==============================================================================*/
{
	struct FE_node *node;

	ENTER(get_FE_node_order_info_next_node);
	if (node_order_info)
	{
		if ((node_order_info->number_of_nodes)&&
			(node_order_info->current_node_number<(node_order_info->number_of_nodes-1)))
		{
			node_order_info->current_node_number++;
			node=node_order_info->nodes[node_order_info->current_node_number];
		}
		else
		{
			node=(struct FE_node *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_order_info_next_node.  Invalid argument");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /*get_FE_node_order_info_next_node  */

struct FE_node *get_FE_node_order_info_prev_node(
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 17 August 2000

DESCRIPTION :
Gets the <node_order_info> next node by incrementing the current_node_number,
and returning the new current node. If at the start of the array, return null.
==============================================================================*/
{
	struct FE_node *node;

	ENTER(get_FE_node_order_info_prev_node);
	if (node_order_info)
	{
		if ((node_order_info->number_of_nodes)&&
			(node_order_info->current_node_number>0))
		{
			node_order_info->current_node_number--;
			node=node_order_info->nodes[node_order_info->current_node_number];
		}
		else
		{
			node=(struct FE_node *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_node_order_info_prev_node.  Invalid argument");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /*get_FE_node_order_info_prev_node  */

int add_nodes_FE_node_order_info(int number_of_nodes_to_add,
	struct FE_node_order_info *node_order_info)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Reallocates space for more  nodes in a previously created
FE_node_order_info.  Use set_FE_node_order_info to set up the nodes.
==============================================================================*/
{
	int i,return_code,old_number_of_nodes;
	struct FE_node **nodes;

	ENTER(add_nodes_FE_node_order_info)
	return_code=0;
	if (node_order_info)
	{
		old_number_of_nodes=node_order_info->number_of_nodes;
		node_order_info->number_of_nodes += number_of_nodes_to_add;
		if (REALLOCATE(nodes,node_order_info->nodes,struct FE_node *,
			node_order_info->number_of_nodes))
		{
			node_order_info->nodes=nodes;
			/*set the new nodes to null*/
			for (i=old_number_of_nodes;i<node_order_info->number_of_nodes;i++)
			{
				node_order_info->nodes[i]=(struct FE_node *)NULL;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_order_info_add_node.  Out of memory ");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_order_info_add_node.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_order_info_add_node */

int fill_FE_node_order_info(struct FE_node *node,void *dummy)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Allocate space for and add an FE_node to previously created
FE_node_order_info (passed in dummy).
Called iteratively.
==============================================================================*/
{
	int return_code;
	struct FE_node **nodes;
	struct FE_node_order_info *node_order_info;

	ENTER(fill_FE_node_order_info)
	return_code=0;
	if (node&&dummy)
	{
		node_order_info=(struct FE_node_order_info *)dummy;
		node_order_info->number_of_nodes++;
		/* reallocate space for pointer to the node */
		if (REALLOCATE(nodes,node_order_info->nodes,struct FE_node *,
			node_order_info->number_of_nodes))
		{
			node_order_info->nodes=nodes;
			/* set the pointer to the node */
			node_order_info->nodes[node_order_info->number_of_nodes-1]=
				ACCESS(FE_node)(node);
			node_order_info->current_node_number=node_order_info->number_of_nodes-1;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fill_FE_node_order_info. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fill_FE_node_order_info */

struct FE_element_order_info *CREATE(FE_element_order_info)(
	int number_of_elements)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Allocate space for an array of pointers to elements of length number_of_elements,
set these to NULL, copy the number_of_elements.
==============================================================================*/
{
	int i;
	struct FE_element_order_info *element_order_info;

	ENTER(CREATE(FE_element_order_info));
	if (ALLOCATE(element_order_info,struct FE_element_order_info,1))
	{
		if (number_of_elements>0)
		{
			if (ALLOCATE(element_order_info->elements,struct FE_element *,
				number_of_elements))
			{
				element_order_info->number_of_elements = number_of_elements;
				for (i=0;i<number_of_elements;i++)
				{
					element_order_info->elements[i]=(struct FE_element *)NULL;
				}
				element_order_info->access_count=0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"CREATE(FE_element_order_info).  Could not allocate memory for element_field_info->elements");
				DEALLOCATE(element_order_info->elements);
				DEALLOCATE(element_order_info);
			}
		}
		else
		{
			element_order_info->number_of_elements=0;
			element_order_info->elements=(struct FE_element **)NULL;
			element_order_info->access_count=0;
		}
		element_order_info->current_element_number=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"CREATE(FE_element_order_info).  Could not allocate memory for element field info");
		DEALLOCATE(element_order_info);
	}
	LEAVE;

	return (element_order_info);
} /* CREATE(FE_element_order_info) */

int DESTROY(FE_element_order_info)(
	struct FE_element_order_info **element_order_info_address)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Frees them memory used by element_order_info.
==============================================================================*/
{
	int i,return_code;
	struct FE_element_order_info *element_order_info;
	struct FE_element *element;

	ENTER(DESTROY(FE_element_order_info));
	return_code=0;
	if ((element_order_info_address)&&
		(element_order_info= *element_order_info_address))
	{
		/* free the components */
		for (i=0;i<element_order_info->number_of_elements;i++)
		{
			element=element_order_info->elements[i];
			DEACCESS(FE_element)(&element);
		}
		DEALLOCATE(element_order_info->elements);
		DEALLOCATE(*element_order_info_address);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_element_order_info) */

DECLARE_OBJECT_FUNCTIONS(FE_element_order_info)

PROTOTYPE_COPY_OBJECT_FUNCTION(FE_element_order_info)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Makes an exacy copy of the FE_element_order_info
==============================================================================*/
{
	int return_code,i;

	ENTER(COPY(FE_element_order_info));
	return_code=0;
	/* check the arguments */
	if (source&&destination)
	{
		/* free any existing the destination things */
		if (destination->number_of_elements)
		{
			for (i=0;i<destination->number_of_elements;i++)
			{
				DEACCESS(FE_element)(&(destination->elements[i]));
			}
			DEALLOCATE(destination->elements);
			destination->number_of_elements=0;
		}
		if (ALLOCATE(destination->elements,struct FE_element*,source->number_of_elements))
		{
			/* copy the new */
			destination->number_of_elements=source->number_of_elements;
			for (i=0;i<destination->number_of_elements;i++)
			{
				destination->elements[i]=ACCESS(FE_element)(source->elements[i]);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"COPY(FE_element_order_info).  Out of memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"COPY(FE_element_order_info).  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* COPY(FE_element_order_info) */

int get_FE_element_order_info_number_of_elements(
	struct FE_element_order_info *element_order_info)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> number_of_elements
==============================================================================*/
{
	int number_of_elements;

	ENTER(get_FE_element_order_info_number_of_elements);
	if (element_order_info)
	{
		number_of_elements=element_order_info->number_of_elements;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_order_info_number_of_elements.  Invalid argument");
		number_of_elements=0;
	}
	LEAVE;

	return (number_of_elements);
} /* get_FE_element_order_info_number_of_elements */

struct FE_element *get_FE_element_order_info_element(
	struct FE_element_order_info *element_order_info,int element_number)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> element at the specified element_number
==============================================================================*/
{
	struct FE_element *element;

	ENTER(get_FE_element_order_info_element);
	if ((element_order_info)&&
		(element_number<=element_order_info->number_of_elements))
	{
		element=element_order_info->elements[element_number];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_order_info_element.  Invalid argument");
		element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
} /* get_FE_element_order_info_element */

int set_FE_element_order_info_element(
	struct FE_element_order_info *element_order_info,int element_number,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Sets the <element_order_info> element at the specified element_number.
Also sets the current_element_number to <the element_number>
==============================================================================*/
{
	int return_code

	ENTER(set_FE_element_order_info_element);
	return_code=0;
	if ((element_order_info)&&
		(element_number<=element_order_info->number_of_elements)&&(element))
	{
		REACCESS(FE_element)(&(element_order_info->elements[element_number]),element);
		element_order_info->current_element_number=element_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_element_order_info_element.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_element_group_order_element */

int get_FE_element_order_info_current_element_number(
	struct FE_element_order_info *element_order_info)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
gets the <element_order_info> <current_element_number>
==============================================================================*/
{
	int current_element_number;

	ENTER(get_FE_element_order_info_current_element_number);
	if (element_order_info&&(element_order_info->number_of_elements>0))
	{
		current_element_number=element_order_info->current_element_number;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_element_order_info_current_element_number.  Invalid argument");
		current_element_number=-1;
	}
	LEAVE;

	return (current_element_number);
} /* set_FE_element_order_info_current_element_number */

int set_FE_element_order_info_current_element_number(
	struct FE_element_order_info *element_order_info,int current_element_number)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Sets the <element_order_info> <current_element_number>
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_element_order_info_current_element_number);
	return_code=0;
	if ((element_order_info)&&(current_element_number>-1)&&
		(current_element_number<=element_order_info->number_of_elements))
	{
		element_order_info->current_element_number=current_element_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_element_order_info_current_element_number.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_element_order_info_current_element_number */

struct FE_element *get_FE_element_order_info_current_element(
	struct FE_element_order_info *element_order_info)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> element at the current_element_number
==============================================================================*/
{
	struct FE_element *element;

	ENTER(get_FE_element_order_info_current_element);
	if (element_order_info)
	{
		if (element_order_info->number_of_elements)
		{
			element=element_order_info->elements[element_order_info->current_element_number];
		}
		else
		{
			element=(struct FE_element *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_order_info_current_element.  Invalid argument");
		element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
} /*get_FE_element_order_info_current_element  */

struct FE_element *get_FE_element_order_info_next_element(
	struct FE_element_order_info *element_order_info)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> next element by incrementing the current_element_number,
and returning the new current element. If at the end of the array, return null.
==============================================================================*/
{
	struct FE_element *element;

	ENTER(get_FE_element_order_info_next_element);
	if (element_order_info)
	{
		if ((element_order_info->number_of_elements)&&
			(element_order_info->current_element_number<(element_order_info->number_of_elements-1)))
		{
			element_order_info->current_element_number++;
			element=element_order_info->elements[element_order_info->current_element_number];
		}
		else
		{
			element=(struct FE_element *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_order_info_next_element.  Invalid argument");
		element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
} /*get_FE_element_order_info_next_element  */

struct FE_element *get_FE_element_order_info_prev_element(
	struct FE_element_order_info *element_order_info)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Gets the <element_order_info> next element by incrementing the current_element_number,
and returning the new current element. If at the start of the array, return null.
==============================================================================*/
{
	struct FE_element *element;

	ENTER(get_FE_element_order_info_prev_element);
	if (element_order_info)
	{
		if ((element_order_info->number_of_elements)&&
			(element_order_info->current_element_number>0))
		{
			element_order_info->current_element_number--;
			element=element_order_info->elements[element_order_info->current_element_number];
		}
		else
		{
			element=(struct FE_element *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_element_order_info_prev_element.  Invalid argument");
		element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
} /*get_FE_element_order_info_prev_element  */

int add_elements_FE_element_order_info(int number_of_elements_to_add,
	struct FE_element_order_info *element_order_info)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Reallocates space for more  elements in a previously created
FE_element_order_info.  Use set_FE_element_order_info to set up the elements.
==============================================================================*/
{
	int i,return_code,old_number_of_elements;
	struct FE_element **elements;

	ENTER(add_elements_FE_element_order_info)
	return_code=0;
	if (element_order_info)
	{
		old_number_of_elements=element_order_info->number_of_elements;
		element_order_info->number_of_elements += number_of_elements_to_add;
		if (REALLOCATE(elements,element_order_info->elements,struct FE_element *,
			element_order_info->number_of_elements))
		{
			element_order_info->elements=elements;
			/*set the new elements to null*/
			for (i=old_number_of_elements;i<element_order_info->number_of_elements;
				i++)
			{
				element_order_info->elements[i]=(struct FE_element *)NULL;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_order_info_add_element.  Out of memory ");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_order_info_add_element.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_order_info_add_element */

int fill_FE_element_order_info(struct FE_element *element,void *dummy)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
Allocate space for and add an FE_element to previously created
FE_element_order_info (passed in dummy).
Called iteratively.
==============================================================================*/
{
	int return_code;
	struct FE_element **elements;
	struct FE_element_order_info *element_order_info;

	ENTER(fill_FE_element_order_info)
	return_code=0;
	if (element&&dummy)
	{
		element_order_info=(struct FE_element_order_info *)dummy;
		element_order_info->number_of_elements++;
		/* reallocate space for pointer to the element */
		if (REALLOCATE(elements,element_order_info->elements,struct FE_element *,
			element_order_info->number_of_elements))
		{
			element_order_info->elements=elements;
			/* set the pointer to the element */
			element_order_info->elements[element_order_info->number_of_elements-1]=
				ACCESS(FE_element)(element);
			element_order_info->current_element_number=element_order_info->number_of_elements-1;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fill_FE_element_order_info. Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fill_FE_element_order_info */

int FE_element_get_number_of_change_to_adjacent_element_permutations(
	struct FE_element *element, FE_value *xi, int face_number)
/*******************************************************************************
LAST MODIFIED : 8 June 2006

DESCRIPTION :
Returns the number of permutations known for the changing to the adjacent
element at face <face_number>.
==============================================================================*/
{
	int number_of_permutations;
	struct FE_element *face;

	ENTER(FE_element_get_number_of_change_to_adjacent_element_permutations);
	USE_PARAMETER(xi);
	FE_element_shape *element_shape = get_FE_element_shape(element);
	if ((element_shape) && (0 <= face_number) && (face_number < element_shape->number_of_faces))
	{
		number_of_permutations = 1;
		const int face_dimension = element_shape->dimension - 1;
		if (NULL != (face=(element->faces)[face_number]))
		{
			number_of_permutations = 1;
			switch (face_dimension)
			{
				case 1:
				{
					number_of_permutations = 2;
				} break;
				case 2:
				{
					FE_element_shape *face_shape = get_FE_element_shape(face);
					if (face_shape && (face_shape->type[0] == SIMPLEX_SHAPE)
						&& (face_shape->type[2] == SIMPLEX_SHAPE))
					{
						number_of_permutations = 6;
					}
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_get_number_of_change_to_adjacent_element_permutations.  "
			"Invalid argument(s).");
		number_of_permutations = 0;
	}
	LEAVE;

	return (number_of_permutations);
} /* FE_element_get_number_of_change_to_adjacent_element_permutations */

int FE_element_change_to_adjacent_element(struct FE_element **element_address,
	FE_value *xi, FE_value *increment, int *face_number, FE_value *xi_face,
	int permutation)
/*******************************************************************************
LAST MODIFIED : 8 June 2006

DESCRIPTION :
Steps into the adjacent element through face <face_number>, updating the
<element_address> location.
If <xi> is not NULL then the <xi_face> coordinates are converted to an xi
location in the new element.
If <increment> is not NULL then it is converted into an equvalent increment
in the new element.
If <fe_region> is not NULL then the function will restrict itself to elements
in that region.
<permutation> is used to resolve the possible rotation and flipping of the
local face xi coordinates between the two parents.
The shape mapping from parents are reused for all elements of the same shape
and do not take into account the relative orientation of the parents.
==============================================================================*/
{
	double dot_product;
	FE_value *face_to_element;
	int i,j,new_face_number,return_code;
	struct FE_element *element,*face,*new_element;

	ENTER(FE_element_change_to_adjacent_element);
	return_code=0;
	int dimension = 0;
	FE_element_shape *element_shape;
	if ((element_address) && (element = *element_address) &&
		(0 != (element_shape = get_FE_element_shape(element))) &&
		(0 < (dimension = element_shape->dimension)) &&
		(0<=*face_number)&&(*face_number<element_shape->number_of_faces))
	{
		FE_value temp_increment[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		FE_value face_normal[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		FE_value local_xi_face[MAXIMUM_ELEMENT_XI_DIMENSIONS - 1];
		FE_element_shape *face_shape;
		if (element->faces && (0 != (face = (element->faces)[*face_number])) &&
			(0 != (face_shape = get_FE_element_shape(face))))
		{
			/* find the other parent */
			new_element = (struct FE_element *)NULL;
			for (i = 0; i < face->number_of_parents; i++)
			{
				if (face->parents[i] != element)
				{
					new_element = face->parents[i];
					break;
				}
			}
			if (new_element)
			{
				new_face_number = FE_element_get_child_face_number(new_element, face);
				/* change xi and increment into element coordinates */
				FE_element_shape *new_element_shape = get_FE_element_shape(new_element);
				if ((new_element_shape) && (dimension == new_element_shape->dimension) && 
					(0 <= new_face_number))
				{
					return_code = 1;
					if (xi)
					{
						for (j = 0 ; j < dimension - 1 ; j++)
							local_xi_face[j] = xi_face[j];
						if (permutation > 0)
						{
							/* Try rotating the face_xi coordinates */
							/* Only implementing the cases required so far and
								enumerated by FE_element_change_get_number_of_permutations */
							switch (face_shape->dimension)
							{
								case 1:
								{
									if (face_shape->type[0] == LINE_SHAPE)
									{
										local_xi_face[0] = 1.0 - xi_face[0];
									}
								} break;
								case 2:
								{
									if ((face_shape->type[0] == SIMPLEX_SHAPE)
										&& (face_shape->type[2] == SIMPLEX_SHAPE))
									{
										switch (permutation)
										{
											case 1:
											{
												local_xi_face[0] = xi_face[1];
												local_xi_face[1] = 1.0 - xi_face[0] - xi_face[1];
											} break;
											case 2:
											{
												local_xi_face[0] = 1.0 - xi_face[0] - xi_face[1];
												local_xi_face[1] = xi_face[0];
											} break;
											case 3:
											{
												local_xi_face[0] = xi_face[1];
												local_xi_face[1] = xi_face[0];
											} break;
											case 4:
											{
												local_xi_face[0] = xi_face[0];
												local_xi_face[1] = 1.0 - xi_face[0] - xi_face[1];
											} break;
											case 5:
											{
												local_xi_face[0] = 1.0 - xi_face[0] - xi_face[1];
												local_xi_face[1] = xi_face[1];
											} break;
										}
									}
								} break;
							}
						}
						face_to_element=(new_element_shape->face_to_element)+
							(new_face_number*dimension*dimension);
						for (i=0;i<dimension;i++)
						{
							xi[i]= *face_to_element;
							face_to_element++;
							for (j=0;j<dimension-1;j++)
							{
								xi[i] += (*face_to_element)*local_xi_face[j];
								face_to_element++;
							}
						}
					}
					if (increment)
					{
						/* convert increment into face+normal coordinates */
						face_to_element=(element_shape->face_to_element)+
							(*face_number*dimension*dimension);
						return_code = face_calculate_xi_normal(element_shape,
							*face_number, face_normal);
						if (return_code)
						{
							for (i=0;i<dimension;i++)
							{
								temp_increment[i]=increment[i];
							}
							for (i=1;i<dimension;i++)
							{
								dot_product=(double)0;
								for (j=0;j<dimension;j++)
								{
									dot_product += (double)(temp_increment[j])*
										(double)(face_to_element[j*dimension+i]);
								}
								increment[i-1]=(FE_value)dot_product;
							}
							dot_product=(double)0;
							for (i=0;i<dimension;i++)
							{
								dot_product += (double)(temp_increment[i])*(double)(face_normal[i]);
							}
							increment[dimension-1]=(FE_value)dot_product;

							/* Convert this back to an increment in the new element */
							return_code = face_calculate_xi_normal(new_element_shape,
								new_face_number, face_normal);
							if (return_code)
							{
								for (i=0;i<dimension;i++)
								{
									temp_increment[i]=increment[i];
								}
								face_to_element=(new_element_shape->face_to_element)+
									(new_face_number*dimension*dimension);
								for (i=0;i<dimension;i++)
								{
									increment[i]=temp_increment[dimension-1]*face_normal[i];
									face_to_element++;
									for (j=0;j<dimension-1;j++)
									{
										increment[i] += (*face_to_element)*temp_increment[j];
										face_to_element++;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"FE_element_change_to_adjacent_element.  "
									"Unable to calculate face_normal for new element and face");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"FE_element_change_to_adjacent_element.  "
								"Unable to calculate face_normal for old element and face");
						}
					}
					if (return_code)
					{
						*element_address=new_element;
						*face_number=new_face_number;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"FE_element_change_to_adjacent_element.  "
						"Invalid new element or element shape");
				}
			}
			else
			{
				/* Valid attempt to change element but there is no adjacent element found */
				return_code = 1;
				*face_number = -1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"FE_element_change_to_adjacent_element.  "
				"Invalid face number %d or face_array", *face_number);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_change_to_adjacent_element.  "
			"Invalid argument(s).  %p %p %d %p %p %d %p",element_address,
			element_address ? *element_address : 0, dimension,
			xi,increment,*face_number,xi_face);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_change_to_adjacent_element */

int FE_element_xi_increment(struct FE_element **element_address,FE_value *xi,
	FE_value *increment)
/*******************************************************************************
LAST MODIFIED : 23 June 2004

DESCRIPTION :
Adds the <increment> to <xi>.  If this moves <xi> outside of the element, then
if an adjacent element is found then the element and xi location are changed
to this element and the stepping continues using the remaining increment.  If
no adjacent element is found then the <xi> will be on the element boundary and
the <increment> will contain the fraction of the increment not used.
==============================================================================*/
{
	FE_value fraction;
	int face_number,i,return_code;
	struct FE_element *element;

	ENTER(FE_element_xi_increment);
	return_code=0;
	int dimension = 0;
	FE_element_shape *element_shape;
	if (element_address && (element= *element_address) &&
		(0 != (element_shape = get_FE_element_shape(element))) &&
		(0 < (dimension = element_shape->dimension)) && xi && increment)
	{
		// working space:
		FE_value xi_face[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		FE_value local_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		FE_value local_increment[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		for (i = 0 ; i < dimension ; i++)
		{
			local_xi[i] = xi[i];
			local_increment[i] = increment[i];
		}
		fraction = 0.0;
		return_code = 1;
		/* Continue stepping until a step within an element is able to do
			all of the remaining increment or there is no adjacent element */
		while (return_code && (fraction != 1.0))
		{
			return_code = FE_element_shape_xi_increment(element_shape,
				local_xi, local_increment, &fraction, &face_number, xi_face);
			if (return_code && (fraction < 1.0))
			{
				return_code = FE_element_change_to_adjacent_element(&element,
					local_xi, local_increment, &face_number, xi_face, /*permutation*/0);
				element_shape = get_FE_element_shape(element);
				if (0 == element_shape)
					return_code = 0;
				if (face_number == -1)
				{
					/* No adjacent face could be found, so stop */
					fraction = 1.0;
					break;
				}
			}
		}
		if (return_code)
		{
			*element_address=element;
			for (i = 0 ; i < dimension ; i++)
			{
				xi[i] = local_xi[i];
				increment[i] = local_increment[i];
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_xi_increment.  "
			"Invalid argument(s).  %p %p %d %p %p",element_address,
			element_address ? *element_address : 0, dimension,
			xi,increment);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_xi_increment */

int FE_element_define_tensor_product_basis(struct FE_element *element,
	int dimension, enum FE_basis_type basis_type, struct FE_field *field)
{
	int *basis_type_array,i,j,k,number_of_components,number_of_nodes,
		number_of_nodes_per_xi,number_of_scale_factors,
		old_number_of_nodes,old_number_of_scale_factor_sets,return_code,
		*xi_basis_type;
	struct FE_basis *element_basis;
	struct FE_element_field_component *component,**components;
	struct Standard_node_to_element_map *standard_node_map;

	ENTER(FE_element_define_tensor_product_basis);
	return_code=1;
	FE_mesh *fe_mesh = FE_element_get_FE_mesh(element);
	if (element && (dimension > 0) && fe_mesh &&
		((LINEAR_LAGRANGE == basis_type) ||
		 (QUADRATIC_LAGRANGE == basis_type) ||
		 (CUBIC_LAGRANGE == basis_type) ||
		 (CUBIC_HERMITE == basis_type)) &&
		(dimension == fe_mesh->getDimension()) && field)
	{
		/* make basis */
		element_basis=(struct FE_basis *)NULL;
		if (return_code)
		{
			/* make default N-linear basis */
			if (ALLOCATE(basis_type_array,int,
					 1+(dimension*(1+dimension))/2))
			{
				xi_basis_type=basis_type_array;
				*xi_basis_type=dimension;
				xi_basis_type++;
				for (i=dimension;0<i;i--)
				{
					for (j=i;0<j;j--)
					{
						if (i==j)
						{
							*xi_basis_type=basis_type;
						}
						else
						{
							*xi_basis_type=NO_RELATION;
						}
						xi_basis_type++;
					}
				}
				if (NULL != (element_basis = make_FE_basis(basis_type_array,
					FE_region_get_basis_manager(fe_mesh->get_FE_region()))))
				{
					ACCESS(FE_basis)(element_basis);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_element_define_tensor_product_basis.  Error creating shape");
					return_code=0;
				}
				DEALLOCATE(basis_type_array);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_define_tensor_product_basis.  Not enough memory");
				return_code=0;
			}
		}
		if (return_code)
		{
			switch (basis_type)
			{
				case QUADRATIC_LAGRANGE:
				{
					number_of_nodes_per_xi = 3;
				} break;
				case CUBIC_LAGRANGE:
				{
					number_of_nodes_per_xi = 4;
				} break;
				default:
				{
					number_of_nodes_per_xi = 2;
				} break;
			}
			number_of_nodes=1;
			for (i=0;i<dimension;i++)
			{
				number_of_nodes *= number_of_nodes_per_xi;
			}
			int number_of_values_per_node = 1;
			if (CUBIC_HERMITE == basis_type)
			{
				for (i = 0; i < dimension; ++i)
					number_of_values_per_node *= 2;
			}
			number_of_scale_factors = number_of_nodes*number_of_values_per_node;
			if (get_FE_element_number_of_nodes(element,&old_number_of_nodes) &&
				get_FE_element_number_of_scale_factor_sets(element,
					&old_number_of_scale_factor_sets))
			{
				char *scale_factor_set_name = FE_basis_get_description_string(element_basis);
				cmzn_mesh_scale_factor_set *scale_factor_set = fe_mesh->find_scale_factor_set_by_name(scale_factor_set_name);
				if (!scale_factor_set)
				{
					scale_factor_set = fe_mesh->create_scale_factor_set();
					scale_factor_set->setName(scale_factor_set_name);
				}
				DEALLOCATE(scale_factor_set_name);

				number_of_nodes += old_number_of_nodes;
				if (old_number_of_scale_factor_sets)
				{
					/* Currently this cannot be increased so we must find it */
					i = 0;
					while ((i < old_number_of_scale_factor_sets) &&
						(get_FE_element_scale_factor_set_identifier_at_index(element, i) != scale_factor_set))
					{
						i++;
					}
					if (i == old_number_of_scale_factor_sets)
					{
						display_message(ERROR_MESSAGE,
							"FE_element_define_tensor_product_basis.  "
							"Currently unable to add to an existing list of scale factors sets");
						return_code = 0;
					}
				}
				else
				{
					set_FE_element_number_of_scale_factor_sets(
						element, /*number_of_scale_factor_sets*/1,
						/*scale_factor_set_identifiers*/&scale_factor_set,
						/*numbers_in_scale_factor_sets*/&number_of_scale_factors);
				}
				cmzn_mesh_scale_factor_set::deaccess(scale_factor_set);
				if (return_code)
				{
					if (set_FE_element_number_of_nodes(element,
							number_of_nodes))
					{
						number_of_components = get_FE_field_number_of_components(
							field);
						if (ALLOCATE(components,struct FE_element_field_component *,
								number_of_components))
						{
							for (i=0;i<number_of_components;i++)
							{
								components[i]=(struct FE_element_field_component *)NULL;
							}
							for (i=0;(i<number_of_components)&&return_code;i++)
							{
								if (NULL != (component=CREATE(FE_element_field_component)(
										 STANDARD_NODE_TO_ELEMENT_MAP,number_of_nodes,
										 element_basis,(FE_element_field_component_modify)NULL)))
								{
									for (j=0;j<number_of_nodes;j++)
									{
										standard_node_map = Standard_node_to_element_map_create(/*node_index*/j, number_of_values_per_node);
										if (!standard_node_map)
										{
											return_code = 0;
											break;
										}
										for (k = 0; k < number_of_values_per_node; ++k)
										{
											if (!(Standard_node_to_element_map_set_nodal_value_type(
													standard_node_map, k, static_cast<FE_nodal_value_type>(FE_NODAL_VALUE + k)) &&
												Standard_node_to_element_map_set_scale_factor_index(
													standard_node_map, k, j * number_of_values_per_node + k) &&
												/* set scale_factors to 1 */
												set_FE_element_scale_factor(element,
													/*scale_factor_number*/j * number_of_values_per_node + k, 1.0)))
											{
												return_code = 0;
												break;
											}
										}
										if (return_code)
										{
											return_code = FE_element_field_component_set_standard_node_map(
												component, /*node_number*/j, standard_node_map);
										}
										else
										{
											Standard_node_to_element_map_destroy(&standard_node_map);
											break;
										}
									}
								}
								else
								{
									return_code=0;
								}
								components[i]=component;
							}
							if (return_code)
							{
								if (!define_FE_field_at_element(element, field, components))
								{
									display_message(ERROR_MESSAGE,
										"FE_element_define_tensor_product_basis.  "
										"Could not define coordinate field at template_element");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"FE_element_define_tensor_product_basis.  "
									"Could not create components");
								return_code = 0;
							}
							for (i=0;i<number_of_components;i++)
							{
								DESTROY(FE_element_field_component)(&(components[i]));
							}
							DEALLOCATE(components);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"FE_element_define_tensor_product_basis.  "
							"Could not allocate components");
						return_code=0;
					}
				}
			}
		}
		/* deaccess basis and shape so at most used by template element */
		if (element_basis)
		{
			DEACCESS(FE_basis)(&element_basis);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_define_tensor_product_basis.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_define_tensor_product_basis */

int cmzn_node_set_identifier(cmzn_node_id node, int identifier)
{
	if (node && node->fields)
		return node->fields->fe_nodeset->change_FE_node_identifier(node, identifier);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_element_set_identifier(cmzn_element_id element, int identifier)
{
	if (element && element->fields)
		return element->fields->fe_mesh->change_FE_element_identifier(element, identifier);
	return CMZN_ERROR_ARGUMENT;
}
