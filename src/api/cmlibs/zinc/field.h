/**
 * @file field.h
 *
 * The public interface to the zinc field base object.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELD_H__
#define CMZN_FIELD_H__

#include "types/differentialoperatorid.h"
#include "types/elementid.h"
#include "types/fieldcacheid.h"
#include "types/fieldid.h"
#include "types/fieldrangeid.h"
#include "types/fieldmoduleid.h"
#include "types/fieldsmoothingid.h"
#include "types/nodeid.h"

#include "cmlibs/zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
Global functions
----------------
*/

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param name  Enumeration name string.
 * @return  Enumeration value or INVALID if not found.
 */
ZINC_API enum cmzn_field_coordinate_system_type
	cmzn_field_coordinate_system_type_enum_from_string(const char *name);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_field_coordinate_system_type_enum_to_string(
	enum cmzn_field_coordinate_system_type type);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param name  Enumeration name string.
 * @return  Enumeration value or INVALID if not found.
 */
ZINC_API enum cmzn_field_domain_type cmzn_field_domain_type_enum_from_string(
	const char *name);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string of the short name of the enum.
 */
ZINC_API char *cmzn_field_domain_type_enum_to_string(
	enum cmzn_field_domain_type type);

/**
 * Get the number of components of the field.
 *
 * @param field  The field to query.
 * @return  The number of components of the field.
 */
ZINC_API int cmzn_field_get_number_of_components(cmzn_field_id field);

/**
 * Returns a new handle to the field with reference count incremented.
 *
 * @param field  The field to obtain a new handle to.
 * @return  New handle to field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_field_access(cmzn_field_id field);

/**
 * Destroys handle to the field (and sets it to NULL).
 * Internally this decrements the reference count.
 *
 * @param field_address  address to the handle to field.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_destroy(cmzn_field_id *field_address);

/**
 * Get managed status of field in its owning field module.
 * @see cmzn_field_set_managed
 *
 * @param field  The field to query.
 * @return  true if field is managed, otherwise false.
 */
ZINC_API bool cmzn_field_is_managed(cmzn_field_id field);

/**
 * Set managed status of field in its owning field module.
 * If set (managed) the field will remain indefinitely in the field module even
 * if no external references are held.
 * If not set (unmanaged) the field will be automatically removed from the
 * module when no longer referenced externally, effectively marking it as
 * pending destruction.
 * All new objects are unmanaged unless stated otherwise.
 *
 * @param field  The field to modify.
 * @param value  The new value for the managed flag: true or false.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_field_set_managed(cmzn_field_id field, bool value);

/**
 * Assign mesh_location field values at location specified in cache. Only
 * supported by stored_mesh_location field type.
 *
 * @param field  The field to assign value to.
 * @param cache  Store of location to assign at and intermediate field values.
 * @param element  The element to set.
 * @param number_of_chart_coordinates  Size of following element local
 * coordinates array. Must be at least the dimension of the element.
 * @param chart_coordinates  Array containing location to set in element's
 * local 'xi' coordinate chart.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_assign_mesh_location(cmzn_field_id field,
	cmzn_fieldcache_id cache, cmzn_element_id element,
	int number_of_chart_coordinates, const double *chart_coordinates);

/**
 * Assign real values to field at location specified in cache.
 * Only supported for some field types, notably finite_element, node_value, and
 * field operators where only one operand is assignable: these back-calculate
 * value of that operand and assign to it (includes types: offset, scale,
 * coordinate_transformation, vector_coordinate_transformation; latter assumes
 * coordinate field is not assignable.)
 * Only supported for some cache locations: node, or anywhere for constants.
 *
 * @param field  The field to assign real values to.
 * @param cache  Store of location to assign at and intermediate field values.
 * @param number_of_values  Size of values array. Checked that it equals or
 * exceeds the number of components of field.
 * @param values  Array of real values to assign to field.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_assign_real(cmzn_field_id field, cmzn_fieldcache_id cache,
	int number_of_values, const double *values);

/**
 * Assign a string value to a field at location specified in cache.
 * Only supported for legacy stored 'finite element' string at node locations,
 * and string_constant at any cache location.
 *
 * @param field  The field to assign a string value to.
 * @param cache  Store of location to assign at and intermediate field values.
 * @param string_value  The string value to assign to field.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_assign_string(cmzn_field_id field, cmzn_fieldcache_id cache,
	const char *string_value);

/**
 * Query if field is a function of another field, meaning it is either the same
 * or the other field is one of its source fields or any further source fields
 * recursively.
 *
 * @param field  The field to query.
 * @param other_field  The field to query for dependence on.
 * @return  true if field depends on other field, otherwise false.
 */
