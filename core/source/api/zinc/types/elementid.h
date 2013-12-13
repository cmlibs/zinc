/**
 * FILE : elementid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/** Handle to a finite element mesh. */
#ifndef CMZN_ELEMENTID_H__
#define CMZN_ELEMENTID_H__

struct cmzn_mesh;
typedef struct cmzn_mesh *cmzn_mesh_id;

struct cmzn_mesh_group;
typedef struct cmzn_mesh_group *cmzn_mesh_group_id;

/** Handle to a template for creating or defining fields over an element. */
struct cmzn_elementtemplate;
typedef struct cmzn_elementtemplate *cmzn_elementtemplate_id;

struct cmzn_element;
/** Handle to a single finite element object from a mesh */
typedef struct cmzn_element *cmzn_element_id;

/**
 * An enum for selecting the faces of elements.
 */
enum cmzn_element_face_type
{
	CMZN_ELEMENT_FACE_TYPE_INVALID = -1,
	CMZN_ELEMENT_FACE_TYPE_ALL,   /*!< include all faces */
	CMZN_ELEMENT_FACE_TYPE_XI1_0, /*!< only faces where top-level xi1 == 0 */
	CMZN_ELEMENT_FACE_TYPE_XI1_1, /*!< only faces where top-level xi1 == 1 */
	CMZN_ELEMENT_FACE_TYPE_XI2_0, /*!< only faces where top-level xi2 == 0 */
	CMZN_ELEMENT_FACE_TYPE_XI2_1, /*!< only faces where top-level xi2 == 1 */
	CMZN_ELEMENT_FACE_TYPE_XI3_0, /*!< only faces where top-level xi3 == 0 */
	CMZN_ELEMENT_FACE_TYPE_XI3_1  /*!< only faces where top-level xi3 == 1 */
};

/**
 * Common element shapes.
 */
enum cmzn_element_shape_type
{
	CMZN_ELEMENT_SHAPE_TYPE_INVALID = 0,/**< unspecified shape of known dimension */
	CMZN_ELEMENT_SHAPE_TYPE_LINE = 1,        /**< 1-D: 0 <= xi1 <= 1 */
	CMZN_ELEMENT_SHAPE_TYPE_SQUARE = 2,      /**< 2-D: 0 <= xi1,xi2 <= 1 */
	CMZN_ELEMENT_SHAPE_TYPE_TRIANGLE = 3,    /**< 3-D: 0 <= xi1,xi2; xi1+xi2 <= 1 */
	CMZN_ELEMENT_SHAPE_TYPE_CUBE = 4,        /**< 3-D: 0 <= xi1,xi2,xi3 <= 1 */
	CMZN_ELEMENT_SHAPE_TYPE_TETRAHEDRON = 5, /**< 3-D: 0 <= xi1,xi2,xi3; xi1+xi2+xi3 <= 1 */
	CMZN_ELEMENT_SHAPE_TYPE_WEDGE12 = 6,     /**< 3-D: 0 <= xi1,xi2; xi1+xi2 <= 1; 0 <= xi3 <= 1 */
	CMZN_ELEMENT_SHAPE_TYPE_WEDGE13 = 7,     /**< 3-D: 0 <= xi1,xi3; xi1+xi3 <= 1; 0 <= xi2 <= 1 */
	CMZN_ELEMENT_SHAPE_TYPE_WEDGE23 = 8      /**< 3-D: 0 <= xi2,xi3; xi2+xi3 <= 1; 0 <= xi1 <= 1 */
};

struct cmzn_elementiterator;
typedef struct cmzn_elementiterator * cmzn_elementiterator_id;

struct cmzn_elementbasis;
/** Handle to an element basis function definition */
typedef struct cmzn_elementbasis *cmzn_elementbasis_id;

/**
 * Common 1-D or linked-dimension basis function types.
 */
enum cmzn_elementbasis_function_type
{
	CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID = 0,
	CMZN_ELEMENTBASIS_FUNCTION_TYPE_CONSTANT = 1,
	CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_LAGRANGE = 2,
	CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_LAGRANGE = 3,
	CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_LAGRANGE = 4,
	CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_SIMPLEX = 5,   /**< linked on 2 or more dimensions */
	CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_SIMPLEX = 6, /**< linked on 2 or more dimensions */
	CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_HERMITE = 7
};

/**
 * Mode controlling how points are sampled from elements.
 */
enum cmzn_element_point_sampling_mode
{
	CMZN_ELEMENT_POINT_SAMPLING_MODE_INVALID = 0,
	CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CENTRES = 1,
		/*!< Sample points at centres of element or tessellation cells */
	CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS = 2,
		/*!< Sample points at corners of element or tessellation cells */
	CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_POISSON = 3,
		/*!< Sample points randomly within each tessellation cell according to a 
		     Poisson distribution with expected number given by:
		     sample density field * cell volume, area or length, depending on dimension.
		     The sample density field should be evaluated at the cell centre. */
	CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION = 4
		/*!< One point at a specified location in the element chart. */
};

	struct cmzn_meshchanges;
	/** Handle to object describing changes to a mesh in a fieldmoduleevent */
	typedef struct cmzn_meshchanges *cmzn_meshchanges_id;

	/**
	 * Bit flags summarising changes to an element or elements in a mesh.
	 */
	enum cmzn_element_change_flag
	{
		CMZN_ELEMENT_CHANGE_FLAG_NONE = 0,
			/*!< element(s) not changed */
		CMZN_ELEMENT_CHANGE_FLAG_ADD = 1,
			/*!< element(s) added */
		CMZN_ELEMENT_CHANGE_FLAG_REMOVE = 2,
			/*!< element(s) removed */
		CMZN_ELEMENT_CHANGE_FLAG_IDENTIFIER = 4,
			/*!< element(s') identifier changed */
		CMZN_ELEMENT_CHANGE_FLAG_DEFINITION = 8,
			/*!< element(s') definition other than identifier changed e.g. shape */
		CMZN_ELEMENT_CHANGE_FLAG_FIELD = 16
			/*!< change to field values mapped to element(s) */
	};
	
	/**
	 * Type for passing logical OR of #cmzn_element_change_flag
	 */
	typedef int cmzn_element_change_flags;

#endif
