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

#include "api/types/cmiss_differential_operator_id.h"
#include "api/types/cmiss_element_id.h"
#include "api/types/cmiss_field_id.h"
#include "api/types/cmiss_field_module_id.h"
#include "api/types/cmiss_node_id.h"

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
	CMISS_FIELD_ATTRIBUTE_NUMBER_OF_SOURCE_FIELDS = 4
	/*!< Integer number of source fields the field is a function of.
	 */
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
enum Cmiss_field_attribute Cmiss_field_attribute_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_field_attribute_enum_to_string(enum Cmiss_field_attribute attribute);

/***************************************************************************//**
 * Get the number of components of the field.
 *
 * @param field  The field to query.
 * @return  The number of components of the field.
 */
int Cmiss_field_get_number_of_components(Cmiss_field_id field);

/***************************************************************************//**
 * Returns a new reference to the field with reference count incremented.
 * Caller is responsible for destroying the new reference.
 * 
 * @param field  The field to obtain a new reference to.
 * @return  New field reference with incremented reference count.
 */
Cmiss_field_id Cmiss_field_access(Cmiss_field_id field);

/***************************************************************************//**
 * Destroys this reference to the field (and sets it to NULL).
 * Internally this just decrements the reference count.
 */
int Cmiss_field_destroy(Cmiss_field_id *field_address);

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
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_assign_mesh_location(Cmiss_field_id field,
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
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_assign_real(Cmiss_field_id field, Cmiss_field_cache_id cache,
	int number_of_values, const double *values);

/***************************************************************************//**
 * Assign a string value to a field at location specified in cache.
 * Only supported for legacy stored 'finite element' string at node locations,
 * and string_constant at any cache location.
 *
 * @param field  The field to assign a string value to.
 * @param cache  Store of location to assign at and intermediate field values.
 * @param string_value  The string value to assign to field.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_assign_string(Cmiss_field_id field, Cmiss_field_cache_id cache,
	const char *string_value);

/***************************************************************************//**
 * Evaluate mesh_location field values at location specified in cache.
 *
 * @param field  The field to evaluate.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * @param number_of_chart_coordinates  Size of chart_coordinates array. Checked
 * that it equals or exceeds the dimension of the returned element.
 * @param chart_coordinates  Array to evaluate chart coordinate location into.
 * @return  Handle to element on success, NULL on failure. Caller is responsible
 * for destroying handle.
 */
Cmiss_element_id Cmiss_field_evaluate_mesh_location(Cmiss_field_id field,
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
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_evaluate_real(Cmiss_field_id field, Cmiss_field_cache_id cache,
	int number_of_values, double *values);

/***************************************************************************//**
 * Evaluate field as string at location specified in cache. Numerical valued
 * fields are written to a string with comma separated components.
 * Caller must free returned string with Cmiss_deallocate().
 *
 * @param field  The field to evaluate.
 * @param cache  Store of location to evaluate at and intermediate field values.
 * @return  Allocated string value, or NULL if failed.
 */
char *Cmiss_field_evaluate_string(Cmiss_field_id field,
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
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_evaluate_derivative(Cmiss_field_id field,
	Cmiss_differential_operator_id differential_operator,
	Cmiss_field_cache_id cache, int number_of_values, double *values);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the field.
 *
 * @param field  The field to query.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
int Cmiss_field_get_attribute_integer(Cmiss_field_id field,
	enum Cmiss_field_attribute attribute);

/***************************************************************************//**
 * Set an integer or Boolean attribute of the field.
 *
 * @param field  The field to set the attribute for.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * for this field.
 */
int Cmiss_field_set_attribute_integer(Cmiss_field_id field,
	enum Cmiss_field_attribute attribute, int value);

/***************************************************************************//**
 * Return the name of the field.
 *
 * @param field  The field whose name is requested.
 * @return  On success: allocated string containing field name. Up to caller to
 * free using Cmiss_deallocate().
 */
char *Cmiss_field_get_name(Cmiss_field_id field);

/***************************************************************************//**
 * Set the name of the field.
 * Fails if the new name is in use by any other field in the same field module.
 *
 * @param field  The field to be named.
 * @param name  The new name for the field.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_set_name(Cmiss_field_id field, const char *name);

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
Cmiss_field_id Cmiss_field_get_source_field(Cmiss_field_id field, int index);

/***************************************************************************//**
 * Returns a reference to the field module which owns this field.
 * 
 * @param field  The field to obtain field module for.
 * @return  Field module which this field belongs to.
 */
Cmiss_field_module_id Cmiss_field_get_field_module(Cmiss_field_id field);

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
enum Cmiss_field_value_type Cmiss_field_get_value_type(Cmiss_field_id field);

/***************************************************************************//**
 * Determines if the field is defined at the location specified in the field
 * cache.
 *
 * @param field  The field to query.
 * @param cache  Store of location to check and intermediate field values.
 * @return  1 if defined, 0 if not or failed.
 */
int Cmiss_field_is_defined_at_location(Cmiss_field_id field,
	Cmiss_field_cache_id cache);

/***************************************************************************//**
 * Creates a field cache for storing a known location and field values and
 * derivatives at that location. Required to evaluate and assign field values.
 *
 * @param field_module  The field module to create a field cache for.
 * @return  New field cache, or NULL if failed.
 */
Cmiss_field_cache_id Cmiss_field_module_create_cache(
	Cmiss_field_module_id field_module);

/***************************************************************************//**
 * Returns a new reference to the field cache with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param cache  The field cache to obtain a new reference to.
 * @return  New field cache reference with incremented reference count.
 */
Cmiss_field_cache_id Cmiss_field_cache_access(Cmiss_field_cache_id cache);

/*******************************************************************************
 * Destroys this reference to the field cache, and sets it to NULL.
 * Internally this just decrements the reference count.
 */
int Cmiss_field_cache_destroy(Cmiss_field_cache_id *cache_address);

/***************************************************************************//**
 * Prescribes an element location without specifying chart coordinates. Suitable
 * only for evaluating fields that are constant across the element.
 * Note: replaces any other spatial location in cache (e.g. node.) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param element  The element to set. Must belong to same region as cache.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_set_element(Cmiss_field_cache_id cache,
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
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_set_mesh_location(Cmiss_field_cache_id cache,
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
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_set_field_real(Cmiss_field_cache_id cache,
	Cmiss_field_id reference_field, int number_of_values, const double *values);

/***************************************************************************//**
 * Prescribes a node location for field evaluation or assignment with the cache.
 * Note: replaces any other spatial location in cache (e.g. element) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param node  The node to set as spatial location. Must belong to same region
 * as cache.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_set_node(Cmiss_field_cache_id cache, Cmiss_node_id node);

/***************************************************************************//**
 * Prescribes the time for field evaluation or assignment with the cache.
 *
 * @param cache  The field cache to set the location in.
 * @param time  The time value to be set.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_set_time(Cmiss_field_cache_id cache, double time);

#endif /* __CMISS_FIELD_H__ */