ZINC_API bool cmzn_field_depends_on_field(cmzn_field_id field,
	cmzn_field_id other_field);

/**
 * Evaluate mesh_location field values at location specified in cache.
 *
 * @param field  The field to evaluate.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * @param number_of_chart_coordinates  Size of following element local
 * coordinates array. Must be at least the dimension of the returned element.
 * @param chart_coordinates  Array to receive the location in the element's
 * local 'xi' coordinate chart.
 * @return  Handle to element, or NULL/invalid handle on failure including
 * field not defined at cache location.
 */
ZINC_API cmzn_element_id cmzn_field_evaluate_mesh_location(cmzn_field_id field,
	cmzn_fieldcache_id cache, int number_of_chart_coordinates,
	double *chart_coordinates);

/**
 * Evaluate real field values at location specified in cache.
 *
 * @param field  The field to evaluate.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * @param number_of_values  Size of values array. Checked that it equals or
 * exceeds the number of components of field.
 * @param values  Array of real values to evaluate into.
 * @return  Status CMZN_OK on success, any other value on failure including if
 * field is not defined at cache location.
 */
ZINC_API int cmzn_field_evaluate_real(cmzn_field_id field, cmzn_fieldcache_id cache,
	int number_of_values, double *values);

/**
 * Evaluate field as string at location specified in cache. Numerical valued
 * fields are written to a string with comma separated components.
 * Caller must free returned string with cmzn_deallocate().
 *
 * @param field  The field to evaluate.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * @return  Allocated string value, or NULL on failure including if field is
 * not defined at cache location.
 */
ZINC_API char *cmzn_field_evaluate_string(cmzn_field_id field,
	cmzn_fieldcache_id cache);

/**
 * Evaluate derivatives of a real-valued field.
 * CURRENT LIMITATIONS:
 * 1. Mesh differential operator must be obtained from mesh owning element
 * derivative is evaluated on. Cannot yet evaluate derivatives with respect to
 * parent element chart.
 * 2. Field parameter derivatives can only be evaluated on the top-level
 * element the field is defined on, not on faces or lines inheriting the field.
 * 3. Not implemented for all field operators and cache locations; falls back
 * to approximate finite difference calculation for many cases.
 *
 * @param field  The field to evaluate derivatives for. Must be real valued.
 * @param differential_operator  The differential operator identifying which
 * derivative to evaluate. Can be obtained from mesh or field parameters.
 * @param cache  Store of location to evaluate at and intermediate field
 * values. Except for constants, can only evaluate at element locations.
 * @param number_of_values  Size of values array, must equal number of
 * components of field times the number of derivative terms. For parameter
 * derivatives, the number of terms equals the number of parameters in each
 * element (obtained from field parameters) to the power of the derivative
 * order.
 * @param values  Array of real values to evaluate derivatives into. For
 * multiple terms, output values cycle slowest by components, then by earliest
 * derivative index etc.
 * @return  Result OK on success, any other value on failure including case
 * of field not being defined at cache location.
 */
ZINC_API int cmzn_field_evaluate_derivative(cmzn_field_id field,
	cmzn_differentialoperator_id differential_operator,
	cmzn_fieldcache_id cache, int number_of_values, double *values);

