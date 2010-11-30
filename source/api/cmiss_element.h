/***************************************************************************//**
 * FILE : cmiss_element.h
 *
 * The public interface to Cmiss_element, finite element meshes.
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
 * Portions created by the Initial Developer are Copyright (C) 2005-2010
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
#ifndef __CMISS_ELEMENT_H__
#define __CMISS_ELEMENT_H__

#include "api/cmiss_node.h"

/*
Global types
------------
*/

#ifndef CMISS_FIELD_FINITE_ELEMENT_ID_DEFINED
	/** Handle to a finite_element type Cmiss_field */
	struct Cmiss_field_finite_element;
	typedef struct Cmiss_field_finite_element *Cmiss_field_finite_element_id;
	#define CMISS_FIELD_FINITE_ELEMENT_ID_DEFINED
#endif /* CMISS_FIELD_FINITE_ELEMENT_ID_DEFINED */

#ifndef CMISS_REGION_ID_DEFINED
	struct Cmiss_region;
	/** Handle to a region object */
   typedef struct Cmiss_region * Cmiss_region_id;
   #define CMISS_REGION_ID_DEFINED
#endif /* CMISS_REGION_ID_DEFINED */

/** Handle to a finite element mesh. */
struct Cmiss_fe_mesh;
typedef struct Cmiss_fe_mesh *Cmiss_fe_mesh_id;

/** Handle to a template for creating or defining fields over an element. */
struct Cmiss_element_template;
typedef struct Cmiss_element_template *Cmiss_element_template_id;

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
/* GRC remove */
#define Cmiss_element FE_element

#ifndef CMISS_ELEMENT_ID_DEFINED
	struct Cmiss_element;
	/** Handle to a single finite element object from a mesh */
	typedef struct Cmiss_element *Cmiss_element_id;
	#define CMISS_ELEMENT_ID_DEFINED
#endif /* CMISS_ELEMENT_ID_DEFINED */

#ifndef CMISS_BASIS_FUNCTION_ID_DEFINED
	struct Cmiss_element_basis;
	/** Handle to an element basis function definition */
	typedef struct Cmiss_element_basis *Cmiss_element_basis_id;
	#define CMISS_BASIS_FUNCTION_ID_DEFINED
#endif /* CMISS_BASIS_FUNCTION_ID_DEFINED */

typedef int (*Cmiss_element_iterator_function)(Cmiss_element_id element,
  void *user_data);
/*******************************************************************************
LAST MODIFIED : 03 March 2005

DESCRIPTION :
Declare a pointer to a function of type
int function(struct Cmiss_element *element, void *user_data);
==============================================================================*/

/***************************************************************************//**
 * Common element shapes.
 */
enum Cmiss_element_shape_type
{
	CMISS_ELEMENT_SHAPE_TYPE_UNKNOWN = 0,
	CMISS_ELEMENT_SHAPE_LINE = 1,        /**< 1-D: 0 <= xi1 <= 1 */
	CMISS_ELEMENT_SHAPE_SQUARE = 2,      /**< 2-D: 0 <= xi1,xi2 <= 1 */
	CMISS_ELEMENT_SHAPE_TRIANGLE = 3,    /**< 3-D: 0 <= xi1,xi2; xi1+xi2 <= 1 */
	CMISS_ELEMENT_SHAPE_CUBE = 4,        /**< 3-D: 0 <= xi1,xi2,xi3 <= 1 */
	CMISS_ELEMENT_SHAPE_TETRAHEDRON = 5, /**< 3-D: 0 <= xi1,xi2,xi3; xi1+xi2+xi3 <= 1 */
	CMISS_ELEMENT_SHAPE_WEDGE12 = 6,     /**< 3-D: 0 <= xi1,xi2; xi1+xi2 <= 1; 0 <= xi3 <= 1 */
	CMISS_ELEMENT_SHAPE_WEDGE13 = 7,     /**< 3-D: 0 <= xi1,xi3; xi1+xi3 <= 1; 0 <= xi2 <= 1 */
	CMISS_ELEMENT_SHAPE_WEDGE23 = 8      /**< 3-D: 0 <= xi2,xi3; xi2+xi3 <= 1; 0 <= xi1 <= 1 */
};

/***************************************************************************//**
 * Common 1-D or linked-dimension basis function types.
 */
