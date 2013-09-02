/***************************************************************************//**
 * FILE : elementid.h
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
 * Portions created by the Initial Developer are Copyright (C) 2005-2011
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
