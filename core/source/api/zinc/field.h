/***************************************************************************//**
 * FILE : cmiss_field.h
 *
 * The public interface to the Cmiss field base object.
 */
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
#ifndef __CMISS_FIELD_H__
#define __CMISS_FIELD_H__

#include "types/differentialoperatorid.h"
#include "types/elementid.h"
#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/nodeid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Labels of field attributes which may be set or obtained using generic
 * get/set_attribute functions.
 */
enum Cmiss_field_attribute
{
	CMISS_FIELD_ATTRIBUTE_INVALID = 0,
	CMISS_FIELD_ATTRIBUTE_IS_MANAGED = 1,
	/*!< Boolean as integer, when 0 (default) field is destroyed when no longer
	 * in use, i.e. when number of external references to it, including use as a
	 * source for other fields, drops to zero. Set to 1 to manage field
	 * indefinitely, or until this attribute is reset to zero (which effectively
	 * marks a formerly managed field as pending destruction).
	 */
	CMISS_FIELD_ATTRIBUTE_IS_COORDINATE = 2,
	/*!< Boolean as integer, set to 1 (true) if field can be interpreted as a
	 * "coordinate" field, i.e. suitable for supplying coordinates for graphics
	 * and other operations. Can only be set for some fields e.g. finite_element
	 * where its default is 0 (false). Some fields e.g. cad geometry have this
	 * attribute fixed at 1; the majority of other fields have it fixed at 0.
	 */
	CMISS_FIELD_ATTRIBUTE_NUMBER_OF_COMPONENTS = 3,
	/*!< Integer number of components of field.
	 */
	CMISS_FIELD_ATTRIBUTE_NUMBER_OF_SOURCE_FIELDS = 4,
	/*!< Integer number of source fields the field is a function of.
	 */
	CMISS_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS = 5,
	/*!< Real focus parameter for coordinate system types PROLATE_SPHEROIDAL and
	 * OBLATE_SPHEROIDAL. Must be positive.
	 */
};

/***************************************************************************//**
 * Field attribute describing the type of space that its values are to be
 * interpreted in. Although it is usually set for all fields (default is
 * rectangular cartesian, RC), the attribute is only relevant when field is
 * used to supply coordinates or vector values, e.g. to graphics, where it
 * prompts automatic conversion to the underlying RC coordinate system.
 */
enum Cmiss_field_coordinate_system_type
{
	CMISS_FIELD_COORDINATE_SYSTEM_TYPE_INVALID = 0,
	CMISS_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN = 1,
	CMISS_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR = 2,
	CMISS_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR = 3,
	CMISS_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL = 4,
		/*!< uses CMISS_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS */
	CMISS_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL = 5,
		/*!< uses CMISS_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS */
	CMISS_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE = 6,
		/*!< For Euler angles specifying fibre axes orientation from default
		 * aligned with element xi coordinates. */
};

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_field_attribute Cmiss_field_attribute_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_field_attribute_enum_to_string(enum Cmiss_field_attribute attribute);

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_field_coordinate_system_type
	Cmiss_field_coordinate_system_type_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_field_coordinate_system_type_enum_to_string(
	enum Cmiss_field_coordinate_system_type coordinate_system_type);

/***************************************************************************//**
 * Get the number of components of the field.
 *
 * @param field  The field to query.
 * @return  The number of components of the field.
 */
ZINC_API int Cmiss_field_get_number_of_components(Cmiss_field_id field);

/***************************************************************************//**
 * Returns a new reference to the field with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param field  The field to obtain a new reference to.
 * @return  New field reference with incremented reference count.
 */
ZINC_API Cmiss_field_id Cmiss_field_access(Cmiss_field_id field);

