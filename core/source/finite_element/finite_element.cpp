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

#include "opencmiss/zinc/status.h"
#include "general/cmiss_set.hpp"
#include "general/indexed_list_stl_private.hpp"
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
#include "general/enumerator_conversion.hpp"
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
	/*???DB.  Have something like enum FE_nodal_value_type ? */
	/*???DB.  Needs to be clarified for constant fields */
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

FULL_DECLARE_CHANGE_LOG_TYPES(FE_field);


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
	/* index into nodeset labels, maps to unique identifier */
	DsLabelIndex index;

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

	inline DsLabelIdentifier getIdentifier() const
	{
		if (this->fields)
			return this->fields->fe_nodeset->getNodeIdentifier(this->index);
		return DS_LABEL_IDENTIFIER_INVALID;
	}

	int getElementUsageCount()
	{
		if (this->fields)
			return this->fields->fe_nodeset->getElementUsageCount(this->index);
		return 0;
	}

	void incrementElementUsageCount()
	{
		if (this->fields)
			this->fields->fe_nodeset->incrementElementUsageCount(this->index);
	}

	void decrementElementUsageCount()
	{
		if (this->fields)
			this->fields->fe_nodeset->decrementElementUsageCount(this->index);
	}

}; /* struct FE_node */

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
	/* specify whether the standard basis arguments should be destroyed (element
		field has been inherited) or not be destroyed (element field is defined for
		the element and the basis arguments are being used) */
	char destroy_standard_basis_arguments;
	/* the number of field components */
	int number_of_components;
	/* the number of values for each component */
	int *component_number_of_values;
	/* the values_storage for each component if grid-based */
	const Value_storage **component_grid_values_storage;
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
 * Get definition of field on element, or higher dimension element it is a face
 * of, or a face of a face. If field is inherited, returns the xi coordinate
 * transformation to the higher dimensional element.
 * Can also force inheritance onto a specified face of the supplied element.
 * Function is recursive.
 *
 * @param element  The element we want to evaluate on.
 * @param inheritFaceNumber  If non-negative, inherit onto this face number
 * of element, as if the face element were supplied to this function.
 * @param topLevelElement  If supplied, forces field definition to be
 * obtained from it, otherwise field is undefined.
 * @param coordinateTransformation  If field is inherited from higher dimension
 * element, gives mapping of element coordinates to return element coordinates.
 * This is a matrix of dimension(return element) rows X dimension(element)+1
 * columns. This represents an affine transformation, b + A xi for calculating
 * the return element xi coordinates from those of element, where b is the
 * first column of the <coordinate_transformation> matrix. Caller must pass an
 * array of size sufficient for the expected dimensions, e.g.
 * MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS.
 * @return  The element the field is actually defined on, or 0 if not defined.
 * Return element either matches supplied element or is an ancestor of it.
 */
cmzn_element *FE_field::getOrInheritOnElement(cmzn_element *element,
	int inheritFaceNumber, cmzn_element *topLevelElement,
	FE_value *coordinateTransformation)
{
	const FE_mesh *mesh = element->getMesh();
	if (!mesh)
		return 0;
	const int dim = mesh->getDimension() - 1;
	// fast case of field directly defined on element:
	// check first component only
	// note this formerly only used topLevelElement for inheritance only
	// i.e. would return initial element if it had field directly defined on it
	const bool useDefinitionOnThisElement = ((!topLevelElement) || (element == topLevelElement))
		&& (0 != this->meshFieldData[dim])
		&& (this->meshFieldData[dim]->getComponentMeshfieldtemplate(0)->getElementEFTIndex(element->getIndex()) >= 0);
	if (useDefinitionOnThisElement && (inheritFaceNumber < 0))
		return element;

	FE_value parentCoordinateTransformation[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS];
	cmzn_element *fieldElement = 0;
	cmzn_element *parent = 0;
	int faceNumber = inheritFaceNumber;
	if (inheritFaceNumber >= 0)
	{
		parent = element;
		if (useDefinitionOnThisElement)
			fieldElement = element;
		else // inherit onto this element, then onto specified face below to handle two coordinate transformations
			fieldElement = this->getOrInheritOnElement(parent, /*inheritFaceNumber*/-1, topLevelElement, parentCoordinateTransformation);
	}
	else
	{
		const FE_mesh *parentMesh = mesh->getParentMesh();
		if (!parentMesh)
			return 0;
		// try to inherit field from any of the element's parents
		const DsLabelIndex *parents;
		const int parentsCount = mesh->getElementParents(element->getIndex(), parents);
		for (int p = 0; p < parentsCount; ++p)
		{
			parent = parentMesh->getElement(parents[p]);
			fieldElement = this->getOrInheritOnElement(parent, /*inheritFaceNumber*/-1, topLevelElement, parentCoordinateTransformation);
			if (fieldElement)
			{
				faceNumber = parentMesh->getElementFaceNumber(parents[p], element->getIndex());
				break;
			}
		}
	}
	if (!fieldElement)
		return 0;
	FE_element_shape *parentShape = parent->getElementShape();
	if (!parentShape)
	{
		display_message(ERROR_MESSAGE, "FE_field::getOrInheritOnElement.  Missing parent shape");
		return 0;
	}
	const int parentDimension = parentShape->dimension;
	const int fieldElementDimension = fieldElement->getDimension();

	const FE_value *faceToElement = (parentShape->face_to_element) +
		(faceNumber*parentDimension*parentDimension);
	if (fieldElementDimension > parentDimension)
	{
		// incorporate the face to element map in the coordinate transformation
		const FE_value *parentValue = parentCoordinateTransformation;
		FE_value *faceValue = coordinateTransformation;
		const int faceDimension = parentDimension - 1;
		// this had used DOUBLE_FOR_DOT_PRODUCT, but FE_value will be at least double precision now
		FE_value sum;
		const FE_value *faceToElementValue;
		for (int i = fieldElementDimension; i > 0; --i)
		{
			/* calculate b entry for this row */
			sum = *parentValue;
			++parentValue;
			faceToElementValue = faceToElement;
			for (int k = parentDimension; k > 0; --k)
			{
				sum += (*parentValue)*(*faceToElementValue);
				++parentValue;
				faceToElementValue += parentDimension;
			}
			*faceValue = sum;
			faceValue++;
			/* calculate A entries for this row */
			for (int j = faceDimension; j > 0; --j)
			{
				++faceToElement;
				faceToElementValue = faceToElement;
				parentValue -= parentDimension;
				sum = 0.0;
				for (int k = parentDimension; k > 0; --k)
				{
					sum += (*parentValue)*(*faceToElementValue);
					++parentValue;
					faceToElementValue += parentDimension;
				}
				*faceValue = sum;
				++faceValue;
			}
			faceToElement -= faceDimension;
		}
	}
	else
	{
		// use the face to element map as the transformation
		memcpy(coordinateTransformation, faceToElement, parentDimension*parentDimension*sizeof(FE_value));
	}
	return fieldElement;
}


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
					node_number=(nodes_in_element[i])->getIdentifier();
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
					element->getDimension(), element->getIdentifier());
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
														xi_dimension, embedding_element->getIdentifier());
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
				field->name, node->getIdentifier());
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

/**
 * The standard function for mapping global parameters to get the local element
 * parameters weighting the basis in the element field template.
 * Uses relative offsets into nodal values array in standard and general node to
 * element maps. Absolute offset for start of field component is obtained from
 * the node_field_component for the field at the node.
 * Does not check arguments as called internally.
 *
 * @param field  The field to get values for.
 * @param componentNumber  The component of the field to get values for, >= 0.
 * @param eft  Element field template describing parameter mapping and basis.
 * @param element  The element to get values for.
 * @param time  The time at which to get parameter values.
 * @param nodeset  The nodeset owning any node indexes mapped.
 * @param elementValues  Pre-allocated array big enough for the number of basis
 * functions used by the EFT basis. Note this is a reference and will be
 * reallocated if basis uses a blending function. Either way, caller is
 * required to deallocate.
 * @return  Number of values calculated or 0 if error.
 */
