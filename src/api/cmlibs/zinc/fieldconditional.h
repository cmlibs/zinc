/**
 * @file fieldconditional.h
 *
 * Implements zinc fields which conditionally calculate their inputs.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDCONDITIONAL_H__
#define CMZN_FIELDCONDITIONAL_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "cmlibs/zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a conditional field returning component values from source field two
 * if the condition source field one is true, otherwise source field three.
 * The field has the value type and number of components from source fields two
 * and three, which must match. Source fields two and three of mesh location
 * value type must have matching host mesh.
 * For real value type, the condition field may be a vector of same length as
 * the other two source fields, in which case the condition is applied per-
 * component: a non-zero/true component gives the corresponding component value
 * from source field two, zero/false gives the value from source field three.
 *
 * @param fieldmodule  Region field module which will own new field.
 * @param source_field_one  Condition field.
 * @param source_field_two  Field components returned on true condition.
 * Must have same value type and number of components as source_field_three.
 * @param source_field_three  Field components returned on false condition.
 * Must have same value type and number of components as source_field_two.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_if(cmzn_fieldmodule_id fieldmodule,
	cmzn_field_id source_field_one,
	cmzn_field_id source_field_two,
	cmzn_field_id source_field_three);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CMZN_FIELDCONDITIONAL_H */
