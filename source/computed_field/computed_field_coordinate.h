/*******************************************************************************
FILE : computed_field_coordinate.h

LAST MODIFIED : 18 July 2000

DESCRIPTION :
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
#if !defined (COMPUTED_FIELD_COORDINATE_H)
#define COMPUTED_FIELD_COORDINATE_H

int Computed_field_extract_rc(struct Computed_field *field,
	int element_dimension,FE_value *rc_coordinates,FE_value *rc_derivatives);
/*******************************************************************************
LAST MODIFIED : 9 February 1999

DESCRIPTION :
Takes the values in <field> and converts them from their current coordinate
system into rectangular cartesian, returning them in the 3 component
<rc_coordinates> array. If <rc_derivatives> is not NULL, the derivatives are
also converted to rc and returned in that 9-component FE_value array.
Note that odd coordinate systems, such as FIBRE are treated as if they are
RECTANGULAR_CARTESIAN, which just causes a copy of values.
If <element_dimension> or the number of components in <field> are less than 3,
the missing places in the <rc_coordinates> and <rc_derivatives> arrays are
cleared to zero.
???RC Uses type float for in-between values x,y,z and jacobian for future
compatibility with coordinate system transformation functions in geometry.c.
This causes a slight drop in performance.

Note the order of derivatives:
1. All the <element_dimension> derivatives of component 1.
2. All the <element_dimension> derivatives of component 2.
3. All the <element_dimension> derivatives of component 3.
==============================================================================*/

/*****************************************************************************//**
 * Creates a field which performs a coordinate transformation from the source
 * field values in their coordinate system into the coordinate system of this
 * field. Returned field has 3 components.
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field with values in their own coordinate system.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_coordinate_transformation(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

/*****************************************************************************//**
 * Create a field which performs a coordinate transformation of vectors from their
 * original coordinate system and coordinate positions, to the coordinate system
 * of this field. Sets the number of components in returned field to 3 times the
 * number of vectors expected from the source vector_field.
 * 
 * @param field_module  Region field module which will own new field.
 * @param vector_field  Vector field to be transformed. Can be a single vector
 * (1,2 or 3 components), two vectors (4 or 6 components) or three vectors
 * (9 components).
 * @param coordinate_field  Field giving location where vector value is from.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_vector_coordinate_transformation(
	struct Cmiss_field_module *field_module,
	struct Computed_field *vector_field, struct Computed_field *coordinate_field);

int Computed_field_register_types_coordinate(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_COORDINATE_H) */