int global_to_element_map_values(FE_field *field, int componentNumber,
	const FE_element_field_template *eft, cmzn_element *element, FE_value time,
	const FE_nodeset *nodeset, FE_value*& elementValues)
{
	if (eft->getElementParameterMappingMode() != CMZN_ELEMENT_PARAMETER_MAPPING_MODE_NODE)
	{
		display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
			"Only implemented for node parameter, first found on field %s component %d at element %d.",
			field->name, componentNumber + 1, element->getIdentifier());
		return 0;
	}
	if (field->value_type != FE_VALUE_VALUE)
	{
		display_message(ERROR_MESSAGE, "global_to_element_map_values.  Field %s is not FE_value type, not implemented", field->name);
		return 0;
	}
	const FE_mesh *mesh = element->getMesh();
	const FE_mesh_element_field_template_data *meshEFTData = mesh->getElementfieldtemplateData(eft->getIndexInMesh());
	FE_basis *basis = eft->getBasis();

	const DsLabelIndex elementIndex = element->getIndex();
	const DsLabelIndex *nodeIndexes = meshEFTData->getElementNodeIndexes(elementIndex);
	if (!nodeIndexes)
	{
		display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
			"Missing local-to-global node map for field %s component %d at element %d.",
			field->name, componentNumber + 1, element->getIdentifier());
		return 0;
	}
	const FE_value *scaleFactors = 0;
	if (0 < eft->getNumberOfLocalScaleFactors())
	{
		scaleFactors = meshEFTData->getElementScaleFactors(elementIndex);
		if (!scaleFactors)
		{
			display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
				"Missing scale factors for field %s component %d at element %d.",
				field->name, componentNumber + 1, element->getIdentifier());
			return 0;
		}
	}

	const int basisFunctionCount = eft->getNumberOfFunctions();
	int lastLocalNodeIndex = -1;
	FE_node *node;
	// Need node_field_component to get absolute offset into nodal
	// values array. Also store node_field_info so we don't have to
	// get node_field_component each time if it is not changing
	FE_node_field_info *node_field_info = 0;
	const FE_node_field_component *node_field_component;
	FE_time_sequence *time_sequence = 0;
	int time_index_one, time_index_two;
	FE_value time_xi;
	int tt = 0; // total term, increments up to eft->totalTermCount
	int tts = 0; // total term scaling, increments up to eft->totalLocalScaleFactorIndexes
	for (int f = 0; f < basisFunctionCount; ++f)
	{
		FE_value termSum = 0.0;
		const int termCount = eft->termCounts[f];
		for (int t = 0; t < termCount; ++t)
		{
			const int localNodeIndex = eft->localNodeIndexes[tt];
			if (localNodeIndex != lastLocalNodeIndex)
			{
				const DsLabelIndex nodeIndex = nodeIndexes[localNodeIndex];
				node = nodeset->getNode(nodeIndex);
				if (!node)
				{
					display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
						"Missing node for field %s component %d in element %d, function %d term %d.",
						field->name, componentNumber + 1, element->getIdentifier(), f + 1, t + 1);
					return 0;
				}
				if (node_field_info != node->fields)
				{
					if (!node->fields)
					{
						display_message(ERROR_MESSAGE, "global_to_element_map_values.  Invalid node");
						return 0;
					}
					const FE_node_field *node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field, field)(field, node->fields->node_field_list);
					if (!((node_field) && (node_field->components)))
					{
						display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
							"Cannot evaluate field %s  component %d in element %d because it is not defined at node %d",
							field->name, componentNumber + 1, element->getIdentifier(), node->getIdentifier());
						return 0;
					}
					node_field_info = node->fields;
					node_field_component = node_field->components + componentNumber;
					if ((node_field->time_sequence != time_sequence) &&
						(time_sequence = node_field->time_sequence))
					{
						FE_time_sequence_get_interpolation_for_time(time_sequence,
							time, &time_index_one, &time_index_two, &time_xi);
					}
				}
				lastLocalNodeIndex = localNodeIndex;
			}
			// GRC should remove following conversion by eliminating FE_nodal_value_type
			const FE_nodal_value_type nodalValueType = cmzn_node_value_label_to_FE_nodal_value_type(eft->nodeValueLabels[tt]);
			const int nodeValueTypesCount = node_field_component->number_of_derivatives + 1;
			int i = 0;
			for (i = 0; i < nodeValueTypesCount; ++i)
			{
				if (node_field_component->nodal_value_types[i] == nodalValueType)
					break;
			}
			if (i >= nodeValueTypesCount)
			{
				display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
					"Parameter '%s' not found for field %s component %d at node %d, used from element %d",
					ENUMERATOR_STRING(FE_nodal_value_type)(nodalValueType), field->name, componentNumber,
					node->getIdentifier(), element->getIdentifier());
				return 0;
			}
			// GRC future: versions per derivative change
			const int version = eft->nodeVersions[tt];
			if (version > node_field_component->number_of_versions)
			{
				display_message(ERROR_MESSAGE, "global_to_element_map_values.  "
					"Parameter '%s' version %d is out of range (%d) for field %s component %d at node %d, used from element %d",
					ENUMERATOR_STRING(FE_nodal_value_type)(nodalValueType),
					version + 1, node_field_component->number_of_versions, field->name, componentNumber,
					node->getIdentifier(), element->getIdentifier());
				return 0;
			}
			const int valueIndex = version*nodeValueTypesCount + i;
			FE_value termValue;
			if (time_sequence)
			{
				// get address of field component parameters in node
				const FE_value *timeValues = *(reinterpret_cast<FE_value **>(node->values_storage + node_field_component->value) + valueIndex);
				termValue = (1.0 - time_xi)*timeValues[time_index_one] + time_xi*timeValues[time_index_two];
			}
			else
			{
				termValue = reinterpret_cast<FE_value *>(node->values_storage + node_field_component->value)[valueIndex];
			}
			if (scaleFactors)
			{
				const int termScaleFactorCount = eft->termScaleFactorCounts[tt];
				for (int s = 0; s < termScaleFactorCount; ++s)
				{
					termValue *= scaleFactors[eft->localScaleFactorIndexes[tts]];
					++tts;
				}
			}
			termSum += termValue;
			++tt;
		}
		elementValues[f] = termSum;
	}
	if (eft->getLegacyModifyThetaMode() != FE_BASIS_MODIFY_THETA_MODE_INVALID)
	{
		if (!FE_basis_modify_theta_in_xi1(eft->basis, eft->getLegacyModifyThetaMode(), elementValues))
		{
			display_message(ERROR_MESSAGE, "global_to_element_map_values.  Error modifying element values");
			return 0;
		}
	}
	const int blendedElementValuesCount = FE_basis_get_number_of_blended_functions(basis);
	if (blendedElementValuesCount > 0)
	{
		FE_value *blendedElementValues = FE_basis_get_blended_element_values(basis, elementValues);
		if (!blendedElementValues)
		{
			display_message(ERROR_MESSAGE,
				"global_to_element_map_values.  Could not allocate memory for blended values");
			return 0;
		}
		DEALLOCATE(elementValues);
		elementValues = blendedElementValues;
		return blendedElementValuesCount;
	}
	return basisFunctionCount;
}

/**
 * The standard function for calculating the nodes used for a field component.
 * Determines a vector of nodes contributing to each basis function.
 * Limitation: Only returns the first node contributing to each basis parameter
 * for general maps from multiple nodes.
 * Does not check arguments as called internally.
 *
 * @param field  The field to get values for.
 * @param componentNumber  The component of the field to get values for, >= 0.
 * @param eft  Element field template describing parameter mapping and basis.
 * @param element  The element to get values for.
 * @param nodeset  The nodeset owning any node indexes mapped.
 * @param basisNodeIndexes  On successful return, allocated to indexes of nodes.
 * Up to caller to deallocate.
 * @return  Number of nodes calculated or 0 if error.
 */