/**
 * Evaluate range of field, currently limited to real-valued fields at
 * element/mesh location.
 * Note this is only designed to work for field functions up to cubic
 * polynomial complexity, and may not be accurate for fields with higher order
 * waviness or extreme distortions.
 *
 * @param field  The field to evaluate range of. Must be real-valued.
 * @param fieldcache  Store of location to evaluate at and intermediate field
 * values. Must be for same region as field. Must have set an element or mesh
 * location in the field cache.
 * @param fieldrange  Field range to store result in. Must have been created
 * from the same fieldcache as supplied here.
 * @return  Result OK on success, ERROR_NOT_FOUND if no range found including
 * field not defined, ERROR_NOT_IMPLEMENTED if not an element location,
 * otherwise ERROR_ARGUMENT including for non-numeric fields.
 */
ZINC_API int cmzn_field_evaluate_fieldrange(cmzn_field_id field,
	cmzn_fieldcache_id fieldcache, cmzn_fieldrange_id fieldrange);

/**
 * Get whether the field is marked as coordinate type i.e. appropriate for
 * giving geometric location of a domain.
 *
 * @param field  The field to query.
 * @return  True if the field is coordinate type, otherwise false.
 */
ZINC_API bool cmzn_field_is_type_coordinate(cmzn_field_id field);

/**
 * Set whether the field is marked as coordinate type i.e. appropriate for
 * giving geometric location of a domain. Can only be set for some fields e.g.
 * finite_element where its default is false. Other fields may be fixed at
 * either state.
 *
 * @param field  The field to modify.
 * @param value  The new state of the coordinate type flag.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_field_set_type_coordinate(cmzn_field_id field, bool value);

/**
 * Get the C++ style class name e.g. "FieldAdd".
 *
 * @param field  The field to query.
 * @return  On success: allocated string containing field class name.
 * Up to caller to free using cmzn_deallocate().
 */
ZINC_API char *cmzn_field_get_class_name(cmzn_field_id field);

/**
 * Return true if field has the exact class name string including case.
 *
 * @param field  The field to query.
 * @param class_name  C++ style class name e.g. "FieldAdd".
 * @return  True if field has exactly matching class name otherwise false.
 */
ZINC_API bool cmzn_field_has_class_name(cmzn_field_id field,
	const char *class_name);

/**
 * Get the name of a component of the field. This is only meaningful for
 * finite element fields; all other fields' component names are the string
 * equivalent of the component number, e.g. 1 -> "1" etc.
 * Also, node_value fields return the base finite element
 * field component names.
 *
 * @param field  The field whose component name is requested.
 * @param component_number  Component number from 1 to number of components.
 * @return  On success: allocated string containing field component name. Up to
 * caller to free using cmzn_deallocate().
 */
ZINC_API char *cmzn_field_get_component_name(cmzn_field_id field,
	int component_number);

/**
 * Set the name of a component of the field. This is only meaningful for
 * finite element fields, which can be serialised.
 *
 * @param field  The field whose component name is requested.
 * @param component_number  Component number from 1 to number of components.
 * @param name  The new name of the field component.
 * @return  On success, CMZN_OK, otherwise any other error code.
 */
ZINC_API int cmzn_field_set_component_name(cmzn_field_id field,
	int component_number, const char *name);

/**
 * Get the coordinate system focus value, used for prolate and oblate
 * spheroidal coordinate system types only.
 *
 * @param field  The field to query.
 * @return  The focus length, or 0.0 if invalid field.
 */
ZINC_API double cmzn_field_get_coordinate_system_focus(cmzn_field_id field);

/**
 * Set the coordinate system focus value, used for prolate and oblate
 * spheroidal coordinate system types only.
 * @see cmzn_field_set_coordinate_system_type
 *
 * @param field  The field to modify.
 * @param focus  The new focus value, > 0.0.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_field_set_coordinate_system_focus(cmzn_field_id field, double focus);

/**
 * Get the coordinate system type to interpret field values in.
 *
 * @param field  The field to query.
 * @return  The type of coordinate system, or INVALID if invalid field.
 */
ZINC_API enum cmzn_field_coordinate_system_type cmzn_field_get_coordinate_system_type(
	cmzn_field_id field);

