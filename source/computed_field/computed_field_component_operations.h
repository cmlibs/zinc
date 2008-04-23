/*******************************************************************************
FILE : computed_field_component_operations.h

LAST MODIFIED : 13 July 2000

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
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
#if !defined (COMPUTED_FIELD_COMPONENT_OPERATIONS_H)
#define COMPUTED_FIELD_COMPONENT_OPERATIONS_H

#include "general/value.h"
#include "api/cmiss_computed_field.h"

/* API functions are prefixed with Cmiss */
#define Computed_field_set_type_sum_components \
	Cmiss_computed_field_set_type_sum_components
#define Computed_field_get_type_sum_components \
	Cmiss_computed_field_get_type_sum_components
#define Computed_field_set_type_add Cmiss_computed_field_set_type_add
#define Computed_field_set_type_subtract Cmiss_computed_field_set_type_subtract
#define Computed_field_set_type_weighted_add Cmiss_computed_field_set_type_weighted_add
#define Computed_field_set_type_multiply_components Cmiss_computed_field_set_type_multiply
#define Computed_field_set_type_divide_components Cmiss_computed_field_set_type_divide
#define Computed_field_set_type_exp Cmiss_computed_field_set_type_exp
#define Computed_field_set_type_log Cmiss_computed_field_set_type_log
#define Computed_field_set_type_power Cmiss_computed_field_set_type_power
#define Computed_field_set_type_sqrt Cmiss_computed_field_set_type_sqrt

int Computed_field_register_types_component_operations(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_add(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Computed_field_set_type_subtract(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD (with a -1 weighting for the
second field) with the supplied fields, <source_field_one> and 
<source_field_two>.  Sets the number of components equal to the source_fields.
==============================================================================*/

int Computed_field_set_type_weighted_add(struct Computed_field *field,
	struct Computed_field *source_field_one, double scale_factor1,
	struct Computed_field *source_field_two, double scale_factor2);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Computed_field_set_type_divide_components(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVIDE_COMPONENTS with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Computed_field_set_type_offset(struct Computed_field *field,
	struct Computed_field *source_field, FE_value *offsets);
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_OFFSET which returns the values of the
<source_field> plus the <offsets>.
The <offsets> array must therefore contain as many FE_values as there are
components in <source_field>; this is the number of components in the field.
==============================================================================*/

int Computed_field_get_type_sum_components(struct Computed_field *field,
	struct Computed_field **source_field, FE_value **weights);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SUM_COMPONENTS, the 
<source_field> and <weights> used by it are returned.
==============================================================================*/

int Computed_field_set_type_sum_components(struct Computed_field *field,
	struct Computed_field *source_field, FE_value *weights);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SUM_COMPONENTS with the supplied which
returns a scalar weighted sum of the components of <source_field>.
The <weights> array must therefore contain as many FE_values as there are
components in <source_field>.
==============================================================================*/

int Computed_field_set_type_multiply_components(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MULTIPLY_COMPONENTS with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Computed_field_set_type_divide_components(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVIDE_COMPONENTS with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
==============================================================================*/

int Computed_field_set_type_exp(struct Computed_field *field,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXP with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

int Computed_field_set_type_log(struct Computed_field *field,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LOG with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

int Computed_field_set_type_power(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_POWER with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
For each component the result is source_field_one to the power of 
source_field_two.
==============================================================================*/

int Computed_field_set_type_sqrt(struct Computed_field *field,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SQRT with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_COMPONENT_OPERATIONS_H) */