int global_to_element_map_nodes(FE_field *field, int componentNumber,
	const FE_element_field_template *eft, cmzn_element *element,
	DsLabelIndex *&basisNodeIndexes)
{
	if (eft->getElementParameterMappingMode() != CMZN_ELEMENT_PARAMETER_MAPPING_MODE_NODE)
	{
		display_message(ERROR_MESSAGE, "global_to_element_map_nodes.  "
			"Only implemented for node parameter, first found on field %s component %d at element %d.",
			field->name, componentNumber + 1, element->getIdentifier());
		return 0;
	}
	const FE_mesh *mesh = element->getMesh();
	const FE_mesh_element_field_template_data *meshEFTData = mesh->getElementfieldtemplateData(eft->getIndexInMesh());
	FE_basis *basis = eft->getBasis();

	const DsLabelIndex elementIndex = element->getIndex();
	const DsLabelIndex *nodeIndexes = meshEFTData->getElementNodeIndexes(elementIndex);
	if (!nodeIndexes)
	{
		display_message(ERROR_MESSAGE, "global_to_element_map_nodes.  "
			"Missing local-to-global node map for field %s component %d at element %d.",
			field->name, componentNumber + 1, element->getIdentifier());
		return 0;
	}
	const int basisFunctionCount = eft->getNumberOfFunctions();
	basisNodeIndexes = new DsLabelIndex[basisFunctionCount];
	if (!basisNodeIndexes)
		return 0;
	int tt = 0; // total term, increments up to eft->totalTermCount
	for (int f = 0; f < basisFunctionCount; ++f)
	{
		const int termCount = eft->termCounts[f];
		if (0 < termCount)
		{
			basisNodeIndexes[f] = nodeIndexes[eft->localNodeIndexes[tt]];
			tt += termCount;
		}
		else
		{
			basisNodeIndexes[f] = DS_LABEL_INDEX_INVALID; // no terms = 'zero' parameter
		}
	}
	return basisFunctionCount;
}

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
	if (node && node->fields && field && (0 <= component_number)
		&& (component_number < field->number_of_components) && (0 <= version))
	{
		if (NULL != (node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field, field)(
			field, node->fields->node_field_list)))
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
			field->number_of_components = 0;
			/* don't allocate component names until we have custom names */
			field->component_names = (char **)NULL;
			field->coordinate_system.type = NOT_APPLICABLE;
			field->number_of_values = 0;
			field->values_storage = (Value_storage *)NULL;
			field->value_type = UNKNOWN_VALUE;
			field->element_xi_host_mesh = 0;
			field->number_of_times = 0;
			field->time_value_type = UNKNOWN_VALUE;
			field->times = (Value_storage *)NULL;
			for (int d = 0; d < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++d)
				field->meshFieldData[d] = 0;
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
	int return_code;
	struct FE_field *field;

	ENTER(DESTROY(FE_field));
	if ((field_address)&&(field= *field_address))
	{
		if (0==field->access_count)
		{
			FE_mesh::deaccess(field->element_xi_host_mesh);
			for (int d = 0; d < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++d)
				delete field->meshFieldData[d];

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

			// free component names
			if (field->component_names)
			{
				for (int c = 0; c < field->number_of_components; ++c)
					DEALLOCATE(field->component_names[c]);
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
							display_message(INFORMATION_MESSAGE, " %" FE_VALUE_STRING,
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
{
	if (!(destination && destination->info && destination->info->fe_region
		&& source && source->info && source->info->fe_region))
	{
		display_message(ERROR_MESSAGE,
			"FE_field_copy_without_identifier.  Invalid argument(s)");
		return 0;
	}
	int return_code = 1;
	const bool externalMerge = destination->info->fe_region != source->info->fe_region;
	char **component_names=(char **)NULL;
	if (source->component_names)
	{
		ALLOCATE(component_names, char *, source->number_of_components);
		if (!component_names)
			return false;
		for (int c = 0; c < source->number_of_components; ++c)
		{
			if (source->component_names[c])
			{
				component_names[c] = duplicate_string(source->component_names[c]);
				if (!component_names[c])
				{
					for (int c2 = 0; c2 < c; ++c)
						DEALLOCATE(component_names[c2]);
					DEALLOCATE(component_names);
					return_code = 0;
					break;
				}
			}
			else
			{
				component_names[c] = 0;
			}
		}
	}
	Value_storage *values_storage = 0;
	Value_storage *times = 0;
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
	FE_field *indexer_field = 0;
	if (INDEXED_FE_FIELD == source->fe_field_type)
	{
		if (!source->indexer_field)
			return_code = 0;
		else
		{
			if (externalMerge)
			{
				indexer_field = FE_region_get_FE_field_from_name(destination->info->fe_region, source->indexer_field->name);
				if (!indexer_field)
					return_code = 0;
			}
			else
			{
				indexer_field = source->indexer_field;
			}
		}
	}
	const FE_mesh *element_xi_host_mesh = 0;
	if (ELEMENT_XI_VALUE == source->value_type)
	{
		if (!source->element_xi_host_mesh)
		{
			display_message(ERROR_MESSAGE,
				"FE_field_copy_without_identifier.  Source element:xi valued field %s does not have a host mesh", source->name);
			return_code = 0;
		}
		else if (externalMerge)
		{
			// find equivalent host mesh in destination FE_region
			// to be fixed in future when arbitrary meshes are allowed:
			element_xi_host_mesh = FE_region_find_FE_mesh_by_dimension(destination->info->fe_region, source->element_xi_host_mesh->getDimension());
			if (strcmp(element_xi_host_mesh->getName(), source->element_xi_host_mesh->getName()) != 0)
			{
				display_message(ERROR_MESSAGE,
					"FE_field_copy_without_identifier.  Cannot find destination host mesh named %s for merging 'element:xi' valued field %s. Needs to be implemented.",
					source->element_xi_host_mesh->getName(), source->name);
				return_code = 0;
			}
		}
		else
		{
			element_xi_host_mesh = source->element_xi_host_mesh;
		}
	}
	if (return_code)
	{
		// don't want to change info if merging to external region. In all other cases should be same info anyway
		if (!externalMerge)
			REACCESS(FE_field_info)(&(destination->info), source->info);
		if (destination->cm_field_type != source->cm_field_type)
		{
			display_message(WARNING_MESSAGE, "Changing field %s CM type from %s to %s",
				source->name, ENUMERATOR_STRING(CM_field_type)(destination->cm_field_type),
				ENUMERATOR_STRING(CM_field_type)(source->cm_field_type));
			destination->cm_field_type = source->cm_field_type;
		}
		destination->fe_field_type=source->fe_field_type;
		REACCESS(FE_field)(&(destination->indexer_field), indexer_field);
		destination->number_of_indexed_values=
			source->number_of_indexed_values;
		if (destination->component_names)
		{
			for (int i = 0; i < destination->number_of_components; i++)
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
		if (element_xi_host_mesh)
			element_xi_host_mesh->access();
		if (destination->element_xi_host_mesh)
			FE_mesh::deaccess(destination->element_xi_host_mesh);
		destination->element_xi_host_mesh = element_xi_host_mesh;
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
			"FE_field_copy_without_identifier.  Invalid source field or could not copy dynamic contents");
	}
	if (!return_code)
	{
		if (component_names)
		{
			for (int i=0;i<source->number_of_components;i++)
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
	return (return_code);
}

bool FE_fields_match_fundamental(struct FE_field *field1,
	struct FE_field *field2)
{
	if (!(field1 && field2))
	{
		display_message(ERROR_MESSAGE,
			"FE_fields_match_fundamental.  Missing field(s)");
		return false;
	}
	if (!((field1->value_type == field2->value_type)
		&& (field1->fe_field_type == field2->fe_field_type)
		&& (field1->number_of_components == field2->number_of_components)
		&& (0 != Coordinate_systems_match(&(field1->coordinate_system),
			&(field2->coordinate_system)))))
		return false;
	if (ELEMENT_XI_VALUE == field1->value_type)
	{
		if (!field1->element_xi_host_mesh)
		{
			display_message(ERROR_MESSAGE,
				"FE_fields_match_fundamental.  Source element xi field %s does not have a host mesh", field1->name);
			return false;
		}
		// This will need to be improved once multiple meshes or meshes from different regions are allowed:
		if (field1->element_xi_host_mesh->getDimension() != field2->element_xi_host_mesh->getDimension())
			return false;
	}
	return true;
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

bool FE_field_uses_non_linear_basis(struct FE_field *fe_field)
{
	if (fe_field && fe_field->info && fe_field->info->fe_region)
	{
		for (int d = 0; d < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++d)
			if ((fe_field->meshFieldData[d]) && fe_field->meshFieldData[d]->usesNonLinearBasis())
				return true;
	}
	return false;
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

void FE_field_clearMeshFieldData(struct FE_field *field, FE_mesh *mesh)
{
	if (field && mesh && (mesh->get_FE_region() == field->info->fe_region))
	{
		const int dim = mesh->getDimension() - 1;
		if (field->meshFieldData[dim])
		{
			delete field->meshFieldData[dim];
			field->meshFieldData[dim] = 0;
			field->info->fe_region->FE_field_change(field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
		}
	}
}

FE_mesh_field_data *FE_field_createMeshFieldData(struct FE_field *field,
	FE_mesh *mesh)
{
	// GRC check field type general, real/int only?
	if (field && mesh && (mesh->get_FE_region() == field->info->fe_region))
	{
		const int dim = mesh->getDimension() - 1;
		if (!field->meshFieldData[dim])
			field->meshFieldData[dim] = FE_mesh_field_data::create(field, mesh);
		return field->meshFieldData[dim];
	}
	return 0;
}

FE_mesh_field_data *FE_field_getMeshFieldData(struct FE_field *field,
	const FE_mesh *mesh)
{
	if (field && mesh && (mesh->get_FE_region() == field->info->fe_region))
		return field->meshFieldData[mesh->getDimension() - 1];
	return 0;
}

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

const FE_mesh *FE_field_get_element_xi_host_mesh(struct FE_field *field)
{
	if (field)
		return field->element_xi_host_mesh;
	return 0;
}

int FE_field_set_element_xi_host_mesh(struct FE_field *field,
	const FE_mesh *hostMesh)
{
	if (!((field) && (field->value_type == ELEMENT_XI_VALUE) && (hostMesh)
		&& (hostMesh->get_FE_region() == field->info->fe_region))) // Current limitation of same region can be removed later
	{
		display_message(ERROR_MESSAGE, "FE_field_set_element_xi_host_mesh.  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
	if (field->element_xi_host_mesh)
	{
		display_message(ERROR_MESSAGE, "FE_field_set_element_xi_host_mesh.  Host mesh is already set");
		return CMZN_ERROR_ALREADY_EXISTS;
	}
	field->element_xi_host_mesh = hostMesh->access();
	return CMZN_OK;
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
			number_of_xi_dimensions = (*element) ? (*element)->getDimension() : 0;
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
		number_of_xi_dimensions = element->getDimension();
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

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(cmzn_element_face_type)
{
	switch (enumerator_value)
	{
		case CMZN_ELEMENT_FACE_TYPE_INVALID:
			break;
		case CMZN_ELEMENT_FACE_TYPE_ALL:
			return "all";
			break;
		case CMZN_ELEMENT_FACE_TYPE_ANY_FACE:
			return "any_face";
			break;
		case CMZN_ELEMENT_FACE_TYPE_NO_FACE:
			return "no_face";
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
	}
	return 0;
}

class cmzn_element_face_type_conversion
{
public:
	static const char *to_string(enum cmzn_element_face_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
		case CMZN_ELEMENT_FACE_TYPE_INVALID:
			break;
		case CMZN_ELEMENT_FACE_TYPE_ALL:
			enum_string = "ALL";
			break;
		case CMZN_ELEMENT_FACE_TYPE_ANY_FACE:
			enum_string = "ANY_FACE";
			break;
		case CMZN_ELEMENT_FACE_TYPE_NO_FACE:
			enum_string = "NO_FACE";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI1_0:
			enum_string = "XI1_0";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI1_1:
			enum_string = "XI1_1";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI2_0:
			enum_string = "XI2_0";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI2_1:
			enum_string = "XI2_1";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI3_0:
			enum_string = "XI3_0";
			break;
		case CMZN_ELEMENT_FACE_TYPE_XI3_1:
			enum_string = "XI3_1";
			break;
		}
		return enum_string;
	}
};

enum cmzn_element_face_type cmzn_element_face_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum cmzn_element_face_type,
		cmzn_element_face_type_conversion>(string);
}

char *cmzn_element_face_type_enum_to_string(enum cmzn_element_face_type type)
{
	const char *string = cmzn_element_face_type_conversion::to_string(type);
	return (string ? duplicate_string(string) : 0);
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

class cmzn_element_point_sampling_mode_conversion
{
public:
	static const char *to_string(enum cmzn_element_point_sampling_mode mode)
	{
		const char *enum_string = 0;
		switch (mode)
		{
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CENTRES:
			enum_string = "CELL_CENTRES";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS:
			enum_string = "CELL_CORNERS";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_POISSON:
			enum_string = "CELL_POISSON";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION:
			enum_string = "SET_LOCATION";
			break;
		case CMZN_ELEMENT_POINT_SAMPLING_MODE_GAUSSIAN_QUADRATURE:
			enum_string = "GAUSSIAN_QUADRATURE";
			break;
		default:
			break;
		}
		return enum_string;
	}
};

enum cmzn_element_point_sampling_mode cmzn_element_point_sampling_mode_enum_from_string(
	const char *string)
{
	return string_to_enum<enum cmzn_element_point_sampling_mode,
		cmzn_element_point_sampling_mode_conversion>(string);
}

char *cmzn_element_point_sampling_mode_enum_to_string(enum cmzn_element_point_sampling_mode mode)
{
	const char *string = cmzn_element_point_sampling_mode_conversion::to_string(mode);
	return (string ? duplicate_string(string) : 0);
}

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

class cmzn_field_domain_type_conversion
{
public:
	static const char *to_string(enum cmzn_field_domain_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
		case CMZN_FIELD_DOMAIN_TYPE_INVALID:
			break;
		case CMZN_FIELD_DOMAIN_TYPE_POINT:
			enum_string = "POINT";
			break;
		case CMZN_FIELD_DOMAIN_TYPE_NODES:
			enum_string = "NODES";
			break;
		case CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS:
			enum_string = "DATAPOINTS";
			break;
		case CMZN_FIELD_DOMAIN_TYPE_MESH1D:
			enum_string = "MESH1D";
			break;
		case CMZN_FIELD_DOMAIN_TYPE_MESH2D:
			enum_string = "MESH2D";
			break;
		case CMZN_FIELD_DOMAIN_TYPE_MESH3D:
			enum_string = "MESH3D";
			break;
		case CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION:
			enum_string = "MESH_HIGHEST_DIMENSION";
			break;
		}
		return enum_string;
	}
};

enum cmzn_field_domain_type cmzn_field_domain_type_enum_from_string(
	const char *string)
{
	return string_to_enum_bitshift<enum cmzn_field_domain_type,
		cmzn_field_domain_type_conversion>(string);
}

char *cmzn_field_domain_type_enum_to_string(
	enum cmzn_field_domain_type type)
{
	const char *string = cmzn_field_domain_type_conversion::to_string(type);
	return (string ? duplicate_string(string) : 0);
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
		// start at 1 so can't undefine FE_NODAL_VALUE
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

/**
 * Creates and returns a node with the specified <cm_node_identifier>.
 * If <fe_nodeset> is supplied a blank node with the given identifier but no
 * fields is returned. If <template_node> is supplied, a copy of it,
 * including all fields and values but with the new identifier, is returned.
 * Exactly one of <fe_nodeset> or <template_node> must be supplied and either
 * directly or indirectly sets the owning nodeset for the new node.
 * Note that <cm_node_identifier> must be non-negative.
 */
struct FE_node *CREATE(FE_node)()
{
	struct FE_node *node;
	if (ALLOCATE(node, struct FE_node, 1))
	{
		// not a global node until nodeset gives a non-negative index:
		node->index = DS_LABEL_INDEX_INVALID;
		node->access_count = 1;
		node->fields = (struct FE_node_field_info *)NULL;
		node->values_storage = (Value_storage *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE, "CREATE(FE_node).  Could not allocate memory for node");
	}
	return (node);
}

/**
 * Frees the memory for the node, sets <*node_address> to NULL.
 */
int DESTROY(FE_node)(struct FE_node **node_address)
{
	int return_code;
	struct FE_node *node;
	if ((node_address) && (node = *node_address))
	{
		if (0 == node->access_count)
		{
			// all nodes should be either templates with invalid index,
			// or have been invalidated by mesh prior to being destroyed
			if (DS_LABEL_IDENTIFIER_INVALID == node->index)
			{
				if (node->fields) // for template nodes only
					FE_node_invalidate(node);
				/* free the memory associated with the element */
				DEALLOCATE(*node_address);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"DESTROY(FE_node).  node has not been invalidated. Index = %d", node->index);
				*node_address = (struct FE_node *)NULL;
				return_code = 0;
			}
			/* free the memory associated with the node */
			DEALLOCATE(*node_address);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_node).  Node has non-zero access count %d",
				node->access_count);
			*node_address = (struct FE_node *)NULL;
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

struct FE_node *create_template_FE_node(FE_node_field_info *node_field_info)
{
	if (!node_field_info)
	{
		display_message(ERROR_MESSAGE, "create_template_FE_node.  Invalid argument");
		return 0;
	}
	struct FE_node *template_node = CREATE(FE_node)();
	if (template_node)
		template_node->fields = ACCESS(FE_node_field_info)(node_field_info);
	return template_node;
}

struct FE_node *create_FE_node_from_template(DsLabelIndex index, struct FE_node *template_node)
{
	// Assumes DS_LABEL_INDEX_INVALID == -1
	if ((index < DS_LABEL_INDEX_INVALID) || (0 == template_node))
	{
		display_message(ERROR_MESSAGE, "create_FE_node_from_template.  Invalid argument(s)");
		return 0;
	}
	struct FE_node *node = CREATE(FE_node)();
	if (node)
	{
		bool success = true;
		node->index = index;
		if (!(node->fields = ACCESS(FE_node_field_info)(template_node->fields)))
		{
			display_message(ERROR_MESSAGE, "create_FE_node_from_template.  Could not set field info from template node");
			success = false;
		}
		if (template_node->values_storage)
		{
			if (!allocate_and_copy_FE_node_values_storage(template_node,
				&node->values_storage))
			{
				display_message(ERROR_MESSAGE,
					"create_FE_node_from_template.  Could not copy values from template node");
				/* values_storage may be corrupt, so clear it */
				node->values_storage = (Value_storage *)NULL;
				success = false;
			}
		}
		if (!success)
			DEACCESS(FE_node)(&node);
	}
	return node;
}

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
		node->index = DS_LABEL_INDEX_INVALID;
	}
}

DECLARE_OBJECT_FUNCTIONS(FE_node)

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_node)
/*****************************************************************************
LAST MODIFIED : 5 November 1997

DESCRIPTION :
Returns the node identifier as a string.
Up to the calling routine to deallocate the returned char string!
============================================================================*/
{
	char temp_string[20];
	int return_code;

	ENTER(GET_NAME(FE_node));
	if (object&&name_ptr)
	{
		sprintf(temp_string,"%i",object->getIdentifier());
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
		/* check if the field is defined at the node */
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
					return_code = CMZN_OK;
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
						if (0 == new_node_field_info->values_storage_size)
						{
							DEALLOCATE(node->values_storage); // avoids warning about zero size
						}
						else if (REALLOCATE(values_storage,node->values_storage,Value_storage,
							new_node_field_info->values_storage_size))
						{
							node->values_storage=values_storage;
						}
						else
						{
							display_message(ERROR_MESSAGE, "undefine_FE_field_at_node.  Reallocate failed");
							return_code = CMZN_ERROR_MEMORY;
						}
					}
					DEACCESS(FE_node_field_info)(&(node->fields));
					node->fields = new_node_field_info;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"undefine_FE_field_at_node.  Could not create node field info");
					return_code= CMZN_ERROR_GENERAL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"undefine_FE_field_at_node.  Could not copy node field list");
				return_code=CMZN_ERROR_GENERAL;
			}
			DESTROY(LIST(FE_node_field))(&exclusion_data.node_field_list);
		}
		else
		{
			//display_message(WARNING_MESSAGE,
			//	"undefine_FE_field_at_node.  Field %s is not defined at node %d",
			//	field->name,node->getIdentifier());
			return_code = CMZN_ERROR_NOT_FOUND;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"undefine_FE_field_at_node.  Invalid argument(s)");
		return_code=CMZN_ERROR_ARGUMENT;
	}
	LEAVE;

	return (return_code);
}

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

int FE_node_get_element_usage_count(struct FE_node *node)
{
	if (node)
		return node->getElementUsageCount();
	return 0;
}

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
					component_number,version,type,&element,xi) && element)
				{
					dimension = element->getDimension();
					error=0;
					if (dimension == 1)
						append_string(string,"L",&error);
					else if (dimension == 2)
						append_string(string,"F",&error);
					else
						append_string(string,"E",&error);
					sprintf(temp_string," %d",element->getIdentifier());
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
						field->name,field->indexer_field->name,node->getIdentifier());
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
	// now require host mesh to be set; legacy inputs must ensure set with first element
	if (node && field && (field->value_type == ELEMENT_XI_VALUE) && (field->element_xi_host_mesh) &&
		(0 <= component_number) && (component_number < field->number_of_components) &&
		(0 <= version) && ((!element) || ((0 < (dimension = element->getDimension())) && xi)))
	{
		if (element && (!field->element_xi_host_mesh->containsElement(element)))
		{
			display_message(ERROR_MESSAGE, "set_FE_nodal_element_xi_value.  %d-D element %d is not from host mest %s for field %s ",
				dimension, element->getIdentifier(), field->element_xi_host_mesh->getName(), field->name);
			return 0;
		}
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
			Multi_range_is_value_in_range(multi_range,node->getIdentifier());
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
			!Multi_range_is_value_in_range(multi_range,node->getIdentifier());
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
			while (0 != (node = node_iter->nextNode()))
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

bool FE_node_can_merge(struct FE_node *targetNode, struct FE_node *sourceNode)
{
	if (targetNode && targetNode->fields && sourceNode && sourceNode->fields)
	{
		if (FOR_EACH_OBJECT_IN_LIST(FE_node_field)(
			FE_node_field_can_be_merged,
			(void *)targetNode->fields->node_field_list,
			sourceNode->fields->node_field_list))
		{
			return true;
		}
	}
	return false;
}

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
						field->name,field->indexer_field->name,node->getIdentifier());
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
				field->name,node->getIdentifier());
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
	USE_PARAMETER(time);
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
					field->name,node->getIdentifier());
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
			"set_FE_nodal_field_FE_value_values.  Field %s is not define at node %d", field->name, node->getIdentifier());
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
			field->name, node->getIdentifier());
		return CMZN_ERROR_NOT_FOUND;
	}
	if (node_field->time_sequence)
	{
		display_message(ERROR_MESSAGE, "FE_node_assign_FE_value_parameters_sparse.  Field %s at node %d is time-varying; case is not implemented",
			field->name, node->getIdentifier());
		return CMZN_ERROR_NOT_IMPLEMENTED;
	}
	int numberAssigned = 0;
	for (int c = 0; c < componentsSize; ++c)
	{
		FE_node_field_component *component = node_field->components + c;
		if (!component->nodal_value_types)
		{
			display_message(ERROR_MESSAGE, "FE_node_assign_FE_value_parameters_sparse.  Field %s at node %d has no nodal value types",
				field->name, node->getIdentifier());
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
			field->name, node->getIdentifier());
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
					field->name,node->getIdentifier());
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
				"Field %s not defined at node %d",field->name,node->getIdentifier());
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
				"Field %s not defined at node %d",field->name,node->getIdentifier());
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
				"Field %s not defined at node %d",field->name,node->getIdentifier());
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
				"Field %s not defined at node %d",field->name,node->getIdentifier());
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
{
	if (node)
		return node->getIdentifier();
	return DS_LABEL_IDENTIFIER_INVALID;
}

