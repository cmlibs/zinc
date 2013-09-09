/***************************************************************************//**
 * FILE : differentialoperator.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_DIFFFERENTIALOPERATOR_HPP__
#define CMZN_DIFFFERENTIALOPERATOR_HPP__

#include "zinc/differentialoperator.h"

namespace OpenCMISS
{
namespace Zinc
{

class DifferentialOperator
{
private:
	cmzn_differential_operator_id id;

public:

	DifferentialOperator() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit DifferentialOperator(cmzn_differential_operator_id in_differential_operator_id) :
		id(in_differential_operator_id)
	{  }

	DifferentialOperator(const DifferentialOperator& differentialOperator) :
		id(cmzn_differential_operator_access(differentialOperator.id))
	{	}

	DifferentialOperator& operator=(const DifferentialOperator& differentialOperator)
	{
		cmzn_differential_operator_id temp_id = cmzn_differential_operator_access(differentialOperator.id);
		if (0 != id)
		{
			cmzn_differential_operator_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~DifferentialOperator()
	{
		if (0 != id)
		{
			cmzn_differential_operator_destroy(&id);
		}
	}

	cmzn_differential_operator_id getId()
	{
		return id;
	}
};

}  // namespace Zinc
}

#endif /* CMZN_DIFFFERENTIAL_OPERATOR_HPP__ */
