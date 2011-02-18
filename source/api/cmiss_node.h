/***************************************************************************//**
 * FILE : cmiss_node.h
 *
 * The public interface to Cmiss_node.
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
#ifndef __CMISS_NODE_H__
#define __CMISS_NODE_H__

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

/** Handle to a nodeset, the container for nodes in a region. */
struct Cmiss_nodeset;
typedef struct Cmiss_nodeset *Cmiss_nodeset_id;

/** Handle to a template for creating or defining fields at a node. */
struct Cmiss_node_template;
typedef struct Cmiss_node_template *Cmiss_node_template_id;

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
/* GRC remove */
#define Cmiss_node FE_node

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_time_sequence FE_time_sequence
struct Cmiss_time_sequence;

#ifndef CMISS_NODE_ID_DEFINED
	struct Cmiss_node;
	/** Handle to a single node object */
	typedef struct Cmiss_node *Cmiss_node_id;
	#define CMISS_NODE_ID_DEFINED
#endif /* CMISS_NODE_ID_DEFINED */

typedef int (*Cmiss_node_iterator_function)(Cmiss_node_id node,
  void *user_data);
/*******************************************************************************
LAST MODIFIED : 03 March 2005

DESCRIPTION :
Declare a pointer to a function of type
int function(struct Cmiss_node *node, void *user_data);
==============================================================================*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_nodal_value_type FE_nodal_value_type

enum FE_nodal_value_type
/*******************************************************************************
LAST MODIFIED : 27 January 1998

DESCRIPTION :
The type of a nodal value.
Must add new enumerators and keep values in sync with functions
ENUMERATOR_STRING, ENUMERATOR_GET_VALID_STRINGS and STRING_TO_ENUMERATOR.
Note these functions expect the first enumerator to be number 1, and all
subsequent enumerators to be sequential, unlike the default behaviour which
starts at 0.
==============================================================================*/
{
	FE_NODAL_VALUE,
	FE_NODAL_D_DS1,
	FE_NODAL_D_DS2,
	FE_NODAL_D_DS3,
	FE_NODAL_D2_DS1DS2,
	FE_NODAL_D2_DS1DS3,
	FE_NODAL_D2_DS2DS3,
	FE_NODAL_D3_DS1DS2DS3,
	FE_NODAL_UNKNOWN
}; /* enum FE_nodal_value_type */

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_node_field_creator FE_node_field_creator

struct Cmiss_node_field_creator;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_node_field_creator * Cmiss_node_field_creator_id;

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_time_version FE_time_version

struct FE_time_version;

typedef struct Cmiss_time_version * Cmiss_time_version_id;

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Get a handle to a nodeset from its name in the region. A nodeset is the
 * container of nodes - i.e. Cmiss_node objects - in the region.
 * Valid names are currently limited to:
 * "cmiss_nodes" = the primary set of nodes for a region, able to be indexed by
 * Cmiss_elements for storing or mapping to finite element field parameters.
 * "cmiss_data" = an additional set of nodes generally used to represent data
 * points, not for finite element field parameters.
 *
 * @param region  The region the nodeset belongs to.
 * @param name  The name of the nodeset: "cmiss_nodes" or "cmiss_data".
 * @return  Handle to the nodeset, or NULL if error.
 */
Cmiss_nodeset_id Cmiss_region_get_nodeset_by_name(Cmiss_region_id region,
	const char *nodeset_name);

/*******************************************************************************
 * Returns a new handle to the nodeset with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param nodeset  The nodeset to obtain a new reference to.
 * @return  New nodeset handle with incremented reference count.
 */
Cmiss_nodeset_id Cmiss_nodeset_access(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Destroys this handle to the nodeset and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param nodeset_address  Address of handle to the nodeset to destroy.
 */
int Cmiss_nodeset_destroy(Cmiss_nodeset_id *nodeset_address);

/***************************************************************************//**
 * Returns whether the node is from the nodeset.
 *
 * @param nodeset  The nodeset to query.
 * @param node  The node to query about.
 * @return  1 if node is in the nodeset, 0 if not or error.
 */
int Cmiss_nodeset_contains_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node);

/***************************************************************************//**
 * Create a blank template from which new nodes can be created in this mesh.
 * Used to describe how finite element fields are defined.
 * Also used for defining new fields on existing nodes.
 *
 * @param nodeset  Handle to the nodeset the template works with.
 * @return  Handle to node_template, or NULL if error.
 */