DsLabelIndex get_FE_node_index(struct FE_node *node)
{
	if (node)
		return node->index;
	return DS_LABEL_INDEX_INVALID;
}

void set_FE_node_index(struct FE_node *node, DsLabelIndex index)
{
	if (node)
		node->index = index;
}

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

const FE_node_field *FE_node_get_FE_node_field(struct FE_node *node,
	struct FE_field *field)
{
	return FIND_BY_IDENTIFIER_IN_LIST(FE_node_field, field)(
		field, node->fields->node_field_list);
	return 0;
}

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
		display_message(INFORMATION_MESSAGE,"node : %d\n",node->getIdentifier());
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

cmzn_nodeiterator_id cmzn_nodeiterator_access(cmzn_nodeiterator_id node_iterator)
{
	if (node_iterator)
		return cmzn::Access(node_iterator);
	return 0;
}

int cmzn_nodeiterator_destroy(cmzn_nodeiterator_id *node_iterator_address)
{
	if (node_iterator_address)
	{
		cmzn::Deaccess(*node_iterator_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_node_id cmzn_nodeiterator_next(cmzn_nodeiterator_id node_iterator)
{
	if (node_iterator)
	{
		cmzn_node *node = node_iterator->nextNode();
		if (node)
			node->access();
		return node;
	}
	return 0;
}

cmzn_node_id cmzn_nodeiterator_next_non_access(cmzn_nodeiterator_id node_iterator)
{
	if (node_iterator)
		return node_iterator->nextNode();
	return 0;
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

int FE_element_shape_get_number_of_faces(const FE_element_shape *element_shape)
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
	const FE_element_shape *shape,int face_number, struct FE_region *fe_region)
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

PROTOTYPE_ACCESS_OBJECT_FUNCTION(FE_element)
{
	if (object)
		return object->access();
	return 0;
}

PROTOTYPE_DEACCESS_OBJECT_FUNCTION(FE_element)
{
	if (object_address)
		return cmzn_element::deaccess(*object_address);
	return CMZN_ERROR_ARGUMENT;
}

PROTOTYPE_REACCESS_OBJECT_FUNCTION(FE_element)
{
	if (object_address)
		cmzn_element::reaccess(*object_address, new_object);
	return CMZN_ERROR_ARGUMENT;
}

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

int adjacent_FE_element(struct FE_element *element,
	int face_number, int *number_of_adjacent_elements,
	struct FE_element ***adjacent_elements)
{
	int return_code = CMZN_OK;
	FE_mesh *mesh;
	if (element && (mesh = element->getMesh()))
	{
		int j = 0;
		FE_mesh *faceMesh = mesh->getFaceMesh();
		const DsLabelIndex elementIndex = element->getIndex();
		DsLabelIndex faceIndex;
		if ((faceMesh) && (0 <= (faceIndex = mesh->getElementFace(elementIndex, face_number))))
		{
			const DsLabelIndex *parents;
			const int parentsCount = faceMesh->getElementParents(faceIndex, parents);
			if (ALLOCATE(*adjacent_elements, struct FE_element *, parentsCount))
			{
				for (int p = 0; p < parentsCount; ++p)
					if (parents[p] != elementIndex)
					{
						(*adjacent_elements)[j] = mesh->getElement(parents[p]);
						++j;
					}
			}
			else
			{
				display_message(ERROR_MESSAGE, "adjacent_FE_element.  Unable to allocate array");
				return_code = CMZN_ERROR_MEMORY;
			}
		}
		*number_of_adjacent_elements = j;
	}
	else
	{
		display_message(ERROR_MESSAGE, "adjacent_FE_element.  Invalid argument(s)");
		return_code = CMZN_ERROR_ARGUMENT;
	}
	return return_code;
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
		element_field_values->destroy_standard_basis_arguments = 0;
		element_field_values->number_of_components = 0;
		element_field_values->component_number_of_values = (int *)NULL;
		element_field_values->component_grid_values_storage =
			(const Value_storage **)NULL;
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

int calculate_FE_element_field_values(cmzn_element *element,
	struct FE_field *field, FE_value time, char calculate_derivatives,
	struct FE_element_field_values *element_field_values,
	cmzn_element *topLevelElement)
{
	FE_value *blending_matrix,
		*derivative_value,*inherited_value,*inherited_values,scalar,
		*second_derivative_value,*transformation,*value,**values_address;
	int cn,**component_number_in_xi,
		*component_base_grid_offset, *element_value_offsets, i,
		j,k,grid_maximum_number_of_values, maximum_number_of_values,
		number_of_grid_based_components,
		number_of_inherited_values,number_of_polygon_verticies,number_of_values,
		*number_of_values_address,offset,order,*orders,polygon_offset,power,
		row_size,**standard_basis_arguments_address;
	Standard_basis_function **standard_basis_address;
	// this had used DOUBLE_FOR_DOT_PRODUCT, but FE_value will be at least double precision now
	FE_value sum;

	if (!((element) && (field) && (element_field_values)))
	{
		display_message(ERROR_MESSAGE, "calculate_FE_element_field_values.  Invalid argument(s)");
		return 0;
	}
	FE_value coordinate_transformation[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS];
	cmzn_element *field_element = field->getOrInheritOnElement(element,
		/*inherit_face_number*/-1, topLevelElement, coordinate_transformation);
	if (!field_element)
	{
		display_message(ERROR_MESSAGE,
			"calculate_FE_element_field_values.  Field %s not defined on %d-D element %d",
			field->name, element->getDimension(), element->getIdentifier());
		return 0;
	}
	const FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "calculate_FE_element_field_values.  Invalid element");
		return 0;
	}
	const FE_nodeset *nodeset = mesh->getNodeset();
	if (!nodeset)
	{
		display_message(ERROR_MESSAGE, "calculate_FE_element_field_values.  No nodeset, invalid mesh");
		return 0;
	}

	int return_code = 1;
	const int element_dimension = element->getDimension();
	const int field_element_dimension = field_element->getDimension();
	const int number_of_components = field->number_of_components;
	switch (field->fe_field_type)
	{
		case CONSTANT_FE_FIELD:
		{
			/* constant fields do not use the values except to remember the
					element and field they are for */
			element_field_values->field=ACCESS(FE_field)(field);
			element_field_values->element=ACCESS(FE_element)(element);
			/* store field_element since we are now able to suggest through the
					topLevelElement clue which one we get. Must compare element
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
				(const Value_storage **)NULL;
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
				time,calculate_derivatives,element_field_values,topLevelElement))
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
				element_field_values->field = field->access();
				element_field_values->element = element->access();
				element_field_values->field_element = field_element->access();
				element_field_values->time_dependent =
					FE_field_has_multiple_times(element_field_values->field);
				element_field_values->time = time;
				element_field_values->derivatives_calculated=calculate_derivatives;
				if (field_element_dimension > element_dimension)
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
					(const Value_storage **)NULL;
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

				const FE_mesh_field_data *meshFieldData = field->meshFieldData[field_element_dimension - 1];
				FE_basis *previous_basis = 0;
				return_code=1;
				number_of_grid_based_components = 0;
				int **component_grid_offset_in_xi = 0;
				const DsLabelIndex elementIndex = element->getIndex();
				for (int component_number = 0; return_code && (component_number < number_of_components); ++component_number)
				{
					const FE_element_field_template *componentEFT = meshFieldData->getComponentMeshfieldtemplate(component_number)->getElementfieldtemplate(elementIndex);
					if (componentEFT->getElementParameterMappingMode() == CMZN_ELEMENT_PARAMETER_MAPPING_MODE_ELEMENT)
					{
						const int *top_level_component_number_in_xi = componentEFT->getLegacyGridNumberInXi();
						if (!top_level_component_number_in_xi)
						{
							display_message(ERROR_MESSAGE,
								"calculate_FE_element_field_values.  Element parameter mapping only implemented for legacy grid");
							return_code = 0;
							break;
						}
						++number_of_grid_based_components;
						if (number_of_grid_based_components == number_of_components)
						{
							element_field_values->derivatives_calculated=1;
						}
						/* one-off allocation of arrays only needed for grid-based components */
						if (NULL == element_field_values->component_grid_values_storage)
						{
							const Value_storage **component_grid_values_storage;
							ALLOCATE(component_grid_values_storage, const Value_storage *, number_of_components);
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
								break;
							}
						}
						// GRC risky to cache pointers into per-element data
						FE_mesh_field_data::ComponentBase *componentBase = meshFieldData->getComponentBase(component_number);
						const int valuesCount = componentEFT->getNumberOfElementDOFs();
						// following could be done as a virtual function
						switch (field->value_type)
						{
						case FE_VALUE_VALUE:
							{
								auto component = static_cast<FE_mesh_field_data::Component<FE_value>*>(componentBase);
								element_field_values->component_grid_values_storage[component_number] =
									reinterpret_cast<const Value_storage*>(component->getElementValues(elementIndex, valuesCount));
							} break;
						case INT_VALUE:
							{
								auto component = static_cast<FE_mesh_field_data::Component<int>*>(componentBase);
								element_field_values->component_grid_values_storage[component_number] =
									reinterpret_cast<const Value_storage*>(component->getElementValues(elementIndex, valuesCount));
							} break;
						default:
							{
								display_message(ERROR_MESSAGE,
									"calculate_FE_element_field_values.  Invalid value type for grid field");
								return_code = 0;
							} break;
						}
						ALLOCATE(component_number_in_xi[component_number], int, element_dimension);
						ALLOCATE(component_grid_offset_in_xi[component_number], int, element_dimension);
						if ((NULL != component_number_in_xi[component_number]) &&
							(NULL != component_grid_offset_in_xi[component_number]))
						{
							element_field_values->component_base_grid_offset[component_number] = 0;
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
								break;
							}
						}
						else
						{
							return_code = 0;
							break;
						}
					}
					else /* not grid-based */
					{
						const int basisFunctionCount = componentEFT->getNumberOfFunctions();
						ALLOCATE(*values_address, FE_value, basisFunctionCount);
						if (!(*values_address))
						{
							return_code = 0;
							break;
						}
						/* calculate element values for the element field component */
						if (0 < (*number_of_values_address = global_to_element_map_values(field, component_number,
							componentEFT, element, time, nodeset, *values_address)))
						{
							if (previous_basis == componentEFT->getBasis())
							{
								*standard_basis_address= *(standard_basis_address-1);
								*standard_basis_arguments_address=
									*(standard_basis_arguments_address-1);
							}
							else
							{
								previous_basis = componentEFT->getBasis();
								if (blending_matrix)
								{
									/* SAB We don't want to keep the old one */
									DEALLOCATE(blending_matrix);
									blending_matrix=NULL;
								}
								*standard_basis_address = FE_basis_get_standard_basis_function(previous_basis);
								if (field_element_dimension > element_dimension)
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
								if (field_element == element)
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
												sum += (*transformation)*(*value);
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
						}
					}
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
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"calculate_FE_element_field_values.  Unknown field type");
			return_code=0;
		} break;
	} /* switch (field->fe_field_type) */
	return (return_code);
}

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
	if (!((element) && (number_of_element_field_nodes_address) &&
		(element_field_nodes_array_address)))
	{
		display_message(ERROR_MESSAGE, "calculate_FE_element_field_nodes.  Invalid argument(s)");
		return 0;
	}
	if (!field)
	{
		field = FE_element_get_default_coordinate_field(top_level_element ? top_level_element : element);
		if (!field)
		{
			display_message(ERROR_MESSAGE, "calculate_FE_element_field_nodes.  No default coordinate field");
			return 0;
		}
	}
	// retrieve the field element from which this element inherits the field
	// and calculate the affine transformation from the element xi coordinates
	// to the xi coordinates for the field element */
	FE_value coordinate_transformation[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS];
	cmzn_element *field_element = field->getOrInheritOnElement(element,
		face_number, top_level_element, coordinate_transformation);
	if (!field_element)
	{
		if (field)
		{
			display_message(ERROR_MESSAGE,
				"calculate_FE_element_field_nodes.  %s not defined for %d-D element %d",
				field->name, element->getDimension(), element->getIdentifier());
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calculate_FE_element_field_nodes.  No coordinate fields defined for %d-D element %d",
				element->getDimension(), element->getIdentifier());
		}
		return 0;
	}
	const FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "calculate_FE_element_field_nodes.  Invalid element");
		return 0;
	}
	const FE_nodeset *nodeset = mesh->getNodeset();
	if (!nodeset)
	{
		display_message(ERROR_MESSAGE, "calculate_FE_element_field_nodes.  No nodeset, invalid mesh");
		return 0;
	}

	int return_code = 1;
	FE_value *blending_matrix, *combined_blending_matrix, *transformation;
	int add, i, *inherited_basis_arguments, j, k,
		number_of_inherited_values, number_of_blended_values;

	const FE_mesh_field_data *meshFieldData = field->meshFieldData[field_element->getDimension() - 1];
	int number_of_element_field_nodes = 0;
	struct FE_node **element_field_nodes_array = 0;
	int element_dimension = element->getDimension();
	if (face_number >= 0)
		--element_dimension;
	const int number_of_components = field->number_of_components;
	struct FE_basis *previous_basis = 0;
	int previous_number_of_element_values = -1;
	DsLabelIndex *element_value, *element_values = 0;
	DsLabelIndex *previous_element_values = 0;
	const DsLabelIndex elementIndex = element->getIndex();
	for (int component_number = 0; return_code && (component_number < number_of_components); ++component_number)
	{
		const FE_element_field_template *componentEFT = meshFieldData->getComponentMeshfieldtemplate(component_number)->getElementfieldtemplate(elementIndex);
		if (componentEFT->getElementParameterMappingMode() == CMZN_ELEMENT_PARAMETER_MAPPING_MODE_NODE)
		{
			const int number_of_element_values = global_to_element_map_nodes(field, component_number,
				componentEFT, element, element_values);
			if (0 == number_of_element_values)
			{
				display_message(ERROR_MESSAGE,
					"calculate_FE_element_field_nodes.  Could not allocate combined_blending_matrix");
				return_code = 0;
				break;
			}
			FE_basis *basis = componentEFT->getBasis();
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
				Standard_basis_function *standard_basis_function;
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
								struct FE_node *node = nodeset->getNode(*element_value);
								if (node)
								{
									k = 0;
									while ((k<number_of_element_field_nodes)&&
										(node != element_field_nodes_array[k]))
									{
										k++;
									}
									if (k>=number_of_element_field_nodes)
									{
										struct FE_node **temp_element_field_nodes_array;
										if (REALLOCATE(temp_element_field_nodes_array,
											element_field_nodes_array, struct FE_node *,
											number_of_element_field_nodes+1))
										{
											element_field_nodes_array = temp_element_field_nodes_array;
											element_field_nodes_array[number_of_element_field_nodes] =
												ACCESS(FE_node)(node);
											number_of_element_field_nodes++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"calculate_FE_element_field_nodes.  "
												"Could not REALLOCATE element_field_nodes_array");
											return_code = 0;
										}
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
	return (return_code);
}

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
	const Value_storage **component_grid_values_storage,*element_values_storage;
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
						element_field_values->element->getDimension(),
						element_field_values->element->getIdentifier());
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
						element_field_values->element->getDimension(),
						element_field_values->element->getIdentifier());
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
						element_field_values->element->getDimension(),
						element_field_values->element->getIdentifier());
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
	if (!(field && element_1 && element_2))
		return false;
	FE_mesh *mesh = element_1->getMesh();
	if ((element_2->getMesh() != mesh) || (!mesh))
		return false;
	const FE_mesh_field_data *meshFieldData = field->meshFieldData[mesh->getDimension() - 1];
	if (!meshFieldData)
		return true; // field not defined on any elements in mesh, hence equivalent
	const int componentCount = field->number_of_components;
	for (int c = 0; c < componentCount; ++c)
	{
		const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
		if (mft->getElementEFTIndex(element_1->getIndex()) != mft->getElementEFTIndex(element_2->getIndex()))
			return false;
	}
	return true;
}

