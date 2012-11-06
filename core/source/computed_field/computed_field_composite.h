/*******************************************************************************
FILE : computed_field_composite.h

LAST MODIFIED : 15 January 2002

DESCRIPTION :
Implements a "composite" computed_field which converts fields, field components
and real values in any order into a single vector field.
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
#if !defined (COMPUTED_FIELD_COMPOSITE_H)
#define COMPUTED_FIELD_COMPOSITE_H

#include "zinc/field.h"
#include "zinc/fieldconstant.h"
#include "zinc/fieldcomposite.h"

#define Computed_field_create_constant Cmiss_field_module_create_constant
#define Computed_field_create_identity Cmiss_field_module_create_identity
#define Computed_field_create_component Cmiss_field_module_create_component
#define Computed_field_create_concatenate Cmiss_field_module_create_concatenate

struct Computed_field_composite_source_data
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Data structure filled by set_Computed_field_composite_source_data.
==============================================================================*/
{
	int number_of_components;
	int number_of_source_fields;
	struct Computed_field **source_fields;
	int number_of_source_values;
	double *source_values;
	int *source_field_numbers;
	int *source_value_numbers;
}; /* struct Computed_field_composite_source_data */

/*******************************************************************************
 * Dangerous: only used for minimise; GRC would like to remove.
 * @param field  A constant field i.e. composite type with only values.
 * @return  Pointer to field source values, or NULL if none or failed.
 */
FE_value *Computed_field_constant_get_values_storage(struct Computed_field *field);

int Computed_field_is_constant(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Returns true if the field is of type composite but with no source_fields, only
source_values.
==============================================================================*/

int Computed_field_is_constant_scalar(struct Computed_field *field,
	FE_value scalar);
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns true if the field is of type composite but with no source_fields, only
a single source value, equal to <scalar>.
==============================================================================*/

struct Computed_field *Computed_field_manager_get_component_wrapper(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Computed_field *field, int component_number);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Returns a composite field that computes <component_number> of <field>. First
tries to find one in the manager that does this, otherwise makes one of name
'field.component_name', adds it to the manager and returns it.
Returned field is ACCESSed once.
==============================================================================*/

struct Computed_field *Computed_field_create_constant(
	struct Cmiss_field_module *field_module,
	int number_of_values, const double *values);
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Creates a constructor for COMPUTED_FIELD_COMPOSITE which has constant
components of the <number_of_values> listed in <values> array.
Since a constant field performs a subset of the capabilities of the composite
field type but the latter is somewhat complicated to set up, this is a
convenience function for building a composite field which has <number_of_values>
<values>.
==============================================================================*/

struct Computed_field *Computed_field_create_identity(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 13 May 2008

DESCRIPTION :
Creates a constructor for COMPOSITE with one input field, the <source_field>.
==============================================================================*/

struct Computed_field *Cmiss_field_module_create_component(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, int component_number);

/****************************************************************************//*
 * Creates a composite field which returns a combination of field components and
 * constants in a given order.
 * The <number_of_source_fields> <source_fields> may be zero if the field is
 * purely constant, but fields may not be repeated.
 * The <number_of_source_values> <source_values> may similarly be omitted.
 * The <source_field_numbers> and <source_value_numbers> must each contain
 * <number_of_components> integers. For each component the <source_field_number>
 * says which <source_field> to use, and the <source_value_number> specifies the
 * component number. If the <source_field_number> is -1 for any component, the
 * <source_value_number> is the offset into the <source_values>.
 *
 * Note: rigorous checking is performed to ensure that no values are invalid and
 * that the <source_fields> are presented in the order they are first used in the
 * <source_field_numbers>, and also that <source_values> are used in the order
 * they are given and that none are used more than once.
 *
 * Some of these restrictions are enforced to ensure type-specific contents can
 * be compared easily -- otherwise there would be more than one way to describe
 * the same source data.
 */
struct Computed_field *Computed_field_create_composite(
	struct Cmiss_field_module *field_module,
	int number_of_components,
	int number_of_source_fields, struct Computed_field **source_fields,
	int number_of_source_values, const double *source_values,
	const int *source_field_numbers, const int *source_value_numbers);

/*****************************************************************************//**
 * Creates a field which concatenates the components of all source fields, in
 * order, into a single vector.
 *
 * @param field_module  Region field module which will own new field.
 * @param number_of_source_fields  The number of source fields in the array.
 * @param source_fields  The array of fields to be concatenating together.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_concatenate(
	struct Cmiss_field_module *field_module,
	int number_of_source_fields, struct Computed_field **source_fields);

#endif /* !defined (COMPUTED_FIELD_COMPOSITE_H) */
