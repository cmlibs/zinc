/*******************************************************************************
FILE : cmiss_node.h

LAST MODIFIED : 3 November 2004

DESCRIPTION :
The public interface to Cmiss_node.
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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

#include "general/object.h"

/*
Global types
------------
*/

struct Cmiss_region;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

#ifndef CMISS_REGION_ID_DEFINED
   typedef struct Cmiss_region * Cmiss_region_id;
   #define CMISS_REGION_ID_DEFINED
#endif /* CMISS_REGION_ID_DEFINED */

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_node FE_node

struct Cmiss_node;
typedef struct Cmiss_node *Cmiss_node_id;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

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

/*
Global functions
----------------
*/

int Cmiss_node_get_identifier(Cmiss_node_id node);
/*******************************************************************************
LAST MODIFIED : 1 April 2004

DESCRIPTION :
Returns the integer identifier of the <node>.
==============================================================================*/

struct Cmiss_node_field_creator *CREATE(Cmiss_node_field_creator)(
	int number_of_components);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
An object for defining the components, number_of_versions,
number_of_derivatives and their types at a node.
By default each component has 1 version and no derivatives.
==============================================================================*/

int DESTROY(Cmiss_node_field_creator)(
	struct Cmiss_node_field_creator **node_field_creator_address);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Frees the memory for the node field creator and sets 
<*node_field_creator_address> to NULL.
==============================================================================*/

#define Cmiss_node_field_creator_define_derivative FE_node_field_creator_define_derivative

int Cmiss_node_field_creator_define_derivative(
	struct Cmiss_node_field_creator *node_field_creator, int component_number,
	enum Cmiss_nodal_value_type derivative_type);
/*******************************************************************************
LAST MODIFIED: 16 November 2001

DESCRIPTION:
Adds the derivative of specified <derivative_type> to the <component_number>
specified.impl, 
==============================================================================*/

#define Cmiss_node_field_creator_define_versions FE_node_field_creator_define_versions

int Cmiss_node_field_creator_define_versions(
	struct Cmiss_node_field_creator *node_field_creator, int component_number,
	int number_of_versions);
/*******************************************************************************
LAST MODIFIED: 16 November 2001

DESCRIPTION:
Specifies the <number_of_versions> for <component_number> specified.
==============================================================================*/

Cmiss_node_id create_Cmiss_node(int node_identifier,
	Cmiss_region_id region);
/*******************************************************************************
LAST MODIFIED : 8 November 2004

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
Note that <cm_node_identifier> must be non-negative.
A blank node with the given identifier but no fields is returned.
The new node is set to belong to the ultimate master FE_region of <region>.
==============================================================================*/

Cmiss_node_id create_Cmiss_node_from_template(int node_identifier,
	Cmiss_node_id template_node);
/*******************************************************************************
LAST MODIFIED : 1 November 2004

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
Note that <cm_node_identifier> must be non-negative.
The node copies all the fields and values of the <template_node> and will
belong to the same region.
==============================================================================*/

int destroy_Cmiss_node(Cmiss_node_id *node_id_address);
/*******************************************************************************
LAST MODIFIED : 1 November 2004

DESCRIPTION :
Deaccesses the node.
==============================================================================*/

#endif /* __CMISS_NODE_H__ */