/***************************************************************************//**
 * Destroys this reference to the field (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param field_address  address to the handle to field.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_destroy(Cmiss_field_id *field_address);

/***************************************************************************//**
 * Assign mesh_location field values at location specified in cache. Only
 * supported by stored_mesh_location field type.
 *
 * @param field  The field to assign value to.
 * @param cache  Store of location to assign at and intermediate field values.
 * @param element  The element to set.
 * @param number_of_chart_coordinates  Size of chart_coordinates array. Checked
 * that it equals or exceeds the dimension of the element.
 * @param chart_coordinates  Array containing chart coordinate location to set.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_assign_mesh_location(Cmiss_field_id field,
	Cmiss_field_cache_id cache, Cmiss_element_id element,
	int number_of_chart_coordinates, const double *chart_coordinates);

/***************************************************************************//**
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
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_assign_real(Cmiss_field_id field, Cmiss_field_cache_id cache,
	int number_of_values, const double *values);

/***************************************************************************//**
 * Assign a string value to a field at location specified in cache.
 * Only supported for legacy stored 'finite element' string at node locations,
 * and string_constant at any cache location.
 *
 * @param field  The field to assign a string value to.
 * @param cache  Store of location to assign at and intermediate field values.
 * @param string_value  The string value to assign to field.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_assign_string(Cmiss_field_id field, Cmiss_field_cache_id cache,
	const char *string_value);

/***************************************************************************//**
 * Evaluate mesh_location field values at location specified in cache.
 *
 * @param field  The field to evaluate.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * @param number_of_chart_coordinates  Size of chart_coordinates array. Checked
 * that it equals or exceeds the dimension of the returned element.
 * @param chart_coordinates  Array to evaluate chart coordinate location into.
 * @return  Handle to element on success, NULL on failure including if field is
 * not defined at cache location. Caller is responsible for destroying handle.
 */
ZINC_API Cmiss_element_id Cmiss_field_evaluate_mesh_location(Cmiss_field_id field,
	Cmiss_field_cache_id cache, int number_of_chart_coordinates,
	double *chart_coordinates);

/***************************************************************************//**
 * Evaluate real field values at location specified in cache.
 *
 * @param field  The field to evaluate.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * @param number_of_values  Size of values array. Checked that it equals or
 * exceeds the number of components of field.
 * @param values  Array of real values to evaluate into.
 * @return  Status CMISS_OK on success, any other value on failure including if
 * field is not defined at cache location.
 */
ZINC_API int Cmiss_field_evaluate_real(Cmiss_field_id field, Cmiss_field_cache_id cache,
	int number_of_values, double *values);

/***************************************************************************//**
 * Evaluate field as string at location specified in cache. Numerical valued
 * fields are written to a string with comma separated components.
 * Caller must free returned string with Cmiss_deallocate().
 *
 * @param field  The field to evaluate.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * @return  Allocated string value, or NULL on failure including if field is
 * not defined at cache location.
 */
ZINC_API char *Cmiss_field_evaluate_string(Cmiss_field_id field,
	Cmiss_field_cache_id cache);

/***************************************************************************//**
 * Evaluate derivatives of a real-valued field.
 * CURRENT LIMITATIONS:
 * 1. Can only evaluate at an element location.
 * 2. Differential operator must be obtained from mesh owning element. It is not
 * yet possible to evaluate derivatives with respect to parent element chart.
 * NOTE:
 * It is currently more efficient to evaluate derivatives before field values
 * since values are cached simultaneously.
 *
 * @param field  The field to evaluate derivatives for. Must be real valued.
 * @param differential_operator  The differential operator identifying which
 * derivative to evaluate. Currently must be obtained from mesh owning element
 * from element location in cache.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * Only element locations are supported by this function.
 * @param number_of_values  Size of values array, must equal number of
 * components of field.
 * @param values  Array of real values to evaluate derivatives into.
 * @return  Status CMISS_OK on success, any other value on failure including
 * if field is not defined at cache location.
 */
ZINC_API int Cmiss_field_evaluate_derivative(Cmiss_field_id field,
	Cmiss_differential_operator_id differential_operator,
	Cmiss_field_cache_id cache, int number_of_values, double *values);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the field.
 *
 * @param field  The field to query.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
ZINC_API int Cmiss_field_get_attribute_integer(Cmiss_field_id field,
	enum Cmiss_field_attribute attribute);