/**
 * Set the coordinate system type to interpret field values in.
 * Note PROLATE_SPHEROIDAL and OBLATE_SPHEROIDAL coordinate system types also
 * require the focus to be set to a positive value.
 * @see cmzn_field_set_coordinate_system_focus
 *
 * @param field  The field to modify.
 * @param coordinate_system_type  The type of coordinate system. Note that
 * non-numeric value type fields have coordinate system type NOT_APPLICABLE,
 * which cannot be changed.
 * @return  Result OK if successfully set, an error code if failed.
 */
ZINC_API int cmzn_field_set_coordinate_system_type(cmzn_field_id field,
	enum cmzn_field_coordinate_system_type coordinate_system_type);

/**
 * Return the name of the field.
 *
 * @param field  The field whose name is requested.
 * @return  On success: allocated string containing field name. Up to caller to
 * free using cmzn_deallocate().
 */
ZINC_API char *cmzn_field_get_name(cmzn_field_id field);

/**
 * Set the name of the field.
 * Fails if the new name is in use by any other field in the same field module.
 *
 * @param field  The field to be named.
 * @param name  The new name for the field.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_set_name(cmzn_field_id field, const char *name);

/**
 * Get the number of source fields of this field.
 *
 * @param field  The field to query.
 * @return  The number of source fields of the field.
 */
ZINC_API int cmzn_field_get_number_of_source_fields(cmzn_field_id field);

/**
 * Return a source field of this field at a given index. Source fields are in
 * the order presented in the field constructor followed by any optional source
 * fields set by type-specific API.
 *
 * @param field  The field to query.
 * @param index  The index from 1 to number of source fields.
 * @return  Handle to source field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_field_get_source_field(cmzn_field_id field, int index);

/**
 * Returns a handle to the field module which owns this field.
 *
 * @param field  The field to obtain field module for.
 * @return  Handle to owning field module, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_fieldmodule_id cmzn_field_get_fieldmodule(cmzn_field_id field);

/**
 * Gets the type of values produced by the field.
 *
 * @param field  The field to query.
 * @return  Value type produced by field
 */
ZINC_API enum cmzn_field_value_type cmzn_field_get_value_type(cmzn_field_id field);

/**
 * Determines if the field is defined at the location specified in the field
 * cache, and hence can be evaluated there.
 * Note that a finite element field will return false for this function at a
 * node location where it has parameters, but cannot be evaluated due to not
 * having a complete set of VALUE parameters for all components. The finite
 * element field has a special function for querying whether it has parameters
 * at a location.
 * @see cmzn_field_finite_element_has_parameters_at_location
 *
 * @param field  The field to query.
 * @param cache  Store of location to check, and intermediate field values.
 * @return  True if defined, false if not defined or error.
 */
ZINC_API bool cmzn_field_is_defined_at_location(cmzn_field_id field,
	cmzn_fieldcache_id cache);

/**
 * Smooth the field according to the supplied smoothing specification.
 *
 * @param field  The field to modify. Only interpolated finite element fields
 * are supported by this function.
 * @param fieldsmoothing  Specification of smoothing algorithm and parameters.
 * @return  Status CMZN_OK on success, otherwise any other error code.
 */
ZINC_API int cmzn_field_smooth(cmzn_field_id field,
	cmzn_fieldsmoothing_id fieldsmoothing);

/**
 * Returns a new handle to the iterator with reference count incremented.
 *
 * @param iterator  The field iterator to obtain a new handle to.
 * @return  New handle to field iterator, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_fielditerator_id cmzn_fielditerator_access(
	cmzn_fielditerator_id iterator);

/**
 * Destroys this handle to the field iterator and sets it to NULL.
 *
 * @param iterator_address  Address of handle to field_iterator to destroy.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_fielditerator_destroy(cmzn_fielditerator_id *iterator_address);

/**
 * Returns a handle to the next field in the container being iterated over then
 * advances the iterator position. The caller is required to destroy the
 * returned field handle.
 *
 * @param iterator  Field iterator to query and advance.
 * @return  Handle to the next field, or NULL/invalid handle if none or failed.
 */
ZINC_API cmzn_field_id cmzn_fielditerator_next(cmzn_fielditerator_id iterator);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_FIELD_H__ */
