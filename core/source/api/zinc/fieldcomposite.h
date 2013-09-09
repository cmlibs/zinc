/*******************************************************************************
FILE : fieldcomposite.h

LAST MODIFIED : 13 May 2008

DESCRIPTION :
The public interface to the cmzn_fields that perform arithmetic operations.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDCOMPOSITE_H__
#define CMZN_FIELDCOMPOSITE_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************//**
 * Creates a field with the single source field.  This field is useful
 * as a placeholder candidate for replacement with more complicated operations
 * later on.
 * Internally this a composite field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field The field the values are copied from.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_identity(cmzn_field_module_id field_module,
	cmzn_field_id source_field);

/*****************************************************************************//**
 * Creates a field returning the component of the source field with the given
 * component_index, starting at 1.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  The field the component value is copied from.
 * @param component_index  The component index from 1 to number of components.
 * @return  Newly created field.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_component(cmzn_field_module_id field_module,
	cmzn_field_id source_field, int component_index);

/*****************************************************************************//**
 * Creates a field which concatenates the components of all source fields, in
 * order, into a single vector.
 *
 * @param field_module  Region field module which will own new field.
 * @param number_of_source_fields  The number of source fields in the array.
 * @param source_fields  The array of fields to be concatenating together.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_concatenate(cmzn_field_module_id field_module,
	int number_of_source_fields, cmzn_field_id *source_fields);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_FIELDCOMPOSITE_H__ */