/***************************************************************************//**
 * Set an integer or Boolean attribute of the field.
 *
 * @param field  The field to set the attribute for.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid for this field.
 */
ZINC_API int Cmiss_field_set_attribute_integer(Cmiss_field_id field,
	enum Cmiss_field_attribute attribute, int value);

/***************************************************************************//**
 * Get a scalar real attribute of the field.
 *
 * @param field  The field to query.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute, or 0.0 if invalid or error.
 */
ZINC_API double Cmiss_field_get_attribute_real(Cmiss_field_id field,
	enum Cmiss_field_attribute attribute);

/***************************************************************************//**
 * Set a scalar real attribute of the field.
 *
 * @param field  The field to set the attribute for.
 * @param attribute  The identifier of the real attribute to set.
 * @param value  The new value for the attribute.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid for this field.
 */
ZINC_API int Cmiss_field_set_attribute_real(Cmiss_field_id field,
	enum Cmiss_field_attribute attribute, double value);

/***************************************************************************//**
 * Return the name of a component of the field.
 *
 * @param field  The field whose component name is requested.
 * @param component_number  Component number from 1 to number of components.
 * @return  On success: allocated string containing field component name. Up to
 * caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_field_get_component_name(Cmiss_field_id field, int component_number);

/***************************************************************************//**
 * Get the coordinate system type to interpret field values in.
 *
 * @param field  The field to query.
 * @return  The type of coordinate system.
 */
ZINC_API enum Cmiss_field_coordinate_system_type Cmiss_field_get_coordinate_system_type(
	Cmiss_field_id field);

/***************************************************************************//**
 * Set the coordinate system type to interpret field values in.
 * Note PROLATE_SPHEROIDAL and OBLATE_SPHEROIDAL coordinate system types also
 * require the real CMISS_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS to be set to a
 * positive value.
 *
 * @param field  The field to modify.
 * @param coordinate_system_type  The type of coordinate system.
 * @return  Status CMISS_OK if successfully set, any other value if failed.
 */
ZINC_API int Cmiss_field_set_coordinate_system_type(Cmiss_field_id field,
	enum Cmiss_field_coordinate_system_type coordinate_system_type);

/***************************************************************************//**
 * Return the name of the field.
 *
 * @param field  The field whose name is requested.
 * @return  On success: allocated string containing field name. Up to caller to
 * free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_field_get_name(Cmiss_field_id field);

/***************************************************************************//**
 * Set the name of the field.
 * Fails if the new name is in use by any other field in the same field module.
 *
 * @param field  The field to be named.
 * @param name  The new name for the field.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_set_name(Cmiss_field_id field, const char *name);

/***************************************************************************//**
 * Return a source field of this field at a given index. Source fields are in
 * the order presented in the field constructor followed by any optional source
 * fields set by type-specific API.
 *
 * @param field  The field to query.
 * @param index  The index from 1 to number of source fields.
 * @return  Handle to the source field at the index, or NULL if none. Caller is
 * responsible for destroying the returned field handle.
 */
ZINC_API Cmiss_field_id Cmiss_field_get_source_field(Cmiss_field_id field, int index);

/***************************************************************************//**
 * Returns a reference to the field module which owns this field.
 *
 * @param field  The field to obtain field module for.
 * @return  Field module which this field belongs to.
 */
ZINC_API Cmiss_field_module_id Cmiss_field_get_field_module(Cmiss_field_id field);

/***************************************************************************//**
 * The types of values fields may produce.
 * @see Cmiss_field_get_value_type
 */
enum Cmiss_field_value_type
{
	CMISS_FIELD_VALUE_TYPE_INVALID = 0,
	CMISS_FIELD_VALUE_TYPE_REAL = 1,
	CMISS_FIELD_VALUE_TYPE_STRING = 2,
	CMISS_FIELD_VALUE_TYPE_MESH_LOCATION = 3
};

/***************************************************************************//**
 * Gets the type of values produced by the field.
 *
 * @param field  The field to query.
 * @return  Value type produced by field
 */
ZINC_API enum Cmiss_field_value_type Cmiss_field_get_value_type(Cmiss_field_id field);

