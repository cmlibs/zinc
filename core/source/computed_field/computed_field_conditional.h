/*******************************************************************************
FILE : computed_field_conditional.h

LAST MODIFIED : 27 July 2007

DESCRIPTION :
Implements computed fields which conditionally calculate their inputs.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_CONDITIONAL_H)
#define COMPUTED_FIELD_CONDITIONAL_H

#include "general/value.h"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldconditional.h"

#define Computed_field_create_if cmzn_fieldmodule_create_field_if

/*****************************************************************************//**
 * Creates a conditional field with the same number of components as each of the
 * source_fields. For each component, if the value of source_field_one is TRUE
 * (non-zero) then the result will be the value of source_field_two, otherwise the
 * component result will be taken from source_field_three.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  Conditional field.
 * @param source_field_two  TRUE = non-zero conditional component results.
 * @param source_field_three  FALSE = zero conditional component results.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_if(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two,
	struct Computed_field *source_field_three);

#endif /* !defined (COMPUTED_FIELD_CONDITIONAL_H) */
