/***************************************************************************//**
 * FILE : differential_operator.hpp
 *
 * Internal header for class representing a differential differential_operator
 * that can be applied to a field to obtain a derivative.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (DIFFERENTIAL_OPERATOR_HPP)
#define DIFFERENTIAL_OPERATOR_HPP

#include "opencmiss/zinc/differentialoperator.h"
#include "opencmiss/zinc/status.h"
#include "finite_element/finite_element_region.h"

/**
 * For now can only represent a differential differential_operator give first derivatives
 * with respect to differential_operator elements of given dimension from fe_region.
 */
struct cmzn_differentialoperator
{
private:
	FE_region *fe_region;
	int dimension;
	int term; // which derivative for multiple dimensions, 1 = d/dx1
	int access_count;

public:
	cmzn_differentialoperator(FE_region *fe_region, int dimension, int term) :
		fe_region(ACCESS(FE_region)(fe_region)),
		dimension(dimension),
		term(term),
		access_count(1)
	{
	}

	cmzn_differentialoperator_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_differentialoperator_id &differential_operator)
	{
		if (!differential_operator)
			return CMZN_ERROR_ARGUMENT;
		--(differential_operator->access_count);
		if (differential_operator->access_count <= 0)
			delete differential_operator;
		differential_operator = 0;
		return CMZN_OK;
	}

	int getDimension() const { return dimension; }
	FE_region *getFeRegion() const { return fe_region; }
	int getTerm() const { return term; }

private:

	~cmzn_differentialoperator()
	{
		DEACCESS(FE_region)(&fe_region);
	}

};

#endif /* !defined (DIFFERENTIAL_OPERATOR_HPP) */