bool equivalent_FE_fields_in_elements(struct FE_element *element_1,
	struct FE_element *element_2)
{
	if (!(element_1 && element_2))
		return false;
	FE_mesh *mesh = element_1->getMesh();
	if ((element_2->getMesh() != mesh) || (!mesh))
		return false;
	return mesh->equivalentFieldsInElements(element_1->getIndex(), element_2->getIndex());
}

int get_FE_element_dimension(struct FE_element *element)
{
	if (element)
		return element->getDimension();
	display_message(ERROR_MESSAGE, "get_FE_element_dimension.  Invalid element");
	return 0;
}

DsLabelIdentifier get_FE_element_identifier(struct FE_element *element)
{
	if (element)
		return element->getIdentifier();
	return DS_LABEL_IDENTIFIER_INVALID;
}

DsLabelIndex get_FE_element_index(struct FE_element *element)
{
	if (element)
		return element->getIndex();
	return DS_LABEL_INDEX_INVALID;
}

FE_field *FE_element_get_default_coordinate_field(struct FE_element *element)
{
	if (!(element && element->getMesh()))
		return 0;
	CMZN_SET(FE_field) *fields = reinterpret_cast<CMZN_SET(FE_field) *>(element->getMesh()->get_FE_region()->fe_field_list);
	for (CMZN_SET(FE_field)::iterator iter = fields->begin(); iter != fields->end(); ++iter)
	{
		FE_field *field = *iter;
		if (field->isTypeCoordinate() && FE_field_is_defined_in_element(field, element))
			return field;
	}
	return 0;
}

