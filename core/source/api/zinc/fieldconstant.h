/*****************************************************************************//**
FILE : fieldconstant.h
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
#ifndef CMZN_FIELDCONSTANT_H__
#define CMZN_FIELDCONSTANT_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************//**
 * Creates a field with the components specified in the array values.
 * Internally this a composite field.
 *
 * @param field_module  Region field module which will own new field.
 * @param number_of_values  The number of values in the array.
 * @param values The array of constant values
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_constant(cmzn_field_module_id field_module,
	int number_of_values, const double *values);

/*****************************************************************************//**
 * Creates a string constant field with the supplied
 * string value in <string_constant>.
 *
 * @param field_module  Region field module which will own new field.
 * @param string_constant The constant char string.
 * @return Newly created field.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_string_constant(cmzn_field_module_id field_module,
	const char *string_constant);

#ifdef __cplusplus
}
#endif

#endif // CMZN_FIELDCONSTANT_H
