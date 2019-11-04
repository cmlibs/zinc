/***************************************************************************//**
 * FILE : differential_operator.cpp
 *
 * Internal implementation for class representing a differential operator that
 * can be applied to a field to obtain a derivative.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "computed_field/differential_operator.hpp"
#include "opencmiss/zinc/status.h"

cmzn_differentialoperator* cmzn_differentialoperator::create(Field_derivative *field_derivative_in, int term_in)
{
	if ((!field_derivative_in) || (term_in >= field_derivative_in->get_term_count()))
	{
		display_message(ERROR_MESSAGE, "cmzn_differentialoperator::create.  Invalid arguments");
		return nullptr;
	}
	return new cmzn_differentialoperator(field_derivative_in, term_in);
}

cmzn_differentialoperator_id cmzn_differentialoperator_access(
	cmzn_differentialoperator_id differential_operator)
{
	if (differential_operator)
		return differential_operator->access();
	return 0;
}

int cmzn_differentialoperator_destroy(
	cmzn_differentialoperator_id *differential_operator_address)
{
	if (differential_operator_address)
		return cmzn_differentialoperator::deaccess(*differential_operator_address);
	return CMZN_ERROR_ARGUMENT;
}