int get_FE_element_number_of_fields(struct FE_element *element)
{
	if (!(element && element->getMesh()))
		return 0;
	int fieldCount = 0;
	const int dim = element->getDimension() - 1;
	const DsLabelIndex elementIndex = element->getIndex();
	CMZN_SET(FE_field) *fields = reinterpret_cast<CMZN_SET(FE_field) *>(element->getMesh()->get_FE_region()->fe_field_list);
	for (CMZN_SET(FE_field)::iterator iter = fields->begin(); iter != fields->end(); ++iter)
	{
		const FE_mesh_field_data *meshFieldData = (*iter)->meshFieldData[dim];
		if ((meshFieldData) && (meshFieldData->getComponentMeshfieldtemplate(0)->getElementEFTIndex(elementIndex) >= 0))
			++fieldCount;
	}
	return fieldCount;
}

FE_element_shape *get_FE_element_shape(struct FE_element *element)
{
	if (element)
		return element->getElementShape();
	display_message(ERROR_MESSAGE,"get_FE_element_shape.  Invalid element");
	return 0;
}

struct FE_element *get_FE_element_face(struct FE_element *element, int face_number)
{
	const FE_mesh *mesh;
	if (element && (mesh = element->getMesh()))
	{
		const FE_mesh::ElementShapeFaces *shapeFaces = mesh->getElementShapeFacesConst(element->getIndex());
		FE_mesh *faceMesh = mesh->getFaceMesh();
		if ((shapeFaces) && (0 <= face_number) && (face_number < shapeFaces->getFaceCount()) && (faceMesh))
		{
			const DsLabelIndex *faces = shapeFaces->getElementFaces(element->getIndex());
			if (faces)
				return faceMesh->getElement(faces[face_number]);
			return 0;
		}
	}
	display_message(ERROR_MESSAGE, "get_FE_element_face.  Invalid argument(s)");
	return 0;
}

int for_each_FE_field_at_element_alphabetical_indexer_priority(
	FE_element_field_iterator_function *iterator,void *user_data,
	struct FE_element *element)
{
	int i, number_of_fields, return_code;
	struct FE_field *field;
	struct FE_field_order_info *field_order_info;
	struct FE_region *fe_region;

