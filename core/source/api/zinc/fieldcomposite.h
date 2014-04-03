/**
 * @file fieldcomposite.h
 *
 * Public interface to the field operators that rearrange components of other
 * fields.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDCOMPOSITE_H__
#define CMZN_FIELDCOMPOSITE_H__

#include "types/fieldid.h"
#include "types/fieldcompositeid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a field with the single source field.  This field is useful
 * as a placeholder candidate for replacement with more complicated operations
 * later on.
 * Internally this a composite field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field The field the values are copied from.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_identity(cmzn_fieldmodule_id field_module,
	cmzn_field_id source_field);

/**
 * Creates a field returning the component of the source field with the given
 * component_index, starting at 1.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  The field the component value is copied from.
 * @param component_index  The component index from 1 to number of components.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_component(cmzn_fieldmodule_id field_module,
	cmzn_field_id source_field, int component_index);

/**
 * Creates a field which concatenates the components of all source fields, in
 * order, into a single vector.
 *
 * @param field_module  Region field module which will own new field.
 * @param number_of_source_fields  The number of source fields in the array.
 * @param source_fields  The array of fields to be concatenating together.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_concatenate(cmzn_fieldmodule_id field_module,
	int number_of_source_fields, cmzn_field_id *source_fields);

/**
 * If the field is of component type, then this function returns the derived
 * component field handle.
 *
 * @param field  The generic field to be cast.
 * @return  Handle to derived component field, or NULL/invalid handle if wrong type or failed.
 */
ZINC_API cmzn_field_component_id cmzn_field_cast_component(cmzn_field_id field);

/**
 * Cast component field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the component argument.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_component_base_cast(component_field), "bob");
 *
 * @param component  Handle to the component field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_component_base_cast(cmzn_field_component_id component)
{
	return (cmzn_field_id)(component);
}

/**
 * Get the component number of the source field returned by this field.
 *
 * @param component  Handle to component field to query.
 * @return  The component number from 1 to number of components, or 0 on error.
 */
ZINC_API int cmzn_field_component_get_component_index(cmzn_field_component_id component);

/**
 * Modify the field to return the component with the given index of the source field.
 *
 * @param component  Handle to component field to modify.
 * @param component_index  The component index from 1 to number of components.
 * @return  Status CMZN_OK if component index is successfully set, any other value on failure.
 */
ZINC_API int cmzn_field_component_set_component_index(cmzn_field_component_id component,
	int component_index);

/**
 * Destroys handle to the component field (and sets it to NULL).
 * Internally this decrements the reference count.
 *
 * @param component_address  Address of handle to the component field.
 * @return  Status CMZN_OK if successfully destroyed the component handle,
 * 		any other value on failure.
 */
ZINC_API int cmzn_field_component_destroy(cmzn_field_component_id *component_address);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_FIELDCOMPOSITE_H__ */
