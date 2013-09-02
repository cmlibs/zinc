/*******************************************************************************
FILE : computed_field_logical_operators.h

LAST MODIFIED : 15 May 2008

DESCRIPTION :
Implements logical operations on computed fields.
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
#if !defined (COMPUTED_FIELD_LOGICAL_OPERATORS_H)
#define COMPUTED_FIELD_LOGICAL_OPERATORS_H

#include "zinc/field.h"
#include "zinc/fieldlogicaloperators.h"

#define Computed_field_create_greater_than cmzn_field_module_create_greater_than
#define Computed_field_create_less_than cmzn_field_module_create_less_than

/***************************************************************************//**
 * Creates a field whose component values are 1 if that component of
 * source_field_one is less than the component value in source_field_two.
 * Automatic scalar broadcast will apply, see cmiss_field.h.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one First input field
 * @param source_field_two Second input field
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_less_than(
	struct cmzn_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);

/***************************************************************************//**
 * Creates a field whose component values are 1 if that component of
 * source_field_one is greater than the component value in source_field_two.
 * Automatic scalar broadcast will apply, see cmiss_field.h.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one First input field
 * @param source_field_two Second input field
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_greater_than(
	struct cmzn_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);

/***************************************************************************//**
 * Creates a scalar field whose value is 1 wherever the source field is defined,
 * and 0 elsewhere (without error).
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field to check whether defined.
 * @return Newly created field
 */
Computed_field *Computed_field_create_is_defined(
	struct cmzn_field_module *field_module,
	struct Computed_field *source_field);

#endif /* !defined (COMPUTED_FIELD_LOGICAL_OPERATORS_H) */