enum Cmiss_basis_function_type
{
	CMISS_BASIS_FUNCTION_TYPE_UNKNOWN = 0,
	CMISS_BASIS_FUNCTION_CONSTANT = 1,
	CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE = 2,
	CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE = 3,
	CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE = 4,
	CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX = 5,   /**< linked on 2 or more dimensions */
	CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX = 6 /**< linked on 2 or more dimensions */
};

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Get a handle to a finite element mesh from its name. An fe_mesh is the
 * container of elements, intended to be of fixed dimension.
 * Valid names are currently limited to:
 * "cmiss_elements" = the region's principle mesh, whether 1-D, 2-D or 3-D.
 * The elements in this mesh have interpolated finite element fields defined
 * over them; the following meshes' elements inherit these fields.
 * "cmiss_faces" = Mesh of the 2-D faces of any 3-D elements, if defined.
 * "cmiss_lines" = Mesh of 1-D lines of any 2-D elements or faces, if defined.
 *
 * @param region  The region the mesh belongs to.
 * @param name  The name of the finite element mesh, currently "cmiss_elements",
 * "cmiss_faces" or "cmiss_lines".
 * @return  Handle to the finite element mesh, or NULL if error.
 */
Cmiss_fe_mesh_id Cmiss_region_get_fe_mesh_by_name(Cmiss_region_id region,
	const char *mesh_name);

/*******************************************************************************
 * Returns a new handle to the fe_mesh with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param mesh  The fe_mesh to obtain a new reference to.
 * @return  New fe_mesh handle with incremented reference count.
 */
Cmiss_fe_mesh_id Cmiss_fe_mesh_access(Cmiss_fe_mesh_id mesh);

/***************************************************************************//**
 * Destroys this handle to the finite element mesh and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param mesh_address  Address of handle to the fe_mesh to destroy.
 */
int Cmiss_fe_mesh_destroy(Cmiss_fe_mesh_id *mesh_address);

/***************************************************************************//**
 * Returns whether the element is from the mesh.
 *
 * @param mesh  The mesh to query.
 * @param element  The element to query about.
 * @return  1 if element is in the mesh, 0 if not or error.
 */
int Cmiss_fe_mesh_contains_element(Cmiss_fe_mesh_id mesh,
	Cmiss_element_id element);

/***************************************************************************//**
 * Creates an element_basis object for describing element basis functions.
 *
 * @param mesh  Handle to a mesh from which to obtain basis.
 * @param dimensions  The number of dimensions of the basis.
 * @param function_type  The basis function type to use in each dimension
 * i.e. basis function is initially homogeneous.
 * @return  Handle to element_basis, or NULL if error.
 */
Cmiss_element_basis_id Cmiss_fe_mesh_create_element_basis(Cmiss_fe_mesh_id mesh,
	int dimensions, enum Cmiss_basis_function_type function_type);

/***************************************************************************//**
 * Create a blank template from which new elements can be created in this mesh.
 * Also used for defining new fields over elements.
 *
 * @param mesh  Handle to the mesh the template works with.
 * @return  Handle to element_template, or NULL if error.
 */
Cmiss_element_template_id Cmiss_fe_mesh_create_element_template(
	Cmiss_fe_mesh_id mesh);

/***************************************************************************//**
 * Create a new element in this mesh with shape and fields described by the
 * element_template. Returns handle to new element.
 * @see Cmiss_fe_mesh_define_element
 *
 * @param mesh  Handle to the mesh to create the new element in.
 * @param identifier  Positive integer identifier of new element, or 0 to
 * automatically generate. Fails if already used by an existing element.
 * @param element_template  Template for element shape and fields.
 * @return  Handle to newly created element, or NULL if error.
 */
Cmiss_element_id Cmiss_fe_mesh_create_element(Cmiss_fe_mesh_id mesh,
	int identifier, Cmiss_element_template_id element_template);

/***************************************************************************//**
 * Create a new element in this mesh with shape and fields described by the
 * element_template. Returns identifier of new element.
 * @see Cmiss_fe_mesh_create_element
 *
 * @param mesh  Handle to the mesh to create the new element in.
 * @param identifier  Positive integer identifier of new element, or 0 to
 * automatically generate. Fails if already used by an existing element.
 * @param element_template  Template for element shape and fields.
 * @return  Identifier of new element or 0 if error.
 */
int Cmiss_fe_mesh_define_element(Cmiss_fe_mesh_id mesh, int identifier,
	Cmiss_element_template_id element_template);

