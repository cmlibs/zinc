/***************************************************************************//**
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
	struct cmzn_element_template;
	typedef struct cmzn_element_template *cmzn_element_template_id;

	struct cmzn_element;
	/** Handle to a single finite element object from a mesh */
	typedef struct cmzn_element *cmzn_element_id;

/**
 * An enum for selecting the faces of elements.
 */
enum cmzn_element_face_type
{
	CMZN_ELEMENT_FACE_INVALID = -1,
	CMZN_ELEMENT_FACE_ALL,   /*!< include all faces */
	CMZN_ELEMENT_FACE_XI1_0, /*!< only faces where top-level xi1 == 0 */
	CMZN_ELEMENT_FACE_XI1_1, /*!< only faces where top-level xi1 == 1 */
	CMZN_ELEMENT_FACE_XI2_0, /*!< only faces where top-level xi2 == 0 */
	CMZN_ELEMENT_FACE_XI2_1, /*!< only faces where top-level xi2 == 1 */
	CMZN_ELEMENT_FACE_XI3_0, /*!< only faces where top-level xi3 == 0 */
	CMZN_ELEMENT_FACE_XI3_1  /*!< only faces where top-level xi3 == 1 */
};

	struct cmzn_element_iterator;
	typedef struct cmzn_element_iterator * cmzn_element_iterator_id;

	struct cmzn_element_basis;
	/** Handle to an element basis function definition */
	typedef struct cmzn_element_basis *cmzn_element_basis_id;

/**
 * Mode controlling how points are sampled from elements.
 */
enum cmzn_element_point_sample_mode
{
	CMZN_ELEMENT_POINT_SAMPLE_MODE_INVALID = 0,
	CMZN_ELEMENT_POINT_SAMPLE_CELL_CENTRES = 1,
		/*!< Sample points at centres of element or tessellation cells */
	CMZN_ELEMENT_POINT_SAMPLE_CELL_CORNERS = 2,
		/*!< Sample points at corners of element or tessellation cells */
	CMZN_ELEMENT_POINT_SAMPLE_CELL_POISSON = 3,
		/*!< Sample points randomly within each tessellation cell according to a 
		     Poisson distribution with expected number given by:
		     sample density field * cell volume, area or length, depending on dimension.
		     The sample density field should be evaluated at the cell centre. */
  CMZN_ELEMENT_POINT_SAMPLE_SET_LOCATION = 4
		/*!< One point at a specified location in the element chart. */
};

#endif
