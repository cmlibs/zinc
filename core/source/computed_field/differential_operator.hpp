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
#include "computed_field/field_derivative.hpp"

/** Currently limited to one or all terms of a FieldDerivative */
struct cmzn_differentialoperator
{
private:
	FieldDerivative *fieldDerivative;  // accessed
	int term; // starting at 0 or negative for all
	int access_count;

	cmzn_differentialoperator(FieldDerivative *fieldDerivativeIn, int termIn) :
		fieldDerivative(fieldDerivativeIn->access()),
		term(termIn),
		access_count(1)
	{
	}

	~cmzn_differentialoperator()
	{
		FieldDerivative::deaccess(this->fieldDerivative);
	}

public:

	/** @param term_in  Term from 0 to number-1, or negative for all terms */
	static cmzn_differentialoperator* create(FieldDerivative *fieldDerivativeIn, int termIn);

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

	int getElementDimension() const
	{
		if (this->fieldDerivative->getType() == FieldDerivative::TYPE_ELEMENT_XI)
			return static_cast<FieldDerivativeMesh*>(this->fieldDerivative)->getElementDimension();
		return 0;
	}

	FieldDerivative& getFieldDerivative() const
	{
		return *this->fieldDerivative;
	}

	int getTerm() const
	{
		return this->term;
	}

};

#endif /* !defined (DIFFERENTIAL_OPERATOR_HPP) */