/***************************************************************************//**
 * Return a handle to the element in the mesh with this identifier.
 *
 * @param mesh  Handle to the mesh to find the element in.
 * @param identifier  Positive integer identifier of element.
 * @return  Handle to the element, or NULL if not found.
 */
Cmiss_element_id Cmiss_fe_mesh_find_element_by_identifier(Cmiss_fe_mesh_id mesh,
	int identifier);

/***************************************************************************//**
 * Destroys this handle to the element_basis and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param element_basis_address  Address of handle to element_basis to destroy.
 */
int Cmiss_element_basis_destroy(Cmiss_element_basis_id *element_basis_address);

/***************************************************************************//**
 * Gets the number of dimensions of the elements this basis works with.
 *
 * @param element_basis  Element basis to query.
 * @return  The number of dimensions.
 */
int Cmiss_element_basis_get_number_of_dimensions(
	Cmiss_element_basis_id element_basis);

/***************************************************************************//**
 * Gets the basis function type for a dimension of the basis.
 *
 * @param element_basis  Element basis to query.
 * @param dimension  The dimension to get the function for from 1 to dimensions.
 * @param basis_type  The basis type to use on the chosen dimension.
 * @return  The basis function type.
 */
enum Cmiss_basis_function_type Cmiss_element_basis_get_function_type(
	Cmiss_element_basis_id element_basis, int dimension);

/***************************************************************************//**
 * Sets a simple basis function type for a dimension of the basis.
 *
 * @param element_basis  Element basis to modify.
 * @param dimension  The dimension to set the function for from 1 to dimensions.
 * @param basis_type  The basis type to use on the chosen dimension.
 * @return  1 on success, 0 on error.
 */
int Cmiss_element_basis_set_function_type(Cmiss_element_basis_id element_basis,
	int dimension, enum Cmiss_basis_function_type function_type);

/***************************************************************************//**
 * If the basis is valid, gets the number of nodes the element_basis expects.
 *
 * @param element_basis  Element basis to query.
 * @return  number of nodes expected, or 0 if basis is incomplete or invalid.
 */
int Cmiss_element_basis_get_number_of_nodes(
	Cmiss_element_basis_id element_basis);

/***************************************************************************//**
 * Destroys this handle to the element_template and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param element_template_address  Address of handle to element_template
 * to destroy.
 */
int Cmiss_element_template_destroy(
	Cmiss_element_template_id *element_template_address);

/***************************************************************************//**
 * Gets the current element shape type set in the element_template.
 *
 * @param element_template  Element template to query.
 * @return  The shape set in the element template.
 */
enum Cmiss_element_shape_type Cmiss_element_template_get_shape_type(
	Cmiss_element_template_id element_template);

/***************************************************************************//**
 * Sets the element shape to a standard element shape type. Must be set before
 * any fields can be defined. If the element_template already has fields
 * defined, the new shape must have the same dimension. Beware that face
 * mappings are lost if shape changes are merged into global elements.
 * Finalised state is removed on changing shape.
 *
 * @param element_template  Element template to modify.
 * @param shape_type  Enumerator of standard element shapes.
 * @return  1 on success, 0 on error.
 */
int Cmiss_element_template_set_shape_type(Cmiss_element_template_id element_template,
	enum Cmiss_element_shape_type shape_type);

/***************************************************************************//**
 * Gets the number of local nodes this element_template can address.
 *
 * @param element_template  Element template to query.
 * @return  The number of local nodes, or 0 on error.
 */
int Cmiss_element_template_get_number_of_nodes(
	Cmiss_element_template_id element_template);

/***************************************************************************//**
 * Sets the number of local nodes this element_template can address. This must
 * be done before definining fields that index them.
 * This number cannot be reduced.
 *
 * @param element_template  Element template to modify.
 * @param number_of_nodes  The number of nodes.
 * @return  1 on success, 0 on error.
 */
int Cmiss_element_template_set_number_of_nodes(
	Cmiss_element_template_id element_template, int number_of_nodes);

