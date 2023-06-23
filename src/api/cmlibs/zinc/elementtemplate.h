/**
 * @file elementtemplate.h
 *
 * The public interface to Zinc element templates.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_ELEMENTTEMPLATE_H__
#define CMZN_ELEMENTTEMPLATE_H__

#include "types/elementid.h"
#include "types/elementbasisid.h"
#include "types/elementfieldtemplateid.h"
#include "types/elementtemplateid.h"
#include "types/fieldid.h"
#include "types/nodeid.h"

#include "cmlibs/zinc/zincsharedobject.h"

/*
Global types
------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
Global functions
----------------
*/

/**
 * Returns a new handle to the element template with reference count incremented.
 *
 * @param elementtemplate  The element template to obtain a new handle to.
 * @return  New handle to element template, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_elementtemplate_id cmzn_elementtemplate_access(
	cmzn_elementtemplate_id elementtemplate);

/**
 * Destroys this handle to the element template and sets it to NULL.
 * Internally this decrements the reference count.
 *
 * @param element_template_address  Address of handle to element_template
 * to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_elementtemplate_destroy(
	cmzn_elementtemplate_id *element_template_address);

/**
 * Get the current element shape type set in the element template.
 *
 * @param element_template  Element template to query.
 * @return  The element shape type, or INVALID if not set or error.
 */
ZINC_API enum cmzn_element_shape_type cmzn_elementtemplate_get_element_shape_type(
	cmzn_elementtemplate_id element_template);

/**
 * Set the element shape to a standard element shape type. The shape must have
 * the same dimension as the mesh from which the element template was created.
 * Beware that face mappings are lost if shape changes are merged into elements.
 *
 * @param element_template  Element template to modify.
 * @param shape_type  Standard element shapes enumerated value. Note can be
 * INVALID which means the shape is not set when merged into an element, but
 * new elements cannot be created unless they have a valid shape.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_elementtemplate_set_element_shape_type(
	cmzn_elementtemplate_id element_template, enum cmzn_element_shape_type shape_type);

/**
 * Define the field component(s) on the element template using the
 * element field template. The element template is not valid until
 * all components are defined for all fields.
 *
 * @param elementtemplate  Element template to modify.
 * @param field  The field to define. May be finite element type only.
 * @param component_number  The component to define from 1 to the number of
 * field components, or -1 to define all components.
 * @param eft  The element field template. Must be for mesh this element
 * template was created from.
 * @return  Result OK on success, any other value on failure.
 */
ZINC_API int cmzn_elementtemplate_define_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field,
	int component_number, cmzn_elementfieldtemplate_id eft);

/**
 * Removes field from list of fields to define or undefine in element template.
 *
 * @param elementtemplate  Element template to modify.
 * @param field  The field to remove. May be finite element type only.
 * @return  Result OK on success, any other value on failure.
 */
ZINC_API int cmzn_elementtemplate_remove_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field);

/**
 * Marks field to be undefined when next merged into an existing element. Has
 * no effect on newly created elements. Removes field from define list if
 * present.
 *
 * @param elementtemplate  Element template to modify.
 * @param field  The field to undefine. May be finite element type only.
 * @return  Result OK on success, any other value on failure.
 */
ZINC_API int cmzn_elementtemplate_undefine_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_ELEMENTTEMPLATE_H__ */
