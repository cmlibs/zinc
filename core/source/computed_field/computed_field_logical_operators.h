/*******************************************************************************
FILE : computed_field_logical_operators.h

LAST MODIFIED : 15 May 2008

DESCRIPTION :
Implements logical operations on computed fields.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_LOGICAL_OPERATORS_H)
#define COMPUTED_FIELD_LOGICAL_OPERATORS_H

#include "zinc/field.h"
#include "zinc/fieldlogicaloperators.h"

#define Computed_field_create_greater_than cmzn_fieldmodule_create_field_greater_than
#define Computed_field_create_less_than cmzn_fieldmodule_create_field_less_than

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
	struct cmzn_fieldmodule *field_module,
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
	struct cmzn_fieldmodule *field_module,
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
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);

#endif /* !defined (COMPUTED_FIELD_LOGICAL_OPERATORS_H) */