	ENTER(for_each_FE_field_at_element_alphabetical_indexer_priority);
	return_code = 0;
	if (iterator && element && element->getMesh())
	{
		// get list of all fields in default alphabetical order
		field_order_info = CREATE(FE_field_order_info)();
		fe_region = element->getMesh()->get_FE_region();
		return_code = FE_region_for_each_FE_field(fe_region,
			FE_field_add_to_FE_field_order_info, (void *)field_order_info);
		FE_field_order_info_prioritise_indexer_fields(field_order_info);
		number_of_fields = get_FE_field_order_info_number_of_fields(field_order_info);
		for (i = 0; i < number_of_fields; i++)
		{
			field = get_FE_field_order_info_field(field_order_info, i);
			if (FE_field_is_defined_in_element_not_inherited(field, element))
				return_code = (iterator)(element, field, user_data);
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

int FE_element_number_is_in_Multi_range(struct FE_element *element,
	void *multi_range_void)
{
	int return_code;
	struct Multi_range *multi_range = (struct Multi_range *)multi_range_void;
	if (element && multi_range)
	{
		return_code = Multi_range_is_value_in_range(multi_range,
			element->getIdentifier());
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
		const DsLabelIdentifier identifier = element->getIdentifier();
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

bool FE_element_is_top_level_parent_of_element(
	struct FE_element *element, struct FE_element *other_element)
{
	if (element && element->getMesh() && other_element && other_element->getMesh())
	{
		if ((element->getMesh()->getElementParentsCount(element->getIndex()) == 0)
			&& (element->getMesh()->isElementAncestor(element->getIndex(),
				other_element->getMesh(), other_element->getIndex())))
			return true;
	}
	else
		display_message(ERROR_MESSAGE, "FE_element_is_top_level_parent_of_element.  Invalid argument(s)");
	return false;
}

struct FE_element *FE_element_get_top_level_element_conversion(
	struct FE_element *element,struct FE_element *check_top_level_element,
	cmzn_element_face_type specified_face, FE_value *element_to_top_level)
{
	struct FE_element *top_level_element;
	if (element && element->getMesh() && element_to_top_level)
	{
		FE_mesh *parentMesh = element->getMesh()->getParentMesh();
		const DsLabelIndex *parents;
		int parentsCount;
		if ((!parentMesh) || (0 == (parentsCount = element->getMesh()->getElementParents(element->getIndex(), parents))))
		{
			/* no parents */
			top_level_element = element;
		}
		else
		{
			DsLabelIndex parentIndex = DS_LABEL_INDEX_INVALID;
			if (check_top_level_element)
			{
				FE_mesh *topLevelMesh = check_top_level_element->getMesh();
				const DsLabelIndex checkTopLevelElementIndex = check_top_level_element->getIndex();
				for (int p = 0; p < parentsCount; ++p)
				{
					if (((parentMesh == topLevelMesh) && (parents[p] == checkTopLevelElementIndex)) ||
						topLevelMesh->isElementAncestor(checkTopLevelElementIndex, parentMesh, parents[p]))
					{
						parentIndex = parents[p];
						break;
					}
				}
			}
			if ((parentIndex < 0) && (CMZN_ELEMENT_FACE_TYPE_XI1_0 <= specified_face))
				parentIndex = element->getMesh()->getElementParentOnFace(element->getIndex(), specified_face);
			if (parentIndex < 0)
				parentIndex = parents[0];
			FE_element *parent = parentMesh->getElement(parentIndex);

			int face_number;
			FE_element_shape *parent_shape = parentMesh->getElementShape(parentIndex);
			if ((parent_shape) && (parent_shape->face_to_element) &&
				(0 <= (face_number = parentMesh->getElementFaceNumber(parentIndex, element->getIndex()))) &&
				(top_level_element = FE_element_get_top_level_element_conversion(
					parent, check_top_level_element, specified_face, element_to_top_level)))
			{
				FE_value *face_to_element = parent_shape->face_to_element +
					(face_number * parent_shape->dimension*parent_shape->dimension);
				const int size = top_level_element->getDimension();
				if (parent == top_level_element)
				{
					/* element_to_top_level = face_to_element of appropriate face */
					for (int i = size*size - 1; 0 <= i; --i)
					{
						element_to_top_level[i] = face_to_element[i];
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
					for (int i = 0; i < size; ++i)
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_get_top_level_element_conversion.  Invalid argument(s)");
		top_level_element=(struct FE_element *)NULL;
	}
	return (top_level_element);
}

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
	if (element && element->getMesh() && xi && top_level_element && top_level_xi &&
		top_level_element_dimension)
	{
		return_code = 1;
		if (element->getMesh()->getElementParentsCount(element->getIndex()) == 0)
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
				CMZN_ELEMENT_FACE_TYPE_ALL, element_to_top_level)))
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
{
	int dimension,i,j,maximum_number_in_xi,return_code,top_level_dimension;

	ENTER(get_FE_element_discretization_from_top_level);
	if (element&&number_in_xi&&top_level_element&&top_level_number_in_xi)
	{
		return_code=1;
		dimension = element->getDimension();
		if (top_level_element==element)
		{
			for (i=0;i<dimension;i++)
			{
				number_in_xi[i]=top_level_number_in_xi[i];
			}
		}
		else if (element_to_top_level)
		{
			top_level_dimension = top_level_element->getDimension();
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
	cmzn_element_face_type face, struct FE_field *native_discretization_field,
	int *top_level_number_in_xi,struct FE_element **top_level_element,
	int *number_in_xi)
{
	FE_value element_to_top_level[9];
	int return_code;

	ENTER(get_FE_element_discretization);
	if (element&&top_level_number_in_xi&&top_level_element&&number_in_xi)
	{
		if (NULL != (*top_level_element = FE_element_get_top_level_element_conversion(
			element, *top_level_element, face, element_to_top_level)))
		{
			/* get the discretization requested for top-level element, from native
				 discretization field if not NULL and is element based in element */
			if (native_discretization_field)
			{
				const int topDimension = (*top_level_element)->getMesh()->getDimension();
				const FE_mesh_field_data *meshFieldData = native_discretization_field->meshFieldData[topDimension - 1];
				if (meshFieldData)
				{
					// use first grid-based field component
					for (int c = 0; c < native_discretization_field->number_of_components; ++c)
					{
						const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
						const FE_element_field_template *eft = mft->getElementfieldtemplate((*top_level_element)->getIndex());
						const int *gridNumberInXi = eft->getLegacyGridNumberInXi();
						if (gridNumberInXi) // only if legacy grid; other element-based do not multiply
						{
							for (int d = 0; d < topDimension; ++d)
								top_level_number_in_xi[d] *= gridNumberInXi[d];
							break;
						}
					}
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

/**
 * Return whether the shape-face mapping for the supplied face number gives the
 * face an inward normal.
 *
 * @param shape  The shape - must be 3 dimensional.
 * @param face_number  Face index from 0 to (shape->number_of_faces - 1).
 * @return  True if face has inward normal, otherwise false.
 */
static bool FE_element_shape_face_has_inward_normal(struct FE_element_shape *shape,
	int face_number)
{
	if (shape && (3 == shape->dimension) &&
		(0 <= face_number) && (face_number <= shape->number_of_faces))
	{
		FE_value *outward_face_normal;
		FE_value face_xi1[3], face_xi2[3], actual_face_normal[3];
		FE_value *face_to_element = shape->face_to_element +
			face_number*shape->dimension*shape->dimension;
		face_xi1[0] = face_to_element[1];
		face_xi1[1] = face_to_element[4];
		face_xi1[2] = face_to_element[7];
		face_xi2[0] = face_to_element[2];
		face_xi2[1] = face_to_element[5];
		face_xi2[2] = face_to_element[8];
		cross_product_FE_value_vector3(face_xi1, face_xi2, actual_face_normal);
		outward_face_normal = shape->face_normals + face_number*shape->dimension;
		FE_value result = actual_face_normal[0]*outward_face_normal[0] +
			actual_face_normal[1]*outward_face_normal[1] +
			actual_face_normal[2]*outward_face_normal[2];
		if (result < 0.0)
			return true;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_shape_face_has_inward_normal.  Invalid argument(s)");
	}
	return false;
}

bool FE_element_is_exterior_face_with_inward_normal(struct FE_element *element)
{
	if (element && element->getMesh())
	{
		FE_mesh *parentMesh = element->getMesh()->getParentMesh();
		const DsLabelIndex *parents;
		if ((parentMesh) && (parentMesh->getDimension() == 3) &&
			(1 == element->getMesh()->getElementParents(element->getIndex(), parents)))
		{
			if (FE_element_shape_face_has_inward_normal(parentMesh->getElementShape(parents[0]),
					parentMesh->getElementFaceNumber(parents[0], element->getIndex())))
				return true;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_is_exterior_face_with_inward_normal.  Invalid argument(s)");
	}
	return false;
}

int cmzn_element_add_nodes_to_labels_group(cmzn_element *element, DsLabelsGroup &nodeLabelsGroup)
{
	if (!(element && element->getMesh()))
		return CMZN_ERROR_ARGUMENT;
	if (!element->getMesh()->addElementNodesToGroup(element->getIndex(), nodeLabelsGroup))
		return CMZN_ERROR_GENERAL;
	int return_code = CMZN_OK;
	if (element->getMesh()->getElementParentsCount(element->getIndex()) > 0)
	{
		int number_of_element_field_nodes;
		cmzn_node_id *element_field_nodes_array;
		if (calculate_FE_element_field_nodes(element, /*face_number*/-1, (struct FE_field *)NULL,
			&number_of_element_field_nodes, &element_field_nodes_array,
			/*top_level_element*/(struct FE_element *)NULL))
		{
			for (int i = 0; i < number_of_element_field_nodes; i++)
			{
				FE_node *node = element_field_nodes_array[i];
				if (node && (node->index >= 0))
				{
					const int result = nodeLabelsGroup.setIndex(node->index, true);
					if ((result != CMZN_OK) && (result != CMZN_ERROR_ALREADY_EXISTS))
						return_code = result; // don't break as need to deaccess nodes
				}
				cmzn_node_destroy(&node);
			}
			DEALLOCATE(element_field_nodes_array);
		}
		else
			return_code = CMZN_ERROR_GENERAL;
	}
	return return_code;
}

int cmzn_element_remove_nodes_from_labels_group(cmzn_element *element, DsLabelsGroup &nodeLabelsGroup)
{
	if (!(element && element->getMesh()))
		return CMZN_ERROR_ARGUMENT;
	if (!element->getMesh()->removeElementNodesFromGroup(element->getIndex(), nodeLabelsGroup))
		return CMZN_ERROR_GENERAL;
	int return_code = CMZN_OK;
	if (element->getMesh()->getElementParentsCount(element->getIndex()) > 0)
	{
		int number_of_element_field_nodes;
		cmzn_node_id *element_field_nodes_array;
		if (calculate_FE_element_field_nodes(element, /*face_number*/-1, (struct FE_field *)NULL,
			&number_of_element_field_nodes, &element_field_nodes_array,
			/*top_level_element*/(struct FE_element *)NULL))
		{
			for (int i = 0; i < number_of_element_field_nodes; i++)
			{
				FE_node *node = element_field_nodes_array[i];
				if (node && (node->index >= 0))
				{
					const int result = nodeLabelsGroup.setIndex(node->index, false);
					if ((result != CMZN_OK) && (result != CMZN_ERROR_NOT_FOUND))
						return_code = result; // don't break as need to deaccess nodes
				}
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
	if (element && (element->getMesh()))
		return (0 == element->getMesh()->getElementParentsCount(element->getIndex()));
	return 0;
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
{
	USE_PARAMETER(dummy_void);
	if (field && field->isTypeCoordinate())
		return 1;
	return 0;
}

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

bool FE_field_is_defined_in_element(struct FE_field *field,
	struct FE_element *element)
{
	if (!(field && element && element->getMesh()))
	{
		display_message(ERROR_MESSAGE, "FE_field_is_defined_in_element.  Invalid argument(s)");
		return false;
	}
	FE_mesh_field_data *meshFieldData = field->meshFieldData[element->getMesh()->getDimension() - 1];
	if (meshFieldData)
	{
		// check only first component
		const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(0);
		if (mft->getElementEFTIndex(element->getIndex()) >= 0)
			return true;
	}
	FE_mesh *parentMesh = element->getMesh()->getParentMesh();
	if (parentMesh)
	{
		const DsLabelIndex *parents;
		const int parentsCount = element->getMesh()->getElementParents(element->getIndex(), parents);
		for (int p = 0; p < parentsCount; ++p)
			if (FE_field_is_defined_in_element(field, parentMesh->getElement(parents[p])))
				return true;
	}
	return false;
}

bool FE_field_is_defined_in_element_not_inherited(struct FE_field *field,
	struct FE_element *element)
{
	if (!(field && element && element->getMesh()))
		return false;
	FE_mesh_field_data *meshFieldData = field->meshFieldData[element->getMesh()->getDimension() - 1];
	if (!meshFieldData)
		return false; // not defined on any elements of mesh
	// check only first component
	const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(0);
	return mft->getElementEFTIndex(element->getIndex()) >= 0;
}

bool FE_element_field_is_grid_based(struct FE_element *element,
	struct FE_field *field)
{
	if (!(element && element->getMesh() && field))
	{
		display_message(ERROR_MESSAGE, "FE_element_field_is_grid_based.  Invalid argument(s)");
		return false;
	}
	FE_mesh_field_data *meshFieldData = field->meshFieldData[element->getMesh()->getDimension() - 1];
	if (!meshFieldData)
		return false; // not defined on any elements of mesh
	for (int c = 0; c < field->number_of_components; ++c)
	{
		const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
		const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
		if (!eft)
			return false;
		if ((eft->getNumberOfElementDOFs() > 0) && (0 != eft->getLegacyGridNumberInXi()))
			return true;
	}
	return false;
}

bool FE_element_has_grid_based_fields(struct FE_element *element)
{
	if (!(element && element->getMesh()))
	{
		display_message(ERROR_MESSAGE, "FE_element_has_grid_based_fields.  Invalid argument(s)");
		return false;
	}
	CMZN_SET(FE_field) *fields = reinterpret_cast<CMZN_SET(FE_field) *>(element->getMesh()->get_FE_region()->fe_field_list);
	for (CMZN_SET(FE_field)::iterator iter = fields->begin(); iter != fields->end(); ++iter)
	{
		if (FE_element_field_is_grid_based(element, *iter))
			return true;
	}
	return false;
}

int FE_node_smooth_FE_field(struct FE_node *node, struct FE_field *fe_field,
	FE_value time, struct FE_field *node_accumulate_fe_field,
	struct FE_field *element_count_fe_field)
{
	int return_code;
	if (node && fe_field && node_accumulate_fe_field && element_count_fe_field)
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
						if (get_FE_nodal_FE_value_value(node, node_accumulate_fe_field, component, version, type, time, &value) &&
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
		undefine_FE_field_at_node(node, node_accumulate_fe_field);
		undefine_FE_field_at_node(node, element_count_fe_field);
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
	const FE_element_field_template *eft;
	FE_nodeset *nodeset;
	const DsLabelIndex *nodeIndexes;
	FE_field *fe_field, *node_accumulate_fe_field, *element_count_fe_field;
	int component_number;
	FE_value time;
	FE_value *component_values;

public:
	FE_element_accumulate_node_values(FE_element *elementIn,
			const FE_element_field_template *eftIn,
			FE_nodeset *nodesetIn,
			const DsLabelIndex *nodeIndexesIn, FE_field *fe_fieldIn,
			FE_field *node_accumulate_fe_fieldIn,
			FE_field *element_count_fe_fieldIn,
			int component_numberIn, FE_value timeIn, FE_value *component_valuesIn) :
		element(elementIn),
		eft(eftIn),
		nodeset(nodesetIn),
		nodeIndexes(nodeIndexesIn),
		fe_field(fe_fieldIn),
		node_accumulate_fe_field(node_accumulate_fe_fieldIn),
		element_count_fe_field(element_count_fe_fieldIn),
		component_number(component_numberIn),
		time(timeIn),
		component_values(component_valuesIn)
	{
	}

	/**
	 * @param xiIndex  Element chart xi index starting at 0.
	 * @param basisNode1  Basis corner/end local node index for edge node 1.
	 * @param basisNode2  Basis corner/end local node index for edge node 2.
	 */
	void accumulate_edge(int xiIndex, int basisNode1, int basisNode2)
	{
		// basis functions for a local node are assumed to be in order of nodal value types:
		const int basisNodeFunctionIndex =
			(xiIndex == 0) ? FE_NODAL_D_DS1 :
			(xiIndex == 1) ? FE_NODAL_D_DS2 :
			(xiIndex == 2) ? FE_NODAL_D_DS3 :
			FE_NODAL_UNKNOWN;
		if (basisNodeFunctionIndex == FE_NODAL_UNKNOWN)
			return;
		const FE_value delta = this->component_values[basisNode2] - this->component_values[basisNode1];
		for (int n = 0; n < 2; ++n)
		{
			const int basisNode = (n == 0) ? basisNode1 : basisNode2;
			const int functionNumber = FE_basis_get_function_number_from_node_function(eft->getBasis(), basisNode, basisNodeFunctionIndex);
			if (functionNumber < 0)
				continue; // derivative not used for basis at node
			const int termCount = this->eft->getFunctionNumberOfTerms(functionNumber);
			if (0 == termCount)
				continue; // permanent zero derivative
			// only use first term, using more is not implemented
			const int term = 0;
			const int termLocalNodeIndex = this->eft->getTermLocalNodeIndex(functionNumber, term);
			FE_node *node = this->nodeset->getNode(this->nodeIndexes[termLocalNodeIndex]);
			if (node)
				FE_node_field_component_accumulate_value(node,
					this->node_accumulate_fe_field, this->element_count_fe_field, this->component_number,
					this->eft->getTermNodeVersion(functionNumber, term),
					cmzn_node_value_label_to_FE_nodal_value_type(this->eft->getTermNodeValueLabel(functionNumber, term)),
					this->time, delta);
		}
	}
};

template <typename ObjectType> class SelfDestruct
{
	typedef int (DestroyFunction)(ObjectType* objectAddress);
	ObjectType& object;
	DestroyFunction *destroyFunction;

public:
	SelfDestruct(ObjectType& objectIn, DestroyFunction *destroyFunctionIn) :
		object(objectIn),
		destroyFunction(destroyFunctionIn)
	{
	}

	~SelfDestruct()
	{
		(this->destroyFunction)(&object);
	}
};

bool FE_element_smooth_FE_field(struct FE_element *element,
	struct FE_field *fe_field, FE_value time,
	struct FE_field *node_accumulate_fe_field,
	struct FE_field *element_count_fe_field)
{
	FE_element_shape *element_shape = element->getElementShape();
	const int componentCount = get_FE_field_number_of_components(fe_field);
	if (!(element_shape && fe_field && node_accumulate_fe_field && element_count_fe_field &&
		(FE_VALUE_VALUE == get_FE_field_value_type(fe_field)) &&
		(INT_VALUE == get_FE_field_value_type(element_count_fe_field)) &&
		(get_FE_field_number_of_components(element_count_fe_field) ==
			componentCount)))
	{
		display_message(ERROR_MESSAGE, "FE_element_smooth_FE_field.  Invalid argument(s)");
		return false;
	}
	if (!FE_element_shape_is_line(element_shape))
		return true; // not implemented for these shapes, or nothing to do
	FE_mesh *mesh = element->getMesh();
	FE_nodeset *nodeset = mesh->getNodeset();
	if (!nodeset)
	{
		display_message(ERROR_MESSAGE, "FE_element_smooth_FE_field.  No nodeset");
		return false;
	}
	const FE_mesh_field_data *meshFieldData = fe_field->meshFieldData[mesh->getDimension() - 1];
	if (!meshFieldData)
	{
		display_message(ERROR_MESSAGE, "FE_element_smooth_FE_field.  Field not defined on mesh");
		return false; // field not defined on any elements of mesh
	}
	const int dimension = element->getDimension();
	const int basisNodeCount =
		(1 == dimension) ? 2 :
		(2 == dimension) ? 4 :
		(3 == dimension) ? 8 :
		0;
	FE_value component_value[8], xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	FE_element_field_values *fe_element_field_values = CREATE(FE_element_field_values)();
	SelfDestruct<FE_element_field_values *> sd1(fe_element_field_values, DESTROY(FE_element_field_values));
	// need to calculate field values to evaluate field in element corners,
	// ensuring optional modify function, version mapping etc. applied
	int return_code = 1;
	if (!calculate_FE_element_field_values(element, fe_field, time,
		/*calculate_derivatives*/0, fe_element_field_values,
		/*top_level_element*/(struct FE_element *)NULL))
	{
		display_message(ERROR_MESSAGE,
			"FE_element_smooth_FE_field.  Could not calculate element field values");
		return 0;
	}
	for (int componentNumber = 0; componentNumber < componentCount; ++componentNumber)
	{
		const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(componentNumber);
		const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
		if (!eft)
			return true; // field not defined on element
		if (eft->getElementParameterMappingMode() != CMZN_ELEMENT_PARAMETER_MAPPING_MODE_NODE)
			continue;
		if (FE_basis_get_number_of_nodes(eft->getBasis()) != basisNodeCount)
			continue; // some cases not implemented e.g. Hermite * quadratic Lagrange
		FE_mesh_element_field_template_data *meshEFTData = mesh->getElementfieldtemplateData(eft->getIndexInMesh());
		const DsLabelIndex *nodeIndexes = meshEFTData->getElementNodeIndexes(element->getIndex());
		if (!nodeIndexes)
		{
			display_message(ERROR_MESSAGE, "FE_element_smooth_FE_field.  "
				"Missing local-to-global node map for field %s component %d at element %d.",
				fe_field->name, componentNumber + 1, element->getIdentifier());
			return false;
		}
		const int localNodeCount = eft->getNumberOfLocalNodes();
		for (int n = 0; n < localNodeCount; ++n)
		{
			FE_node *node = nodeset->getNode(nodeIndexes[n]);
			if (!node)
			{
				display_message(WARNING_MESSAGE, "FE_element_smooth_FE_field.  Element %d is missing a node",
					element->getIdentifier());
				continue;
			}
			FE_node_field *node_field = FIND_BY_IDENTIFIER_IN_LIST(FE_node_field, field)(
				fe_field, node->fields->node_field_list);
			if (!node_field)
			{
				display_message(ERROR_MESSAGE, "FE_element_smooth_FE_field.  Field not defined at node %d used by element %d",
					node->getIdentifier(), element->getIdentifier());
				return false;
			}
			if (!FE_field_is_defined_at_node(node_accumulate_fe_field, node))
			{
				// define node_accumulate_fe_field and element_count_fe_field identically to fe_field at node
				// note: node field DOFs are zeroed by define_FE_field_at_node
				FE_node_field_creator *fe_node_field_creator = create_FE_node_field_creator_from_node_field(node, fe_field);
				if (!(define_FE_field_at_node(node, node_accumulate_fe_field, (struct FE_time_sequence *)NULL, fe_node_field_creator) &&
					define_FE_field_at_node(node, element_count_fe_field, (struct FE_time_sequence *)NULL, fe_node_field_creator)))
				{
					display_message(ERROR_MESSAGE, "FE_element_smooth_FE_field.  Could not define temporary fields at node");
					return false;
				}
				DESTROY(FE_node_field_creator)(&fe_node_field_creator);
			}
		}
		/* set unit scale factors */
		// GRC A bit brutal, this does not take into account how they are used
		const int localScaleFactorCount = eft->getNumberOfLocalScaleFactors();
		if (localScaleFactorCount)
		{
			FE_value *scaleFactors = meshEFTData->getElementScaleFactors(element->getIndex());
			if (!scaleFactors)
			{
				display_message(ERROR_MESSAGE, "FE_element_smooth_FE_field.  Element %d is missing scale factors",
					element->getIdentifier());
				return false;
			}
			for (int s = 0; s < localScaleFactorCount; ++s)
				scaleFactors[s] = 0.0;
			// since other fields may share scale factors, safest to mark all fields as changed for now
			mesh->get_FE_region()->FE_field_all_change(CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
		}
		// get element corner/end values
		for (int n = 0; n < basisNodeCount; n++)
		{
			xi[0] = (FE_value)(n & 1);
			xi[1] = (FE_value)((n & 2)/2);
			xi[2] = (FE_value)((n & 4)/4);
			if (!calculate_FE_element_field(componentNumber,
				fe_element_field_values, xi, &(component_value[n]),
				/*jacobian*/(FE_value *)NULL))
			{
				display_message(ERROR_MESSAGE,
					"FE_element_smooth_FE_field.  Could not calculate element field");
				return 0;
			}
		}
		if (!return_code)
			return 0;

		FE_element_accumulate_node_values element_accumulate_node_values(element,
			eft, nodeset, nodeIndexes, fe_field, node_accumulate_fe_field,
			element_count_fe_field, componentNumber, time, component_value);
		element_accumulate_node_values.accumulate_edge(/*xi*/0, 0, 1);
		if (1 < dimension)
		{
			element_accumulate_node_values.accumulate_edge(/*xi*/0, 2, 3);
			element_accumulate_node_values.accumulate_edge(/*xi*/1, 0, 2);
			element_accumulate_node_values.accumulate_edge(/*xi*/1, 1, 3);
			if (2 < dimension)
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
	return true;
}

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

int FE_element_get_number_of_change_to_adjacent_element_permutations(
	struct FE_element *element, FE_value *xi, int face_number)
{
	int number_of_permutations;

	USE_PARAMETER(xi);
	FE_mesh *faceMesh, *fe_mesh;
	if ((element) && (fe_mesh = element->getMesh()) &&
		(faceMesh = fe_mesh->getFaceMesh()))
	{
		number_of_permutations = 1;
		DsLabelIndex faceIndex = fe_mesh->getElementFace(element->getIndex(), face_number);
		if (faceIndex >= 0)
		{
			number_of_permutations = 1;
			switch (faceMesh->getDimension())
			{
				case 1:
				{
					number_of_permutations = 2;
				} break;
				case 2:
				{
					// doesn't yet support all [8] permutations for square faces...
					// need to implement in FE_element_change_to_adjacent_element
					FE_element_shape *face_shape = faceMesh->getElementShape(faceIndex);
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
	return (number_of_permutations);
}

int FE_element_change_to_adjacent_element(struct FE_element **element_address,
	FE_value *xi, FE_value *increment, int *face_number, FE_value *xi_face,
	int permutation)
{
	int return_code = 0;
	int dimension = 0;
	struct FE_element *element;
	FE_element_shape *element_shape;
	FE_mesh *faceMesh, *fe_mesh;
	if ((element_address) && (element = *element_address) &&
		(fe_mesh = element->getMesh()) &&
		(faceMesh = fe_mesh->getFaceMesh()) &&
		(0 != (element_shape = get_FE_element_shape(element))) &&
		(0 < (dimension = element_shape->dimension)) &&
		(0<=*face_number)&&(*face_number<element_shape->number_of_faces))
	{
		int new_face_number;
		DsLabelIndex newElementIndex = fe_mesh->getElementFirstNeighbour(element->getIndex(), *face_number, new_face_number);
		if (newElementIndex < 0)
		{
			/* no adjacent element found */
			*face_number = -1;
			return_code = 1;
		}
		else
		{
			DsLabelIndex faceIndex = fe_mesh->getElementFace(element->getIndex(), *face_number);
			FE_element_shape *face_shape = faceMesh->getElementShape(faceIndex);
			FE_element *new_element = fe_mesh->getElement(newElementIndex);
			if (face_shape && new_element)
			{
				FE_value temp_increment[MAXIMUM_ELEMENT_XI_DIMENSIONS];
				FE_value face_normal[MAXIMUM_ELEMENT_XI_DIMENSIONS];
				FE_value local_xi_face[MAXIMUM_ELEMENT_XI_DIMENSIONS - 1];
				/* change xi and increment into element coordinates */
				FE_element_shape *new_element_shape = get_FE_element_shape(new_element);
				if ((new_element_shape) && (dimension == new_element_shape->dimension) && 
					(0 <= new_face_number))
				{
					return_code = 1;
					if (xi)
					{
						for (int j = 0 ; j < dimension - 1 ; j++)
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
						FE_value *face_to_element = (new_element_shape->face_to_element)+
							(new_face_number*dimension*dimension);
						for (int i=0;i<dimension;i++)
						{
							xi[i]= *face_to_element;
							face_to_element++;
							for (int j=0;j<dimension-1;j++)
							{
								xi[i] += (*face_to_element)*local_xi_face[j];
								face_to_element++;
							}
						}
					}
					if (increment)
					{
						/* convert increment into face+normal coordinates */
						FE_value *face_to_element = (element_shape->face_to_element)+
							(*face_number*dimension*dimension);
						return_code = face_calculate_xi_normal(element_shape,
							*face_number, face_normal);
						if (return_code)
						{
							double dot_product;
							for (int i=0;i<dimension;i++)
							{
								temp_increment[i]=increment[i];
							}
							for (int i=1;i<dimension;i++)
							{
								dot_product=(double)0;
								for (int j=0;j<dimension;j++)
								{
									dot_product += (double)(temp_increment[j])*
										(double)(face_to_element[j*dimension+i]);
								}
								increment[i-1]=(FE_value)dot_product;
							}
							dot_product=(double)0;
							for (int i=0;i<dimension;i++)
							{
								dot_product += (double)(temp_increment[i])*(double)(face_normal[i]);
							}
							increment[dimension-1]=(FE_value)dot_product;

							/* Convert this back to an increment in the new element */
							return_code = face_calculate_xi_normal(new_element_shape,
								new_face_number, face_normal);
							if (return_code)
							{
								for (int i=0;i<dimension;i++)
								{
									temp_increment[i]=increment[i];
								}
								face_to_element=(new_element_shape->face_to_element)+
									(new_face_number*dimension*dimension);
								for (int i=0;i<dimension;i++)
								{
									increment[i]=temp_increment[dimension-1]*face_normal[i];
									face_to_element++;
									for (int j=0;j<dimension-1;j++)
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
						"Invalid new element shape or face number");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"FE_element_change_to_adjacent_element.  "
					"Invalid new element or element shape");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_change_to_adjacent_element.  "
			"Invalid argument(s).  %p %p %d %p %p %d %p",element_address,
			element_address ? *element_address : 0, dimension,
			xi,increment,*face_number,xi_face);
		return_code = 0;
	}
	return (return_code);
}

int FE_element_xi_increment(struct FE_element **element_address,FE_value *xi,
	FE_value *increment)
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

int cmzn_node_set_identifier(cmzn_node_id node, int identifier)
{
	if (node && node->fields)
		return node->fields->fe_nodeset->change_FE_node_identifier(node, identifier);
	return CMZN_ERROR_ARGUMENT;
}
