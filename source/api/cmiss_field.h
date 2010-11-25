/*******************************************************************************
FILE : cmiss_field.h

LAST MODIFIED : 8 May 2008

DESCRIPTION :
The public interface to the Cmiss fields.
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
 * Shane Blackett (shane at blackett.co.nz)
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
#ifndef __CMISS_FIELD_H__
#define __CMISS_FIELD_H__

#include "api/cmiss_node.h"
#include "api/cmiss_element.h"
#include "api/cmiss_field_module.h"

/*******************************************************************************
 Automatic scalar broadcast

For Field_constructor_create functions which specify that they apply
automatic scalar broadcast for their source fields arguments,
if the one of the source fields has multiple components and the
other is a scalar, then the scalar will be automatically replicated so that
it matches the number of components in the multiple component field.
For example the result of
ADD(CONSTANT([1 2 3 4], CONSTANT([10]) is [11 12 13 14].
==============================================================================*/

/*
Global types
------------
*/

#ifndef CMISS_FIELD_ID_DEFINED
	/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
	#define Cmiss_field Computed_field
	struct Cmiss_field;
	typedef struct Cmiss_field *Cmiss_field_id;
	#define CMISS_FIELD_ID_DEFINED
#endif /* CMISS_FIELD_ID_DEFINED */
   
/* Forward declared types */
/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_time_sequence FE_time_sequence
struct Cmiss_time_sequence;

struct Cmiss_node_field_creator;

/* Global Functions */

int Cmiss_field_get_number_of_components(Cmiss_field_id field);
/*******************************************************************************
LAST MODIFIED : 23 December 1998

DESCRIPTION :
==============================================================================*/

/*******************************************************************************
 * Returns a new reference to the field with reference count incremented.
 * Caller is responsible for destroying the new reference.
 * 
 * @param field  The field to obtain a new reference to.
 * @return  New field reference with incremented reference count.
 */
Cmiss_field_id Cmiss_field_access(Cmiss_field_id field);

/*******************************************************************************
 * Destroys this reference to the field (and sets it to NULL).
 * Internally this just decrements the reference count.
 */
int Cmiss_field_destroy(Cmiss_field_id *field_address);

/*******************************************************************************
 * Returns a reference to the field module which owns this field.
 * 
 * @param field  The field to obtain field module for.
 * @return  Field module which this field belongs to.
 */
Cmiss_field_module_id Cmiss_field_get_field_module(Cmiss_field_id field);

int Cmiss_field_evaluate_at_node(struct Cmiss_field *field,
	struct Cmiss_node *node, double time, int number_of_values, double *values);
/*******************************************************************************
LAST MODIFIED : 29 March 2004

DESCRIPTION :
Returns the <values> of <field> at <node> and <time> if it is defined there.

The <values> array must be large enough to store as many floats as there are
number_of_components, the function checks that <number_of_values> is
greater than or equal to the number of components.
==============================================================================*/

int Cmiss_field_set_values_at_node(struct Cmiss_field *field,
	struct Cmiss_node *node, double time, int number_of_values, double *values);
/*******************************************************************************
LAST MODIFIED : 21 April 2005

DESCRIPTION :
Sets the <values> of the computed <field> at <node>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the node are changed or a field
is reached for which its calculation is not reversible, or is not supported yet.
==============================================================================*/

int Cmiss_field_evaluate_in_element(struct Cmiss_field *field,
	struct Cmiss_element *element, double *xi, double time,
	struct Cmiss_element *top_level_element, int number_of_values,
	double *values, int number_of_derivatives, double *derivatives);
/*******************************************************************************
LAST MODIFIED : 29 March 2004

DESCRIPTION :
Returns the values and derivatives (if <derivatives> != NULL) of <field> at
<element>:<xi>, if it is defined over the element.

The optional <top_level_element> may be supplied for the benefit of this or
any source fields that may require calculation on it instead of a face or line.
FIBRE_AXES and GRADIENT are examples of such fields, since they require
top-level coordinate derivatives. The term "top_level" refers to an ultimate
parent element for the face or line, eg. the 3-D element parent to 2-D faces.
If no such top level element is supplied and one is required, then the first
available parent element will be chosen - if the user requires a top-level
element in the same group as the face or with the face on the correct side,
then they should supply the top_level_element here.

The <values> and <derivatives> arrays must be large enough to store all the
values and derivatives for this field in the given element, ie. values is
number_of_components in size, derivatives has the element dimension times the
number_of_components
==============================================================================*/

char *Cmiss_field_evaluate_as_string_at_node(
	struct Cmiss_field *field, struct Cmiss_node *node, double time);
/*******************************************************************************
LAST MODIFIED : 17 January 2007

DESCRIPTION :
Returns a string describing the value/s of the <field> at the <node>. If the
field is based on an FE_field but not returning FE_values, it is asked to supply
the string. Otherwise, a string built up of comma separated values evaluated
for the field in field_evaluate_cache_at_node. The FE_value exception
is used since it is likely the values are already in the cache in most cases,
or can be used by other fields again if calculated now.
Creates a string which represents all the components.
Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/

int Cmiss_field_is_defined_at_node(Cmiss_field_id field,
	struct Cmiss_node *node);
/*******************************************************************************
LAST MODIFIED : 17 January 2007

DESCRIPTION :
Returns true if <field> can be calculated at <node>. If the field depends on
any other fields, this function is recursively called for them.
==============================================================================*/

int Cmiss_field_evaluate_at_field_coordinates(
	Cmiss_field_id field,
	Cmiss_field_id reference_field, int number_of_input_values,
	double *input_values, double time, double *values);
/*******************************************************************************
LAST MODIFIED : 25 March 2008

DESCRIPTION :
Returns the <values> of <field> at the location of <input_values>
with respect to the <reference_field> if it is defined there.

The <values> array must be large enough to store as many FE_values as there are
number_of_components.
==============================================================================*/

/***************************************************************************//**
 * Return the name of the field. 
 * 
 * @param field  The field whose name is requested.
 * @return  On success: allocated string containing field name.
 */
char *Cmiss_field_get_name(Cmiss_field_id field);

/***************************************************************************//**
 * Set the name of the field.
 * Fails if the new name is in use by any other field in the same field module.
 * 
 * @param field  The field to be named.
 * @param name  The new name for the field.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_set_name(Cmiss_field_id field, const char *name);

/***************************************************************************//**
 * Returns the persistent property flag of the field.
 *
 * @see Cmiss_field_set_persistent
 * @param field  The field to query.
 * @return  1 if field is persistent, 0 if non-persistent.
 */
int Cmiss_field_get_persistent(Cmiss_field_id field);

/***************************************************************************//**
 * Sets the persistent property flag of the field.
 * Default setting persistent=0 means the field is destroyed when the number of
 * external references to it, including use as a source for other fields, drops
 * to zero.
 * Setting persistent=1 means the field exists in field module even if no
 * external references to it are held, whence it can be found by name or other
 * search criteria.
 * 
 * @param field  The field to set the persistent flag for.
 * @param persistent  Non-zero for persistent, 0 for non-persistent.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_set_persistent(Cmiss_field_id field, int persistent);

#endif /* __CMISS_FIELD_H__ */
