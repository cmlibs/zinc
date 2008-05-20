/*******************************************************************************
FILE : cmiss_field_arithmetic_operators.h

LAST MODIFIED : 13 May 2008

DESCRIPTION :
The public interface to the Cmiss_fields that perform arithmetic operators.
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
 * Portions created by the Initial Developer are Copyright (C) 2008
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
#ifndef __CMISS_FIELD_ARITHMETIC_OPERATORS_H__
#define __CMISS_FIELD_ARITHMETIC_OPERATORS_H__

Cmiss_field_id Cmiss_field_create_power(
	Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_POWER with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

Cmiss_field_id Cmiss_field_create_multiply(
	Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 14 May 2008

DESCRIPTION :
Creates a field of type COMPUTED_FIELD_MULTIPLY with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

Cmiss_field_id Cmiss_field_create_divide(
	Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVIDE with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

Cmiss_field_id Cmiss_field_create_add(
	Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

Cmiss_field_id Cmiss_field_create_subtract(
	Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
Automatic scalar broadcast will apply, see cmiss_field.h.
==============================================================================*/

Cmiss_field_id Cmiss_field_create_sum_components(
	Cmiss_field_id source_field, FE_value *weights);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SUM_COMPONENTS with the supplied which
returns a scalar weighted sum of the components of <source_field>.
The <weights> array must therefore contain as many FE_values as there are
components in <source_field>.
==============================================================================*/

Cmiss_field_id Cmiss_field_create_log(
	Cmiss_field_id source_field);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LOG with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

Cmiss_field_id Cmiss_field_create_sqrt(
	Cmiss_field_id source_field);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SQRT with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

Cmiss_field_id Cmiss_field_create_exp(
	Cmiss_field_id source_field);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXP with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/

#endif /* __CMISS_FIELD_ARITHMETIC_OPERATORS_H__ */