/***************************************************************************//**
 * Determines if the field is defined at the location specified in the field
 * cache.
 *
 * @param field  The field to query.
 * @param cache  Store of location to check and intermediate field values.
 * @return  1 if defined, 0 if not or failed.
 */
ZINC_API int Cmiss_field_is_defined_at_location(Cmiss_field_id field,
	Cmiss_field_cache_id cache);

/***************************************************************************//**
 * Creates a field cache for storing a known location and field values and
 * derivatives at that location. Required to evaluate and assign field values.
 *
 * @param field_module  The field module to create a field cache for.
 * @return  New field cache, or NULL if failed.
 */
ZINC_API Cmiss_field_cache_id Cmiss_field_module_create_cache(
	Cmiss_field_module_id field_module);

/***************************************************************************//**
 * Returns a new reference to the field cache with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param cache  The field cache to obtain a new reference to.
 * @return  New field cache reference with incremented reference count.
 */
ZINC_API Cmiss_field_cache_id Cmiss_field_cache_access(Cmiss_field_cache_id cache);

/*******************************************************************************
 * Destroys this reference to the field cache, and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param cache_address  Address of handle to field cache to destroy.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_cache_destroy(Cmiss_field_cache_id *cache_address);

/***************************************************************************//**
 * Prescribes an element location without specifying chart coordinates. Suitable
 * only for evaluating fields that are constant across the element.
 * Note: replaces any other spatial location in cache (e.g. node.) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param element  The element to set. Must belong to same region as cache.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_cache_set_element(Cmiss_field_cache_id cache,
	Cmiss_element_id element);

/***************************************************************************//**
 * Prescribes a location in an element for field evaluation or assignment with
 * the cache.
 * Note: replaces any other spatial location in cache (e.g. node.) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param element  The element the location is in. Must belong to same region
 * as cache.
 * @param number_of_chart_coordinates  The size of the chart_coordinates array,
 * checked to be not less than the element dimension.
 * @param chart_coordinates  Location in element chart. Value is not checked;
 * caller is responsible for supplying locations within the bounds of the
 * element shape.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_cache_set_mesh_location(Cmiss_field_cache_id cache,
	Cmiss_element_id element, int number_of_chart_coordinates,
	const double *chart_coordinates);

/***************************************************************************//**
 * Prescribes a value of a field for subsequent evaluation and assignment with
 * the cache.
 * Note: currently treated as a spatial location, replacing any other spatial
 * location in cache (e.g. element, node) but time is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param reference_field  The field whose values are to be prescribed.
 * @param number_of_values  The size of the values array: number of field
 * components.
 * @param values  The field values to set.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_cache_set_field_real(Cmiss_field_cache_id cache,
	Cmiss_field_id reference_field, int number_of_values, const double *values);

/***************************************************************************//**
 * Prescribes a node location for field evaluation or assignment with the cache.
 * Note: replaces any other spatial location in cache (e.g. element) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param node  The node to set as spatial location. Must belong to same region
 * as cache.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_cache_set_node(Cmiss_field_cache_id cache, Cmiss_node_id node);

/***************************************************************************//**
 * Prescribes the time for field evaluation or assignment with the cache.
 *
 * @param cache  The field cache to set the location in.
 * @param time  The time value to be set.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_cache_set_time(Cmiss_field_cache_id cache, double time);

/***************************************************************************//**
 * Destroys this handle to the field_iterator and sets it to NULL.
 *
 * @param field_iterator_address  Address of handle to field_iterator to destroy.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_field_iterator_destroy(Cmiss_field_iterator_id *field_iterator_address);

/***************************************************************************//**
 * Returns a handle to the next field in the container being iterated over then
 * advances the iterator position. The caller is required to destroy the
 * returned field handle.
 *
 * @param field_iterator  Field iterator to query and advance.
 * @return  Handle to the next field, or NULL if none remaining.
 */
ZINC_API Cmiss_field_id Cmiss_field_iterator_next(Cmiss_field_iterator_id field_iterator);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_FIELD_H__ */