Cmiss_node_template_id Cmiss_nodeset_create_node_template(
	Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Create a new node in this nodeset with fields defined as in the
 * node_template. Returns handle to new element.
 *
 * @param nodeset  Handle to the nodeset to create the new node in.
 * @param identifier  Positive integer identifier of new node, or 0 to
 * automatically generate. Fails if already used by an existing node.
 * @param node_template  Template for defining node fields.
 * @return  Handle to newly created node, or NULL if error.
 */
Cmiss_node_id Cmiss_nodeset_create_node(Cmiss_nodeset_id nodeset,
	int identifier, Cmiss_node_template_id node_template);

/***************************************************************************//**
 * Return a handle to the node in the nodeset with this identifier.
 *
 * @param nodeset  Handle to the nodeset to find the node in.
 * @param identifier  Positive integer identifier of node.
 * @return  Handle to the node, or NULL if not found.
 */
Cmiss_node_id Cmiss_nodeset_find_node_by_identifier(Cmiss_nodeset_id nodeset,
	int identifier);

/***************************************************************************//**
 * Return the number of nodes in the nodeset.
 *
 * @param nodeset  Handle to the nodeset to query.
 * @return  Number of nodes in nodeset.
 */
int Cmiss_nodeset_get_size(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Destroys this handle to the node_template and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param node_template_address  Address of handle to node_template
 * to destroy.
 */
int Cmiss_node_template_destroy(Cmiss_node_template_id *node_template_address);

/***************************************************************************//**
 * Defines the field on the node_template with just a single node value per
 * field component with no time variation.
 * Per-component derivatives and multiple versions can be added subsequently.
 * Finalised state is removed by this call.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define.
 * @return  1 on success, 0 on error.
 */
int Cmiss_node_template_define_field(Cmiss_node_template_id node_template,
	Cmiss_field_finite_element_id field);

/***************************************************************************//**
 * Adds storage for the supplied derivative type for the component/s of the
 * field in the node template. Finalised state is removed by this call.
 * Must have first called Cmiss_node_template_define_field for field.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define derivatives for.
 * @param component_number  The component from 1 to the number of field
 * components, or 0 to define the derivative for all components.
 * @param derivative_type  The type of nodal derivative to define.
 * @return  1 on success, 0 on error.
 */
int Cmiss_node_template_define_derivative(Cmiss_node_template_id node_template,
	Cmiss_field_finite_element_id field, int component_number,
	enum Cmiss_nodal_value_type derivative_type);

/***************************************************************************//**
 * Defines variation of all nodal values/derivatives * versions with the
 * supplied time sequence for all components of the field in the node template.
 * Hence there will be as many of each parameter as times in the time sequence.
 * Finalised state is removed by this call.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define versions for.
 * @param time_sequence  Time sequence object defining the number of times for
 * which field parameters are stored, and the times they are for (increasing).
 * @param node_field_creator  Optionally defines different versions and/or
 * derivative types. If it is NULL then a single nodal value for each component
 * will be defined.
 */
int Cmiss_node_template_define_time_sequence(
	Cmiss_node_template_id node_template, Cmiss_field_finite_element_id field,
	struct Cmiss_time_sequence *time_sequence);

/***************************************************************************//**
 * Adds storage for multiple versions of nodal values and derivatives for the
 * component/s of the field in the node template. Finalised state is removed by
 * this call.
 * Must have first called Cmiss_node_template_define_field for field.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define versions for.
 * @param component_number  The component from 1 to the number of field
 * components, or 0 to define the number of versions for all components.
 * @param number_of_versions  The number of versions of each value & derivative
 * stored for the component/s, at least 1 (the default).
 * @return  1 on success, 0 on error.
 */
int Cmiss_node_template_define_versions(Cmiss_node_template_id node_template,
	Cmiss_field_finite_element_id field, int component_number,
	int number_of_versions);

/***************************************************************************//**
 * Checks the definition of node fields and prepares the node template for
 * creating new nodes or merging into existing nodes. This function must be
 * successfully called before using the template for these tasks.
 * Finalise state is removed by functions which modify the node template.
 *
 * @param node_template  Node template to finalise.
 * @return  1 if finalised successfully, 0 if field definitions are invalid.
 */
int Cmiss_node_template_finalise(Cmiss_node_template_id node_template);

/*******************************************************************************
 * Returns a new handle to the node with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param node  The node to obtain a new reference to.
 * @return  New node handle with incremented reference count.
 */
Cmiss_node_id Cmiss_node_access(Cmiss_node_id node);

/***************************************************************************//**
 * Destroys this handle to the node and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param node_address  Address of handle to the node to destroy.
 */
int Cmiss_node_destroy(Cmiss_node_id *node_address);

/***************************************************************************//**
 * Returns the positive integer uniquely identifying the node in its nodeset.
 *
 * @param node  The node to query.
 * @return  The integer identifier of the node.
 */
int Cmiss_node_get_identifier(Cmiss_node_id node);

/***************************************************************************//**
 * Modifies the node to define fields as described in the node_template.
 *
 * @param node  The node to modify.
 * @param node_template  Template containing node field descriptions.
 * @return  1 on success, 0 on error.
 */
int Cmiss_node_merge(Cmiss_node_id node, Cmiss_node_template_id node_template);

#endif /* __CMISS_NODE_H__ */
