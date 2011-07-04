/***************************************************************************//**
 * FILE : cmiss_field_parameters.h
 * 
 * Implements a field storing parameters as a semi-dense array indexed by
 * N ensembles.
 * Warning: prototype!
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
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
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

#if !defined (CMISS_FIELD_PARAMETERS_H)
#define CMISS_FIELD_PARAMETERS_H

#include "api/types/cmiss_c_inline.h"
#include "api/types/cmiss_field_id.h"
#include "api/types/cmiss_field_ensemble_id.h"
#include "api/types/cmiss_field_module_id.h"
#include "api/types/cmiss_field_parameters_id.h"

/***************************************************************************//**
 * Create a double-precision real-valued parameter set indexed by N ensembles.
 * Parameters effectively cycle through first ensemble fast to last ensemble
 * index slowest. If one ensemble is expected to grow during processing, best
 * performance is achieved by making it the last ensemble.
 * Note: Currently defaults to 1 component; Eventually need to specify component
 * ensemble, e.g. via FieldML valueDomain.
 */
Cmiss_field *Cmiss_field_module_create_real_parameters(Cmiss_field_module_id field_module, 
	int number_of_index_ensembles, Cmiss_field_ensemble_id *index_ensemble_fields);

Cmiss_field_real_parameters_id Cmiss_field_cast_real_parameters(Cmiss_field_id field);

CMISS_C_INLINE Cmiss_field_id Cmiss_field_real_parameters_base_cast(
	Cmiss_field_real_parameters_id real_parameters_field)
{
	return (Cmiss_field_id)(real_parameters_field);
}

int Cmiss_field_real_parameters_destroy(
	Cmiss_field_real_parameters_id *real_parameters_field_address);

Cmiss_ensemble_index_id Cmiss_field_real_parameters_create_index(
	Cmiss_field_real_parameters_id real_parameters_field);

int Cmiss_field_real_parameters_get_values(
	Cmiss_field_real_parameters_id real_parameters_field,
	Cmiss_ensemble_index_id index, unsigned int number_of_values, double *values);

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
int Cmiss_field_real_parameters_get_values_sparse(
	Cmiss_field_real_parameters_id real_parameters_field,
	Cmiss_ensemble_index_id index, unsigned int number_of_values, double *values,
	int *value_exists, int *number_of_values_read);

int Cmiss_field_real_parameters_set_values(
	Cmiss_field_real_parameters_id real_parameters_field,
	Cmiss_ensemble_index_id index, unsigned int number_of_values, double *values);


/***************************************************************************//**
 * The ensemble type specific handle to an integer parameters Cmiss_field.
 */
struct Cmiss_field_integer_parameters;
typedef struct Cmiss_field_integer_parameters *Cmiss_field_integer_parameters_id;

/***************************************************************************//**
 * Create an integer-valued parameter set indexed by N ensembles.
 * Parameters effectively cycle through first ensemble fast to last ensemble
 * index slowest. If one ensemble is expected to grow during processing, best
 * performance is achieved by making it the last ensemble.
 * Note: Currently defaults to 1 component; Eventually need to specify component
 * ensemble, e.g. via FieldML valueDomain.
 */
Cmiss_field *Cmiss_field_module_create_integer_parameters(Cmiss_field_module_id field_module,
	int number_of_index_ensembles, Cmiss_field_ensemble_id *index_ensemble_fields);

Cmiss_field_integer_parameters_id Cmiss_field_cast_integer_parameters(Cmiss_field_id field);

CMISS_C_INLINE Cmiss_field_id Cmiss_field_integer_parameters_base_cast(
	Cmiss_field_integer_parameters_id integer_parameters_field)
{
	return (Cmiss_field_id)(integer_parameters_field);
}

int Cmiss_field_integer_parameters_destroy(
	Cmiss_field_integer_parameters_id *integer_parameters_field_address);

Cmiss_ensemble_index_id Cmiss_field_integer_parameters_create_index(
	Cmiss_field_integer_parameters_id integer_parameters_field);

int Cmiss_field_integer_parameters_get_values(
	Cmiss_field_integer_parameters_id integer_parameters_field,
	Cmiss_ensemble_index_id index, unsigned int number_of_values, int *values);

int Cmiss_field_integer_parameters_set_values(
	Cmiss_field_integer_parameters_id integer_parameters_field,
	Cmiss_ensemble_index_id index, unsigned int number_of_values, int *values);

#endif /* !defined (CMISS_FIELD_PARAMETERS_H) */
