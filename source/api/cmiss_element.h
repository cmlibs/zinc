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

#include "api/types/cmiss_c_inline.h"
#include "api/types/cmiss_element_id.h"
#include "api/types/cmiss_field_id.h"
#include "api/types/cmiss_field_module_id.h"
#include "api/types/cmiss_node_id.h"

/*
Global types
------------
*/

/***************************************************************************//**
 * Common element shapes.
 */
enum Cmiss_element_shape_type
{
	CMISS_ELEMENT_SHAPE_TYPE_INVALID = 0,/**< unspecified shape of known dimension */
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
	CMISS_BASIS_FUNCTION_TYPE_INVALID = 0,
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
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_element_shape_type Cmiss_element_shape_type_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_element_shape_type_enum_to_string(enum Cmiss_element_shape_type type);

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_basis_function_type Cmiss_basis_function_type_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_basis_function_type_enum_to_string(
	enum Cmiss_basis_function_type type);

/***************************************************************************//**
 * Get a handle to the default mesh of a given dimension. Cmgui is currently
 * limited to 1 mesh of each dimension from 1 to 3. These meshes have default
 * names of "cmiss_mesh_Nd", where "N" is the dimension.
 *
 * @param field_module  The field module the mesh belongs to.
 * @param dimension  The dimension of the mesh from 1 to 3.
 * @return  Handle to the finite element mesh, or NULL if error.
 */
Cmiss_mesh_id Cmiss_field_module_find_mesh_by_dimension(
	Cmiss_field_module_id field_module, int dimension);

/***************************************************************************//**
 * Get a handle to a finite element mesh from its name. A mesh is the container
 * of elements of a fixed dimension. Valid names may be any element_group field,
 * or any of the following special names:
 * "cmiss_mesh_3d" = 3-D elements.
 * "cmiss_mesh_2d" = 2-D elements including faces of 3-D elements.
 * "cmiss_mesh_1d" = 1-D elements including faces (lines) of 2-D elements.
 * The above special mesh names may be preceded by a group region name with '.'
 * separator to obtain the submesh from that group, i.e.
 * "GROUPNAME.cmiss_mesh_Nd", where "N" is the dimension.
 *
 * @param field_module  The field module the mesh belongs to.
 * @param name  The name of the finite element mesh.
 * @return  Handle to the finite element mesh, or NULL if error.
 */
Cmiss_mesh_id Cmiss_field_module_find_mesh_by_name(
	Cmiss_field_module_id field_module, const char *mesh_name);

/*******************************************************************************
 * Returns a new handle to the mesh with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param mesh  The mesh to obtain a new reference to.
 * @return  New mesh handle with incremented reference count.
 */
Cmiss_mesh_id Cmiss_mesh_access(Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Destroys this handle to the finite element mesh and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param mesh_address  Address of handle to the mesh to destroy.
 */
int Cmiss_mesh_destroy(Cmiss_mesh_id *mesh_address);

/***************************************************************************//**
 * Returns whether the element is from the mesh.
 *
 * @param mesh  The mesh to query.
 * @param element  The element to query about.
 * @return  1 if element is in the mesh, 0 if not or error.
 */
int Cmiss_mesh_contains_element(Cmiss_mesh_id mesh,
	Cmiss_element_id element);

/***************************************************************************//**
 * Creates an element_basis object for describing element basis functions.
 * The basis has the same number of dimensions as the mesh.
 *
 * @param mesh  Handle to a mesh from which to obtain basis.
 * @param function_type  The basis function type to use in each dimension
 * i.e. basis function is initially homogeneous.
 * @return  Handle to element_basis, or NULL if error.
 */
Cmiss_element_basis_id Cmiss_mesh_create_element_basis(Cmiss_mesh_id mesh,
	enum Cmiss_basis_function_type function_type);

/***************************************************************************//**
 * Create a blank template from which new elements can be created in this mesh.
 * Also used for defining new fields over elements.
 *
 * @param mesh  Handle to the mesh the template works with.
 * @return  Handle to element_template, or NULL if error.
 */
Cmiss_element_template_id Cmiss_mesh_create_element_template(
	Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Create a new element in this mesh with shape and fields described by the
 * element_template. Returns handle to new element.
 * @see Cmiss_mesh_define_element
 *
 * @param mesh  Handle to the mesh to create the new element in.
 * @param identifier  Non-negative integer identifier of new element, or -1 to
 * automatically generate, starting from 1. Fails if supplied identifier already
 * used by an existing element.
 * @param element_template  Template for element shape and fields.
 * @return  Handle to newly created element, or NULL if error.
 */
Cmiss_element_id Cmiss_mesh_create_element(Cmiss_mesh_id mesh,
	int identifier, Cmiss_element_template_id element_template);

/***************************************************************************//**
 * Create an element iterator object for iterating through the elements in the
 * mesh which are ordered from lowest to highest identifier. The iterator
 * initially points at the position before the first element, so the first call
 * to Cmiss_element_iterator_next() returns the first element and advances the
 * iterator.
 *
 * @param mesh  Handle to the mesh whose elements are to be iterated over.
 * @return  Handle to element_iterator at position before first, or NULL if
 * error.
 */
Cmiss_element_iterator_id Cmiss_mesh_create_element_iterator(
	Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Create a new element in this mesh with shape and fields described by the
 * element_template.
 * @see Cmiss_mesh_create_element
 *
 * @param mesh  Handle to the mesh to create the new element in.
 * @param identifier  Non-negative integer identifier of new element, or -1 to
 * automatically generate, starting from 1. Fails if supplied identifier already
 * used by an existing element.
 * @param element_template  Template for element shape and fields.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_mesh_define_element(Cmiss_mesh_id mesh, int identifier,
	Cmiss_element_template_id element_template);

/***************************************************************************//**
 * Destroy all elements in mesh, also removing them from any related groups.
 * All handles to the destroyed element become invalid.
 *
 * @param mesh  Handle to mesh to destroy elements from.
 * @return  1 if all elements destroyed, 0 if failed.
 */
int Cmiss_mesh_destroy_all_elements(Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Destroy the element if it is in the mesh. Removes element from any related
 * groups it is in. All handles to the destroyed element become invalid.
 *
 * @param mesh  Handle to the mesh whose element is to be destroyed.
 * @param element  The element to destroy.
 * @return  1 if element is successfully destroyed, 0 if error.
 */
int Cmiss_mesh_destroy_element(Cmiss_mesh_id mesh, Cmiss_element_id element);

/***************************************************************************//**
 * Destroy all elements in the mesh for which the conditional field is true i.e.
 * non-zero valued in element. These elements are removed from any related
 * groups they are in. All handles to removed elements become invalid.
 * Results are undefined if conditional field is not constant over element.
 * Note that group and element_group fields are valid conditional fields.
 *
 * @param mesh  Handle to the mesh to destroy elements from.
 * @param conditional_field  Field which if non-zero in the element indicates it
 * is to be destroyed.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_mesh_destroy_elements_conditional(Cmiss_mesh_id mesh,
   Cmiss_field_id conditional_field);

/***************************************************************************//**
 * Return a handle to the element in the mesh with this identifier.
 *
 * @param mesh  Handle to the mesh to find the element in.
 * @param identifier  Non-negative integer identifier of element.
 * @return  Handle to the element, or NULL if not found.
 */
Cmiss_element_id Cmiss_mesh_find_element_by_identifier(Cmiss_mesh_id mesh,
	int identifier);

/***************************************************************************//**
 * Returns the number of dimensions of the mesh.
 *
 * @param mesh  Handle to the mesh to query.
 * @return  dimension of mesh.
 */
int Cmiss_mesh_get_dimension(Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Get the master mesh which owns the elements for this mesh. Can be the
 * same as the supplied mesh if it is a master.
 *
 * @param mesh  The mesh to query.
 * @return  Handle to the master mesh. Caller is responsible for destroying
 * the returned handle.
 */
Cmiss_mesh_id Cmiss_mesh_get_master(Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Return the name of the mesh.
 *
 * @see Cmiss_deallocate()
 * @param mesh  The mesh whose name is requested.
 * @return  On success: allocated string containing mesh name. Up to caller to
 * free using Cmiss_deallocate().
 */
char *Cmiss_mesh_get_name(Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Return the number of elements in the mesh.
 *
 * @param mesh  Handle to the mesh to query.
 * @return  Number of elements in mesh.
 */
int Cmiss_mesh_get_size(Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Check if two mesh handles refer to the same object.
 *
 * @param mesh1  The first mesh to match.
 * @param mesh2  The second mesh to match.
 * @return  1 if the two meshes match, 0 if not.
 */
int Cmiss_mesh_match(Cmiss_mesh_id mesh1, Cmiss_mesh_id mesh2);

/***************************************************************************//**
 * If the mesh is a mesh group i.e. subset of elements from a master mesh,
 * get the mesh group specific interface for add/remove functions.
 * Caller is responsible for destroying the returned reference.
 *
 * @param field  The mesh to be cast.
 * @return  Mesh group specific representation if the input mesh is of this
 * type, otherwise returns NULL.
 */
Cmiss_mesh_group_id Cmiss_mesh_cast_group(Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Destroys this handle to the mesh group and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param mesh_group_address  Address of mesh group handle to destroy.
 */
int Cmiss_mesh_group_destroy(Cmiss_mesh_group_id *mesh_group_address);

/***************************************************************************//**
 * Cast mesh group back to its base mesh class.
 * IMPORTANT NOTE: Returned mesh does not have incremented reference count and
 * must not be destroyed. Use Cmiss_mesh_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the mesh_group.
 * Use this function to call base-class API, e.g.:
 * int size = Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(mesh_group);
 *
 * @param mesh_group  Handle to the mesh group to cast.
 * @return  Non-accessed handle to the mesh or NULL if failed.
 */
CMISS_C_INLINE Cmiss_mesh_id Cmiss_mesh_group_base_cast(
	Cmiss_mesh_group_id mesh_group)
{
	return (Cmiss_mesh_id)(mesh_group);
}

/***************************************************************************//**
 * Add specified element to mesh group.
 *
 * @param mesh_group  Handle to mesh group to modify.
 * @param element  Handle to element to add. Must be from the group's master mesh.
 * @return  1 if element added, 0 if failed.
 */
int Cmiss_mesh_group_add_element(Cmiss_mesh_group_id mesh_group,
	Cmiss_element_id element);

/***************************************************************************//**
 * Remove all elements from mesh group.
 *
 * @param mesh_group  Handle to mesh group to modify.
 * @return  1 if all elements removed, 0 if failed.
 */
int Cmiss_mesh_group_remove_all_elements(Cmiss_mesh_group_id mesh_group);

/***************************************************************************//**
 * Remove specified element from mesh group.
 *
 * @param mesh_group  Handle to mesh group to modify.
 * @param element  Handle to element to remove.
 * @return  1 if element removed, 0 if failed.
 */
int Cmiss_mesh_group_remove_element(Cmiss_mesh_group_id mesh_group,
	Cmiss_element_id element);

/***************************************************************************//**
 * Remove all elements from the mesh group for which the conditional field is
 * true i.e. non-zero valued in the element.
 * Results are undefined if conditional field is not constant over element.
 * Note that group and element_group fields are valid conditional fields.
 *
 * @param mesh_group  Handle to the mesh group to remove elements from.
 * @param conditional_field  Field which if non-zero in the element indicates it
 * is to be removed.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_mesh_group_remove_elements_conditional(Cmiss_mesh_group_id mesh_group,
   Cmiss_field_id conditional_field);

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
int Cmiss_element_basis_get_dimension(Cmiss_element_basis_id element_basis);

/***************************************************************************//**
 * Gets the basis function type for a component of the basis.
 *
 * @param element_basis  Element basis to query.
 * @param chart_component  The chart component to get the function for from 1 to
 * dimension.
 * @return  The basis function type.
 */
enum Cmiss_basis_function_type Cmiss_element_basis_get_function_type(
	Cmiss_element_basis_id element_basis, int chart_component);

/***************************************************************************//**
 * Sets a simple basis function type for a component of the basis.
 *
 * @param element_basis  Element basis to modify.
 * @param chart_component  The chart component to set the function for from 1 to
 * dimension.
 * @param basis_type  The basis type to use on the chosen chart component.
 * @return  1 on success, 0 on error.
 */
int Cmiss_element_basis_set_function_type(Cmiss_element_basis_id element_basis,
	int chart_component, enum Cmiss_basis_function_type function_type);

/***************************************************************************//**
 * If the basis is valid, gets the number of nodes the element_basis expects.
 *
 * @param element_basis  Element basis to query.
 * @return  number of nodes expected, or 0 if basis is incomplete or invalid.
 */
int Cmiss_element_basis_get_number_of_nodes(
	Cmiss_element_basis_id element_basis);

/***************************************************************************//**
 * Destroys this handle to the element_iterator and sets it to NULL.
 *
 * @param element_iterator_address  Address of handle to element_iterator to
 * destroy.
 */
int Cmiss_element_iterator_destroy(
	Cmiss_element_iterator_id *element_iterator_address);

/***************************************************************************//**
 * Returns a handle to the next element in the container being iterated over
 * then advances the iterator position. The caller is required to destroy the
 * returned element handle.
 *
 * @param element_iterator  Element iterator to query and advance.
 * @return  Handle to the next element, or NULL if none remaining.
 */
Cmiss_element_id Cmiss_element_iterator_next(
	Cmiss_element_iterator_id element_iterator);

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
 * Sets the element shape to a standard element shape type. The shape must have
 * the same dimension as the mesh from which the element template was created.
 * Special value CMISS_ELEMENT_SHAPE_TYPE_INVALID indicates an unspecified shape
 * of known; when this is set in the template it does not override the shape
 * of any elements it is merged into. Beware that face mappings are lost if
 * shape changes are merged into global elements.
 * Shape must be set before template can be finalised.
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
 * be done before defining fields that index them.
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
 * @param field  The field to define. Must be finite_element type.
 * @param component_number  The component to define from 1 to the number of
 * field components, or -1 to define all components with identical basis and
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
	Cmiss_field_id field,  int component_number,
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
 * @return  Handle to the global node, or NULL if none or error.
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
 * Returns the number of dimensions of the element's chart.
 *
 * @param element  The element to query.
 * @return  The dimension.
 */
int Cmiss_element_get_dimension(Cmiss_element_id element);

/***************************************************************************//**
 * Returns the non-negative integer uniquely identifying the element in its
 * mesh.
 *
 * @param element  The element to query.
 * @return  The integer identifier of the element, or -1 if element is invalid.
 */
int Cmiss_element_get_identifier(Cmiss_element_id element);

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
