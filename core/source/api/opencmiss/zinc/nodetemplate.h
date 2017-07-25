/**
 * @file nodetemplate.h
 *
 * The public interface to node template, used for describing
 * node fields for creating new nodes and merging into existing nodes.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_NODETEMPLATE_H__
#define CMZN_NODETEMPLATE_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/nodetemplateid.h"
#include "types/timesequenceid.h"

#include "opencmiss/zinc/zincsharedobject.h"

/*
Global functions
----------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new handle to the node template with reference count incremented.
 *
 * @param node_template  Handle to a node template.
 * @return  New handle to the node template, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_nodetemplate_id cmzn_nodetemplate_access(
	cmzn_nodetemplate_id node_template);

/**
 * Destroys this handle to the node_template and sets it to NULL.
 * Internally this decrements the reference count.
 *
 * @param node_template_address  Address of handle to node_template
 * to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodetemplate_destroy(cmzn_nodetemplate_id *node_template_address);

/**
 * Defines the field on the node_template with just a single node value per
 * field component with no time variation. Replaces any existing definition
 * (or undefine state) of the field in the template.
 * Per-component derivatives and multiple versions can be added subsequently.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define. May be finite_element, stored_string or
 * stored_mesh_location type only.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodetemplate_define_field(cmzn_nodetemplate_id node_template,
	cmzn_field_id field);

/**
 * Defines the field on the node template based on its definition in the
 * supplied node. Replaces any existing definition (or undefine state) in the
 * template. If the field is not defined on the node it is removed as a
 * defined/undefined field in the template.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define. May be finite_element, stored_string or
 * stored_mesh_location type only.
 * @param node  The node to obtain the field definition from.
 * @return  Status CMZN_OK on success, CMZN_ERROR_NOT_FOUND if field was not
 * defined on node, and any other value on failure.
 */
ZINC_API int cmzn_nodetemplate_define_field_from_node(
	cmzn_nodetemplate_id node_template, cmzn_field_id field,
	cmzn_node_id node);

/**
 * Gets the number of value/derivative parameters under a given node value label
 * for a field component in the node template.
 *
 * @param node_template  Node template to query.
 * @param field  The field to query value/derivative parameters for. Must be
 * finite_element type only.
 * @param component_number  The component from 1 to the number of field
 * components, or -1 to get maximum number of versions in any component.
 * @param node_value_label  The label of the node value/derivative to query.
 * @return  Number of versions value label in component of field, or maximum in
 * any component if component_number is -1. Valid return value can be 0.
 * Returns 0 if field not defined or invalid arguments are supplied.
 */
ZINC_API int cmzn_nodetemplate_get_value_number_of_versions(
	cmzn_nodetemplate_id node_template, cmzn_field_id field, int component_number,
	enum cmzn_node_value_label node_value_label);

/**
 * Sets the number of value/derivative parameters under a given node value label
 * for a field component in the node template.
 * Must have first called cmzn_nodetemplate_define_field() for field.
 * Limitation: Zinc is currently limited to having either 0 versions for a
 * derivative label (but at least 1 for the VALUE), or the same positive
 * number of versions for all value/derivative perameters.
 * Callers should assume this will be fixed in a future release, and therefore
 * set the number of versions independently for each value/derivative.
 * For now the largest number of versions set for any value/derivative will be
 * the amount of storage set aside per-field-component, and the field will need
 * to be undefined then re-defined to reduce the number of versions.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define value/derivative parameters for. Must be
 * finite_element type only.
 * @param component_number  The component from 1 to the number of field
 * components, or -1 to define identically for all components.
 * @param node_value_label  The label of the node value/derivative to define.
 * @param number_of_versions  The number of versions of the value to define.
 * Can be 0 to request no parameters be stored, however the storage allocated
 * may be greater.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodetemplate_set_value_number_of_versions(
	cmzn_nodetemplate_id node_template, cmzn_field_id field, int component_number,
	enum cmzn_node_value_label node_value_label, int number_of_versions);

/**
 * Returns the time sequence defined for field in node_template, if any.
 *
 * @param node_template  Node template to query.
 * @param field  The field to get time sequence for. May be finite_element
 * type only.
 * @return  Handle to time sequence, or NULL/invalid handle if none set or failed.
 */
ZINC_API cmzn_timesequence_id cmzn_nodetemplate_get_timesequence(
	cmzn_nodetemplate_id node_template, cmzn_field_id field);

/**
 * Defines variation of all nodal values/derivatives * versions with the
 * supplied time sequence for all components of the field in the node template.
 * Hence there will be as many of each parameter as times in the time sequence.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define versions for. May be finite_element type
 * only.
 * @param timesequence  Time sequence object defining the number of times for
 * which field parameters are stored, and the times they are for (increasing).
 * @param node_field_creator  Optionally defines different versions and/or
 * derivative types. If it is NULL then a single nodal value for each component
 * will be defined.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodetemplate_set_timesequence(
	cmzn_nodetemplate_id node_template, cmzn_field_id field,
	struct cmzn_timesequence *timesequence);

/**
 * Removes field from list of fields to define or undefine in node template.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to remove.
 * @return  Result OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodetemplate_remove_field(cmzn_nodetemplate_id node_template,
	cmzn_field_id field);

/**
 * Marks field to be undefined when next merged into an existing node. Has no
 * effect on newly created nodes. Removes field from define list if present.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to undefine. May be finite_element, stored_string or
 * stored_mesh_location type only.
 * @return  Result OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodetemplate_undefine_field(cmzn_nodetemplate_id node_template,
	cmzn_field_id field);

#ifdef __cplusplus
}
#endif

#endif // CMZN_NODETEMPLATE_H__
