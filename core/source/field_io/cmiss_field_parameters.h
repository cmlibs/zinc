/***************************************************************************//**
 * FILE : cmiss_field_parameters.h
 *
 * Implements a field storing parameters as a semi-dense array indexed by
 * N ensembles.
 * Warning: prototype!
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_FIELD_PARAMETERS_H)
#define CMZN_FIELD_PARAMETERS_H

#include "zinc/types/fieldid.h"
#include "zinc/types/fieldmoduleid.h"
#include "field_io/cmiss_field_ensemble_id.h"
#include "field_io/cmiss_field_parameters_id.h"

/***************************************************************************//**
 * Create a double-precision real-valued parameter set indexed by N ensembles.
 * Parameters effectively cycle through first ensemble slowest to last ensemble
 * index fastest. If one ensemble is expected to grow during processing, best
 * performance is achieved by making it the first ensemble.
 * Note: Currently defaults to 1 component; Eventually need to specify component
 * ensemble, e.g. via FieldML valueDomain.
 */
cmzn_field *cmzn_fieldmodule_create_field_real_parameters(cmzn_fieldmodule_id field_module,
	int number_of_index_ensembles, cmzn_field_ensemble_id *index_ensemble_fields);

cmzn_field_real_parameters_id cmzn_field_cast_real_parameters(cmzn_field_id field);

ZINC_C_INLINE cmzn_field_id cmzn_field_real_parameters_base_cast(
	cmzn_field_real_parameters_id real_parameters_field)
{
	return (cmzn_field_id)(real_parameters_field);
}

int cmzn_field_real_parameters_destroy(
	cmzn_field_real_parameters_id *real_parameters_field_address);

cmzn_ensemble_index_id cmzn_field_real_parameters_create_index(
	cmzn_field_real_parameters_id real_parameters_field);

int cmzn_field_real_parameters_get_values(
	cmzn_field_real_parameters_id real_parameters_field,
	cmzn_ensemble_index_id index, unsigned int number_of_values, double *values);

/***************************************************************************//**
 * @param number_of_values  The size of the values and value_exists arrays. This
 * must match the number identified by the index.
 * @param values  Array to receive parameter values.
 * @param value_exists  Array to receive flag indicating if each value exists; 1
 * if it exists, 0 if not.
 * @param number_of_values_read  The number of values read.
 * @return  1 on success, 0 on error. Success includes when the index and
 * number of values expected are matching, but no values are stored.
 */
int cmzn_field_real_parameters_get_values_sparse(
	cmzn_field_real_parameters_id real_parameters_field,
	cmzn_ensemble_index_id index, unsigned int number_of_values, double *values,
	int *value_exists, int *number_of_values_read);

int cmzn_field_real_parameters_set_values(
	cmzn_field_real_parameters_id real_parameters_field,
	cmzn_ensemble_index_id index, unsigned int number_of_values, double *values);


/***************************************************************************//**
 * The ensemble type specific handle to an integer parameters cmzn_field.
 */
struct cmzn_field_integer_parameters;
typedef struct cmzn_field_integer_parameters *cmzn_field_integer_parameters_id;

/***************************************************************************//**
 * Create an integer-valued parameter set indexed by N ensembles.
 * Parameters effectively cycle through first ensemble slowest to last ensemble
 * index fastest. If one ensemble is expected to grow during processing, best
 * performance is achieved by making it the first ensemble.
 * Note: Currently defaults to 1 component; Eventually need to specify component
 * ensemble, e.g. via FieldML valueDomain.
 */
cmzn_field *cmzn_fieldmodule_create_field_integer_parameters(cmzn_fieldmodule_id field_module,
	int number_of_index_ensembles, cmzn_field_ensemble_id *index_ensemble_fields);

cmzn_field_integer_parameters_id cmzn_field_cast_integer_parameters(cmzn_field_id field);

ZINC_C_INLINE cmzn_field_id cmzn_field_integer_parameters_base_cast(
	cmzn_field_integer_parameters_id integer_parameters_field)
{
	return (cmzn_field_id)(integer_parameters_field);
}

int cmzn_field_integer_parameters_destroy(
	cmzn_field_integer_parameters_id *integer_parameters_field_address);

cmzn_ensemble_index_id cmzn_field_integer_parameters_create_index(
	cmzn_field_integer_parameters_id integer_parameters_field);

int cmzn_field_integer_parameters_get_values(
	cmzn_field_integer_parameters_id integer_parameters_field,
	cmzn_ensemble_index_id index, unsigned int number_of_values, int *values);

int cmzn_field_integer_parameters_set_values(
	cmzn_field_integer_parameters_id integer_parameters_field,
	cmzn_ensemble_index_id index, unsigned int number_of_values, int *values);

#endif /* !defined (CMZN_FIELD_PARAMETERS_H) */
