/*******************************************************************************
FILE : iso_field_calculation.h

LAST MODIFIED : 24 September 1997

DESCRIPTION :
Functions/types for creating iso-surfaces (other than using FE_fields).
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
#if !defined (ISO_FIELD_CALCULATION_H)
#define ISO_FIELD_CALCULATION_H

#include "general/object.h"

/*
Global types
------------
*/
enum Iso_field_calculation_type
/*******************************************************************************
LAST MODIFIED : 24 September 1997

DESCRIPTION :
Types of iso-surface.
==============================================================================*/
{
	NULL_TYPE,
	SCALAR_FIELD,
	COORDINATE_PLANE,
	COORDINATE_SPHERE,
	VERTICAL_POINT_TRACE
}; /* enum Iso_field_calculation_type */

struct Iso_field_calculation_data;
	/* details private */

/*
Global functions
----------------
*/
struct Iso_field_calculation_data *CREATE(Iso_field_calculation_data)();
/*******************************************************************************
LAST MODIFIED : 24 September 1997

DESCRIPTION :
Allocates memory and assigns fields for a Iso_field_calculation_data object.
==============================================================================*/

int DESTROY(Iso_field_calculation_data)(
	struct Iso_field_calculation_data **data_ptr);
/*******************************************************************************
LAST MODIFIED : 24 September 1997

DESCRIPTION :
Frees the memory for the fields of <**Iso_field_calculation_data>, frees the
memory for <**Iso_field_calculation_data> and sets <*Iso_field_calculation_data>
to NULL.
==============================================================================*/

enum Iso_field_calculation_type get_Iso_field_calculation_type( 
	struct Iso_field_calculation_data *data);
/*******************************************************************************
LAST MODIFIED : 24 September 1997

DESCRIPTION :
Gets the enumerated value that identifies the calculation type
==============================================================================*/

FE_value evaluate_Iso_field_clip(struct Iso_field_calculation_data *data,
	int number_of_components,int iso_field_component_number,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 24 September 1997

DESCRIPTION :
Calculates a single value that represents the clipping field at the values given
in values.  If the iso_field_component_number is -1 then all the components are
assumed to be available.  Otherwise just the single component identified by the
iso_field_component number is expected.
==============================================================================*/

int set_Iso_field_calculation(struct Iso_field_calculation_data *data,
	enum Iso_field_calculation_type type);
/*******************************************************************************
LAST MODIFIED : 24 September 1997

DESCRIPTION :
Sets the data structure so that it represents a calculation of the type
identified in the type enum.
==============================================================================*/

int set_Iso_field_calculation_with_floats(
	struct Iso_field_calculation_data *data,enum Iso_field_calculation_type type,
	int number_of_components,float *coeffs);
/*******************************************************************************
LAST MODIFIED : 24 September 1997

DESCRIPTION :
Sets the data structure so that it represents a calculation of the type
identified in the type enum and also sets 'number_of_components' parameters.
==============================================================================*/

FE_value evaluate_Iso_field_calculation(struct Iso_field_calculation_data *data,
	int number_of_components,int iso_field_component_number,FE_value *values);
/*******************************************************************************
LAST MODIFIED : 24 September 1997

DESCRIPTION :
Calculates a single value that represents the field at the values given in
values.  If the iso_field_component_number is -1 then all the components are
assumed to be available.  Otherwise just the single component identified by the
iso_field_component number is expected.
==============================================================================*/
#endif /* !defined (ISO_FIELD_CALCULATION_H) */