/***************************************************************************//**
 * Defines a nodally interpolated element field or field component in the
 * element_template. Only Lagrange, simplex and constant basis function types
 * may be used with this function, i.e. where only a simple node value is
 * mapped. Shape must be set before calling this function.
 * Finalised state is removed on defining fields.
 *
 * @param element_template  Element template to modify.
 * @param field  The field to define.
 * @param component_number  The component to define from 1 to the number of
 * field components, or 0 to define all components with identical basis and
 * nodal mappings.
 * @param basis  The element basis to use for all field components.
 * @param basis_number_of_nodes  The number of nodes indexed by the basis,
 * equals the size of the local_node_indexes array.
 * @param local_node_indexes  Array containing the local node indexes of the
 * nodes from which element field parameters are mapped, which range from 1 to
 * the number of nodes set for the element_template. Local nodes are ordered
 * by lowest xi coordinate varying fastest, e.g. for biquadratic Lagrange:
 * xi = (0,0), (0.5,0), (1,0), (0,0.5), (0.5,0.5) ...
 * @return  1 on success, 0 on error.
 */
int Cmiss_element_template_define_field_simple_nodal(
	Cmiss_element_template_id element_template,
	Cmiss_field_finite_element_id field,  int component_number,
	Cmiss_element_basis_id basis, int basis_number_of_nodes,
	const int *local_node_indexes);

/***************************************************************************//**
 * Checks the definition of element fields and if all are complete and valid
 * prepares the element template for setting local nodes, creating new elements
 * and merging into existing elements.
 *
 * @param element_template  Element template to finalise.
 * @return  1 if finalised successfully, 0 if field definitions are invalid.
 */
int Cmiss_element_template_finalise(Cmiss_element_template_id element_template);

/***************************************************************************//**
 * Gets the global node at a given local node index in the element_template.
 * May only be called after a successful Cmiss_element_template_finalise call.
 *
 * @param element_template  Element template to query.
 * @param local_node_index  The index from 1 to number of nodes in template.
 * @return  Handle to the global node, or NULL if none or erro.
 */
Cmiss_node_id Cmiss_element_template_get_node(
	Cmiss_element_template_id element_template, int local_node_index);

/***************************************************************************//**
 * Sets the global node at a given local node index in the element_template.
 * May only be called after a successful Cmiss_element_template_finalise call.
 *
 * @param element_template  Element template to modify.
 * @param local_node_index  The index from 1 to number of nodes in template.
 * @param node  The global node to set at that index.
 * @return  1 on success, 0 on error.
 */
int Cmiss_element_template_set_node(Cmiss_element_template_id element_template,
	int local_node_index, Cmiss_node_id node);

/*******************************************************************************
 * Returns a new handle to the element with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param element  The element to obtain a new reference to.
 * @return  New element handle with incremented reference count.
 */
Cmiss_element_id Cmiss_element_access(Cmiss_element_id element);

/***************************************************************************//**
 * Destroys this handle to the element and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param element_address  Address of handle to the element to destroy.
 */
int Cmiss_element_destroy(Cmiss_element_id *element_address);

/***************************************************************************//**
 * Returns the dimension of the element's chart.
 *
 * @param element  The element to query.
 * @return  The dimension.
 */
int Cmiss_element_get_dimension(Cmiss_element_id element);

/***************************************************************************//**
 * Returns the positive integer uniquely identifying the element in its mesh.
 *
 * @param element  The element to query.
 * @return  The integer identifier of the element.
 */
int Cmiss_element_get_identifier(Cmiss_element_id element);

/***************************************************************************//**
 * Returns the region the element belongs to.
 *
 * @param element  The element to query.
 * @return  Handle to the owning region.
 */
Cmiss_region_id Cmiss_element_get_region(Cmiss_element_id element);

/***************************************************************************//**
 * Gets the shape type of the element. Note that legacy meshes may return an
 * unknown shape type for certain custom element shapes e.g. polygon shapes.
 * It is intended that future revisions of the API will offer more detailed
 * shape query and modification functions.
 *
 * @param element  Element to query.
 * @return  The element's shape type.
 */
enum Cmiss_element_shape_type Cmiss_element_get_shape_type(
	Cmiss_element_id element);

/***************************************************************************//**
 * Modifies the element to use the fields as defined in the element_template.
 * Note that mappings may be optimised or modified in the merge process, often
 * to minimise the number of local nodes in the merged element.
 *
 * @param element  The element to modify.
 * @param element_template  Template containing element field definitions.
 * @return  1 on success, 0 on error.
 */
int Cmiss_element_merge(Cmiss_element_id element,
	Cmiss_element_template_id element_template);

#endif /* __CMISS_ELEMENT_H__ */
