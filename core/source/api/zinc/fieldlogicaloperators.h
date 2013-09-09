/*******************************************************************************
FILE : fieldlogicaloperators.h

LAST MODIFIED : 16 May 2008

DESCRIPTION :
The public interface to the cmzn_fields that perform logical operations.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDLOGICALOPERATORS_H__
#define CMZN_FIELDLOGICALOPERATORS_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Creates a field whose component values are 1 if that component of
 * source_field_one AND source_field_two is non-zero, 0 otherwise.
 * Automatic scalar broadcast will apply, see field.h.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  First input field
 * @param source_field_two  Second input field
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_and(cmzn_field_module_id field_module,
	cmzn_field_id source_field_one, cmzn_field_id source_field_two);

/***************************************************************************//**
 * Creates a field whose component values are 1 if that component of
 * source_field_one EQUALS that component of source_field_two, 0 otherwise.
 * Automatic scalar broadcast will apply, see field.h.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  First input field
 * @param source_field_two  Second input field
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_equal_to(
	cmzn_field_module_id field_module,
	cmzn_field_id source_field_one, cmzn_field_id source_field_two);

/*****************************************************************************//**
 * Creates a field whose component values are 1 if that component of
 * source_field_one is greater than the component value in source_field_two.
 * Automatic scalar broadcast will apply, see field.h.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one First input field
 * @param source_field_two Second input field
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_greater_than(
	cmzn_field_module_id field_module,
	cmzn_field_id source_field_one, cmzn_field_id source_field_two);

/*****************************************************************************//**
 * Creates a field whose component values are 1 if that component of
 * source_field_one is less than the component value in source_field_two.
 * Automatic scalar broadcast will apply, see field.h.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one First input field
 * @param source_field_two Second input field
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_less_than(
	cmzn_field_module_id field_module,
	cmzn_field_id source_field_one, cmzn_field_id source_field_two);

/***************************************************************************//**
 * Creates a field whose component values are 1 if that component of
 * source_field_one OR source_field_two is non-zero, 0 otherwise.
 * Automatic scalar broadcast will apply, see field.h.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  First input field
 * @param source_field_two  Second input field
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_or(cmzn_field_module_id field_module,
	cmzn_field_id source_field_one, cmzn_field_id source_field_two);

/***************************************************************************//**
 * Creates a field whose component values are 1 if that component of the
 * source_field is zero, 0 otherwise; effectively a component-wise logical not
 * operator. Returned field has same number of components as source field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  The source field.
 * @return  Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_not(cmzn_field_module_id field_module,
	cmzn_field_id source_field);

/***************************************************************************//**
 * Creates a field whose component values are 1 if that component of
 * source_field_one OR source_field_two is non-zero (but not both), 0 otherwise.
 * Automatic scalar broadcast will apply, see field.h.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  First input field
 * @param source_field_two  Second input field
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_xor(cmzn_field_module_id field_module,
	cmzn_field_id source_field_one, cmzn_field_id source_field_two);

#ifdef __cplusplus
}
#endif

#endif
