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
#include "zinc/status.h"

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
